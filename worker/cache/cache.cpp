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