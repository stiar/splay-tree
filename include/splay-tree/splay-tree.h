#ifndef SPLAY_TREE_SPLAY_TREE_H_
#define SPLAY_TREE_SPLAY_TREE_H_

#include <iterator>
#include <type_traits>
#include <cstddef>
#include <cassert>
#include <memory>

namespace splay_tree {

template <
    typename Key,
    typename Value,
    typename KeyOfValue,
    // TODO Custom comparators are ignored so far.
    typename Compare,
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
    //      consisting of value (and maybe something else?).
    struct SplayTreeNode {
        template<typename... Args>
        SplayTreeNode(Args&&... args) :
            value(std::forward<Args>(args)...) {
        }

        value_type value;
        SplayTreeNode* parent{nullptr};
        SplayTreeNode* leftSon{nullptr};
        SplayTreeNode* rightSon{nullptr};
    };

    typedef
        typename Allocator::template rebind<SplayTreeNode>::other
        NodeAllocator;

    SplayTreeNode* allocateNode() {
        return nodeAllocator_.allocate(1);
    }

    void deallocateNode(SplayTreeNode* node) {
        nodeAllocator_.deallocate(node, 1);
    }

    template<typename... Args>
    SplayTreeNode* createNode(Args&&... args) {
        auto newNode = allocateNode();
        try {
            std::allocator_traits<NodeAllocator>::construct(
                nodeAllocator_,
                newNode,
                std::forward<Args>(args)...);
        } catch (...) {
            deallocateNode(newNode);
            throw;
        }

        return newNode;
    }

    void destroyNode(SplayTreeNode* node) {
        nodeAllocator_.destroy(node);
        deallocateNode(node);
    }

    void destroyTree(SplayTreeNode* root) {
        if (root) {
            if (root->leftSon) {
                destroyTree(root->leftSon);
            }
            if (root->rightSon) {
                destroyTree(root->rightSon);
            }
            destroyNode(root);
        }
    }

    SplayTreeNode* copyTree(SplayTreeNode* root) {
        SplayTreeNode* rootCopy;
        try {
            rootCopy = createNode(root->value);
            if (root->leftSon) {
                rootCopy->leftSon = copyTree(root->leftSon);
                rootCopy->leftSon->parent = rootCopy;
            }
            if (root->rightSon) {
                rootCopy->rightSon = copyTree(root->rightSon);
                rootCopy->rightSon->parent = rootCopy;
            }
            return rootCopy;
        } catch (...) {
            destroyTree(rootCopy);
            throw;
        }
    }

    static SplayTreeNode* getLeftMostNode(SplayTreeNode* root) {
        auto currentNode = root;
        while (currentNode->leftSon) {
            currentNode = currentNode->leftSon;
        }
        return currentNode;
    }

