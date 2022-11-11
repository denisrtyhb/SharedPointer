#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <cstddef>   // std::nullptr_t

class ControlBlockBasic {
public:
    virtual void DecreaseStrong() {
    }
    virtual void DecreaseWeak() {
    }
    virtual void IncreaseStrong() {
        ++strong_cnt;
    }
    virtual void IncreaseWeak() {
        ++weak_cnt;
    }
    ControlBlockBasic() {
    }
    virtual ~ControlBlockBasic() {
    }

    size_t strong_cnt = 0;
    size_t weak_cnt = 0;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBasic {
public:
    ControlBlockPointer(T* other) {
        x = other;
        ++strong_cnt;
    }
    void DecreaseStrong() override {
        --strong_cnt;
        if (strong_cnt == 0) {
            delete x;
        }
        if (strong_cnt == 0 && weak_cnt == 0) {
            delete this;
        }
    }
    void DecreaseWeak() override {
        --weak_cnt;
        if (strong_cnt == 0 && weak_cnt == 0) {
            delete this;
        }
    }
    ~ControlBlockPointer() override {
    }
    T* x;
};

template <typename T>
class ControlBlockRawMemory : public ControlBlockBasic {
public:
    template <typename... Args>
    ControlBlockRawMemory(Args&&... args) {
        new (&x) T(std::forward<Args>(args)...);
        ++strong_cnt;
    }
    void DecreaseStrong() override {
        --strong_cnt;
        if (strong_cnt == 0) {
            reinterpret_cast<T*>(&x)->~T();
        }
        if (strong_cnt == 0 && weak_cnt == 0) {
            delete this;
        }
    }
    void DecreaseWeak() override {
        --weak_cnt;
        if (strong_cnt == 0 && weak_cnt == 0) {
            delete this;
        }
    }
    ~ControlBlockRawMemory() override {
    }
    alignas(sizeof(T) > 1 ? alignof(T) : 8) char x[sizeof(T)];
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    template <typename F>
    explicit SharedPtr(F* ptr) {
        buffer = new ControlBlockPointer<F>(ptr);
        x = ptr;
    }

    SharedPtr(const SharedPtr& other) {
        buffer = other.buffer;
        x = other.x;
        IncreaseStrong();
    }

    template <typename TOther>
    SharedPtr(const SharedPtr<TOther>& other) {
        buffer = other.buffer;
        x = other.x;
        IncreaseStrong();
    }

    template <typename TOther>
    SharedPtr(SharedPtr<TOther>&& other) {
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr, other.x = nullptr;
    }

    SharedPtr(SharedPtr&& other) {
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr, other.x = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        buffer = other.buffer;
        x = ptr;
        IncreaseStrong();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr<T>& other) {
        DecreaseStrong();
        buffer = other.buffer;
        x = other.x;
        IncreaseStrong();
        return *this;
    }

    template <typename TOther>
    SharedPtr& operator=(const SharedPtr<TOther>& other) {
        DecreaseStrong();
        buffer = other.buffer;
        x = other.x;
        IncreaseStrong();
        return *this;
    }

    template <typename TOther>
    SharedPtr& operator=(SharedPtr<TOther>&& other) {
        DecreaseStrong();
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr, other.x = nullptr;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (&other == this) {
            return *this;
        }
        DecreaseStrong();
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr, other.x = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (buffer != nullptr) {
            buffer->DecreaseStrong();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecreaseStrong();
        buffer = nullptr, x = nullptr;
    }

    template <typename TOther>
    void Reset(TOther* ptr) {
        DecreaseStrong();
        buffer = new ControlBlockPointer<TOther>(ptr);
        x = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(buffer, other.buffer);
        std::swap(x, other.x);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return x;
    }
    T& operator*() const {
        return *x;
    }
    T* operator->() const {
        return x;
    }
    size_t UseCount() const {
        if (buffer != nullptr) {
            return buffer->strong_cnt;
        } else {
            return 0;
        }
    }
    explicit operator bool() const {
        return buffer != nullptr;
    }

    void IncreaseStrong() {
        if (buffer != nullptr) {
            buffer->IncreaseStrong();
        }
    }
    void DecreaseStrong() {
        if (buffer != nullptr) {
            buffer->DecreaseStrong();
        }
    }
    ControlBlockBasic* buffer = nullptr;
    T* x = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

#include <iostream>

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockRawMemory<T>(std::forward<Args>(args)...);
    SharedPtr<T> res;
    res.buffer = block;
    res.x = reinterpret_cast<T*>(&(block->x));
    return res;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
