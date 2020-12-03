#include "utils.h"
#include <fstream>
#include <iostream>

#include <experimental/filesystem>
#include <iomanip>
#include <openssl/sha.h>

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
        char block[SHA512_CBLOCK]{};
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
                    file.clear();
                    file.close();
                    return std::nullopt;
                }
            }
            file.clear();
            file.close();

            //Get final digest.
            unsigned char digest[SHA512_DIGEST_LENGTH]{};
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

    pair<string,string> SplitString(const string& str, const string& delimitator)
    {
        string firstStr;
        string secondStr;

        const size_t pos = str.find(delimitator);
        if (pos != string::npos)
        {
            firstStr = str.substr(0, pos);
            secondStr = str.substr(pos + delimitator.length());
        }
        return std::make_pair(firstStr, secondStr);
    }

    vector<string> GetSubstrings(string str, const string& delimitator)
    {
        vector<string> substrings;

        size_t pos = 0;
        string token;
        while ((pos = str.find(delimitator)) != std::string::npos)
        {
            token = str.substr(0, pos);
            substrings.push_back(token);
            str.erase(0, pos + delimitator.length());
        }
        substrings.push_back(str);
        return substrings;
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

    [[nodiscard]] ssize_t SendFile(tcp::socket& socket, const string& fileToSendPath, const size_t bufferSize, const size_t cursorPos, const string& mexToPreAppend)
    {
        ssize_t bytesRead = -1;

        if (fs::exists(fileToSendPath) == false)
            return bytesRead;
        if (fs::is_directory(fileToSendPath) == true)
            return bytesRead;

        //Open file.
        std::ifstream ifs(fileToSendPath, std::ios::in | std::ios::binary);
        if (ifs.is_open() == false)
            return bytesRead;
        std::ofstream ofs("cpp-output", std::ios::app | std::ios::binary);

        //Move file cursor to the wanted chunk.
        ifs.seekg(cursorPos, std::ifstream::beg);

        //Allocate a buffer.
        char chunk[bufferSize + 1];

        //Read a chunk of the file.
        ifs.read(chunk, bufferSize);
        if (ifs.good() == true) //If was correctly read a number of characters equal to the buffer size.
        {
            bytesRead = ifs.gcount();
            chunk[bytesRead + 1] = '\0';
            utils::SendDataSynchronously(socket, mexToPreAppend + chunk);
            ofs.write(chunk, bytesRead);
        }

        //Close file and pay memory debt.
        ifs.clear();
        ifs.close();

        return bytesRead;
    }
}
