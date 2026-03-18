#ifndef PARSER_H
#define PARSER_H

#include <string>

struct ParsedSetRequest {
    bool success;
    std::string key;
    std::string val;
};

struct ParsedGetRequest {
    bool success;
    std::string key;
};

struct ParsedDelRequest {
    bool success;
    std::string key;
};

struct ParsedScanRequest {
    bool success;
    std::string prefix;
    int limit;
};

struct ParsedIncrRequest {
    bool success;
    std::string key;
    int delta;
};

struct ParsedAppendRequest {
    bool success;
    std::string key;
    std::string value;
};

struct ParsedKey {
    bool success;
    std::string key;
};

enum CommandType {
    other = 0,
    set = 1,
    get = 2,
    del = 3,
    scan = 4,
    incr = 5,
    decr = 6,
    append = 7,
    exists = 8
};

enum StatusCode {
    success = 0,
    parsingFailure = 1,
    keyNotFound = 2,
    invalidCommand = 3,
    unexpectedError = 4,
    notInteger = 5
};

struct HandlerResponse {
    StatusCode statusCode;
    std::string result;
};

const char COMMAND_SEPARATOR = '|';

// set||name||geoffrey -> set
CommandType getCommandType(std::string command);

// set||name||geoffrey -> {true, name, geoffrey}
ParsedSetRequest parseSet(std::string command);

// get||name -> {true, name}
ParsedGetRequest parseGet(std::string command);

// del||name -> {true, name}
ParsedDelRequest parseDel(std::string command);

// scan||prefix||limit -> {true, prefix, limit}
ParsedScanRequest parseScan(std::string command);

// incr||key||delta -> {true, key, delta}
ParsedIncrRequest parseIncr(std::string command);

// decr||key||delta -> {true, key, delta}
ParsedIncrRequest parseDecr(std::string command);

// append||key||value -> {true, key, value}
ParsedAppendRequest parseAppend(std::string command);

// exists||key -> {true, key}
ParsedGetRequest parseExists(std::string command);

// name -> {true, name}
ParsedKey parseKey(std::string command);

// { 0, "geoffrey" } -> "0||geoffrey"
std::string formatResponseString(HandlerResponse response);

// "0||geoffrey" -> { success, "geoffrey" }
HandlerResponse parseResponseString(std::string response);

#endif