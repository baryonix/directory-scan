#include <string>

#include <boost/chrono/system_clocks.hpp>

#include <gtest/gtest.h>

#include "directory_scan.h"


static const std::string path = "my/test/path";
static const uint64_t size = 123456789012345ULL;
static const directory_scan::FileInformation::time_type timestamp = boost::chrono::system_clock::now();


TEST(FileInformation, properties) {
    directory_scan::FileInformation fileInformation{path, size, timestamp};
    EXPECT_EQ(fileInformation.path(), path);
    EXPECT_EQ(fileInformation.size(), size);
    EXPECT_EQ(fileInformation.lastWriteTime(), timestamp);
}

TEST(FileInformation, comparison) {
    directory_scan::FileInformation left{"abc", size, timestamp};
    directory_scan::FileInformation right{"abd", size, timestamp};
    EXPECT_TRUE(left < right);
    EXPECT_FALSE(right < left);
    EXPECT_FALSE(left < left);
}
