#include <string>
#include <filesystem>
#include <fstream>

#include <boost/any.hpp>
#include <boost/chrono/system_clocks.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "directory_scan.h"


using namespace testing;
namespace fs = std::filesystem;


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


static const fs::path TEST_ROOT{"TEST_ROOT"};
static const fs::path TEST_FILE = TEST_ROOT / "file";
static const fs::path TEST_SUBDIR = TEST_ROOT / "subdir";

static const std::string TEST_FILE_CONTENT = "foobar";

static void setUpTestDirectory() {
    if (fs::exists(TEST_ROOT))
        fs::remove_all(TEST_ROOT);

    fs::create_directory(TEST_ROOT);
    fs::create_directory(TEST_SUBDIR);
    std::ofstream(TEST_FILE) << TEST_FILE_CONTENT;
}

class MockConsumer {
public:
    void operator()(const directory_scan::FileInformation &t) {
        consume(t);
    }

    MOCK_METHOD1(consume, void(
            const directory_scan::FileInformation&));
};

class MockExecutor;

typedef directory_scan::PathScanner<MockExecutor &, MockConsumer> StubbedPathScanner;

class MockExecutor {
public:
    void operator()(const StubbedPathScanner &t) {
        execute(t);
    }

    MOCK_METHOD1(execute, void(
            const StubbedPathScanner&));
};


TEST(PathScanner, scan) {
    setUpTestDirectory();

    MockExecutor executor;
    MockConsumer consumer;
    StubbedPathScanner pathScanner{TEST_ROOT, executor, consumer};

    EXPECT_CALL(executor, execute(Property(&StubbedPathScanner::getPath, Eq(TEST_SUBDIR))));
    EXPECT_CALL(consumer, consume(AllOf(
            Property(&directory_scan::FileInformation::path, Eq(TEST_FILE)),
            Property(&directory_scan::FileInformation::size, Eq(TEST_FILE_CONTENT.length())))
    ));

    pathScanner();
}
