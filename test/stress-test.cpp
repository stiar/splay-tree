#include "gtest/gtest.h"
#include "splay-tree/set.h"
#include "splay-tree/multiset.h"

#include <set>
#include <random>

// Test for insert, erase, count, lower_bound, upper_bound, size.
TEST(splay_tree_test, stressTestWithSet) {
    splay_tree::set<int> splayTreeSet;
    std::set<int> stlSet;

    constexpr unsigned int NUMBER_OF_ITERATIONS = 10000;
    constexpr int MAX_ABS_VALUE = 1000;

    std::random_device rd;
    auto seed = rd();
    std::cerr << "Running stress test with " << NUMBER_OF_ITERATIONS <<
        " iterations, maximal absolute value of keys " << MAX_ABS_VALUE <<
        ", seed " << seed << "\n";
    std::mt19937 mt(seed);

    enum Operation {
        INSERT,
        ERASE,
        COUNT,
        LOWER_BOUND,
        UPPER_BOUND,
        SIZE,
        VALIDATE_SIZE,
        NUMBER_OF_OPERATIONS
    };

    std::uniform_int_distribution<int> randomOperation(0, NUMBER_OF_OPERATIONS);
    std::uniform_int_distribution<int> randomInt(-MAX_ABS_VALUE, MAX_ABS_VALUE);

    for (unsigned int i = 0; i < NUMBER_OF_ITERATIONS; ++i) {
        Operation operation = static_cast<Operation>(randomOperation(mt));
        switch (operation) {
            case INSERT:
                {
                    int key = randomInt(mt);
                    auto stlSetInsertResult = stlSet.insert(key);
                    auto splayTreeInsertResult = splayTreeSet.insert(key);
                    EXPECT_EQ(
                        std::distance(stlSet.begin(), stlSetInsertResult.first),
                        std::distance(
                            splayTreeSet.begin(),
                            splayTreeInsertResult.first));
                    EXPECT_EQ(
                        stlSetInsertResult.second,
                        splayTreeInsertResult.second);
                }
                break;
            case ERASE:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(stlSet.erase(key), splayTreeSet.erase(key));
                }
                break;
            case COUNT:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(stlSet.count(key), splayTreeSet.count(key));
                }
                break;
            case LOWER_BOUND:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(
                        std::distance(stlSet.begin(), stlSet.lower_bound(key)),
                        std::distance(
                            splayTreeSet.begin(),
                            splayTreeSet.lower_bound(key)));
                }
                break;
            case UPPER_BOUND:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(
                        std::distance(stlSet.begin(), stlSet.upper_bound(key)),
                        std::distance(
                            splayTreeSet.begin(),
                            splayTreeSet.upper_bound(key)));
                }
                break;
            case SIZE:
                EXPECT_EQ(stlSet.size(), splayTreeSet.size());
                break;
            case VALIDATE_SIZE:
                EXPECT_EQ(
                    splayTreeSet.size(),
                    std::distance(
                        splayTreeSet.begin(),
                        splayTreeSet.end()));
                break;
        }
    }
}

// Test for insert, erase, count, lower_bound, upper_bound, size.
TEST(splay_tree_test, stressTestWithMultiSet) {
    splay_tree::multiset<int> splayTreeMultiSet;
    std::multiset<int> stlMultiSet;

    constexpr unsigned int NUMBER_OF_ITERATIONS = 10000;
    constexpr int MAX_ABS_VALUE = 1000;

    std::random_device rd;
    auto seed = rd();
    std::cerr << "Running stress test with " << NUMBER_OF_ITERATIONS <<
        " iterations, maximal absolute value of keys " << MAX_ABS_VALUE <<
        ", seed " << seed << "\n";
    std::mt19937 mt(seed);

    enum Operation {
        INSERT,
        ERASE,
        COUNT,
        LOWER_BOUND,
        UPPER_BOUND,
        SIZE,
        VALIDATE_SIZE,
        NUMBER_OF_OPERATIONS
    };

    std::uniform_int_distribution<int> randomOperation(0, NUMBER_OF_OPERATIONS);
    std::uniform_int_distribution<int> randomInt(-MAX_ABS_VALUE, MAX_ABS_VALUE);

    for (unsigned int i = 0; i < NUMBER_OF_ITERATIONS; ++i) {
        Operation operation = static_cast<Operation>(randomOperation(mt));
        switch (operation) {
            case INSERT:
                {
                    int key = randomInt(mt);
                    auto stlMultiSetInsertResult = stlMultiSet.insert(key);
                    auto splayTreeInsertResult =
                        splayTreeMultiSet.insert(key);
                    EXPECT_EQ(
                        std::distance(
                            stlMultiSet.begin(),
                            stlMultiSetInsertResult),
                        std::distance(
                            splayTreeMultiSet.begin(),
                            splayTreeInsertResult));
                }
                break;
            case ERASE:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(stlMultiSet.erase(key), splayTreeMultiSet.erase(key));
                }
                break;
            case COUNT:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(stlMultiSet.count(key), splayTreeMultiSet.count(key));
                }
                break;
            case LOWER_BOUND:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(
                        std::distance(
                            stlMultiSet.begin(),
                            stlMultiSet.lower_bound(key)),
                        std::distance(
                            splayTreeMultiSet.begin(),
                            splayTreeMultiSet.lower_bound(key)));
                }
                break;
            case UPPER_BOUND:
                {
                    int key = randomInt(mt);
                    EXPECT_EQ(
                        std::distance(
                            stlMultiSet.begin(),
                            stlMultiSet.upper_bound(key)),
                        std::distance(
                            splayTreeMultiSet.begin(),
                            splayTreeMultiSet.upper_bound(key)));
                }
                break;
            case SIZE:
                EXPECT_EQ(stlMultiSet.size(), splayTreeMultiSet.size());
                break;
            case VALIDATE_SIZE:
                EXPECT_EQ(
                    splayTreeMultiSet.size(),
                    std::distance(
                        splayTreeMultiSet.begin(),
                        splayTreeMultiSet.end()));
                break;
        }
    }
}
