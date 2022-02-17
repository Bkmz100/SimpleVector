#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    RawMemory(RawMemory&& other) noexcept {
        Swap(other);
    }

    RawMemory(const RawMemory&) = delete;

    ~RawMemory() {
        Deallocate(buffer_);
    }

public:
    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory& operator=(RawMemory&& rhs) noexcept {
        if (this != &rhs) {
            buffer_ = std::exchange(rhs.buffer_, nullptr);
            capacity_ = std::exchange(rhs.capacity_, 0u);
        }

        return *this;
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const noexcept {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

private:
    T* buffer_ = nullptr;
    size_t capacity_ = 0u;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

public:
    Vector() = default;

    explicit Vector(size_t size)
        : data_(size)
        , size_(size) {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector& other)
        : data_(other.size_)
        , size_(other.size_) {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
    }

    Vector(Vector&& other) noexcept {
        Swap(other);
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

public:
    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }

        RawMemory<T> new_data(new_capacity);

        UnitializedN(data_.GetAddress(), size_, new_data.GetAddress());

        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }

    void Resize(size_t new_size) {
        if (size_ > new_size) {
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
        }
        else {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
        }

        size_ = new_size;
    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (rhs.size_ > data_.Capacity()) {
                Vector rhs_copy(rhs);
                Swap(rhs_copy);
            }
            else {
                std::copy_n(rhs.data_.GetAddress(), std::min(rhs.size_, size_), data_.GetAddress());

                if (size_ < rhs.size_) {
                    std::uninitialized_copy_n(
                        rhs.data_.GetAddress() + size_, 
                        rhs.size_ - size_, 
                        data_.GetAddress() + size_
                    );
                }
                else {
                    std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
                }

                size_ = rhs.size_;
            }
        }

        return *this;
    }

    Vector& operator=(Vector&& rhs) noexcept {
        if (this != &rhs) {
            data_ = std::move(rhs.data_);
            size_ = std::exchange(rhs.size_, 0);
        }

        return *this;
    }

    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        T* elem;

        if (Capacity() == size_) {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            elem = new(new_data + size_) T(std::forward<Args>(args)...);

            UnitializedN(data_.GetAddress(), size_, new_data.GetAddress());

            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);

        }
        else {
            elem = new (data_ + size_) T(std::forward<Args>(args)...);
        }
        ++size_;

        return *elem;
    }

    void PushBack(const T& value) {
        EmplaceBack(value);
    }

    void PushBack(T&& value) {
        EmplaceBack(std::move(value));
    }

    void PopBack() {
        --size_;
        std::destroy_at(data_ + size_);
    }

    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    iterator begin() noexcept {
        return data_.GetAddress();
    }

    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }

    const_iterator begin() const noexcept {
        return cbegin();
    }

    const_iterator end() const noexcept {
        return cend();
    }

    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }

    const_iterator cend() const noexcept {
        return data_.GetAddress() + size_;
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        if (pos == cend()) {
            return &EmplaceBack(std::forward<Args>(args)...);
        }

        auto distance = std::distance(cbegin(), pos);
        if (Capacity() == size_) {

            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            new(new_data + distance) T(std::forward<Args>(args)...);

            UnitializedN(data_.GetAddress(), distance, new_data.GetAddress());
            UnitializedN(data_.GetAddress() + distance, size_ - distance, new_data.GetAddress() + distance + 1);

            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);

        }
        else {
            T temp(std::forward<Args>(args)...);
            std::uninitialized_move_n(end() - 1, 1, end());
            std::move_backward(begin() + distance, end() - 1, end());

            *(begin() + distance) = std::move(temp);
        }

        ++size_;

        return begin() + distance;
    }

    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (pos == cend()) {
            PopBack();
            return end();
        }

        auto distance = std::distance(cbegin(), pos);

        std::move(begin() + distance + 1, end(), begin() + distance);
        PopBack();

        return begin() + distance;
    }

    iterator Insert(const_iterator pos, const T& value) {
        return Emplace(pos, value);
    }

    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::move(value));
    }

private:
    static void UnitializedN(T* from, size_t n, T* to) {
        // constexpr оператор if будет вычислен во время компиляции
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(from, n, to);
        }
        else {
            std::uninitialized_copy_n(from, n, to);
        }
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0u;
};
