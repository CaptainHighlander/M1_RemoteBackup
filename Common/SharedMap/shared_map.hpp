#pragma once

#include <mutex>
#include <map>

///This class allows to thread-safe use a generic and reduced map.
template <class T1, class T2>
class SharedMap
{
public:
    typedef typename std::map<T1,T2> myMap;
    typedef typename myMap::iterator iterator;
    typedef typename myMap::const_iterator const_iterator;

    SharedMap(void) = default;

    void InsertOrUpdate(const T1& key, const T2& value)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        this->map[key] = value;
    }

    [[nodiscard]] std::optional<T2> Get(const T1& key)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return (this->map.count(key) >= 1) ? this->map.at(key) : std::optional<T2>();
    }

    [[nodiscard]] std::optional<std::pair<T1,T2>> Extract(void)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);

        if (this->map.empty() == true)
            return std::nullopt; //There isn't an element to return.

        //Get first element of the map
        const auto element = this->map.begin();
        auto returnPair = std::make_pair(element->first, element->second);

        //Remove element from the map
        this->map.erase(element->first);

        //Return extracted element
        return returnPair;
    }

    void Remove(const T1& key)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        this->map.erase(key);
    }

    [[nodiscard]] bool Contains(const T1& key)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return (this->map.count(key) >= 1);
    }

    [[nodiscard]] bool IsEmpty(void)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return this->map.empty();
    }

    void Clear(void)
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        this->map.clear();
    }

    #pragma region Iterators:
    inline iterator begin(void) noexcept
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return this->map.begin();
    }

    inline const_iterator cbegin(void) const noexcept
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return this->map.cbegin();
    }

    inline iterator end(void) noexcept
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return this->map.end();
    }

    inline const_iterator cend(void) const noexcept
    {
        const std::lock_guard<std::mutex> lg(this->mapMutex);
        return this->map.cend();
    }
    #pragma endregion
private:
    SharedMap<T1,T2>& operator=(const SharedMap<T1,T2>& that);
    SharedMap<T1,T2>(const SharedMap<T1,T2>& that);

    std::mutex mapMutex;
    myMap map;
};
