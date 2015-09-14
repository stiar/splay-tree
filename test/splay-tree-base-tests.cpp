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

TEST(splay_tree_test, erase) {
    SplayTree<int, int, int, std::less<int>> set;
    set.insertUnique(1);
    set.insertUnique(3);
    set.insertUnique(5);

    set.erase(3);
    EXPECT_EQ(1, *set.begin());
    EXPECT_EQ(5, *set.rbegin());
    set.erase(1);
    EXPECT_EQ(1, set.size());
    EXPECT_EQ(5, *set.begin());
}

TEST(splay_tree_test, customComparator) {
    SplayTree<int, int, int, std::greater<int>> set;
    set.insertUnique(1);
    set.insertUnique(2);

    EXPECT_EQ(2, *set.begin());
    EXPECT_EQ(1, *set.rbegin());
}

TEST(splay_tree_test, construction) {
    SplayTree<int, int, int, std::less<int>> set1;
    set1.insertUnique(3);
    set1.insertUnique(4);
    set1.insertUnique(1);
    EXPECT_EQ(1, set1.count(1));
    EXPECT_EQ(0, set1.count(2));
    EXPECT_EQ(1, set1.count(3));
    EXPECT_EQ(1, set1.count(4));

    SplayTree<int, int, int, std::less<int>> set2(set1);
    EXPECT_EQ(1, set2.count(1));
    EXPECT_EQ(0, set2.count(2));
    EXPECT_EQ(1, set2.count(3));
    EXPECT_EQ(1, set2.count(4));
}
