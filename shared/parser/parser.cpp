#include "parser.h"
#include <stdexcept>

StatusCode intToStatusCode(int code) {
    switch (code) {
        case 0: return StatusCode::success;
        case 1: return StatusCode::parsingFailure;
        case 2: return StatusCode::keyNotFound;
        case 3: return StatusCode::invalidCommand;
        case 4: return StatusCode::unexpectedError;
        default:
            throw std::invalid_argument("No such status code: " + std::to_string(code));
    }
}

int find_separator(const std::string str, int start) {
    for (int i = start; i < str.length() - 1; i++) {
        if (str[i] == COMMAND_SEPARATOR && str[i + 1] == COMMAND_SEPARATOR) {
            return i;
        }
    }
    return -1;
}

CommandType getCommandType(std::string command) {
    int separator_index = find_separator(command, 0);
    if (separator_index == -1) {
        return CommandType::other;
    }
    std::string commandTypeString = command.substr(0, separator_index);
    if (commandTypeString == "get") return CommandType::get;
    if (commandTypeString == "del") return CommandType::del;
    if (commandTypeString == "set") return CommandType::set;
    return CommandType::other;
}

ParsedSetRequest parseSet(std::string command) {
    int first_separator_index = find_separator(command, 0);
    if (first_separator_index == -1) {
        return {false, "", ""};
    }
    
    int second_separator_index = find_separator(command, first_separator_index + 2);
    if (second_separator_index == -1) {
        return {false, "", ""};
    }

    int key_start = first_separator_index + 2;
    int key_length = second_separator_index - key_start;
    int val_start = second_separator_index + 2;
    int val_length = command.length() - val_start;

    return {true, command.substr(key_start, key_length), command.substr(val_start, val_length)};
}

ParsedGetRequest parseGet(std::string command) {
    int separator_index = find_separator(command, 0);
    if (separator_index == -1) {
        return {false, ""};
    }
    int key_start = separator_index + 2;
    int key_length = command.length() - key_start;
    return {true, command.substr(key_start, key_length)};
}

ParsedDelRequest parseDel(std::string command) {
    int separator_index = find_separator(command, 0);
    if (separator_index == -1) {
        return {false, ""};
    }
    int key_start = separator_index + 2;
    int key_length = command.length() - key_start;
    return {true, command.substr(key_start, key_length)};
}

ParsedKey parseKey(std::string command) {
    CommandType commandType = getCommandType(command);

    if (commandType == CommandType::get) {
        ParsedGetRequest req = parseGet(command);
        if (req.success) return {true, req.key};
    }
    if (commandType == CommandType::del) {
        ParsedDelRequest req = parseDel(command);
        if (req.success) return {true, req.key};
    }
    if (commandType == CommandType::set) {
        ParsedSetRequest req = parseSet(command);
        if (req.success) return {true, req.key};
    }
    return {false, ""};
}

std::string formatResponseString(HandlerResponse response) {
    return std::to_string(response.statusCode) + "||" + response.result;
}

HandlerResponse parseResponseString(std::string response) {
    int separator_index = find_separator(response, 0);
    if (separator_index == -1) {
        return {StatusCode::parsingFailure, ""};
    }
    std::string status_code_str = response.substr(0, separator_index);
    StatusCode statusCode = intToStatusCode(std::stoi(status_code_str));
    std::string result = response.substr(separator_index + 2);
    return {statusCode, result};
}