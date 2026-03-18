#ifndef CPP_CLIENT_H
#define CPP_CLIENT_H

#include <string>
#include <parser.h>

class SocketClient;

class RemoteCache {
  public:
    RemoteCache(const std::string& ip, int port);
    ~RemoteCache();

    HandlerResponse get(const std::string& key);
    bool set(const std::string& key, const std::string& value);
    bool del(const std::string& key);
    HandlerResponse incr(const std::string& key, int delta = 1);
    HandlerResponse decr(const std::string& key, int delta = 1);
    HandlerResponse append(const std::string& key, const std::string& value);
    HandlerResponse exists(const std::string& key);
    HandlerResponse scan(const std::string& prefix, int limit = 100);
    void initiateAndCloseConnection();

  private:
    void establishConnection();
    void reestablishConnection();
    void closeConnection();

    SocketClient* client;
    std::string ip;
    int port;
    int maxRetries;
};

#endif