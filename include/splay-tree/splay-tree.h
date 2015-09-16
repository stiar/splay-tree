#ifndef SPLAY_TREE_SPLAY_TREE_H_
#define SPLAY_TREE_SPLAY_TREE_H_

#include <iterator>
#include <type_traits>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <memory>

namespace splay_tree {

template <
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<Value>
>
class SplayTree {
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Allocator allocator_type;

private:
    // TODO Split this struct into a non-template base class and a derived class
    //      with a field containing the value.
    struct SplayTreeNode {
        template<typename... Args>
        SplayTreeNode(Args&&... args) :
            value(std::forward<Args>(args)...) {
        }

        value_type value;
        SplayTreeNode* parent{nullptr};
        SplayTreeNode* leftChild{nullptr};
        SplayTreeNode* rightChild{nullptr};
    };

    typedef
        typename Allocator::template rebind<SplayTreeNode>::other
        NodeAllocator;

    template<typename... Args>
    SplayTreeNode* createNode(Args&&... args) {
        auto newNode = nodeAllocator_.allocate(1);
        try {
            std::allocator_traits<NodeAllocator>::construct(
                nodeAllocator_,
                newNode,
                std::forward<Args>(args)...);
        } catch (...) {
            nodeAllocator_.deallocate(newNode, 1);
            throw;
        }

        return newNode;
    }

    void destroyNode(SplayTreeNode* node) {
        nodeAllocator_.destroy(node);
        nodeAllocator_.deallocate(node, 1);
    }

    // Non-recursive implementation, because the height of the splay tree is not
    // guaranteed to be O(log(n)) in the worst case.
    void destroyTree(SplayTreeNode* root) noexcept {
        auto currentNode = root;
        while (currentNode) {
            if (currentNode->leftChild) {
                currentNode = currentNode->leftChild;
                continue;
            }
            if (currentNode->rightChild) {
                currentNode = currentNode->rightChild;
                continue;
            }
            auto parent = currentNode->parent;
            if (parent) {
                if (parent->leftChild == currentNode) {
                    parent->leftChild = nullptr;
                } else {
                    parent->rightChild = nullptr;
                }
            }
            destroyNode(currentNode);
            currentNode = parent;
        }
    }

    SplayTreeNode* copyTree(SplayTreeNode* root) {
        SplayTreeNode* rootCopy;
        try {
            rootCopy = createNode(root->value);
            if (root->leftChild) {
                rootCopy->leftChild = copyTree(root->leftChild);
                rootCopy->leftChild->parent = rootCopy;
            }
            if (root->rightChild) {
                rootCopy->rightChild = copyTree(root->rightChild);
                rootCopy->rightChild->parent = rootCopy;
            }
            return rootCopy;
        } catch (...) {
            destroyTree(rootCopy);
            throw;
        }
    }

    SplayTreeNode* getLeftMostNode() {
        if (!root_) {
            return nullptr;
        }
        auto currentNode = root_;
        while (currentNode->leftChild) {
            currentNode = currentNode->leftChild;
        }
        return currentNode;
    }

    SplayTreeNode* getRightMostNode() {
        if (!root_) {
            return nullptr;
        }
        auto currentNode = root_;
        while (currentNode->rightChild) {
            currentNode = currentNode->rightChild;
        }
        return currentNode;
    }

    template<bool IsConstIterator>
    class SplayTreeIterator {
    public:
        typedef key_type value_type;
        typedef
            typename std::conditional<
                IsConstIterator,
                const key_type&,
                key_type&>::type
            reference; 
        typedef
            typename std::conditional<
                IsConstIterator,
                const key_type*,
                key_type*>::type
            pointer;
        typedef ptrdiff_t difference_type;
        typedef std::bidirectional_iterator_tag iterator_category;

        SplayTreeIterator(SplayTreeNode* node, SplayTreeNode* root) :
            node_(node),
            root_(root) {
        }

