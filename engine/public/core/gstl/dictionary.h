#pragma once

#include "str.h"
#include <any>
#include <typeindex>


class Dictionary {
    HashMap<String, std::any> data;

public:
    Dictionary() = default;

    size_t size() const;

    bool is_empty() const;

    void clear();

    template <typename T>
    void set(const String& key, const T& value) {
        data[key] = value;
    }

    template <typename T>
    T get(const String& key, const T& default_value = T{}) const {
        auto it = data.find(key);
        if (it == data.end()) {
            return default_value;
        }

        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return default_value;
        }
    }

    bool has(const String& key) const;

    bool erase(const String& key);

    Vector<String> keys() const;

    template <typename T>
    bool is_type(const String& key) const {
        auto it = data.find(key);

        if (it == data.end()) {
            return false;
        }

        return it->second.type() == typeid(T);
    }

    std::type_index get_type(const String& key) const;

    void merge(const Dictionary& other, bool overwrite = false);

    template <typename Func>
    void for_each(Func func);

    auto begin();
    auto end();
    auto begin() const;
    auto end() const;
};

template <typename Func>
inline void Dictionary::for_each(Func func) {
    for (auto& pair : data) {
        func(pair.first, pair.second);
    }
}
