#ifndef SPLAY_TREE_SET_H_
#define SPLAY_TREE_SET_H_

#include "splay-tree.h"
#include "key-of-value.h"

#include <initializer_list>

namespace splay_tree {

template <
    typename Key,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<Key>
>
class set {
private:
    typedef typename Allocator::template rebind<Key>::other KeyAllocator;

    typedef SplayTree<Key, Key, Identity, Compare, KeyAllocator> SetImpl;

public:
    typedef Compare key_compare;
    typedef Compare value_compare;
    typedef Allocator allocator_type;

    typedef typename SetImpl::key_type key_type;
    typedef typename SetImpl::value_type value_type;
    typedef typename SetImpl::pointer pointer;
    typedef typename SetImpl::const_pointer const_pointer;
    typedef typename SetImpl::reference reference;
    typedef typename SetImpl::const_reference const_reference;
    typedef typename SetImpl::size_type size_type;
    typedef typename SetImpl::difference_type difference_type;

    typedef typename SetImpl::const_iterator iterator;
    typedef typename SetImpl::const_iterator const_iterator;
    typedef typename SetImpl::reverse_iterator reverse_iterator;
    typedef typename SetImpl::const_reverse_iterator const_reverse_iterator;

    set() :
        setImpl_() {
    }

    explicit set(const Compare& comparator, const Allocator& allocator = Allocator()) :
        setImpl_(comparator, KeyAllocator(allocator)) {
    }

    template<typename InputIterator>
    set(InputIterator first, InputIterator last) :
            setImpl_() {
        setImpl_.insertUnique(first, last);
    }

    template<typename InputIterator>
    set(
        InputIterator first,
        InputIterator last,
        const Compare& comparator,
        const Allocator& allocator = Allocator()) :
            setImpl_(comparator, KeyAllocator(allocator)) {
        setImpl_.insertUnique(first, last);
    }

    set(
        std::initializer_list<Key> initializerList,
        const Compare& comparator = Compare(),
        const Allocator& allocator = Allocator()) :
            setImpl_(comparator, KeyAllocator(allocator)) {
        setImpl_.insertUnique(initializerList.begin(), initializerList.end());
    }

    set(const set& rhs) = default;

    // TODO Make noexcept.
    set(set&& rhs) = default;

    set& operator=(const set& rhs) {
        set temp = rhs;
        swap(temp);
        return *this;
    }

    set& operator=(set&& rhs) {
        clear();
        swap(rhs);
        return *this;
    }

    set& operator=(std::initializer_list<Key> initializerList) {
        clear();
        insert(initializerList.begin(), initializerList.end());
        return *this;
    }

    iterator begin() const noexcept {
        return setImpl_.begin();
    }

    iterator cbegin() const noexcept {
        return setImpl_.cbegin();
    }

    iterator end() const noexcept {
        return setImpl_.end();
    }

    iterator cend() const noexcept {
        return setImpl_.cend();
    }

    iterator rbegin() const noexcept {
        return setImpl_.rbegin();
    }

    iterator crbegin() const noexcept {
        return setImpl_.crbegin();
    }

    iterator rend() const noexcept {
        return setImpl_.rend();
    }

    iterator crend() const noexcept {
        return setImpl_.crend();
    }

    bool empty() const noexcept {
        return setImpl_.empty();
    }

    size_type size() const noexcept {
        return setImpl_.size();
    }

    size_type max_size() const noexcept {
        return setImpl_.max_size();
    }

    void swap(set& rhs) {
        setImpl_.swap(rhs.setImpl_);
    }

    void clear() noexcept {
        setImpl_.clear();
    }

    // Insert/erase operations.
    std::pair<iterator, bool> insert(const Key& key) {
        return setImpl_.insertUnique(key);
    }

    std::pair<iterator, bool> insert(Key&& key) {
        return setImpl_.insertUnique(std::move(key));
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        setImpl_.insertUnique(first, last);
    }

    void insert(std::initializer_list<Key> initializerList) {
        insert(initializerList.begin(), initializerList.end());
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return setImpl_.emplaceUnique(std::forward<Args>(args)...);
    }

    iterator erase(const_iterator position) {
        return setImpl_.erase(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        return setImpl_.erase(first, last);
    }

    size_type erase(const Key& key) {
        return setImpl_.erase(key);
    }

    // Find operations.
    // Does a splay if the key is found.
    iterator find(const Key& key) {
        return setImpl_.find(key);
    }

    const_iterator find(const Key& key) const {
        return setImpl_.find(key);
    }

    // Does a splay if the key is found.
    size_type count(const Key& key) {
        return find(key) == end() ? 0 : 1;
    }

    size_type count(const Key& key) const {
        return find(key) == end() ? 0 : 1;
    }

    iterator lower_bound(const Key& key) {
        return setImpl_.lower_bound(key);
    }

    const_iterator lower_bound(const Key& key) const {
        return setImpl_.lower_bound(key);
    }

    iterator upper_bound(const Key& key) {
        return setImpl_.upper_bound(key);
    }

    iterator upper_bound(const Key& key) const {
        return setImpl_.upper_bound(key);
    }

    std::pair<iterator, iterator> equal_range(const Key& key) {
        return setImpl_.equal_range(key);
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
        return setImpl_.equal_range(key);
    }

private:

    SetImpl setImpl_;
};

template<typename Key, typename Compare, typename Allocator>
inline bool operator==(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ == rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator!=(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ != rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator<(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ < rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator>=(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ >= rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator>(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ > rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline bool operator<=(
        const set<Key, Compare, Allocator>& lhs,
        const set<Key, Compare, Allocator>& rhs) {
    return lhs.setImpl_ <= rhs.setImpl_;
}

template<typename Key, typename Compare, typename Allocator>
inline void swap(
        set<Key, Compare, Allocator>& lhs,
        set<Key, Compare, Allocator>& rhs) {
    lhs.swap(rhs);
}

} // namespace splay_tree

#endif // SPLAY_TREE_SET_H_
