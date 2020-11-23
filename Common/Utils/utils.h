#pragma once
#include <string>
using std::string;

namespace utils
{
    [[nodiscard]] string DigestFromFile(const string& path);
}
