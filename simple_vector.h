#pragma once
#include <algorithm>
#include <initializer_list>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        : capacity_(capacity) {
    }

public:
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

private:
    size_t capacity_ = 0u;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

public:
    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : items_(size)
        , size_(size)
        , capacity_(size) {
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const ReserveProxyObj& new_capacity)
        : items_(new_capacity.GetCapacity())
        , capacity_(new_capacity.GetCapacity()) {
    }

    SimpleVector(const SimpleVector& other)
        : items_(other.GetCapacity())
        , size_(other.GetSize())
        , capacity_(other.GetCapacity()) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) noexcept {
        swap(other);
    }

public:
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return GetSize() == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }

        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }

        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0u;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items = ReallocateCopy(new_capacity);
            std::fill(new_items.Get() + size_, new_items.Get() + new_size, Type());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        else {
            if (new_size > size_) {
                std::fill(begin() + size_, begin() + new_size, Type());
            }
        }

        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{ items_.Get() + size_ };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{ items_.Get() + size_ };
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }

        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            SimpleVector tmp(std::move(rhs));
            swap(tmp);
        }

        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items = ReallocateCopy(new_capacity);
            new_items[size_] = item;
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        else {
            items_[size_] = item;
        }

        ++size_;
    }

    void PushBack(Type&& item) {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items = ReallocateCopy(new_capacity);
            new_items[size_] = std::move(item);
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        else {
            items_[size_] = std::move(item);
        }

        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t offset = std::distance(cbegin(), pos);
        assert(pos <= size_);
        Iterator mutable_pos = begin() + offset;
        size_t new_size = size_ + 1;
        if (new_size <= capacity_) {
            std::copy_backward(mutable_pos, end(), end() + 1);
            *mutable_pos = value;
        }
        else {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), mutable_pos, new_items.Get());
            std::copy(mutable_pos, end(), new_items.Get() + offset + 1);
            mutable_pos = new_items.Get() + offset;
            *mutable_pos = value;
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        ++size_;

        return mutable_pos;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos <= size_);
        size_t offset = std::distance(cbegin(), pos);
        size_t new_size = size_ + 1;
        Iterator mutable_pos = begin() + offset;
        if (new_size <= capacity_) {
            std::move_backward(mutable_pos, end(), end() + 1);
            *mutable_pos = std::move(value);
        }
        else {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), mutable_pos, new_items.Get());
            std::move(mutable_pos, end(), new_items.Get() + offset + 1);
            mutable_pos = new_items.Get() + offset;
            *mutable_pos = std::move(value);
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        ++size_;

        return mutable_pos;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (IsEmpty()) return;
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t index = std::distance(cbegin(), pos);
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;

        return Iterator{ &items_[index] };
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items = ReallocateCopy(new_capacity);
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

private:
    // Выделяет копию текущего массива с заданной вместимостью
    ArrayPtr<Type> ReallocateCopy(size_t new_capacity) {
        ArrayPtr<Type> result(new_capacity);
        size_t move_size = std::min(new_capacity, size_);
        std::move(begin(), begin() + move_size, result.Get());

        return ArrayPtr<Type>(result.Release());
    }

private:
    ArrayPtr<Type> items_{};
    size_t size_ = 0u;
    size_t capacity_ = 0u;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