    static SplayTreeNode* getRightMostNode(SplayTreeNode* root) {
        auto currentNode = root;
        while (currentNode->rightSon) {
            currentNode = currentNode->rightSon;
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
            node_(rhs.node_) {
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

    private:
        SplayTreeNode* node_;
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

    SplayTree(Compare comparator, const Allocator& allocator = Allocator()) :
        comparator_(std::move(comparator)),
        nodeAllocator_(allocator) {
    }

    SplayTree(
            std::initializer_list<Value> initializerList,
            Compare = Compare(),
            const Allocator& allocator = Allocator()) {
        insertUnique(initializerList.begin(), initializerList.end());
    }

    SplayTree(const SplayTree& rhs) :
        root_(copyTree(rhs.root_)),
        leftMostNode_(getLeftMostNode(root_)),
        rightMostNode_(getRightMostNode(root_)),
        numberOfNodes_(rhs.numberOfNodes_),
        comparator_(rhs.comparator_),
        nodeAllocator_(rhs.nodeAllocator_) {
    }

    SplayTree(SplayTree&& rhs) = default;

    ~SplayTree() noexcept {
        destroyTree(root_);
    }

    SplayTree& operator=(const SplayTree& rhs) {
        SplayTree temp(rhs);
        swap(*this, temp);
        return *this;
    }

    SplayTree& operator=(SplayTree&& rhs) noexcept {
        clear();
        swap(*this, rhs);
        return *this;
    }

    iterator begin() noexcept {
        return iterator(leftMostNode_, root_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(rightMostNode_, root_);
    }
    
    iterator end() noexcept {
        return {nullptr, root_};
    }

    const_iterator end() const noexcept {
        return {nullptr, root_};
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
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

    void swap(SplayTree& rhs) noexcept {
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

    // Find operations.
    iterator find(const key_type& key) {
        return iterator(innerFind(key), root_);
    }

    const_iterator find(const key_type& key) const {
        return const_iterator(innerFind(key), root_);
    }

    size_type count(const key_type& key) const;

    iterator lower_bound(const key_type& key) {
        return iterator(innerLowerBound(key), root_);
    }

    const_iterator lower_bound(const key_type& key) const {
        return const_iterator(innerLowerBound(key), root_);
    }

    iterator upper_bound(const key_type& key) {
        return iterator(innerUpperBound(key), root_);
    }

    const_iterator upper_bound(const key_type& key) const {
        return const_iterator(innerUpperBound(key), root_);
    }

    std::pair<iterator, iterator> equal_range(const key_type& key);

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& key) const;

private:
    SplayTreeNode* splay(SplayTreeNode* node);

    SplayTreeNode* zigStep(SplayTreeNode* node);

    SplayTreeNode* zigZigStep(SplayTreeNode* node);

    SplayTreeNode* zigZagStep(SplayTreeNode* node);

    SplayTreeNode* leftRotation(SplayTreeNode* node);

    SplayTreeNode* rightRotation(SplayTreeNode* node);

    SplayTreeNode* innerLowerBound(const key_type& key) const;

    SplayTreeNode* innerUpperBound(const key_type& key) const;

    SplayTreeNode* findPlaceToInsert(const key_type& key) const;

    SplayTreeNode* innerFind(const key_type& key) const;

    template<typename Arg>
    SplayTreeNode* innerInsert(
        Arg&& value,
        SplayTreeNode* placeToInsert,
        SplayTreeNode* newNode = nullptr);

    SplayTreeNode* root_{nullptr};
    SplayTreeNode* leftMostNode_{nullptr};
    SplayTreeNode* rightMostNode_{nullptr};
    size_type numberOfNodes_{0};
    Compare comparator_;
    NodeAllocator nodeAllocator_;
};

// TODO Write compare operators + swap function.

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

        if (currentNode->rightSon) {
            currentNode = currentNode->rightSon;
            while (currentNode->leftSon) {
                currentNode = currentNode->leftSon;
            }
            node_ = currentNode;
        } else {
            // TODO Because of the check currentNode->parent operator++ runs a
            //      bit slower than the corresponding one in STL. Should think
            //      about how to fix it (can a fake root help?)
            while (currentNode->parent &&
                    currentNode->parent->rightSon == currentNode) {
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
        while (currentNode && currentNode->rightSon) {
            currentNode = currentNode->rightSon;
        }
        node_ = currentNode;
    } else {
        SplayTreeNode* currentNode = node_;

        if (currentNode->leftSon) {
            currentNode = currentNode->leftSon;
            while (currentNode->rightSon) {
                currentNode = currentNode->rightSon;
            }
            node_ = currentNode;
        } else {
            while (currentNode->parent &&
                    currentNode->parent->leftSon == currentNode) {
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
template<typename Arg>
std::pair<typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::iterator, bool>
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::insertUnique(Arg&& value) {
    const auto& key = KeyOfValue(value);
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);

    if (placeToInsert && KeyOfValue(placeToInsert->value) == key) {
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
    const auto& key = KeyOfValue(value);
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);

    auto newNode = innerInsert(std::forward(value), placeToInsert);
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
        const auto& key = KeyOfValue(newNode->value);
        SplayTreeNode* placeToInsert = findPlaceToInsert(key);

        if (placeToInsert && KeyOfValue(placeToInsert->value) == key) {
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
        const auto& key = KeyOfValue(newNode->value);
        SplayTreeNode* placeToInsert = findPlaceToInsert(key);

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

            if ((grandParent->leftSon == parent) == (parent->leftSon == node)) {
                node = zigZigStep(node);
            } else {
                node = zigZagStep(node);
            }
            if (!node->parent) {
                root_ = node;
            }
        }
    }

    // FIXME Write this more clever.
    leftMostNode_ = getLeftMostNode(root_);
    rightMostNode_ = getRightMostNode(root_);

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
    if (node->parent->leftSon == node) {
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
    if (parent->leftSon == node) {
        parent = rightRotation(parent);
        node = parent->leftSon;
        return rightRotation(node);
    } else {
        parent = leftRotation(parent);
        node = parent->rightSon;
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
    if (parent->leftSon == node) {
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
    assert(parent->rightSon == node);
    SplayTreeNode* grandParent = parent->parent;

    parent->rightSon = node->leftSon;
    if (node->leftSon) {
        node->leftSon->parent = parent;
    }

    node->leftSon = parent;
    node->parent = grandParent;
    if (grandParent) {
        if (grandParent->leftSon == parent) {
            grandParent->leftSon = node;
        } else {
            grandParent->rightSon = node;
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
    assert(parent->leftSon == node);
    SplayTreeNode* grandParent = parent->parent;

    parent->leftSon = node->rightSon;
    if (node->rightSon) {
        node->rightSon->parent = parent;
    }

    node->rightSon = parent;
    node->parent = grandParent;
    if (grandParent) {
        if (grandParent->leftSon == parent) {
            grandParent->leftSon = node;
        } else {
            grandParent->rightSon = node;
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
        if (!comparator_(KeyOfValue(currentNode->value), key)) {
            lowerBound = currentNode;
            currentNode = currentNode->leftSon;
        } else {
            currentNode = currentNode->rightSon;
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
        if (comparator_(key, KeyOfValue(currentNode->value))) {
            upperBound = currentNode;
            currentNode = currentNode->leftSon;
        } else {
            currentNode = currentNode->rightSon;
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
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::findPlaceToInsert(
        const Key& key) const {
    SplayTreeNode* currentNode = root_;
    while (currentNode) {
        const auto& currentNodeKey = KeyOfValue(currentNode->value);
        if (currentNodeKey == key) {
            return currentNode;
        }
        if (comparator_(key, currentNodeKey)) {
            if (!currentNode->leftSon) {
                return currentNode;
            } else {
                currentNode = currentNode->leftSon;
            }
        } else {
            if (!currentNode->rightSon) {
                return currentNode;
            } else {
                currentNode = currentNode->rightSon;
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
        const auto& key = KeyOfValue(newNode->value);
        newNode->parent = placeToInsert;
        if (comparator_(KeyOfValue(placeToInsert->value), key)) {
            placeToInsert->rightSon = newNode;
            if (placeToInsert == rightMostNode_) {
                rightMostNode_ = newNode;
            }
        } else {
            placeToInsert->leftSon = newNode;
            if (placeToInsert == leftMostNode_) {
                leftMostNode_ = newNode;
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
typename SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Value, KeyOfValue, Compare, Allocator>::innerFind(
        const Key& key) const {
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    if (placeToInsert && KeyOfValue(placeToInsert->value) == key) {
        return placeToInsert;
    } else {
        return nullptr;
    }
}

} // splay_tree

#endif // SPLAY_TREE_SPLAY_TREE_H_
