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
    optional<string> DigestFromFile(const string& path)
    {
        //Folders have always an empty digest.
        if (fs::is_regular_file(path) == false)
            return "";

        //Digest can't be calculated on an empty file. So, an empty string will be returned.
        if (fs::is_empty(path) == true)
            return "";

        /* Computation of the digest */
        char block[SHA512_CBLOCK];
        SHA512_CTX mdContext;
        if (SHA512_Init(&mdContext) == false)
            return std::nullopt;
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (file.is_open() == true)
        {
            //Update digest using every chunk of the file.
            while (file.good() == true)
            {
                file.read(block, sizeof(block));
                if(SHA512_Update(&mdContext, block, file.gcount()) == false)
                {
                    file.close();
                    return std::nullopt;
                }
            }
            file.close();

            //Get final digest.
            unsigned char digest[SHA512_DIGEST_LENGTH];
            if (SHA512_Final(digest, &mdContext) == false)
                return std::nullopt;

            //Conversion of the digest to a string: transform byte-array to string
            std::ostringstream digestOStr;
            digestOStr << std::hex << std::setfill('0');
            for (const auto& byte: digest)
                digestOStr << std::setw(2) << (int) byte;

            return digestOStr.str();
        }

        return std::nullopt;
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

    pair<string,string> SplitString(const string& str, const char delimitator)
    {
        string firstStr;
        string secondStr;

        const size_t pos = str.find_first_of(delimitator);
        if (pos != string::npos)
        {
            firstStr = str.substr(0, pos);
            secondStr = str.substr(pos + 1);
        }
        return std::make_pair(firstStr, secondStr);
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
        // Popping last character '\n'
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
