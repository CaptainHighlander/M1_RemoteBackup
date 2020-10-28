#include "dao.h"
#include <sqlite3.h>
#include <iostream> //TMP, only for debug print.

namespace DAO
{
    #pragma region Class UserInfo:
    string Dao::UserInfo::GetUserId(void) const
    {
        return this->userID;
    }

    string Dao::UserInfo::GetPasswordHash(void) const
    {
        return this->passwordHash;
    }

    string Dao::UserInfo::GetSalt(void) const
    {
        return this->salt;
    }
    #pragma endregion

    #pragma region Class Dao:
    pair<bool,Dao::UserInfo> Dao::GetUserInfo(const std::string& username)
    {
        //At first, we can assume that username will not found in the database.
        pair<bool, Dao::UserInfo> result(false, Dao::UserInfo());

        sqlite3* dbPtr; //Database.
        //Open db and checks if everything goes fine
        if (sqlite3_open_v2("Database/users.db", &dbPtr, SQLITE_OPEN_READONLY, NULL) == SQLITE_OK)
        {
            //Create SQL query and excute it.
            sqlite3_stmt* statement; //Statement variable. It gets the query sql result
            const string query = "SELECT Username, Hash, Salt "
                                 "FROM USERS WHERE Username = ?1 COLLATE NOCASE;";
            sqlite3_prepare_v2(dbPtr, query.c_str(), -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_STATIC);

            //Get the result of the SQL query
            uint32_t numberOfRow = 0;
            int returnCode; //Return code from the query.
            while ((returnCode = sqlite3_step(statement)) == SQLITE_ROW)
            {
                //Dead a new line created by previous SQL query
                numberOfRow += 1;
                if (numberOfRow > 1)
                {
                    //Database error. UserID isn't unique. Manage it in some way.
                    break;
                }
                //Set user info
                result.second.userID = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 0)));
                result.second.passwordHash = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
                result.second.salt = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
            }

            //Username was found in the database during the previous SQL query
            if (numberOfRow == 1)
                result.first = true;
            else
            {
                std::cout << "[DEBUG] Nothing was read" << std::endl;
                //Manage uncorrect read from database.
            }


            //Finalization in order to avoid resource leaks
            sqlite3_finalize(statement); //Delete sql statement from memory.
            sqlite3_close(dbPtr); //Close db.
        }
        else
        {
            std::cout << "[DEBUG] Database not found" << std::endl;
            //Manage unexisting database.
        }

        return result;
    }
    #pragma endregion
};
