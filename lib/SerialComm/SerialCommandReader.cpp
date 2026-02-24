#include "SerialCommandReader.hpp"

bool SerialCommandReader::getInt32(int32_t& value) {
    String token = getNextToken();
    if (token.length() == 0) {
        return false;
    }
    long read_value = token.toInt();
    // toInt() returns 0 on failure, check if it's actually zero or error
    if (read_value == 0 && token != "0") {
        return false;
    }
    if (read_value < INT32_MIN || read_value > INT32_MAX) {
        return false; // Out of range for int32_t
    }
    value = static_cast<int32_t>(read_value);
    return true;
}

bool SerialCommandReader::getUInt32(uint32_t& value) {
    String token = getNextToken();
    if (token.length() == 0) {
        return false;
    }
    long read_value = token.toInt();
    // toInt() returns 0 on failure, check if it's actually zero or error
    if (read_value == 0 && token != "0") {
        return false;
    }
    if (read_value < 0 || read_value > UINT32_MAX) {
        return false; // Out of range for uint32_t
    }
    value = static_cast<uint32_t>(read_value);
    return true;
}

bool SerialCommandReader::getFloat(float& value) {
    String token = getNextToken();
    if (token.length() == 0) {
        return false;
    }
    value = token.toFloat();
    // toFloat() returns 0.0 on failure, check if it's actually zero or error
    if (value == 0.0f && token != "0" && token != "0.0") {
        return false;
    }
    return true;
}

bool SerialCommandReader::getBool(bool& value) {
    String token = getNextToken();
    if (token.length() == 0) {
        return false;
    }
    token.toLowerCase();
    if (token == "true" || token == "1") {
        value = true;
        return true;
    } else if (token == "false" || token == "0") {
        value = false;
        return true;
    }
    return false;
}