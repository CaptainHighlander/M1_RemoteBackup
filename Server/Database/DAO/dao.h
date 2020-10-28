#pragma once

#include <string>
#include <utility>
using std::pair;
using std::string;

namespace DAO
{
    class Dao
    {
    public:
        class UserInfo
        {
        public:
            [[nodiscard]] string GetUserId(void) const;
            [[nodiscard]] string GetPasswordHash(void) const;
            [[nodiscard]] string GetSalt(void) const;
        private:
            UserInfo(void) = default;
            string userID;
            string passwordHash;
            string salt;
            friend class Dao;
        };

        [[nodiscard]] static pair<bool, UserInfo> GetUserInfo(const std::string& username);
    private:
        Dao(void) = default;
    };
};
