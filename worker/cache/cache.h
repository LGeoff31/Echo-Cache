#ifndef CACHE_H
#define CACHE_H

#include <parser.h>
#include <string>
#include <map>

class Cache {
public:
    Cache(int maxSize);
    ~Cache();

    HandlerResponse get(std::string key);
    HandlerResponse set(std::string key, std::string val);
    HandlerResponse del(std::string key);

private:
    std::map<std::string, std::string>* table;
};

#endif