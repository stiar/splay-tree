#include "splay-tree.h"

#include <iostream>

int main() {
    splay_tree::SplayTree<int> a;
    a.insert(2);
    a.insert(1);
    a.insert(4);
    a.insert(3);
    a.insert(5);
    std::cout << a.count(1) << "\n";
    std::cout << a.count(2) << "\n";
    std::cout << a.count(3) << "\n";
    std::cout << a.count(4) << "\n";

    std::cout << "\n";

    for (const auto& x : a) {
        std::cout << x << "\n";
    }

    std::cout << "\n";
    auto it = a.begin();
    std::cout << *it << "\n";
    ++it;
    std::cout << *it << "\n";
    ++it;
    std::cout << *it << "\n";
    --it;
    std::cout << *it << "\n";
    ++it;
    ++it;
    ++it;
    ++it;
    --it;
    std::cout << *it << "\n";
}
