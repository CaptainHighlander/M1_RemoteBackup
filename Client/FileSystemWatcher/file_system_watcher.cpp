#include "file_system_watcher.h"

#pragma region Constructor:
FileSystemWatcher::FileSystemWatcher(const string& _pathToWatch, const bool _bWatching)
    : pathToWatch(_pathToWatch), bWatching(_bWatching)
{
    //Store information about path (only regular files or folders) that are already in the monitored folder
    for (auto& path_it: fs::recursive_directory_iterator(this->pathToWatch))
    {
        FileInfo_s sFI;
        //Check if the current path is a directory or a regular file
        if (fs::is_directory(path_it) == true)
            sFI.bIsFolder = true;
        else if (fs::is_regular_file(path_it) == true)
            sFI.bIsRegularFile = true;
        else
            continue;

        //Store thet information inherent to the current path
        sFI.fileTimeType = fs::last_write_time(path_it);
        this->monitoredFiles[path_it.path().string()] = sFI;
    }
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

void FileSystemWatcher::Watching(const std::function<void (std::string, FileStatus)> &action)
{
    while (this->bWatching == true)
    {
        this->CheckForDeletedPath(action);
        this->CheckForCreatedOrModifiedPath(action);
    }
}
#pragma endregion

#pragma region Private members:
void FileSystemWatcher::CheckForDeletedPath(const std::function<void(std::string, FileStatus)>& action)
{
    //Iterates over all monitored path
    auto path_it = this->monitoredFiles.begin();
    while (path_it != this->monitoredFiles.end())
    {
        if (fs::exists(path_it->first) == false)
        {
            action(path_it->first, FileStatus::erased);
            path_it = this->monitoredFiles.erase(path_it); //Delete current element from the map and then go to the next path.
        }
        else
            path_it++; //Go to the next path.
    }
}

void FileSystemWatcher::CheckForCreatedOrModifiedPath(const std::function<void(std::string, FileStatus)>& action)
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

        //Check when file has been updated last time.
        sFI.fileTimeType = fs::last_write_time(path_iterator);

        //File creation
        if (this->monitoredFiles.count(pathName) < 1)
        {
            action(pathName, FileStatus::created);
        }
        //File modification (Temporary, digest required to detect a real changement)
        else if (this->monitoredFiles.at(pathName).fileTimeType != sFI.fileTimeType)
        {
            action(pathName, FileStatus::modified);
        }

        //Update information about file.
        this->monitoredFiles[pathName] = sFI;
    }
}
#pragma endregion
