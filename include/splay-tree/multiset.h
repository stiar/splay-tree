#ifndef SPLAY_TREE_MULTISET_H_
#define SPLAY_TREE_MULTISET_H_

#include "splay-tree.h"
#include "key-of-value.h"

#include <initializer_list>

namespace splay_tree {

template <
    typename Key,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<Key>
>
class multiset {
private:
    typedef typename Allocator::template rebind<Key>::other KeyAllocator;

    typedef SplayTree<Key, Key, Identity, Compare, KeyAllocator> MultisetImpl;

public:
    typedef Compare key_compare;
    typedef Compare value_compare;
    typedef Allocator allocator_type;

    typedef typename MultisetImpl::key_type key_type;
    typedef typename MultisetImpl::value_type value_type;
    typedef typename MultisetImpl::pointer pointer;
    typedef typename MultisetImpl::const_pointer const_pointer;
    typedef typename MultisetImpl::reference reference;
    typedef typename MultisetImpl::const_reference const_reference;
    typedef typename MultisetImpl::size_type size_type;
    typedef typename MultisetImpl::difference_type difference_type;

    typedef typename MultisetImpl::const_iterator iterator;
    typedef typename MultisetImpl::const_iterator const_iterator;
    typedef typename MultisetImpl::reverse_iterator reverse_iterator;
    typedef typename MultisetImpl::const_reverse_iterator const_reverse_iterator;

    multiset() :
        multisetImpl_() {
    }

    explicit multiset(
            const Compare& comparator,
            const Allocator& allocator = Allocator()) :
        multisetImpl_(comparator, KeyAllocator(allocator)) {
    }

    template<typename InputIterator>
    multiset(InputIterator first, InputIterator last) :
            multisetImpl_() {
        multisetImpl_.insertEqual(first, last);
    }

    template<typename InputIterator>
    multiset(
        InputIterator first,
        InputIterator last,
        const Compare& comparator,
        const Allocator& allocator = Allocator()) :
            multisetImpl_(comparator, KeyAllocator(allocator)) {
        multisetImpl_.insertEqual(first, last);
    }

    multiset(
        std::initializer_list<Key> initializerList,
        const Compare& comparator = Compare(),
        const Allocator& allocator = Allocator()) :
            multisetImpl_(comparator, KeyAllocator(allocator)) {
        multisetImpl_.insertEqual(initializerList.begin(), initializerList.end());
    }

    multiset(const multiset& rhs) = default;

    // TODO Make noexcept.
    multiset(multiset&& rhs) = default;

    multiset& operator=(const multiset& rhs) {
        multiset temp = rhs;
        swap(temp);
        return *this;
    }

    multiset& operator=(multiset&& rhs) {
        clear();
        swap(rhs);
        return *this;
    }

    multiset& operator=(std::initializer_list<Key> initializerList) {
        clear();
        insert(initializerList.begin(), initializerList.end());
        return *this;
    }

    iterator begin() const noexcept {
        return multisetImpl_.begin();
    }

    iterator cbegin() const noexcept {
        return multisetImpl_.cbegin();
    }

    iterator end() const noexcept {
        return multisetImpl_.end();
    }

    iterator cend() const noexcept {
        return multisetImpl_.cend();
    }

    iterator rbegin() const noexcept {
        return multisetImpl_.rbegin();
    }

    iterator crbegin() const noexcept {
        return multisetImpl_.crbegin();
    }

    iterator rend() const noexcept {
        return multisetImpl_.rend();
    }

    iterator crend() const noexcept {
        return multisetImpl_.crend();
    }

    bool empty() const noexcept {
        return multisetImpl_.empty();
    }

    size_type size() const noexcept {
        return multisetImpl_.size();
    }

    size_type max_size() const noexcept {
        return multisetImpl_.max_size();
    }

    void swap(multiset& rhs) {
        multisetImpl_.swap(rhs.multisetImpl_);
    }

    void clear() noexcept {
        multisetImpl_.clear();
    }

    // Insert/erase operations.
    iterator insert(const Key& key) {
        return multisetImpl_.insertEqual(key);
    }

    iterator insert(Key&& key) {
        return multisetImpl_.insertEqual(std::move(key));
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        multisetImpl_.insertEqual(first, last);
    }

    void insert(std::initializer_list<Key> initializerList) {
        insert(initializerList.begin(), initializerList.end());
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return multisetImpl_.emplaceUnique(std::forward<Args>(args)...);
    }

    iterator erase(const_iterator position) {
        return multisetImpl_.erase(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        return multisetImpl_.erase(first, last);
    }

    size_type erase(const Key& key) {
        return multisetImpl_.erase(key);
    }

    // Find operations.
    // Does a splay if the key is found.
    iterator find(const Key& key) {
        return multisetImpl_.find(key);
    }

    const_iterator find(const Key& key) const {
        return multisetImpl_.find(key);
    }

    size_type count(const Key& key) {
        return multisetImpl_.count(key);
    }

    size_type count(const Key& key) const {
        return multisetImpl_.count(key);
    }

    iterator lower_bound(const Key& key) {
        return multisetImpl_.lower_bound(key);
    }

    const_iterator lower_bound(const Key& key) const {
        return multisetImpl_.lower_bound(key);
    }

    iterator upper_bound(const Key& key) {
        return multisetImpl_.upper_bound(key);
    }

    iterator upper_bound(const Key& key) const {
        return multisetImpl_.upper_bound(key);
    }

    std::pair<iterator, iterator> equal_range(const Key& key) {
        return multisetImpl_.equal_range(key);
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
        return multisetImpl_.equal_range(key);
    }

private:

    MultisetImpl multisetImpl_;
};

template<typename Key, typename Compare, typename Allocator>
inline bool operator==(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ == rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator!=(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ != rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator<(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ < rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator>=(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ >= rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator>(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ > rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator<=(
        const multiset<Key, Compare, Allocator>& lhs,
        const multiset<Key, Compare, Allocator>& rhs) {
    return lhs.multisetImpl_ <= rhs.multisetImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline void swap(
        multiset<Key, Compare, Allocator>& lhs,
        multiset<Key, Compare, Allocator>& rhs) {
    lhs.swap(rhs);
}

} // namespace splay_tree

#endif // SPLAY_TREE_SET_H_
