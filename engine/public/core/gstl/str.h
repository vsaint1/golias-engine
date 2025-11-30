#pragma once

#include "type_alias.h"
#include <string>
#include "SDL3/SDL_stdinc.h"


class String : public std::string {
public:
    using std::string::string;

    String(const std::string& str) : std::string(str) {}

    String(std::string&& str) : std::string(std::move(str)) {}

    size_t length() const { return size(); }

    bool is_empty() const { return empty(); }

    String to_lower() const;

    String to_upper() const;

    String substr(size_t from, size_t len = std::string::npos) const;

    int find(const String& what, int from = 0) const;

    int find(char what, int from =0) const;

    int rfind(const String& what) const;

    int findn(const String& what, int from = 0) const;

    bool begins_with(const String& text) const;

    bool ends_with(const String& text) const;

    bool contains(const String& what) const;

    String replace(const String& what, const String& forwhat) const;

    String replacen(const String& what, const String& forwhat) const;

    String strip_edges() const;

    String lstrip(const String& chars = " \t\n\r") const;

    String rstrip(const String& chars = " \t\n\r") const;

    Vector<String> split(const char delimiter) const;

    String join(const Vector<String>& parts) const;

    String insert(size_t position, const String& what) const;

    String reverse() const;

    template <typename... Args>
    static String format(const char* fmt, Args... args);

    size_t hash() const;

    String operator+(const String& other) const;

    String operator+(const char* other) const;
};


namespace std {
    template<>
    struct hash<String> {
        size_t operator()(const String& s) const noexcept {
            return hash<std::string>{}(s);
        }
    };
}

template <typename... Args>
inline  String String::format(const char* fmt, Args... args) {
    char buffer[1024];
    SDL_snprintf(buffer, sizeof(buffer), fmt, args...);
    return buffer;
}
