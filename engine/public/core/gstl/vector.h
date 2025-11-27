#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <iterator>

template<typename T>
class TypedArray : public std::vector<T> {
public:
    using std::vector<T>::vector;

    bool is_empty() const;

    void push_front(const T& value);

    void append(const T& value);

    void append_array(const TypedArray<T>& other);

    void pop_front();

    void erase(size_t position);

    void remove(size_t position);

    void remove_at(size_t position);

    T& get(size_t index);

    const T& get(size_t index) const;

    int find(const T& what, int from = 0) const;

    int rfind(const T& what) const;

    bool has(const T& what) const;

    int count(const T& what) const;

    TypedArray<T> duplicate(bool deep = false) const;

    TypedArray<T> slice(int begin_pos, int end_pos = INT32_MAX) const;

    void reverse();

    void sort();

    template <typename Func>
    void sort(Func comparator);

    T min() const;

    T max() const;

    template<typename Func>
    TypedArray<T> filter(Func predicate) const;

    template <typename Func>
    auto map(Func func) const;

    template <typename Func>
    void for_each(Func func);

    template <typename U, typename Func>
    U reduce(Func func, U initial_value) const;

    bool all(std::function<bool(const T&)> predicate) const;

    bool any(std::function<bool(const T&)> predicate) const;

    T& first();

    const T& first() const;

    T& last();

    const T& last() const;
};


template <typename T>
bool TypedArray<T>::is_empty() const {
    return this->empty();
}

template <typename T>
void TypedArray<T>::push_front(const T& value) {
    this->insert(this->begin(), value);
}

template <typename T>
void TypedArray<T>::append(const T& value) {
    this->push_back(value);
}

template <typename T>
void TypedArray<T>::append_array(const TypedArray<T>& other) {
    this->insert(this->end(), other.begin(), other.end());
}

template <typename T>
void TypedArray<T>::pop_front() {
    if (!this->empty()) {
        this->erase(this->begin());
    }
}

template <typename T>
void TypedArray<T>::erase(size_t position) {
    if (position < this->size()) {
        std::vector<T>::erase(this->begin() + position);
    }
}

template <typename T>
void TypedArray<T>::remove(size_t position) {
    erase(position);
}

template <typename T>
void TypedArray<T>::remove_at(size_t position) {
    erase(position);
}

template <typename T>
T& TypedArray<T>::get(size_t index) {
    return (*this)[index];
}

template <typename T>
const T& TypedArray<T>::get(size_t index) const {
    return (*this)[index];
}

template <typename T>
int TypedArray<T>::find(const T& what, int from) const {
    for (size_t i = from; i < this->size(); ++i) {
        if ((*this)[i] == what) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

template <typename T>
int TypedArray<T>::rfind(const T& what) const {
    for (int i = this->size() - 1; i >= 0; --i) {
        if ((*this)[i] == what) {
            return i;
        }
    }

    return -1;
}

template <typename T>
bool TypedArray<T>::has(const T& what) const {
    return find(what) != -1;
}

template <typename T>
int TypedArray<T>::count(const T& what) const {
    int result = 0;
    for (const auto& item : *this) {
        if (item == what) {
            ++result;
        }
    }

    return result;
}

template <typename T>
TypedArray<T> TypedArray<T>::duplicate(bool deep) const {
    TypedArray<T> result;
    result.assign(this->begin(), this->end());
    return result;
}

template <typename T>
TypedArray<T> TypedArray<T>::slice(int begin_pos, int end_pos) const {
    TypedArray<T> result;
    if (begin_pos < 0) {
        begin_pos = 0;
    }

    if (end_pos > static_cast<int>(this->size())) {
        end_pos = this->size();
    }

    if (begin_pos < end_pos) {
        result.assign(this->begin() + begin_pos, this->begin() + end_pos);
    }

    return result;
}

template <typename T>
void TypedArray<T>::reverse() {
    std::reverse(this->begin(), this->end());
}

template <typename T>
void TypedArray<T>::sort() {
    std::sort(this->begin(), this->end());
}

template <typename T>
template <typename Func>
void TypedArray<T>::sort(Func comparator) {
    std::sort(this->begin(), this->end(), comparator);
}

template <typename T>
T TypedArray<T>::min() const {
    if (this->empty()) {
        throw std::runtime_error("Cannot find min of empty array");
    }

    return *std::min_element(this->begin(), this->end());
}

template <typename T>
T TypedArray<T>::max() const {
    if (this->empty()) {
        throw std::runtime_error("Cannot find max of empty array");
    }

    return *std::max_element(this->begin(), this->end());
}

template <typename T>
template <typename Func>
TypedArray<T> TypedArray<T>::filter(Func predicate) const {
    TypedArray<T> result;
    std::copy_if(this->begin(), this->end(), std::back_inserter(result), predicate);
    return result;
}

template <typename T>
template <typename Func>
auto TypedArray<T>::map(Func func) const {
    using ReturnType = decltype(func(std::declval<T>()));
    TypedArray<ReturnType> result;
    result.reserve(this->size());
    std::transform(this->begin(), this->end(), std::back_inserter(result), func);
    return result;
}

template <typename T>
template <typename Func>
void TypedArray<T>::for_each(Func func) {
    std::for_each(this->begin(), this->end(), func);
}

template <typename T>
template <typename U, typename Func>
U TypedArray<T>::reduce(Func func, U initial_value) const {
    U result = initial_value;
    for (const auto& item : *this) {
        result = func(result, item);
    }
    return result;
}

template <typename T>
bool TypedArray<T>::all(std::function<bool(const T&)> predicate) const {
    return std::all_of(this->begin(), this->end(), predicate);
}

template <typename T>
bool TypedArray<T>::any(std::function<bool(const T&)> predicate) const {
    return std::any_of(this->begin(), this->end(), predicate);
}

template <typename T>
T& TypedArray<T>::first() {
    if (this->empty()) {
        throw std::out_of_range("Array is empty");
    }

    return this->front();
}

template <typename T>
const T& TypedArray<T>::first() const {
    if (this->empty()) {
        throw std::out_of_range("Array is empty");
    }

    return this->front();
}

template <typename T>
T& TypedArray<T>::last() {
    if (this->empty()) {
        throw std::out_of_range("Array is empty");
    }

    return this->back();
}

template <typename T>
const T& TypedArray<T>::last() const {
    if (this->empty()) {
        throw std::out_of_range("Array is empty");
    }

    return this->back();
}