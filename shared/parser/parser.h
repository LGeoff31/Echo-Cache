#ifndef PARSER_H
#define PARSER_H

#include <string>

struct ParsedSetRequest {
    std::string key;
    std::string value;
    bool success;
}

struct ParsedGetRequest {
    std::string key;
    bool success;
}

struct ParsedDeleteRequest {
    std::string key;
    bool success;
}

struct ParsedKey {
    bool success;
    std::string key;
}

enum CommandType {
    other = 0,
    set = 1,
    get = 2,
    del = 3
}

enum StatusCode {
    success = 0,
    parsingFailure = 1,
    keyNotFound = 2,
    invalidCommand = 3,
    unexpectedError = 4,
}

struct HandlerResponse {
    StatusCode statusCode,
    std::string result;
}

const char COMMAND_SEPARATOR = '|';

// set||name||geoffrey -> set
CommandType getCommandType(std::string command);

// set||name||geoffrey -> {true, name, geoffrey}
ParsedSetRequest parseSet(std::string command);

// get||name -> {true, name}
ParsedGetRequest parseGet(std::string command);

// del||name -> {true, name}
ParsedDeleteRequest parseDelete(std::string command);

// name -> {true, name}
ParsedKey parseKey(std::string command);

// { 0, "geoffrey" } -> "0||geoffrey"
std::string formatResponseString(HandlerResponse response);

// "0||geoffrey" -> { success, "geoffrey" }
HandlerResponse parseResponseString(std::string response);

#endif