#ifndef SPLAY_TREE_KEY_OF_VALUE_H_
#define SPLAY_TREE_KEY_OF_VALUE_H_

#include <utility>

namespace splay_tree {

struct Identity {
    template<typename T>
    constexpr auto operator()(T&& arg) const noexcept ->
            decltype(std::forward<T>(arg)) {
        return std::forward<T>(arg);
    }
};

} // namespace splay_tree

#endif // SPLAY_TREE_KEY_OF_VALUE_H_
