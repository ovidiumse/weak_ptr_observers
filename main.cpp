#include <memory> // weak_ptr, shared_ptr
#include <iostream>

#include <set>
#include <vector>

class IObserver
{
public:
    virtual void onSomething(int something) {}
};

class Cache
{
private:
    struct Comparator
    {
        bool operator()(const std::weak_ptr<IObserver>& ptr1, const std::weak_ptr<IObserver>& ptr2)
        {
            const auto sptr1 = ptr1.lock();
            const auto sptr2 = ptr2.lock();

            if (!sptr1 && !sptr2)
            {
                return false;
            }

            if (!sptr1)
            {
                return true;
            }

            if (!sptr2)
            {
                return false;
            }

            return sptr1.get() < sptr2.get();
        }
    };

public:
    void addObserver(const std::weak_ptr<IObserver>& observer)
    {
        observers_.insert(observer);
    }

    void notify(int something)
    {
        std::vector<std::weak_ptr<IObserver>> deadObservers;
        deadObservers.reserve(observers_.size());

        for (const auto& o : observers_)
        {
            if (const auto observer = o.lock())
            {
                observer->onSomething(something);
            }
            else
            {
                deadObservers.push_back(o);
            }
        }

        for (const auto& o : deadObservers)
        {
            std::cout << "Removing one...\n";
            observers_.erase(o);
        }
    }
private:
    std::set<std::weak_ptr<IObserver>, Comparator> observers_;
};

class Manager : public IObserver
{
    void onSomething(int something) override
    {
        std::cout << "onSomething called with " << something << " for " << std::hex << this << "!\n";
    }
};

int main()
{
    Cache cache;

    auto a = std::make_shared<Manager>();

    {
        auto b = std::make_shared<Manager>();

        cache.addObserver(a);
        cache.addObserver(a);
        cache.addObserver(b);

        std::cout << "Notifying 5...\n";
        cache.notify(5);
    }

    std::cout << "Notifying 3...\n";
    cache.notify(3);
    
    return 0;
}
