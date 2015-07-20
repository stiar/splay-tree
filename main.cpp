#include "splay-tree.h"

#include <iostream>
#include <string>
#include <sstream>
#include <set>

int main() {
    std::string s;
    splay_tree::SplayTree<int> splayTree;
    std::set<int> set;
    while (getline(std::cin, s)) {
        std::stringstream stream(s);
        std::string command;
        stream >> command;
        if (command == "count") {
            assert(splayTree.size() == set.size());
        }
        if (command == "insert") {
            int value;
            stream >> value;
            splayTree.insert(value);
            set.insert(value);
        }
        if (command == "check") {
            int value;
            stream >> value;
            assert(splayTree.count(value) == set.count(value));
        }
    }

    return 0;
}
