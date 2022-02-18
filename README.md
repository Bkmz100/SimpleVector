# SimpleVector

Контейнер, упрощенный аналог std::vector.

Для этого контейнера была написана обёртка указателя. Реализован с использованием идиомы RAII.

# Использование
Для использования необходима установка и настройка требуемых компонентов.

Пример использования:
```
#include "vector.h"

#include <iostream>
#include <numeric>
#include <random>

int main() {
    Vector<int> v(10);
    std::iota(v.begin(), v.end(), -4);

    std::cout << "Contents of the vector: ";
    for (auto i : v) std::cout << i << ' ';
    std::cout << '\n';

    std::shuffle(v.begin(), v.end(), std::mt19937{ std::random_device{}() });

    std::cout << "Contents of the vector, shuffled: ";
    for (auto i : v) std::cout << i << ' ';
    std::cout << '\n';
}
```
Больше примеров использования и сравнение с std::vector показаны в main.cpp

# Системные требования

1. C++17 (STL)
2. GCC (MinGW-w64) 11.2.0

# Планы по доработке

1. Добавить возможность создание контейнера из определенного количества элементов, инициализированных значением определенным значением.
2. Добавить возможность создания контейнера из std::initializer_list
3. Реализовать методы std::vector, не реализованные в контейнере.
