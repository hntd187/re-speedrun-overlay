#include <cstdarg>

#include <Windows.h>

#include "String.h"

namespace utility {
std::string narrow(std::wstring_view str) {
    auto length = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.length(), nullptr, 0, nullptr, nullptr);
    std::string narrowStr{};

    narrowStr.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.length(), (LPSTR)narrowStr.c_str(), length, nullptr, nullptr);

    return narrowStr;
}

std::wstring widen(std::string_view str) {
    auto length = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.length(), nullptr, 0);
    std::wstring wideStr{};

    wideStr.resize(length);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.length(), (LPWSTR)wideStr.c_str(), length);

    return wideStr;
}

std::string format_string(const char* format, va_list args) {
    va_list argsCopy{};

    va_copy(argsCopy, args);

    auto len = vsnprintf(nullptr, 0, format, argsCopy);

    va_end(argsCopy);

    if (len <= 0) {
        return {};
    }

    std::string buffer{};

    buffer.resize(len + 1, 0);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    buffer.resize(buffer.size() - 1); // Removes the extra 0 vsnprintf adds.

    return buffer;
}
} // namespace utility