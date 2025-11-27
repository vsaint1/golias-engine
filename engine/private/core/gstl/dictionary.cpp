#include "core/gstl/dictionary.h"


size_t Dictionary::size() const {
    return data.size();
}

bool Dictionary::is_empty() const {
    return data.empty();
}

void Dictionary::clear() {
    data.clear();
}

bool Dictionary::has(const String& key) const {
    return data.find(key) != data.end();
}

bool Dictionary::erase(const String& key) {
    return data.erase(key) > 0;
}

Vector<String> Dictionary::keys() const {
    Vector<String> result;
    result.reserve(data.size());
    for (const auto& pair : data) {
        result.push_back(pair.first);
    }
    return result;
}

std::type_index Dictionary::get_type(const String& key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return typeid(void);
    }
    return it->second.type();
}

void Dictionary::merge(const Dictionary& other, bool overwrite) {
    for (const auto& pair : other.data) {
        if (overwrite || !has(pair.first)) {
            data[pair.first] = pair.second;
        }
    }
}


auto Dictionary::begin() {
    return data.begin();
}

auto Dictionary::end() {
    return data.end();
}

auto Dictionary::begin() const {
    return data.begin();
}

auto Dictionary::end() const {
    return data.end();
}

