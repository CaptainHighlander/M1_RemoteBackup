#include "utils.h"

#include <experimental/filesystem>
#include <iomanip>
#include <openssl/sha.h>
#include <boost/iostreams/device/mapped_file.hpp>

namespace fs = std::experimental::filesystem;

namespace utils
{
    string DigestFromFile(const string& path)
    {
        //Folders have always an empty digest.
        if (fs::is_regular_file(path) == false)
            return "";

        //Digest can't be calculated on an empty file. So, an empty string will be returned.
        if (fs::is_empty(path) == true)
            return "";

        /* Computation of the digest */
        unsigned char digest[SHA512_DIGEST_LENGTH];
        //First of all, we try to access to the memory-mapped file.
        //(The advantage of memory mapping a file is increasing I/O performance, especially when used on big files).
        //If it has been correctly opened, we compute digest using the strong algorithm SHA512.
        //Finally, we convert computed digest to a string.
        boost::iostreams::mapped_file_source mfs;
        mfs.open(path, boost::iostreams::mapped_file_source::readwrite);
        if (mfs.is_open() == true)
        {
            SHA512((unsigned char*) mfs.data(), mfs.size(), digest);
            mfs.close();

            //Conversion of the digest to a string
            std::ostringstream digestOStr;
            digestOStr << std::hex << std::setfill('0');
            for (auto& c: digest)
                digestOStr << std::setw(2) << (int) c;

            return digestOStr.str();
        }

        return "";
    }
}
