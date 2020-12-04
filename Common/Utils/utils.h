#pragma once

#include <string>
#include <vector>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::vector;
using std::string;
using std::pair;
using std::optional;

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

namespace utils
{
    //Compute the digest of a file.
    [[nodiscard]] optional<string> DigestFromFile(const string& path);

    //Erase first occurrence of given substring from main string.
    void EraseSubStr(string& mainStr, const string& toErase);

    //Split a string into two substrings.
    [[nodiscard]] pair<string,string> SplitString(const string& str, const string& delimitator);
    //Split string into a list of substring using a delimator.
    [[nodiscard]] vector<string> GetSubstrings(string str, const string& delimitator);

    void SendStringSynchronously(tcp::socket& socket, const string& message);
    [[nodiscard]] string GetStringSynchronously(tcp::socket& socket);
    void SendBytesSynchronously(tcp::socket& socket, const char bufferOfBytes[], const size_t bufferSize);
    ssize_t GetBytesSynchronously(tcp::socket& socket, char bufferOfBytes[], const size_t bufferSize);

    ssize_t SendFile(tcp::socket& socket, const string& fileToSendPath, const size_t cursorPos);
}
