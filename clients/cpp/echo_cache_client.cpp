#include "echo_cache_client.h"

#include <string>
#include <iostream>
#include <exception>

#include <parser.h>
#include <socket_client.h>

RemoteCache::RemoteCache(const std::string& ip, int port) {
    this->ip = ip;
    this->port = port;
    this->establishConnection();
    this->maxRetries = 3;
}

RemoteCache::~RemoteCache() {
    if (this->client != nullptr) {
        this->closeConnection();
    }
}

HandlerResponse RemoteCache::get(const std::string& key) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("get||" + key);
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            HandlerResponse parsed = parseResponseString(response);
            return parsed;
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

bool RemoteCache::set(const std::string& key, const std::string& value) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("set||" + key + "||" + value);
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return true;
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return false;
}

bool RemoteCache::del(const std::string& key) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("del||" + key);
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return true;
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return false;
}

HandlerResponse RemoteCache::incr(const std::string& key, int delta) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("incr||" + key + "||" + std::to_string(delta));
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return parseResponseString(response);
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

HandlerResponse RemoteCache::decr(const std::string& key, int delta) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("decr||" + key + "||" + std::to_string(delta));
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return parseResponseString(response);
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

HandlerResponse RemoteCache::append(const std::string& key, const std::string& value) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("append||" + key + "||" + value);
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return parseResponseString(response);
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

HandlerResponse RemoteCache::exists(const std::string& key) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("exists||" + key);
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return parseResponseString(response);
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

HandlerResponse RemoteCache::scan(const std::string& prefix, int limit) {
    int retriesLeft = this->maxRetries;
    while (retriesLeft >= 1) {
        try {
            this->client->sendMessage("scan||" + prefix + "||" + std::to_string(limit));
            std::string response = this->client->receiveResponse();
            if (response.empty()) {
                throw std::runtime_error("bad connection");
            }
            return parseResponseString(response);
        } catch (...) {
            this->reestablishConnection();
        }
        --retriesLeft;
    }
    return {StatusCode::unexpectedError, ""};
}

void RemoteCache::reestablishConnection() {
    this->closeConnection();
    this->establishConnection();
}

void RemoteCache::establishConnection() {
    SocketClient* client = new SocketClient{this->ip, this->port};
    this->client = client;
}

void RemoteCache::closeConnection() {
    delete this->client;
    this->client = nullptr;
}

void RemoteCache::initiateAndCloseConnection() {
    try {
        this->client->sendMessage("end");
    } catch (...) {}
    this->closeConnection();
}