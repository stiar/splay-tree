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
           // TODO: maybe also check roots? 
        }

        bool operator==(const SplayTreeIterator& rhs) const {
            return node_ == rhs.node_;
           // TODO: maybe also check roots? 
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
            return node_->key;
        }

        pointer operator->() const {
            return &node_->key;
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

    SplayTree() :
        root_(nullptr),
        numberOfNodes_(0) {
    }

    // TODO
    SplayTree(const SplayTree& rhs);

    // TODO
    SplayTree(SplayTree&& rhs);

    ~SplayTree() {
        destroyTree(root_);
    }

    // TODO
    SplayTree& operator=(const SplayTree& rhs);

    // TODO
    SplayTree& operator=(SplayTree&& rhs);

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

    size_type maxsize() const noexcept {
        return nodeAllocator_.max_size();
    }

    void swap(SplayTree& rhs) {
        swap(root_, rhs.root_);
        swap(leftMostNode_, rhs.leftMostNode_);
        swap(rightMostNode_, rhs.rightMostNode_);
        swap(numberOfNodes_, rhs.numberOfNodes_);
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

    template<typename Arg>
    iterator insertEqual(Arg&& value);

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

    SplayTreeNode* root_{nullptr};
    SplayTreeNode* leftMostNode_{nullptr};
    SplayTreeNode* rightMostNode_{nullptr};
    size_type numberOfNodes_{0};
    NodeAllocator nodeAllocator_;
};

// TODO Write compare operators.

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

// TODO Remove code duplication in inserts.
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
    if (!root_) {
        root_ = createNode(std::forward<Arg>(value));
        ++numberOfNodes_;
        leftMostNode_ = root_;
        rightMostNode_ = root_;
        return {iterator(root_, root_), true};
    }

    const auto& key = KeyOfValue(value);
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    assert(placeToInsert);

    if (KeyOfValue(placeToInsert->value) == key) {
        return {iterator(placeToInsert, root_), false};
    }

    SplayTreeNode* newNode = createNode(std::forward<Arg>(value));
    newNode->parent = placeToInsert;
    if (KeyOfValue(placeToInsert->value) < key) {
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
    assert(root_ == newNode);

    ++numberOfNodes_;

    return {iterator(newNode, root_), true};
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
    if (!root_) {
        root_ = createNode(std::forward<Arg>(value));
        leftMostNode_ = root_;
        rightMostNode_ = root_;
        return {iterator(root_, root_), true};
    }

    const auto& key = KeyOfValue(value);
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    assert(placeToInsert);

    SplayTreeNode* newNode = createNode(std::forward<Arg>(value));
    newNode->parent = placeToInsert;
    if (KeyOfValue(placeToInsert->value) < key) {
        placeToInsert->rightSon = newNode;
    } else {
        placeToInsert->leftSon = newNode;
    }

    newNode = splay(newNode);
    assert(root_ == newNode);

    ++numberOfNodes_;

    return iterator(newNode, root_);
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

    assert(!node->parent);
    // FIXME Write this more clever.
    leftMostNode_ = root_;
    while (leftMostNode_->leftSon) {
        leftMostNode_ = leftMostNode_->leftSon;
    }
    rightMostNode_ = root_;
    while (rightMostNode_->rightSon) {
        rightMostNode_ = rightMostNode_->rightSon;
    }

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
        if (KeyOfValue(currentNode->value) >= key) {
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
        if (KeyOfValue(currentNode->value) > key) {
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
        if (currentNodeKey > key) {
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
