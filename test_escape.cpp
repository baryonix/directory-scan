#include <gtest/gtest.h>

#include "escape.h"


TEST(escape, emptyString) {
    EXPECT_EQ(escape::escapeSpecialChars(""), "");
}

TEST(escape, nothingToEscape) {
    static const std::string subject = "foobarfasel!@#$%^&*()[]{}/.,?><";
    EXPECT_EQ(escape::escapeSpecialChars(subject), subject);
}

TEST(escape, somethingToEscape) {
    EXPECT_EQ(escape::escapeSpecialChars("abc\rdef\nghi\\jkl"), "abc\\\rdef\\\nghi\\\\jkl");
}
