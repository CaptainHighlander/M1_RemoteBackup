#pragma once

#include "../../Common/ThreadGuard/thread_guard.h"

#include <experimental/filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <list>

using std::string;
using std::unordered_map;
using std::list;
using std::pair;
namespace fs = std::experimental::filesystem;

class FileSystemWatcher
{
public:
    enum class FileStatus { FS_Created, FS_Modified, FS_Erased };
    typedef std::unordered_map<string,string> digestsMap;
    typedef std::function<void (const string&, const FileStatus)> notificationFunc;

    #pragma region Constructors and destructor:
    FileSystemWatcher(const string& _pathToWatch, const digestsMap& _digestComputedByServer, const notificationFunc& _action);
    //We don't allow operator= because it's unnecessary.
    //On the other hand, we need CopyConstructor since to create a thread inside the class.
    //For now, we use automatic CopyConstructor, since it seems we don't need to overwrite it.
    FileSystemWatcher& operator=(FileSystemWatcher const& other) = delete;
    ~FileSystemWatcher(void);
    #pragma endregion

    #pragma region Public members:
    void StartWatch(void);
    void StopWatch(void);
    #pragma endregion
private:
    //Copy constructor
    FileSystemWatcher(FileSystemWatcher const&);

    bool bWatching;
    string pathToWatch;
    digestsMap monitoredFiles;
    notificationFunc actionFunc;

    //Each of two lists contain only an element.
    //We are using list in order to don't create a thread and a ThreadGuard before the time.
    //Indeed, if we hadn't used a list, the thread and the ThreadGuard would be instantiated during the creation
    // of a FileSystemWatcher object.
    list<std::thread> watchingThread;
    list<ThreadGuard> watchingThreadGuard;

    #pragma region Private members:
    void Watching(const std::function<void (const string&, const FileStatus)> &action);
    void CheckForSomething(void);
    void CheckForDeletedPath(void);
    void CheckForCreatedOrModifiedPath(void);
    #pragma endregion
};
