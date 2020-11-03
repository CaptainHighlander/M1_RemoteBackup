#include "file_system_watcher.h"
#include <iostream> //Tmp for debug
#include <iomanip>
#include <openssl/sha.h>
#include <boost/iostreams/device/mapped_file.hpp>

#pragma region Constructor:
FileSystemWatcher::FileSystemWatcher(const string& _pathToWatch, const bool _bWatching)
    : pathToWatch(_pathToWatch), bWatching(_bWatching)
{
    //Store information about path (only regular files or folders) that are already in the monitored folder
    this->CheckForCreatedOrModifiedPath(nullptr);
}
#pragma endregion

#pragma region Public members:
void FileSystemWatcher::StartWatch(void)
{
    this->bWatching = true;
}

void FileSystemWatcher::StopWatch(void)
{
    this->bWatching = false;
}

void FileSystemWatcher::Watching(const std::function<void (string, FileStatus)> &actionFunct)
{
    while (this->bWatching == true)
    {
        this->CheckForDeletedPath(actionFunct);
        this->CheckForCreatedOrModifiedPath(actionFunct);
    }
}
#pragma endregion

#pragma region Private members:
void FileSystemWatcher::CheckForDeletedPath(const std::function<void(string, FileStatus)>& actionFunct)
{
    //Iterates over all monitored path
    auto path_it = this->monitoredFiles.begin();
    while (path_it != this->monitoredFiles.end())
    {
        if (fs::exists(path_it->first) == false)
        {
            actionFunct(path_it->first, FileStatus::FS_Erased);
            path_it = this->monitoredFiles.erase(path_it); //Delete current element from the map and then go to the next path.
        }
        else
            path_it++; //Go to the next path.
    }
}

void FileSystemWatcher::CheckForCreatedOrModifiedPath(const std::function<void(string, FileStatus)>& actionFunct)
{
    for (auto &path_iterator : fs::recursive_directory_iterator(this->pathToWatch))
    {
        const std::string pathName = path_iterator.path().string();
        FileInfo_s sFI;

        //Check if the current path is a directory or a regular file
        if (fs::is_directory(path_iterator) == true)
            sFI.bIsFolder = true;
        else if (fs::is_regular_file(path_iterator) == true)
            sFI.bIsRegularFile = true;
        else
            continue;

        //Compute digest of the file. Folder will always have empty digest.
        sFI.digest = FileSystemWatcher::DigestFromFile(pathName);
        //std::cout << "[DEBUG] " << pathName << " has digest:\n\t" << sFI.digest << std::endl;

        //Check when file has been updated last time.
        sFI.fileTimeType = fs::last_write_time(path_iterator);

        //If actionFunct is a pointer to some function run this code block.
        //actionFunct should be nullptr only if FileSystemWatcher::CheckForCreatedOrModifiedPath is called by the constructor of this class.
        if (actionFunct.operator bool() == true)
        {
            //Created (new or renamed) file
            if (this->monitoredFiles.count(pathName) < 1)
            {
                actionFunct(pathName, FileStatus::FS_Created);
            }
            //Modified file
            else if (this->monitoredFiles.at(pathName).fileTimeType != sFI.fileTimeType)
            {
                actionFunct(pathName, FileStatus::FS_Modified);
            }
        }
        //Update information about file.
        this->monitoredFiles[pathName] = sFI;
    }
}

string FileSystemWatcher::DigestFromFile(const string& path)
{
    //Folders have always an empty digest.
    if (fs::is_regular_file(path) == false)
        return "";

    //Computation of the digest
    unsigned char digest[SHA512_DIGEST_LENGTH];
    boost::iostreams::mapped_file_source src { path };
    SHA512((unsigned char*)src.data(), src.size(), digest);

    //Conversion of the digest to a string
    std::ostringstream digestOStr;
    digestOStr << std::hex << std::setfill('0');
    for (auto& c: digest)
        digestOStr << std::setw(2) <<(int)c;

    return digestOStr.str();
}
#pragma endregion
