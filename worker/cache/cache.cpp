#include "cache.h"
#include <iostream>
#include <string>

Cache::Cache(int maxSize) : table(new std::map<std::string, std::string>()) {}

Cache::~Cache() {delete table;}

HandlerResponse Cache::get(std::string key) {
    if (table->find(key) == table->end()) {
        return {StatusCode::keyNotFound, ""};
    }
    return {StatusCode::success, table->at(key)};
}

HandlerResponse Cache::set(std::string key, std::string val) {
    table->insert({key, val});
    return {StatusCode::success, "Value set successfully"};
}

HandlerResponse Cache::del(std::string key) {
    if (table->find(key) == table->end()) {
        return {StatusCode::keyNotFound, ""};
    }
    table->erase(key);
    return {StatusCode::success, "Value deleted successfully"};
}

HandlerResponse Cache::scan(std::string prefix, int limit) {
    std::string result;
    int count = 0;
    auto it = table->lower_bound(prefix);
    while (it != table->end() && count < limit) {
        if (it->first.compare(0, prefix.size(), prefix) != 0) {
            break;
        }
        if (count > 0) result += "||";
        result += it->first;
        ++count;
        ++it;
    }
    return {StatusCode::success, result};
}

HandlerResponse Cache::incr(std::string key, int delta) {
    int current = 0;
    if (table->find(key) != table->end()) {
        try {
            current = std::stoi(table->at(key));
        } catch (...) {
            return {StatusCode::notInteger, "Value is not an integer"};
        }
    }
    int newVal = current + delta;
    table->operator[](key) = std::to_string(newVal);
    return {StatusCode::success, std::to_string(newVal)};
}

HandlerResponse Cache::append(std::string key, std::string value) {
    std::string current = "";
    if (table->find(key) != table->end()) {
        current = table->at(key);
    }
    std::string newVal = current + value;
    table->operator[](key) = newVal;
    return {StatusCode::success, std::to_string(newVal.size())};
}

HandlerResponse Cache::exists(std::string key) {
    bool found = table->find(key) != table->end();
    return {StatusCode::success, found ? "1" : "0"};
}