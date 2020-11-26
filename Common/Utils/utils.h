#pragma once

#include <string>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;
using std::pair;

namespace utils
{
    //Compute the digest of a file.
    [[nodiscard]] string DigestFromFile(const string& path);

    //Erase first occurrence of given substring from main string.
    void EraseSubStr(string& mainStr, const string& toErase);

    //Split a string into two substrings.
    [[nodiscard]] pair<string,string> SplitString(const string& str, const char delimitator);

    [[nodiscard]] size_t GetFileSize(std::fstream& fs);

    void SendDataSynchronously(tcp::socket& socket, const string& message);
    [[nodiscard]]string GetDataSynchronously(tcp::socket& socket);

    void SendFile(tcp::socket& socket, const string& namePath);
}
