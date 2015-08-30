#include "splay-tree.h"

#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <chrono>

template <typename Set>
class SetBenchmarkHolder {
public:
    explicit SetBenchmarkHolder(const std::string& name) :
        name_(name),
        timeSpend_(0) {
    }

    ~SetBenchmarkHolder() {
        std::cerr << "Total time for " << name_ << " : " << timeSpend_ << "\n";
    }

    void runCommand(const std::string& command) {
        std::stringstream stream(command);
        std::string commandName;
        stream >> commandName;

        if (commandName == "insert") {
            int value;
            stream >> value;
            auto startTime = std::chrono::high_resolution_clock::now();
            set_.insert(value);
            timeSpend_ += std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - startTime).count();
        }
        if (commandName == "check") {
            int value;
            auto startTime = std::chrono::high_resolution_clock::now();
            set_.count(value);
            timeSpend_ += std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - startTime).count();
        }
    }

private:
    const std::string name_;

    Set set_;
    double timeSpend_;
};

int main() {
    std::string s;
    SetBenchmarkHolder<splay_tree::SplayTree<int>> splayTreeBenchmark(
        "splay tree");
    SetBenchmarkHolder<std::set<int>> setBenchmark("set");
    while (getline(std::cin, s)) {
        splayTreeBenchmark.runCommand(s);
        setBenchmark.runCommand(s);
    }

    return 0;
}
