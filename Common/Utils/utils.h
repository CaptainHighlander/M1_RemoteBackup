#pragma once

#include <string>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;

namespace utils
{
    //Compute the digest of a file.
    [[nodiscard]] string DigestFromFile(const string& path);

    //Erase first occurrence of given substring from main string.
    void EraseSubStr(string& mainStr, const string& toErase);

    [[nodiscard]] size_t GetFileSize(std::fstream& fs);

    void SendDataSynchronously(tcp::socket& socket, const string& message);
    string GetDataSynchronously(tcp::socket& socket);

    void SendFile(tcp::socket& socket, const string& namePath);
}
