#include "../../../public/core/gstl/str.h"
#include <algorithm>

String String::to_lower() const {
    String result = *this;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

String String::to_upper() const {
    String result = *this;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

String String::substr(size_t from, size_t len) const {
    return String(std::string::substr(from, len));
}

int String::find(const String& what, int from) const {
    size_t pos = std::string::find(what, from);
    return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

int String::find(char what, int from) const {
    size_t pos = std::string::find(what, from);
    return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

int String::rfind(const String& what) const {
    size_t pos = std::string::rfind(what);
    return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

int String::findn(const String& what, int from) const {
    String lower_this = to_lower();
    String lower_what = what.to_lower();
    size_t pos        = lower_this.std::string::find(lower_what, from);
    return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

bool String::begins_with(const String& text) const {
    return std::string::find(text) == 0;
}

bool String::ends_with(const String& text) const {
    if (text.size() > size()) {
        return false;
    }
    return compare(size() - text.size(), text.size(), text) == 0;
}

bool String::contains(const String& what) const {
    return std::string::find(what) != std::string::npos;
}

String String::replace(const String& what, const String& forwhat) const {
    String result = *this;
    size_t pos    = 0;
    while ((pos = result.std::string::find(what, pos)) != std::string::npos) {
        result.std::string::replace(pos, what.length(), forwhat);
        pos += forwhat.length();
    }
    return result;
}

String String::replacen(const String& what, const String& forwhat) const {
    String result       = *this;
    String lower_result = result.to_lower();
    String lower_what   = what.to_lower();
    size_t pos          = 0;

    while ((pos = lower_result.std::string::find(lower_what, pos)) != std::string::npos) {
        result.std::string::replace(pos, what.length(), forwhat);
        lower_result = result.to_lower();
        pos += forwhat.length();
    }

    return result;
}

String String::strip_edges() const {
    size_t start = find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return String("");
    }
    size_t end = find_last_not_of(" \t\n\r");
    return String(std::string::substr(start, end - start + 1));
}

String String::lstrip(const String& chars) const {
    size_t start = find_first_not_of(chars);

    if (start == std::string::npos) {
        return String("");
    }

    return String(std::string::substr(start));
}

String String::rstrip(const String& chars) const {
    size_t end = find_last_not_of(chars);
    if (end == std::string::npos) {
        return String("");
    }
    return String(std::string::substr(0, end + 1));
}

Vector<String> String::split(char delimiter) const {
    Vector<String> result;
    size_t start = 0;
    size_t end = this->find(delimiter);

    while (end != std::string::npos) {
        result.push_back(this->substr(start, end - start));
        start = end + 1;
        end = this->find(delimiter, start);
    }
    result.push_back(this->substr(start));
    return result;
}

String String::join(const Vector<String>& parts) const {
    if (parts.is_empty()) return String("");

    String result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += *this;
        result += parts[i];
    }

    return result;
}

String String::insert(size_t position, const String& what) const {
    String result = *this;
    result.std::string::insert(position, what);
    return result;
}

String String::reverse() const {
    String result = *this;
    std::reverse(result.begin(), result.end());
    return result;
}


size_t String::hash() const {
    return std::hash<std::string>{}(*this);
}

String String::operator+(const String& other) const {
    return String(static_cast<std::string>(*this) + static_cast<std::string>(other));
}

String String::operator+(const char* other) const {
    return String(static_cast<std::string>(*this) + other);
}
