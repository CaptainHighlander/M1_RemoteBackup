#pragma once

#include <mutex>
#include <set>
#include <iterator>

///This class allows to thread-safe use a generic and reduced set.
template <class T1>
class SharedSet
{
public:
    typedef typename std::set<T1> mySet;
    typedef typename mySet::iterator iterator;
    typedef typename mySet::const_iterator const_iterator;

    SharedSet(void) = default;

    void Insert(const T1& element)
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        this->set.insert(element);
    }

    [[nodiscard]] std::optional<T1> Extract(void)
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);

        if (this->set.empty() == true)
            return std::nullopt; //There isn't an element to return.

        //Get first element of the set
        const auto element = this->set.begin();
        auto elementToReturn = (*element);

        //Remove element from the set
        this->set.erase(element);

        //Return extracted element
        return elementToReturn;
    }

    void Remove(const T1& element)
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        this->set.erase(element);
    }

    [[nodiscard]] bool Contains(const T1& element)
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return (this->set.count(element) >= 1);
    }

    [[nodiscard]] bool IsEmpty(void)
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return this->set.empty();
    }

    #pragma region Iterators:
    inline iterator begin(void) noexcept
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return this->set.begin();
    }

    inline const_iterator cbegin(void) const noexcept
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return this->set.cbegin();
    }

    inline iterator end(void) noexcept
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return this->set.end();
    }

    inline const_iterator cend(void) const noexcept
    {
        const std::lock_guard<std::mutex> lg(this->setMutex);
        return this->set.cend();
    }
    #pragma endregion
private:
    SharedSet<T1>& operator=(const SharedSet<T1>& that);
    SharedSet<T1>(const SharedSet<T1>& that);

    std::mutex setMutex;
    mySet set;
};
