#include "file_system_watcher.h"
#include "../../Common/Utils/utils.h"
#include <iostream> //Tmp for debug
#include <boost/algorithm/string.hpp>

#pragma region Constructors and destructor:
FileSystemWatcher::FileSystemWatcher(const string& _pathToWatch, const digestsMap& _digestComputedByServer, const notificationFunc& _action)
    : pathToWatch(_pathToWatch), bWatching(false), actionFunc(_action)
{
    //Use the map containg the pair <fileName, digest> as initial monitored files to verify
    // if client and server are synchronized.
    this->monitoredFiles = _digestComputedByServer;
    //Check synchronization between cliend and server.
    this->CheckForSomething();
}

FileSystemWatcher::~FileSystemWatcher(void)
{
    std::cout << "[DEBUG] Destructor FileSystemWatcher" << std::endl;
    //If monitoring was previously started, it will stop concurrent thread.
    this->StopWatch();
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
    this->watchingThread.push_back(std::thread(&FileSystemWatcher::Watching, this, this->actionFunc));
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
void FileSystemWatcher::Watching(const std::function<void (const string&, const FileStatus)> &actionFunct)
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
            //TODO: Notificare al thread principale che c'è stato un problema con quella che era la cartella da monitorare.
            //Probabilmente è stata cancellata completamente oppure ha cambiato nome.

            this->bWatching = false;
            std::cerr << "The path to be monitored wasn't found" << std::endl;
            throw std::exception();
        }
        //Ignore the exception
        ;
    }
    catch (const boost::exception& boostException)
    {
        //Ignore the exception
        ;
    }
    catch (const std::exception& e)
    {
        //TODO: Notificare al thread principale che c'è stato un altro tipo di problema nel monitor.

        this->bWatching = false;
        std::cerr << "Exception FSW\n\t" << e.what() << std::endl;
        throw e;
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
        const string digest = utils::DigestFromFile(pathName);
        //std::cout << "[DEBUG] " << pathName << " has digest:\n\t" << sFI.digest << std::endl;

        //Check when file has been updated last time.
        //sFI.fileTimeType = fs::last_write_time(path_iterator);

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
        this->monitoredFiles[pathName] = digest;
    }
}
#pragma endregion
