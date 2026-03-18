#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "helpers.h"
#include <parser.h>
#include <socket_client.h>
#include <socket_server.h>

struct ConnectionUsage {
    int connection;
    int lastUsed;
};

std::mutex clientConnectionsMutex;
std::map<int, ConnectionUsage> clientConnections;

std::string handleRequest(char *buff, const std::string &worker_ip, int worker_port) {
    std::string command(buff);

    SocketClient client{worker_ip, worker_port};
    if (!client.connectionSucceeded) {
        std::cerr << "Failed to connect to worker at " << worker_ip << ":" << worker_port << std::endl;
        return "";
    }
    client.sendMessage(command);
    std::string response = client.receiveResponse();
    return response;
}

void handleClient(CommandLineArguments commandLineArguments, int connection,
    const std::string &printPrefix, int id) {
    std::cout << printPrefix << "new client" << std::endl;
    char recv_buf[65536];

    while (true) {
        memset(recv_buf, '\0', sizeof(recv_buf));
        ssize_t bytes_received = recv(connection, recv_buf, sizeof(recv_buf), 0);

        if (bytes_received == -1) {
            perror("recv");
            break;
        }
        if (bytes_received == 0) {
            std::cout << printPrefix << "connection closed by client" << std::endl;
            close(connection);
            std::lock_guard<std::mutex> lock(clientConnectionsMutex);
            clientConnections.erase(id);
            break;
        }
        std::cout << printPrefix << "got " << recv_buf << std::endl;

        std::string command = recv_buf;
        if (command == "end") {
            std::cout << printPrefix << "client requests connection close" << std::endl;
            close(connection);
            std::lock_guard<std::mutex> lock(clientConnectionsMutex);
            clientConnections.erase(id);
            break;
        }

        ParsedKey parsedKey = parseKey(command);
        CommandType commandType = getCommandType(command);
        if (!parsedKey.success || commandType == CommandType::other) {
            break;
        }

        // Update lastUsed time
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        clientConnections[id].lastUsed = getCurrentTime();

        // Hash the key to pick 2 workers (not used for scan - it broadcasts to all)
        int workerIndex1 = hashKey(parsedKey.key, commandLineArguments.numWorkers);
        int workerIndex2 = (workerIndex1 + 1) % commandLineArguments.numWorkers;
        IpAndPort workerIpAndPort1 = commandLineArguments.workers[workerIndex1];
        IpAndPort workerIpAndPort2 = commandLineArguments.workers[workerIndex2];

        if (commandType == CommandType::scan) {
            // Broadcast to all workers and merge results
            ParsedScanRequest scanReq = parseScan(command);
            if (!scanReq.success) break;
            std::set<std::string> allKeys;
            for (int i = 0; i < commandLineArguments.numWorkers; i++) {
                std::string response = handleRequest(
                    recv_buf, commandLineArguments.workers[i].ip,
                    commandLineArguments.workers[i].port);
                if (!response.empty()) {
                    HandlerResponse parsed = parseResponseString(response);
                    if (parsed.statusCode == StatusCode::success && !parsed.result.empty()) {
                        std::string s = parsed.result;
                        size_t pos = 0;
                        while (pos < s.size()) {
                            size_t next = s.find("||", pos);
                            if (next == std::string::npos) {
                                allKeys.insert(s.substr(pos));
                                break;
                            }
                            allKeys.insert(s.substr(pos, next - pos));
                            pos = next + 2;
                        }
                    }
                }
            }
            // Build merged result, limit to scanReq.limit
            std::string merged;
            int count = 0;
            for (const auto& k : allKeys) {
                if (count >= scanReq.limit) break;
                if (count > 0) merged += "||";
                merged += k;
                count++;
            }
            std::string responseStr = formatResponseString({StatusCode::success, merged});
            send(connection, responseStr.c_str(), responseStr.size(), 0);
        } else if (commandType == CommandType::set) {
            // Write to BOTH workers (replication)
            std::string response1 =
            handleRequest(recv_buf, workerIpAndPort1.ip, workerIpAndPort1.port);
            std::string response2 =
            handleRequest(recv_buf, workerIpAndPort2.ip, workerIpAndPort2.port);
            int responseCode = send(connection, response1.c_str(), response1.size(), 0);
            if (responseCode == -1) {
            perror("send");
            }
        } else if (commandType == CommandType::get) {
            // Try worker 1 first
            std::string response1 =
            handleRequest(recv_buf, workerIpAndPort1.ip, workerIpAndPort1.port);
            if (!response1.empty()) {
                int responseCode =
                    send(connection, response1.c_str(), response1.size(), 0);
                if (responseCode == -1) {
                    perror("send");
                }
            } else {
                // Fallback to worker 2
                std::string response2 =
                    handleRequest(recv_buf, workerIpAndPort2.ip, workerIpAndPort2.port);
                int responseCode =
                    send(connection, response2.c_str(), response2.size(), 0);
                if (responseCode == -1) {
                    perror("send");
                }
            }
        } else if (commandType == CommandType::del) {
            // Delete from BOTH workers
            std::string response1 =
            handleRequest(recv_buf, workerIpAndPort1.ip, workerIpAndPort1.port);
            std::string response2 =
            handleRequest(recv_buf, workerIpAndPort2.ip, workerIpAndPort2.port);
            int responseCode =
            send(connection, response1.c_str(), response1.size(), 0);
            if (responseCode == -1) {
                perror("send");
            }
        } else if (commandType == CommandType::incr ||
                   commandType == CommandType::decr ||
                   commandType == CommandType::append) {
            // Write to BOTH workers
            std::string response1 =
                handleRequest(recv_buf, workerIpAndPort1.ip, workerIpAndPort1.port);
            std::string response2 =
                handleRequest(recv_buf, workerIpAndPort2.ip, workerIpAndPort2.port);
            int responseCode = send(connection, response1.c_str(), response1.size(), 0);
            if (responseCode == -1) {
                perror("send");
            }
        } else if (commandType == CommandType::exists) {
            // Try worker 1 first, fallback to worker 2
            std::string response1 =
                handleRequest(recv_buf, workerIpAndPort1.ip, workerIpAndPort1.port);
            if (!response1.empty()) {
                send(connection, response1.c_str(), response1.size(), 0);
            } else {
                std::string response2 =
                    handleRequest(recv_buf, workerIpAndPort2.ip, workerIpAndPort2.port);
                send(connection, response2.c_str(), response2.size(), 0);
            }
        }
    }
    std::cout << printPrefix << "end connection" << std::endl;
}

