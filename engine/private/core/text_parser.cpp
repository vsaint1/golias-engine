#include "core/text_parser.h"




std::vector<TextToken> TextToken::parse_bbcode(const std::string& text, const glm::vec4& default_color) {
    std::vector<TextToken> tokens;
    std::vector<glm::vec4> color_stack = { default_color };
    std::vector<bool> bold_stack = { false };
    std::vector<bool> italic_stack = { false };
    std::vector<bool> strike_stack = { false };

    glm::vec4 current_color = default_color;
    bool is_bold = false;
    bool is_italic = false;
    bool is_strike = false;

    std::string buffer;

    auto flush = [&]() {
        if (!buffer.empty()) {
            tokens.push_back({ buffer, current_color, is_bold, is_italic, is_strike });
            buffer.clear();
        }
    };

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '[') {
            size_t close = text.find(']', i);
            if (close != std::string::npos) {
                std::string tag = text.substr(i + 1, close - i - 1);
                const std::string full_tag = text.substr(i, close - i + 1); // [tag]

                if (tag == "b") {
                    flush();
                    bold_stack.push_back(is_bold);
                    is_bold = true;
                } else if (tag == "/b") {
                    flush();
                    if (!bold_stack.empty()) {
                        is_bold = bold_stack.back();
                        bold_stack.pop_back();
                    }
                } else if (tag == "i") {
                    flush();
                    italic_stack.push_back(is_italic);
                    is_italic = true;
                } else if (tag == "/i") {
                    flush();
                    if (!italic_stack.empty()) {
                        is_italic = italic_stack.back();
                        italic_stack.pop_back();
                    }
                } else if (tag == "s") {
                    flush();
                    strike_stack.push_back(is_strike);
                    is_strike = true;
                } else if (tag == "/s") {
                    flush();
                    if (!strike_stack.empty()) {
                        is_strike = strike_stack.back();
                        strike_stack.pop_back();
                    }
                } else if (tag.rfind("color=", 0) == 0) {
                    std::string hex = tag.substr(6);
                    if (hex.length() == 7 && hex[0] == '#') {
                        try {
                            int r = std::stoi(hex.substr(1, 2), nullptr, 16);
                            int g = std::stoi(hex.substr(3, 2), nullptr, 16);
                            int b = std::stoi(hex.substr(5, 2), nullptr, 16);
                            flush();
                            color_stack.push_back(current_color);
                            current_color = Color(r, g, b, 255).normalize_color();
                        } catch (...) {
                            buffer += full_tag; // invalid hex
                        }
                    } else {
                        buffer += full_tag; // invalid format
                    }
                } else if (tag == "/color") {
                    flush();
                    if (!color_stack.empty()) {
                        current_color = color_stack.back();
                        color_stack.pop_back();
                    }
                } else {
                    buffer += full_tag; // Unknown tag
                }

                i = close;
                continue;
            }
        }

        buffer += text[i];
    }

    flush();
    return tokens;
}
