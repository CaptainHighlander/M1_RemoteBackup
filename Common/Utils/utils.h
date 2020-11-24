#pragma once
#include <string>
using std::string;

namespace utils
{
    //Compute the digest of a file.
    [[nodiscard]] string DigestFromFile(const string& path);

    //Erase first occurrence of given substring from main string.
    void EraseSubStr(string& mainStr, const string& toErase);
}
