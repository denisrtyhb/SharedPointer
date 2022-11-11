#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

struct Slug {};

// Primary template
template <typename T, typename Deleter = Slug>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) noexcept : buffer(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) noexcept : buffer(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : buffer(std::move(other.buffer.GetFirst()), std::move(other.buffer.GetSecond())) {
        other.buffer.GetFirst() = nullptr;
    }

    template <typename TOther, typename TOtherDeleter>
    UniquePtr(UniquePtr<TOther, TOtherDeleter>&& other) noexcept
        : buffer(std::move(other.buffer.GetFirst()), std::move(other.buffer.GetSecond())) {
        other.buffer.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        Delete();
        std::swap(buffer, other.buffer);
        return *this;
    }

    template <typename TOther, typename TOtherDeleter>
    UniquePtr& operator=(UniquePtr<TOther, TOtherDeleter>&& other) noexcept {
        Delete();
        buffer.GetFirst() = std::move(other.buffer.GetFirst());
        buffer.GetSecond() = std::move(other.buffer.GetSecond());
        other.buffer.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Delete();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Delete();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* res = buffer.GetFirst();
        buffer.GetFirst() = nullptr;
        return res;
    }
    void Reset(T* ptr = nullptr) {
        if (buffer.GetFirst() == reinterpret_cast<T*>(this)) {
            buffer.GetFirst() = nullptr;
            delete this;
            return;
        }
        Delete();
        buffer.GetFirst() = ptr;
    }
    void Swap(UniquePtr& other) {
        std::swap(buffer.GetFirst(), other.buffer.GetFirst());
        std::swap(buffer.GetSecond(), other.buffer.GetSecond());
    }

    // ////////////////////////////////////////////////////////////////////////////////////////////////
    // // Observers

    T* Get() const {
        return buffer.GetFirst();
    }
    Deleter& GetDeleter() {
        return buffer.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return buffer.GetSecond();
    }
    explicit operator bool() const {
        return buffer.GetFirst() != nullptr;
    }

    // ////////////////////////////////////////////////////////////////////////////////////////////////
    // // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *buffer.GetFirst();
    }
    T* operator->() const {
        return buffer.GetFirst();
    }

    CompressedPair<T*, Deleter> buffer;

    void Delete() {
        if constexpr (std::is_same_v<Deleter, Slug>) {
            delete buffer.GetFirst();
        } else {
            buffer.GetSecond()(buffer.GetFirst());
        }
        buffer.GetFirst() = nullptr;
    }
};
// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T ptr[] = nullptr) noexcept : buffer(ptr, Deleter()) {
    }
    UniquePtr(T ptr[], Deleter deleter) noexcept : buffer(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : buffer(std::move(other.buffer.GetFirst()), std::move(other.buffer.GetSecond())) {
        other.buffer.GetFirst() = nullptr;
    }

    template <typename TOther, typename TOtherDeleter>
    UniquePtr(UniquePtr<TOther, TOtherDeleter>&& other) noexcept
        : buffer(std::move(other.buffer.GetFirst()), std::move(other.buffer.GetSecond())) {
        other.buffer.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        Delete();
        std::swap(buffer, other.buffer);
        return *this;
    }

    template <typename TOther, typename TOtherDeleter>
    UniquePtr& operator=(UniquePtr<TOther, TOtherDeleter>&& other) noexcept {
        Delete();
        buffer.GetFirst() = std::move(other.buffer.GetFirst());
        buffer.GetSecond() = std::move(other.buffer.GetSecond());
        other.buffer.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Delete();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Delete();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* res = buffer.GetFirst();
        buffer.GetFirst() = nullptr;
        return res;
    }
    void Reset(T ptr[] = nullptr) {
        if (reinterpret_cast<UniquePtr<T[], Deleter>*>(buffer.GetFirst()) == this) {
            buffer.GetFirst() = nullptr;
            delete this;
            return;
        }
        Delete();
        buffer.GetFirst() = ptr;
    }
    void Swap(UniquePtr& other) {
        std::swap(buffer.GetFirst(), other.buffer.GetFirst());
        std::swap(buffer.GetSecond(), other.buffer.GetSecond());
    }

    // ////////////////////////////////////////////////////////////////////////////////////////////////
    // // Observers

    T* Get() const {
        return buffer.GetFirst();
    }
    Deleter& GetDeleter() {
        return buffer.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return buffer.GetSecond();
    }
    explicit operator bool() const {
        return buffer.GetFirst() != nullptr;
    }

    // ////////////////////////////////////////////////////////////////////////////////////////////////
    // // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *buffer.GetFirst();
    }
    T* operator->() const {
        return buffer.GetFirst();
    }

    T& operator[](size_t ind) {
        return buffer.GetFirst()[ind];
    }

    CompressedPair<T*, Deleter> buffer;

    void Delete() {
        if constexpr (std::is_same_v<Deleter, Slug>) {
            delete[] buffer.GetFirst();
        } else {
            buffer.GetSecond()(buffer.GetFirst());
        }
        buffer.GetFirst() = nullptr;
    }
};
