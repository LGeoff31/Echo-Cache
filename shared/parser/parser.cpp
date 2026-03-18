#include "parser.h"
#include <stdexcept>

StatusCode intToStatusCode(int code) {
    switch (code) {
        case 0: return StatusCode::success;
        case 1: return StatusCode::parsingFailure;
        case 2: return StatusCode::keyNotFound;
        case 3: return StatusCode::invalidCommand;
        case 4: return StatusCode::unexpectedError;
        case 5: return StatusCode::notInteger;
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
    if (commandTypeString == "scan") return CommandType::scan;
    if (commandTypeString == "incr") return CommandType::incr;
    if (commandTypeString == "decr") return CommandType::decr;
    if (commandTypeString == "append") return CommandType::append;
    if (commandTypeString == "exists") return CommandType::exists;
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

ParsedScanRequest parseScan(std::string command) {
    int first_separator_index = find_separator(command, 0);
    if (first_separator_index == -1) {
        return {false, "", 0};
    }
    int second_separator_index = find_separator(command, first_separator_index + 2);
    if (second_separator_index == -1) {
        return {false, "", 0};
    }
    std::string prefix = command.substr(first_separator_index + 2,
                                       second_separator_index - first_separator_index - 2);
    std::string limit_str = command.substr(second_separator_index + 2);
    try {
        int limit = std::stoi(limit_str);
        if (limit < 0) return {false, "", 0};
        return {true, prefix, limit};
    } catch (...) {
        return {false, "", 0};
    }
}

ParsedIncrRequest parseIncr(std::string command) {
    int first_separator_index = find_separator(command, 0);
    if (first_separator_index == -1) {
        return {false, "", 0};
    }
    int second_separator_index = find_separator(command, first_separator_index + 2);
    if (second_separator_index == -1) {
        return {false, "", 0};
    }
    std::string key = command.substr(first_separator_index + 2,
                                    second_separator_index - first_separator_index - 2);
    std::string delta_str = command.substr(second_separator_index + 2);
    try {
        int delta = std::stoi(delta_str);
        return {true, key, delta};
    } catch (...) {
        return {false, "", 0};
    }
}

ParsedIncrRequest parseDecr(std::string command) {
    ParsedIncrRequest req = parseIncr(command);
    if (req.success) {
        req.delta = -req.delta;
    }
    return req;
}

ParsedAppendRequest parseAppend(std::string command) {
    int first_separator_index = find_separator(command, 0);
    if (first_separator_index == -1) {
        return {false, "", ""};
    }
    int second_separator_index = find_separator(command, first_separator_index + 2);
    if (second_separator_index == -1) {
        return {false, "", ""};
    }
    std::string key = command.substr(first_separator_index + 2,
                                    second_separator_index - first_separator_index - 2);
    std::string value = command.substr(second_separator_index + 2);
    return {true, key, value};
}

ParsedGetRequest parseExists(std::string command) {
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
    if (commandType == CommandType::scan) {
        ParsedScanRequest req = parseScan(command);
        if (req.success) return {true, req.prefix};
    }
    if (commandType == CommandType::incr || commandType == CommandType::decr) {
        ParsedIncrRequest req = parseIncr(command);
        if (req.success) return {true, req.key};
    }
    if (commandType == CommandType::append) {
        ParsedAppendRequest req = parseAppend(command);
        if (req.success) return {true, req.key};
    }
    if (commandType == CommandType::exists) {
        ParsedGetRequest req = parseExists(command);
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