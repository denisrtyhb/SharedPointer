#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) {
        buffer = other.buffer;
        x = other.x;
        IncreaseWeak();
    }
    WeakPtr(WeakPtr&& other) {
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr;
        other.x = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        buffer = other.buffer;
        x = other.x;
        IncreaseWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        DecreaseWeak();
        buffer = other.buffer;
        x = other.x;
        IncreaseWeak();
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        DecreaseWeak();
        buffer = other.buffer;
        x = other.x;
        other.buffer = nullptr;
        other.x = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        DecreaseWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecreaseWeak();
        buffer = nullptr;
        x = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(buffer, other.buffer);
        std::swap(x, other.x);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return buffer == nullptr ? 0 : buffer->strong_cnt;
    }
    bool Expired() const {
        return buffer == nullptr || buffer->strong_cnt == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        SharedPtr<T> res;
        res.buffer = buffer;
        res.x = x;
        res.IncreaseStrong();
        return res;
    }

    void IncreaseWeak() {
        if (buffer != nullptr) {
            buffer->IncreaseWeak();
        }
    }
    void DecreaseWeak() {
        if (buffer != nullptr) {
            buffer->DecreaseWeak();
        }
    }

    ControlBlockBasic* buffer = nullptr;
    T* x = nullptr;
};
