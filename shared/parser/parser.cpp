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
    std::string command_type_str = command.substr(0, separator_index);
    switch (command_type_str) {
        case "set": return CommandType::set;
        case "get": return CommandType::get;
        case "del": return CommandType::del;
        default: return CommandType::other;
    }
}

ParsedSetRequest parseSet(std::string command) {
    int first_separator_index = find_separator(command, 0);
    if (first_separator_index == -1) {
        return ParsedSetRequest{ false, "", ""};
    }
    
    int second_separator_index = find_separator(command, first_separator_index + 2);
    if (second_separator_index == -1) {
        return ParsedSetRequest{ false, "", ""};
    }

    return {true, command.substr(first_separator_index + 2, second_separator_index - first_separator_index - 2), command.substr(second_separator_index + 2)};
}

ParsedGetRequest parseGet(std::string command) {
    int separator_index = find_separator(command, 0);
    if (separator_index == -1) {
        return {"", false};
    }
    std::string key = command.substr(separator_index + 2);
    return ParsedGetRequest{ true, key };
}

ParsedDeleteRequest parseDelete(std::string command) {
    int separator_index = find_separator(command, 0);
    if (separator_index == -1) {
        return {"", false};
    }
    std::string key = command.substr(separator_index + 2);
    return ParsedDeleteRequest{ true, key };
}

ParsedKey parseKey(std::string command) {
    CommandType commandType = getCommandType(command);
    switch (commandType) {
        case CommandType::set:
            ParsedSetRequest parsedSetRequest = parseSet(command);
            if (parsedSetRequest.success) {
                return {true, parsedSetRequest.key};
            }
            break;
        case CommandType::get:
            ParsedGetRequest parsedGetRequest = parseGet(command);
            if (parsedGetRequest.success) {
                return {true, parsedGetRequest.key};
            }
            break;
        case CommandType::del:
            ParsedDeleteRequest parsedDeleteRequest = parseDelete(command);
            if (parsedDeleteRequest.success) {
                return {true, parsedDeleteRequest.key};
            }
            break;
        default:
            break;
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