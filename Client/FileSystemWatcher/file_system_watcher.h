#pragma once

#include <experimental/filesystem>
#include <functional>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;
namespace fs = std::experimental::filesystem;

class FileSystemWatcher
{
public:
    enum class FileStatus { created, modified, erased };

    FileSystemWatcher(const string& _pathToWatch, const bool _bWatching);
    void StartWatch(void);
    void StopWatch(void);
    void Watching(const std::function<void (std::string, FileStatus)> &action);
private:
    struct FileInfo_s
    {
        bool bUnvalid = false;
        bool bIsFolder = false;
        bool bIsRegularFile = false;
        fs::file_time_type fileTimeType;
    };
    bool bWatching;
    string pathToWatch;
    unordered_map<string, FileInfo_s> monitoredFiles;

    void CheckForDeletedPath(const std::function<void (std::string, FileStatus)> &action);
    void CheckForCreatedOrModifiedPath(const std::function<void (std::string, FileStatus)> &action);
};