        SplayTreeIterator(const SplayTreeIterator<false>& rhs) :
            node_(rhs.node_),
            root_(rhs.root_) {
        }

        bool operator==(const SplayTreeIterator& rhs) const {
            return node_ == rhs.node_;
        }

        bool operator!=(const SplayTreeIterator& rhs) const {
            return !(*this == rhs);
        }

        SplayTreeIterator& operator++();

        SplayTreeIterator operator++(int) {
            const SplayTreeIterator old(*this);
            ++(*this);
            return old;
        }

        SplayTreeIterator& operator--();

        SplayTreeIterator operator--(int) {
            const SplayTreeIterator old(*this);
            ++(*this);
            return old;
        }

        reference operator*() const {
            return node_->value;
        }

        pointer operator->() const {
            return &node_->value;
        }

        friend class SplayTreeIterator<true>;
        friend class SplayTree;

    private:
        SplayTreeNode* node_;
        // TODO Remove the root_ field. This can be done if the end() iterator
        //      holds an auxiliary node containing special info, not nullptr.
        SplayTreeNode* root_;
    };

public:
    typedef SplayTreeIterator<false> iterator;
    typedef SplayTreeIterator<true> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Construction/destruction.
    SplayTree() {
    }

    explicit SplayTree(
        const Compare& comparator,
        const Allocator& allocator = Allocator()) :
            comparator_(comparator),
            nodeAllocator_(allocator) {
    }

    SplayTree(const SplayTree& rhs) :
        root_(copyTree(rhs.root_)),
        leftMostNode_(getLeftMostNode()),
        rightMostNode_(getRightMostNode()),
        numberOfNodes_(rhs.numberOfNodes_),
        comparator_(rhs.comparator_),
        nodeAllocator_(rhs.nodeAllocator_) {
    }

private:
    SplayTree(
        SplayTreeNode* root,
        const Compare& comparator,
        const NodeAllocator& nodeAllocator) :
            root_(root),
            leftMostNode_(getLeftMostNode()),
            rightMostNode_(getRightMostNode()),
            numberOfNodes_(std::distance(cbegin(), cend())),
            comparator_(comparator),
            nodeAllocator_(nodeAllocator) {
    }

public:

    // TODO Make noexcept.
    // TODO What if default constuctors of Compare and Allocator are time
    //      consuming?
    SplayTree(SplayTree&& rhs) :
            SplayTree() {
        swap(rhs);
    }

    SplayTree& operator=(const SplayTree& rhs) {
        SplayTree temp(rhs);
        swap(temp);
        return *this;
    }

    // TODO Make noexcept.
    SplayTree& operator=(SplayTree&& rhs) {
        clear();
        swap(rhs);
        return *this;
    }

    ~SplayTree() noexcept {
        destroyTree(root_);
    }

    iterator begin() noexcept {
        return iterator(leftMostNode_, root_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(leftMostNode_, root_);
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }
    
    iterator end() noexcept {
        return {nullptr, root_};
    }

    const_iterator end() const noexcept {
        return {nullptr, root_};
    }

    const_iterator cend() const noexcept {
        return end();
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    bool empty() const noexcept {
        return !root_;
    }

    size_type size() const noexcept {
        return numberOfNodes_;
    }

    size_type max_size() const noexcept {
        return nodeAllocator_.max_size();
    }

    // TODO Make noexcept.
    void swap(SplayTree& rhs) {
        using std::swap;
        swap(root_, rhs.root_);
        swap(leftMostNode_, rhs.leftMostNode_);
        swap(rightMostNode_, rhs.rightMostNode_);
        swap(numberOfNodes_, rhs.numberOfNodes_);
        swap(comparator_, rhs.comparator_);
        // TODO Check if everything is ok with swapping allocators.
        swap(nodeAllocator_, rhs.nodeAllocator_);
    }

    void clear() noexcept {
        destroyTree(root_);
        root_ = nullptr;
        leftMostNode_ = nullptr;
        rightMostNode_ = nullptr;
        numberOfNodes_ = 0;
    }

    // Insert/erase operations.
    template<typename Arg>
    std::pair<iterator, bool> insertUnique(Arg&& value);

    template<typename InputIterator>
    void insertUnique(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insertUnique(*first);
        }
    }

    template<typename Arg>
    iterator insertEqual(Arg&& value);

    template<typename InputIterator>
    void insertEqual(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insertEqual(*first);
        }
    }