void cleanUpThreads() {
    while (true) {
        {
            clientConnectionsMutex.lock();
            for (auto it = clientConnections.cbegin();
                 it != clientConnections.cend();) {
                int lastUsed = it->second.lastUsed;
                int currentTime = getCurrentTime();
                if (currentTime - lastUsed > 50) {
                    std::cout << "Kicking off a thread" << std::endl;
                    int connection = it->second.connection;
                    close(connection);
                    it = clientConnections.erase(it);
                } else {
                    ++it;
                }
            }
            clientConnectionsMutex.unlock();
            std::cout << "Number of active threads: " << clientConnections.size()
                      << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main(int argc, char *argv[]) {
    CommandLineArguments commandLineArguments =
        parseCommandLineArguments(argc, argv);
    if (!commandLineArguments.success) {
        std::cerr << "Usage: " << argv[0]
                  << " <orchestrator port> <worker ip 1> <worker port 1> ... "
                     "<worker ip n> <worker port n>"
                  << std::endl;
        return 1;
    }

    int server_sockfd = buildSocketServer(commandLineArguments.port);
    std::cout << "Listening on port: " << commandLineArguments.port << std::endl;

    std::vector<std::thread> clientHandlerThreads;
    std::thread custodianThread = std::thread(cleanUpThreads);

    while (true) {
        sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        int conn = accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
        if (conn < 0) {
            perror("Orchestrator error connect");
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = (int)ntohs(client_addr.sin_port);
        std::string printPrefix =
            client_ip + ":" + std::to_string(client_port) + " - ";

        int threadId = getNextId();
        clientHandlerThreads.push_back(std::thread(
            handleClient, commandLineArguments, conn, printPrefix, threadId));

        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        clientConnections[threadId] = {conn, getCurrentTime()};
    }

    close(server_sockfd);
    return 0;
}