#pragma once

#include "../ThreadGuard/thread_guard.hpp"

#include <experimental/filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <list>

using std::string;
using std::unordered_map;
using std::list;
namespace fs = std::experimental::filesystem;

class FileSystemWatcher
{
public:
    enum class FileStatus { FS_Created, FS_Modified, FS_Erased };

    #pragma region Constructors and destructor:
    explicit FileSystemWatcher(const string& _pathToWatch);
    //We don't allow operator= because it's unnecessary.
    //On the other hand, we need CopyConstructor since to create a thread inside the class.
    //For now, we use automatic CopyConstructor, since it seems we don't need to overwrite it.
    FileSystemWatcher& operator=(FileSystemWatcher const& other) = delete;
    ~FileSystemWatcher(void);
    #pragma endregion

    #pragma region Public members:
    void StartWatch(const std::function<void (const string&, const FileStatus)> &action);
    void StopWatch(void);
    #pragma endregion
private:
    struct FileInfo_s
    {
        bool bUnvalid = false;
        bool bIsFolder = false;
        bool bIsRegularFile = false;
        fs::file_time_type fileTimeType;
        string digest;
    };
    bool bWatching;
    string pathToWatch;
    unordered_map<string, FileInfo_s> monitoredFiles;

    //Each of two lists contain only an element.
    //We are using list in order to don't create a thread and a ThreadGuard before the time.
    //Indeed, if we hadn't used a list, the thread and the ThreadGuard would be instantiated during the creation
    // of a FileSystemWatcher object.
    list<std::thread> watchingThread;
    list<ThreadGuard> watchingThreadGuard;

    #pragma region Private members:
    void Watching(const std::function<void (const string&, const FileStatus)> &action);
    void CheckForSomething(const bool bCheckAlsoDeletedPath, const std::function<void (const string&, const FileStatus)> &actionFunct);
    void CheckForDeletedPath(const std::function<void (const string&, const FileStatus)> &action);
    void CheckForCreatedOrModifiedPath(const std::function<void (const string&, const FileStatus)> &action);
    [[nodiscard]] static string DigestFromFile(const string& path);

    #pragma endregion
};
