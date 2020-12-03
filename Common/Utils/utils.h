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

namespace utils
{
    //Compute the digest of a file.
    [[nodiscard]] optional<string> DigestFromFile(const string& path);

    //Erase first occurrence of given substring from main string.
    void EraseSubStr(string& mainStr, const string& toErase);

    //Split a string into two substrings.
    [[nodiscard]] pair<string,string> SplitString(const string& str, const string& delimitator);

    [[nodiscard]] vector<string> GetSubstrings(string str, const string& delimitator);

    void SendDataSynchronously(tcp::socket& socket, const string& message);
    [[nodiscard]]string GetDataSynchronously(tcp::socket& socket);

    [[nodiscard]] ssize_t SendFile(tcp::socket& socket, const string& fileToSendPath, const size_t bufferSize, const size_t cursorPos, const string& mexToPreAppend);
}
