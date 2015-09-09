#include "gtest/gtest.h"
#include "splay-tree/splay-tree.h"

#include <string>

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

TEST(splay_tree_test, emplaceUnique) {
    SplayTree<std::string, std::string, std::string, std::less<std::string>> set;
    const std::string a("a");
    const std::string b("b");
    auto val1 = set.emplaceUnique(a.c_str());
    EXPECT_EQ(1, set.count(a));
    auto val1Copy = set.insertUnique(a);
    EXPECT_EQ(val1.first, val1Copy.first);
    EXPECT_TRUE(val1.second);
    EXPECT_FALSE(val1Copy.second);

    EXPECT_EQ(1, set.count(a));
    EXPECT_EQ(0, set.count(b));
    set.emplaceUnique(b);
    EXPECT_EQ(1, set.count(a));
    EXPECT_EQ(1, set.count(b));
}
