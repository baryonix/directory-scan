#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "collector.h"


using namespace testing;


typedef collector::VectorCollector<std::string> StringCollector;


TEST(VectorCollector, emptyAfterInstantiation) {
    EXPECT_THAT(StringCollector{}.retrieveAndClear(), IsEmpty());
}

TEST(VectorCollector, oneElement) {
    StringCollector collector;
    collector("foo");
    EXPECT_THAT(collector.retrieveAndClear(), ElementsAre("foo"));
    EXPECT_THAT(collector.retrieveAndClear(), IsEmpty());
}

TEST(VectorCollector, twoElements) {
    StringCollector collector;
    collector("foo");
    collector("bar");
    EXPECT_THAT(collector.retrieveAndClear(), ElementsAre("foo", "bar"));
    EXPECT_THAT(collector.retrieveAndClear(), IsEmpty());
}

TEST(VectorCollector, reuseAfterRetrieve) {
    StringCollector collector;
    collector("foo");
    collector.retrieveAndClear();
    collector("bar");
    EXPECT_THAT(collector.retrieveAndClear(), ElementsAre("bar"));
}
