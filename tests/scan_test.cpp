#include <gtest/gtest.h>
#include <print>

#include "scan.hpp"

TEST(ScanTest, DefaultPlaceholderInt) {
    auto r = stdx::scan<int>("42", "{}");
    ASSERT_TRUE(r.has_value());
    auto [v] = r->values();
    ASSERT_EQ(v, 42);
}

TEST(ScanTest, DefaultPlaceholderString) {
    auto r = stdx::scan<std::string>("hello", "{}");
    ASSERT_TRUE(r.has_value());
    auto [s] = r->values();
    ASSERT_EQ(s, "hello");
}

TEST(ScanTest, SignedIntegerSpecifier) {
    auto r = stdx::scan<int>("-17", "{%d}");
    ASSERT_TRUE(r.has_value());
    auto [v] = r->values();
    ASSERT_EQ(v, -17);
}

TEST(ScanTest, UnsignedIntegerSpecifier) {
    auto r = stdx::scan<unsigned>("255", "{%u}");
    ASSERT_TRUE(r.has_value());
    auto [v] = r->values();
    ASSERT_EQ(v, 255u);
}

TEST(ScanTest, FloatingPointSpecifier) {
    auto r = stdx::scan<double>("3.14", "{%f}");
    ASSERT_TRUE(r.has_value());
    auto [v] = r->values();
    ASSERT_DOUBLE_EQ(v, 3.14);
}

TEST(ScanTest, StringSpecifier) {
    auto r = stdx::scan<std::string>("world", "{%s}");
    ASSERT_TRUE(r.has_value());
    auto [s] = r->values();
    ASSERT_EQ(s, "world");
}

TEST(ScanTest, MixedTypes) {
    auto r = stdx::scan<int, std::string, double>("7 foo 2.5", "{%d} {%s} {%f}");
    ASSERT_TRUE(r.has_value());
    auto [i, s, d] = r->values();
    ASSERT_EQ(i, 7);
    ASSERT_EQ(s, "foo");
    ASSERT_DOUBLE_EQ(d, 2.5);
}

TEST(ScanTest, MixedDefaultAndFloat) {
    auto r = stdx::scan<int, double>("5 6.78", "{} {%f}");
    ASSERT_TRUE(r.has_value());
    auto [i, d] = r->values();
    ASSERT_EQ(i, 5);
    ASSERT_DOUBLE_EQ(d, 6.78);
}

TEST(ScanTest, LeadingLiteral) {
    auto r = stdx::scan<int>("val=9", "val={%d}");
    ASSERT_TRUE(r.has_value());
    auto [i] = r->values();
    ASSERT_EQ(i, 9);
}

TEST(ScanTest, TrailingLiteral) {
    auto r = stdx::scan<long>("100kg", "{%d}kg");
    ASSERT_TRUE(r.has_value());
    auto [l] = r->values();
    ASSERT_EQ(l, 100L);
}

TEST(ScanTest, MissingPercent) {
    auto r = stdx::scan<int>("1", "{d}");
    ASSERT_FALSE(r.has_value());
}

TEST(ScanTest, TooLongSpecifier) {
    auto r = stdx::scan<int>("1", "{%df}");
    ASSERT_FALSE(r.has_value());
}

TEST(ScanTest, NegativeUnsigned) {
    auto r = stdx::scan<unsigned>("-1", "{%u}");
    ASSERT_FALSE(r.has_value());
}

TEST(ScanTest, LiteralMismatch) {
    auto r = stdx::scan<int>("foo1bar", "foo {%d} baz");
    ASSERT_FALSE(r.has_value());
}