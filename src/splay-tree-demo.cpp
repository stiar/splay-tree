#include "splay-tree/splay-tree.h"

#include <iostream>
#include <string>
#include <sstream>

int main() {
    splay_tree::SplayTree<int> set;
    std::string command;
    while (getline(std::cin, command)) {
        std::stringstream stream(command);
        std::string commandName;
        stream >> commandName;

        if (commandName == "insert") {
            int value;
            stream >> value;
            set.insert(value);
        }
        if (commandName == "count") {
            int value;
            stream >> value;
            std::cout << set.count(value) << "\n";
        }
    }

    return 0;
}
