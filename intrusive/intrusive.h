#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        return --count_;
    }
    size_t RefCount() const {
        return count_;
    }

    SimpleCounter& operator=(const SimpleCounter& other) {
        return *this;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (counter_.DecRef() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : object(nullptr) {
    }
    IntrusivePtr(std::nullptr_t) : object(nullptr) {
    }
    IntrusivePtr(T* ptr) : object(ptr) {
        IncRef();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : object(other.object) {
        IncRef();
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : object(other.object) {
        other.object = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : object(other.object) {
        IncRef();
    }
    IntrusivePtr(IntrusivePtr&& other) : object(other.object) {
        other.object = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (object == other.object) {
            return *this;
        }
        DecRef();
        object = other.object;
        IncRef();
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this == &other) {
            return *this;
        }
        DecRef();
        object = other.object;
        other.object = nullptr;
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        DecRef();
    }

    // Modifiers
    void Reset() {
        DecRef();
        object = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr == object) {
            return;
        }
        DecRef();
        object = ptr;
        IncRef();
    }
    void Swap(IntrusivePtr& other) {
        std::swap(object, other.object);
    }

    // Observers
    T* Get() const {
        return object;
    }
    T& operator*() const {
        return *object;
    }
    T* operator->() const {
        return object;
    }
    size_t UseCount() const {
        return object == nullptr ? 0 : object->RefCount();
    }
    explicit operator bool() const {
        return object != nullptr;
    }

    void DecRef() {
        if (object == nullptr) {
            return;
        }
        object->DecRef();
    }

    void IncRef() {
        if (object == nullptr) {
            return;
        }
        object->IncRef();
    }

    T* object = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