    template<typename... Args>
    std::pair<iterator, bool> emplaceUnique(Args&&... args);

    template<typename... Args>
    iterator emplaceEqual(Args&&... args);

    iterator erase(const_iterator position) {
        SplayTreeNode* node = position.node_;
        iterator result(node, root_);
        ++result;
        innerErase(node);
        return result;
    }

    iterator erase(iterator position) {
        auto result = position;
        ++result;
        innerErase(position.node_);
        return result;
    }

    // TODO Remove code dupcliation.
    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return first;
    }

    iterator erase(iterator first, iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return first;
    }

    size_type erase(const Key& key) {
        auto range = equal_range(key);
        const size_type oldSize = size();
        erase(range.first, range.second);
        return oldSize - size();
    }

    // Split/merge operations.
    // TODO Implement split.
    SplayTree split(iterator position) {
        return innerSplit(position.node_);
    }

    SplayTree split(const_iterator position) {
        return innerSplit(position.node_);
    }

    SplayTree split(const Key& key) {
        auto node = innerLowerBound(key);
        if (!node || KeyOfValue()(node->value) != key) {
            throw std::runtime_error(
                "Requested split with a key that is not present in the tree.");
        }
        return innerSplit(node);
    }

    // Merge assumes that the tree given as a parameter has its minimum key
    // greater than the maximum key in the current tree (or greater or equal in
    // case of mergeEqual).
    void mergeUnique(SplayTree&& rhs);

    void mergeUnique(const SplayTree& rhs) {
        auto temp = rhs;
        mergeUnique(std::move(temp));
    }

    void mergeEqual(SplayTree&& rhs);

    void mergeEqual(const SplayTree& rhs) {
        auto temp = rhs;
        mergeEqual(std::move(temp));
    }

    // Find operations.
    iterator find(const Key& key) {
        auto node = innerFind(key);
        if (node) {
            node = splay(node);
        }
        return {node, root_};
    }

    const_iterator find(const Key& key) const {
        return {innerFind(key), root_};
    }

    size_type count(const Key& key);

    size_type count(const Key& key) const;

    iterator lower_bound(const Key& key) {
        return {innerLowerBound(key), root_};
    }

    const_iterator lower_bound(const Key& key) const {
        return {innerLowerBound(key), root_};
    }

    iterator upper_bound(const Key& key) {
        return {innerUpperBound(key), root_};
    }

    const_iterator upper_bound(const Key& key) const {
        return {innerUpperBound(key), root_};
    }

    std::pair<iterator, iterator> equal_range(const Key& key);

    std::pair<const_iterator, const_iterator>
    equal_range(const Key& key) const;

private:
    // Splay and rotations.
    // TODO When SplayTreeNode struct is appropriately split, following
    //      methods will no longer depend on keys, values and template
    //      parameters and should be moved from here.
    SplayTreeNode* splay(SplayTreeNode* node);

    SplayTreeNode* zigStep(SplayTreeNode* node);

    SplayTreeNode* zigZigStep(SplayTreeNode* node);

    SplayTreeNode* zigZagStep(SplayTreeNode* node);

    SplayTreeNode* leftRotation(SplayTreeNode* node);

    SplayTreeNode* rightRotation(SplayTreeNode* node);

    // Helpers.
    SplayTreeNode* innerLowerBound(const key_type& key) const;

    SplayTreeNode* innerUpperBound(const key_type& key) const;

