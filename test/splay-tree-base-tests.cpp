#include "gtest/gtest.h"
#include "splay-tree/splay-tree.h"

using namespace splay_tree;

TEST(splay_tree_test, insertUnique) {
    SplayTree<int, int, int, std::less<int>> set;
    auto val1 = set.insertUnique(1);
    EXPECT_EQ(1, set.count(1));
    auto val1Copy = set.insertUnique(1);
    EXPECT_EQ(val1.first, val1Copy.first);
    EXPECT_TRUE(val1.second);
    EXPECT_FALSE(val1Copy.second);

    EXPECT_EQ(1, set.count(1));
    EXPECT_EQ(0, set.count(2));
    set.insertUnique(2);
    EXPECT_EQ(1, set.count(1));
    EXPECT_EQ(1, set.count(2));
}
