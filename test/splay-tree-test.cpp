#include "gtest/gtest.h"
#include "splay-tree/splay-tree.h"

using namespace splay_tree;

TEST(splay_tree_test, basicMethodsWork) {
    SplayTree<int> set;
    auto val1 = set.insert(1);
    auto val1Copy = set.insert(1);
    EXPECT_EQ(val1.first, val1Copy.first);
    EXPECT_TRUE(val1.second);
    EXPECT_FALSE(val1Copy.second);
}
