#include "file_system_watcher.h"
#include "../../Common/Utils/utils.h"
#include <iostream> //Tmp for debug
#include <boost/algorithm/string.hpp>

#pragma region Constructors and destructor:
FileSystemWatcher::FileSystemWatcher(const string& _pathToWatch, const digestsMap& _digestComputedByServer, const notificationFunc& _action)
    :   pathToWatch(_pathToWatch), bWatching(false), actionFunc(_action),
        //Use the map containg the pair <fileName, digest> as initial monitored files to verifyif client and server are synchronized.
        monitoredFiles(_digestComputedByServer)
{
    //Check synchronization between client and server.
    this->CheckForSomething();
}

FileSystemWatcher::~FileSystemWatcher(void)
{
    //If monitoring was previously started, it will stop concurrent thread.
    this->StopWatch();
    std::cout << "[DEBUG] Destructor of FileSystemWatcher" << std::endl;
}
#pragma endregion

#pragma region Static members:
FileSystemWatcher::FSW_up FileSystemWatcher::Create(const string& _pathToWatch, const FileSystemWatcher::digestsMap& _digestComputedByServer, const FileSystemWatcher::notificationFunc& _action)
{
    return std::unique_ptr<FileSystemWatcher>(new FileSystemWatcher(_pathToWatch, _digestComputedByServer, _action));
}
#pragma endregion

#pragma region Public members:
void FileSystemWatcher::StartWatch(void)
{
    //File system watching is already monitoring, so return from current function.
    if (this->bWatching == true)
        return;

    this->bWatching = true;
    //Create a new thread and it's associated to a ThreadGuard to guarantee RAII idiom.
    //Monitoring will be runned asynchronously, in an other thread.
    this->watchingThread.push_back(std::thread(&FileSystemWatcher::Watching, this));
    this->watchingThread.back().detach();
    this->watchingThreadGuard.push_back(ThreadGuard{this->watchingThread.back()});
}

void FileSystemWatcher::StopWatch(void)
{
    //File system watcher isn't monitoring, so return from current function.
    if (this->bWatching == false)
        return;

    this->bWatching = false;
}
#pragma endregion

#pragma region Private members:
void FileSystemWatcher::Watching(void)
{
    std::cout << "[DEBUG] Start watching" << std::endl;
    while (this->bWatching == true)
    {
        this->CheckForSomething();
    }
    std::cout << "[DEBUG] Stop watching" << std::endl;
}

void FileSystemWatcher::CheckForSomething(void)
{
    /*
     * Exception handling: how it's done
     * During monitoring, it's possible that some folder/file is deleted during the execution of some istruction
     * following an existence check.
     * It can throw some exception during the execution of some instructions like, for instance, last_write_time,
     * or during the computation of the digest.
     * These exceptions will be ignored, because everything will be solved by the next run of CheckForDeletedPath.
     * However, if the deleted folder is the one to monitor, the exeception will not be ignored, file system watcher
     * will be stopped and a notification will be sent. The same behavior will be produced by other types of exceptions.
     */
    try
    {
        this->CheckForDeletedPath();
        this->CheckForCreatedOrModifiedPath();
    }
    catch (const fs::filesystem_error& fsException)
    {
        //Consider the exception
        if (fsException.path1() == this->pathToWatch)
        {
            //Send a notification to the main thread in order to inform it that the folder to be monitored was not found.
            //(i.e. it has been deleted or it has been renamed).
            this->StopWatch();
            this->actionFunc("", FileStatus::FS_Error_MissingMainFolder);
            //std::cerr << "[DEBUG] FSW: The path to be monitored wasn't found" << std::endl;
        }
        //Ignore the exception
    }
    catch (const std::exception& e)
    {
        //Send a notification to the main thread in order to inform it that there was a problem during the monitoring.
        this->StopWatch();
        this->actionFunc("", FileStatus::FS_Error_Generic);
        //std::cerr << "[DEBUG] FSW: Exception:\n\t" << e.what() << std::endl;
    }
}

void FileSystemWatcher::CheckForDeletedPath(void)
{
    //Iterates over all monitored path
    auto path_it = this->monitoredFiles.begin();
    while (path_it != this->monitoredFiles.end())
    {
        const string pathName = this->pathToWatch + path_it->first;
        if (fs::exists(pathName) == false)
        {
            fs::path p = path_it->first;
            if (fs::exists(this->pathToWatch + p.parent_path().string()) == true)
                this->actionFunc(path_it->first, FileStatus::FS_Erased);
            path_it = this->monitoredFiles.erase(path_it); //Delete current element from the map and then go to the next path.
        }
        else
            path_it++; //Go to the next path.
    }
}

void FileSystemWatcher::CheckForCreatedOrModifiedPath(void)
{
    for (auto& path_iterator : fs::recursive_directory_iterator(this->pathToWatch))
    {
        std::string pathName = path_iterator.path().string();
        if (boost::algorithm::contains(pathName, "goutputstream") == true)
            continue;

        //Check if the current path is a directory or a regular file
        if (fs::is_directory(path_iterator.path()) == false && fs::is_regular_file(path_iterator.path()) == false)
            continue;

        //Compute digest of the file. Folders will always have an empty digest.
        std::optional<string> digest = utils::DigestFromFile(pathName);
        if (digest.has_value() == false)
            continue;
        //std::cout << "[DEBUG] " << pathName << " has digest:\n\t" << digest.value() << std::endl;

        //Remove main path from the current path
        utils::EraseSubStr(pathName, this->pathToWatch);

        //Created (new or renamed) file
        if (this->monitoredFiles.count(pathName) < 1)
        {
            this->actionFunc(pathName, FileStatus::FS_Created);
        }
        //Modified file
        else if (this->monitoredFiles.at(pathName) != digest)
        {
            this->actionFunc(pathName, FileStatus::FS_Modified);
        }

        //Update information about file.
        this->monitoredFiles[pathName] = std::move(digest.value());
    }
}
#pragma endregion
