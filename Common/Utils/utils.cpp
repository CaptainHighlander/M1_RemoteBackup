#include "utils.h"
#include <fstream>
#include <iostream>

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

    void EraseSubStr(string& mainStr, const string& toErase)
    {
        // Search for the substring in string
        const size_t pos = mainStr.find(toErase);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            mainStr.erase(pos, toErase.length());
        }
    }

    size_t GetFileSize(std::ifstream& fs)
    {
        size_t length = 0;
        if(fs)
        {
            fs.seekg(0, std::ifstream::end);
            length = fs.tellg();
            fs.seekg(0, std::ifstream::beg);
        }
        return length;
    }

    void SendDataSynchronously(tcp::socket& socket, const string& message)
    {
        write(socket, buffer(message + "\n"));
    }

    string GetDataSynchronously(tcp::socket& socket)
    {
        boost::asio::streambuf buf;
        read_until(socket, buf, '\n');
        string data = buffer_cast<const char*>(buf.data());
        data.pop_back();
        return data;
    }

    void SendFile(tcp::socket& socket, const string& namePath)
    {
        if (fs::exists(namePath) == false)
            return;
        if (fs::is_directory(namePath) == true)
            return;

        //Open file.
        std::ifstream ifs(namePath, std::ios::in | std::ios::binary);
        if (ifs.is_open() == false)
            return;
        std::ofstream TmpSaveFile("cpp-output", std::ios::out | std::ios::binary);
        if (TmpSaveFile.is_open() == false)
        {
            ifs.close();
            return;
        }

        //Get file size.
        const size_t length = utils::GetFileSize(ifs);
        //Allocate a buffer.
        char* buffer = new char[length + 1];
        //Read file in block.
        ifs.read(buffer, length);
        if (ifs.good() == true)
        {
            buffer[length] = '\0';
            TmpSaveFile.write(buffer, length);
        }

        //Close files and pay memory debt.
        ifs.close();
        TmpSaveFile.close();
        delete[] buffer;
        buffer = nullptr;
    }
}