    SplayTreeNode* findPlaceToInsertUnique(const key_type& key) const;

    SplayTreeNode* findPlaceToInsertEqual(const key_type& key) const;

    template<typename Arg>
    SplayTreeNode* innerInsert(
        Arg&& value,
        SplayTreeNode* placeToInsert,
        SplayTreeNode* newNode = nullptr);

    void innerErase(SplayTreeNode* node);

    SplayTree innerSplit(SplayTreeNode* node);

    void innerMerge(SplayTree&& rhs);

    SplayTreeNode* innerFind(const key_type& key) const;

    bool keysAreEqual(const Key& lhs, const Key& rhs) const {
        return !comparator_(lhs, rhs) && !comparator_(rhs, lhs);
    }

    SplayTreeNode* root_{nullptr};
    SplayTreeNode* leftMostNode_{nullptr};
    SplayTreeNode* rightMostNode_{nullptr};
    size_type numberOfNodes_{0};
    Compare comparator_;
    NodeAllocator nodeAllocator_;
};

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template <bool IsConstIterator>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::template SplayTreeIterator<IsConstIterator>&
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeIterator<IsConstIterator>::operator++() {
    if (node_) {
        SplayTreeNode* currentNode = node_;

        if (currentNode->rightChild) {
            currentNode = currentNode->rightChild;
            while (currentNode->leftChild) {
                currentNode = currentNode->leftChild;
            }
            node_ = currentNode;
        } else {
            while (currentNode->parent &&
                    currentNode->parent->rightChild == currentNode) {
                currentNode = currentNode->parent;
            }
            node_ = currentNode->parent;
        }
    }

    return *this;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template <bool IsConstIterator>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::template SplayTreeIterator<IsConstIterator>&
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeIterator<IsConstIterator>::operator--() {
    if (!node_) {
        SplayTreeNode* currentNode = root_;
        while (currentNode && currentNode->rightChild) {
            currentNode = currentNode->rightChild;
        }
        node_ = currentNode;
    } else {
        SplayTreeNode* currentNode = node_;

        if (currentNode->leftChild) {
            currentNode = currentNode->leftChild;
            while (currentNode->rightChild) {
                currentNode = currentNode->rightChild;
            }
            node_ = currentNode;
        } else {
            while (currentNode->parent &&
                    currentNode->parent->leftChild == currentNode) {
                currentNode = currentNode->parent;
            }
            node_ = currentNode->parent;
        }
    }
    return *this;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator==(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return lhs.size() == rhs.size() &&
        std::equal(lhs.cbegin, lhs.cend(), rhs.cbegin());
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator!=(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return !(lhs == rhs);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator<(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return std::lexicographical_compare(
        lhs.cbegin,
        lhs.cend(),
        rhs.cbegin(),
        rhs.cend());
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator>(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return rhs < lhs;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator>=(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return !(lhs < rhs);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline bool operator<=(
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        const SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    return rhs >= lhs;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
inline void swap(
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& lhs,
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>& rhs) {
    lhs.swap(rhs);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template<typename Arg>
std::pair<typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator, bool>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::insertUnique(Arg&& value) {
    Key key = KeyOfValue()(value);
    SplayTreeNode* placeToInsert = findPlaceToInsertUnique(key);

    if (placeToInsert && keysAreEqual(KeyOfValue()(placeToInsert->value), key)) {
        return {iterator(placeToInsert, root_), false};
    } else {
        auto newNode = innerInsert(std::forward<Arg>(value), placeToInsert);
        return {iterator(newNode, root_), true};
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template<typename Arg>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::insertEqual(Arg&& value) {
    const auto& key = KeyOfValue()(value);
    SplayTreeNode* placeToInsert = findPlaceToInsertEqual(key);

    auto newNode = innerInsert(std::forward<Arg>(value), placeToInsert);
    return iterator(newNode, root_);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template<typename... Args>
std::pair<
    typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator,
    bool
> SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::emplaceUnique(Args&&... args) {
    SplayTreeNode* newNode = createNode(std::forward<Args>(args)...);
    try {
        const auto& key = KeyOfValue()(newNode->value);
        SplayTreeNode* placeToInsert = findPlaceToInsertUnique(key);

        if (placeToInsert && keysAreEqual(KeyOfValue()(placeToInsert->value), key)) {
            destroyNode(newNode);
            return {iterator(placeToInsert, root_), false};
        } else {
            newNode = innerInsert(
                newNode->value,
                placeToInsert,
                newNode);
            return {iterator(newNode, root_), true};
        }
    } catch (...) {
        destroyNode(newNode);
        throw;
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template<typename... Args>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::emplaceEqual(Args&&... args) {
    SplayTreeNode* newNode = createNode(std::forward<Args>(args)...);
    try {
        const auto& key = KeyOfValue()(newNode->value);
        SplayTreeNode* placeToInsert = findPlaceToInsertEqual(key);

        auto newNode = innerInsert(
            newNode->value,
            placeToInsert,
            newNode);
        return iterator(newNode, root_);
    } catch (...) {
        destroyNode(newNode);
        throw;
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
void SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::mergeUnique(
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>&& rhs) {
    if (!comparator_(
            KeyOfValue()(rightMostNode_->value),
            KeyOfValue()(rhs.leftMostNode_->value))) {
        throw std::runtime_error(
            "Trying to merge two splay trees with no key separation property.");
    }
    innerMerge(std::move(rhs));
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
void SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::mergeEqual(
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>&& rhs) {
    if (comparator_(
            KeyOfValue()(rhs.leftMostNode_->value),
            KeyOfValue()(rightMostNode_->value))) {
        throw std::runtime_error(
            "Trying to merge two splay trees with no key separation property.");
    }
    innerMerge(std::move(rhs));
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::size_type
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::count(
        const Key& key) {
    auto range = equal_range(key);
    return std::distance(range.first, range.second);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::size_type
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::count(
        const Key& key) const {
    auto range = equal_range(key);
    return std::distance(range.first, range.second);
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
std::pair<
    typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator,
    typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator
>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::equal_range(
        const Key& key) {
    // TODO Implement more efficiently.
    return {lower_bound(key), upper_bound(key)};
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
std::pair<
    typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::const_iterator,
    typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::const_iterator
>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::equal_range(
        const Key& key) const {
    return {lower_bound(key), upper_bound(key)};
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::splay(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    while (node != root_) {
        if (node->parent == root_) {
            node = zigStep(node);
            assert(!node->parent);
            root_ = node;
        } else {
            SplayTreeNode* parent = node->parent;
            SplayTreeNode* grandParent = parent->parent;

            if ((grandParent->leftChild == parent) == (parent->leftChild == node)) {
                node = zigZigStep(node);
            } else {
                node = zigZagStep(node);
            }
            if (!node->parent) {
                root_ = node;
            }
        }
    }

    // TODO Or maybe maintain those nodes during rotations?
    leftMostNode_ = getLeftMostNode();
    rightMostNode_ = getRightMostNode();

    return root_;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::zigStep(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    if (node->parent->leftChild == node) {
        return rightRotation(node);
    } else {
        return leftRotation(node);
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::zigZigStep(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    SplayTreeNode* parent = node->parent;
    if (parent->leftChild == node) {
        parent = rightRotation(parent);
        node = parent->leftChild;
        return rightRotation(node);
    } else {
        parent = leftRotation(parent);
        node = parent->rightChild;
        return leftRotation(node);
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::zigZagStep(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    SplayTreeNode* parent = node->parent;
    if (parent->leftChild == node) {
        node = rightRotation(node);
        return leftRotation(node);
    } else {
        node = leftRotation(node);
        return rightRotation(node);
    }
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::leftRotation(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    SplayTreeNode* parent = node->parent;
    assert(parent);
    assert(parent->rightChild == node);
    SplayTreeNode* grandParent = parent->parent;

    parent->rightChild = node->leftChild;
    if (node->leftChild) {
        node->leftChild->parent = parent;
    }

    node->leftChild = parent;
    node->parent = grandParent;
    if (grandParent) {
        if (grandParent->leftChild == parent) {
            grandParent->leftChild = node;
        } else {
            grandParent->rightChild = node;
        }
    }
    parent->parent = node;

    return node;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::rightRotation(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    SplayTreeNode* parent = node->parent;
    assert(parent);
    assert(parent->leftChild == node);
    SplayTreeNode* grandParent = parent->parent;

    parent->leftChild = node->rightChild;
    if (node->rightChild) {
        node->rightChild->parent = parent;
    }

    node->rightChild = parent;
    node->parent = grandParent;
    if (grandParent) {
        if (grandParent->leftChild == parent) {
            grandParent->leftChild = node;
        } else {
            grandParent->rightChild = node;
        }
    }
    parent->parent = node;

    return node;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerLowerBound(
        const Key& key) const {
    SplayTreeNode* currentNode = root_;
    SplayTreeNode* lowerBound = nullptr;
    while (currentNode) {
        if (!comparator_(KeyOfValue()(currentNode->value), key)) {
            lowerBound = currentNode;
            currentNode = currentNode->leftChild;
        } else {
            currentNode = currentNode->rightChild;
        }
    }
    return lowerBound;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerUpperBound(
        const Key& key) const {
    SplayTreeNode* currentNode = root_;
    SplayTreeNode* upperBound = nullptr;
    while (currentNode) {
        if (comparator_(key, KeyOfValue()(currentNode->value))) {
            upperBound = currentNode;
            currentNode = currentNode->leftChild;
        } else {
            currentNode = currentNode->rightChild;
        }
    }
    return upperBound;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::findPlaceToInsertUnique(
        const Key& key) const {
    SplayTreeNode* currentNode = root_;
    while (currentNode) {
        const auto& currentNodeKey = KeyOfValue()(currentNode->value);
        if (keysAreEqual(currentNodeKey, key)) {
            return currentNode;
        }
        if (comparator_(key, currentNodeKey)) {
            if (!currentNode->leftChild) {
                return currentNode;
            } else {
                currentNode = currentNode->leftChild;
            }
        } else {
            if (!currentNode->rightChild) {
                return currentNode;
            } else {
                currentNode = currentNode->rightChild;
            }
        }
    }
    return currentNode;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::findPlaceToInsertEqual(
        const Key& key) const {
    SplayTreeNode* currentNode = root_;
    while (currentNode) {
        const auto& currentNodeKey = KeyOfValue()(currentNode->value);
        if (comparator_(key, currentNodeKey)) {
            if (!currentNode->leftChild) {
                return currentNode;
            } else {
                currentNode = currentNode->leftChild;
            }
        } else {
            if (!currentNode->rightChild) {
                return currentNode;
            } else {
                currentNode = currentNode->rightChild;
            }
        }
    }
    return currentNode;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
template<typename Arg>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerInsert(
        Arg&& value,
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
            placeToInsert,
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
            newNode) {
    if (!newNode) {
        newNode = createNode(std::forward<Arg>(value));
    }

    if (!placeToInsert) {
        // That means the tree is empty.
        root_ = newNode;
        leftMostNode_ = root_;
        rightMostNode_ = root_;
    } else {
        const auto& key = KeyOfValue()(newNode->value);
        newNode->parent = placeToInsert;
        if (comparator_(key, KeyOfValue()(placeToInsert->value))) {
            assert(!placeToInsert->leftChild);
            placeToInsert->leftChild = newNode;
            if (placeToInsert == leftMostNode_) {
                leftMostNode_ = newNode;
            }
        } else {
            assert(!placeToInsert->rightChild);
            placeToInsert->rightChild = newNode;
            if (placeToInsert == rightMostNode_) {
                rightMostNode_ = newNode;
            }
        }

        newNode = splay(newNode);
    }

    ++numberOfNodes_;

    return newNode;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
void SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerErase(
        typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    if (node->leftChild && node->rightChild) {
        SplayTreeNode* nodeToExchange = node->rightChild;
        while (nodeToExchange->leftChild) {
            nodeToExchange = nodeToExchange->leftChild;
        }

        nodeToExchange->leftChild = node->leftChild;
        node->leftChild->parent = nodeToExchange;
        node->leftChild = nullptr;

        auto nodeParent = node->parent;
        if (nodeToExchange->parent->leftChild == nodeToExchange) {
            nodeToExchange->parent->leftChild = node;
            node->parent = nodeToExchange->parent;
            std::swap(nodeToExchange->rightChild, node->rightChild);
            if (node->rightChild) {
                node->rightChild->parent = node;
            }
            nodeToExchange->rightChild->parent = nodeToExchange;
        } else {
            node->rightChild = nodeToExchange->rightChild;
            if (node->rightChild) {
                node->rightChild->parent = node;
            }
            nodeToExchange->rightChild = node;
            node->parent = nodeToExchange;
        }

        if (nodeParent) {
            if (nodeParent->leftChild == node) {
                nodeParent->leftChild = nodeToExchange;
            } else {
                nodeParent->rightChild = nodeToExchange;
            }
        } else {
            root_ = nodeToExchange;
        }
        nodeToExchange->parent = nodeParent;
    }

    if (!node->parent) {
        if (node->leftChild) {
            root_ = node->leftChild;
        } else {
            root_ = node->rightChild;
        }
        if (root_) {
            root_->parent = nullptr;
        }
    } else {
        auto parent = node->parent;
        auto child = node->leftChild ? node->leftChild : node->rightChild;
        if (parent->leftChild == node) {
            parent->leftChild = child;
        } else {
            parent->rightChild = child;
        }
        if (child) {
            child->parent = parent;
        }
        splay(parent);
    }

    // TODO Update leftMostNode and rightMostNode during erase.
    leftMostNode_ = getLeftMostNode();
    rightMostNode_ = getRightMostNode();

    destroyNode(node);
    --numberOfNodes_;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerSplit(
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode* node) {
    splay(node);

    auto leftRoot = root_->leftChild;
    root_->leftChild = nullptr;
    leftRoot->parent = nullptr;
    auto rightRoot = root_->rightChild;
    root_->rightChild = nullptr;
    rightRoot->parent = nullptr;

    // TODO Doesn't look nice and invokes unnecessary copies/moves,
    //      should be rewritten.
    *this = SplayTree(leftRoot, comparator_, nodeAllocator_);

    return {rightRoot, comparator_, nodeAllocator_};
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
void SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerMerge(
        SplayTree<Key, Value, KeyOfValue, Compare, Allocator>&& rhs) {
    splay(rightMostNode_);
    assert(!root_->rightChild);

    root_->rightChild = rhs.root_;
    root_->rightChild->parent = root_;
    rightMostNode_ = rhs.rightMostNode_;
    numberOfNodes_ += rhs.numberOfNodes_;

    rhs.root_ = nullptr;
    rhs.leftMostNode_ = nullptr;
    rhs.rightMostNode_ = nullptr;
    rhs.numberOfNodes_ = 0;
}

template<
    typename Key,
    typename Value,
    typename KeyOfValue,
    typename Compare,
    typename Allocator
>
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerFind(
        const Key& key) const {
    SplayTreeNode* placeToInsert = findPlaceToInsertUnique(key);
    if (placeToInsert && keysAreEqual(KeyOfValue()(placeToInsert->value), key)) {
        return placeToInsert;
    } else {
        return nullptr;
    }
}

} // namespace splay_tree

#endif // SPLAY_TREE_SPLAY_TREE_H_
