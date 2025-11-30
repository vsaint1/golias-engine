#include <gtest/gtest.h>
#include "core/gstl/str.h"

TEST(StringTest, Construction) {
    String empty;
    EXPECT_TRUE(empty.is_empty());
    EXPECT_EQ(empty.length(), 0);

    String from_cstr = "Hello";
    EXPECT_EQ(from_cstr, "Hello");
    EXPECT_EQ(from_cstr.length(), 5);

    String copy = from_cstr;
    EXPECT_EQ(copy, from_cstr);
}

TEST(StringTest, CaseConversion) {
    String text = "Hello World";

    EXPECT_EQ(text.to_upper(), "HELLO WORLD");
    EXPECT_EQ(text.to_lower(), "hello world");

    String mixed = "HeLLo WoRLd";
    EXPECT_EQ(mixed.to_upper(), "HELLO WORLD");
    EXPECT_EQ(mixed.to_lower(), "hello world");
}

TEST(StringTest, Substring) {
    String text = "Hello World";

    EXPECT_EQ(text.substr(0, 5), "Hello");
    EXPECT_EQ(text.substr(6, 5), "World");
    EXPECT_EQ(text.substr(0, 11), "Hello World");
}

TEST(StringTest, Find) {
    String text = "Hello World";

    EXPECT_EQ(text.find("World"), 6);
    EXPECT_EQ(text.find("Hello"), 0);
    EXPECT_EQ(text.find("xyz"), -1);
    EXPECT_TRUE(text.contains("World"));
    EXPECT_FALSE(text.contains("xyz"));
}

TEST(StringTest, Replace) {
    String text = "Hello World";

    EXPECT_EQ(text.replace("World", "Universe"), "Hello Universe");
    EXPECT_EQ(text.replace("Hello", "Hi"), "Hi World");
    EXPECT_EQ(text.replace("xyz", "abc"), "Hello World");
}

TEST(StringTest, Trimming) {
    String text = "  Hello World  ";

    EXPECT_EQ(text.strip_edges(), "Hello World");
    EXPECT_EQ(text.lstrip(), "Hello World  ");
    EXPECT_EQ(text.rstrip(), "  Hello World");
}

TEST(StringTest, Split) {
    String text = "one,two,three";
    auto parts = text.split(',');

    EXPECT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "one");
    EXPECT_EQ(parts[1], "two");
    EXPECT_EQ(parts[2], "three");
}

TEST(StringTest, StartsWith_EndsWith) {
    String text = "Hello World";

    EXPECT_TRUE(text.begins_with("Hello"));
    EXPECT_TRUE(text.ends_with("World"));
    EXPECT_FALSE(text.begins_with("World"));
    EXPECT_FALSE(text.ends_with("Hello"));
}

TEST(StringTest, Concatenation) {
    String a = "Hello";
    String b = " World";

    EXPECT_EQ(a + b, "Hello World");

    String c = a;
    c += b;
    EXPECT_EQ(c, "Hello World");
}

TEST(StringTest, Comparison) {
    String a = "abc";
    String b = "abc";
    String c = "def";

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
}

TEST(StringTest, EmptyString) {
    String empty;
    EXPECT_TRUE(empty.is_empty());
    EXPECT_EQ(empty.length(), 0);

    String spaces = "   ";
    EXPECT_FALSE(spaces.is_empty());
    EXPECT_EQ(spaces.length(), 3);
}

TEST(StringTest, IndexAccess) {
    String text = "Hello";
    EXPECT_EQ(text[0], 'H');
    EXPECT_EQ(text[1], 'e');
    EXPECT_EQ(text[4], 'o');
}

TEST(StringTest, Insert) {
    String text = "Hello World";
    EXPECT_EQ(text.insert(6, "Test "), "Hello Test World");
}

TEST(StringTest, Join) {
    String separator = ",";
    Vector<String> parts;
    parts.push_back("one");
    parts.push_back("two");
    parts.push_back("three");

    String joined = separator.join(parts);
    EXPECT_EQ(joined, "one,two,three");
}

TEST(StringTest, Reverse) {
    String text = "Hello";
    EXPECT_EQ(text.reverse(), "olleH");
}

TEST(StringTest, ToCharArray) {
    String text = "Hello";
    const char* chars = text.c_str();
    EXPECT_STREQ(chars, "Hello");
}

TEST(StringTest, Hash) {
    String text1 = "Hello";
    String text2 = "Hello";
    String text3 = "World";

    EXPECT_EQ(text1.hash(), text2.hash());
    EXPECT_NE(text1.hash(), text3.hash());
}

