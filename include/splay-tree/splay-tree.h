#ifndef SPLAY_TREE_SPLAY_TREE_H_
#define SPLAY_TREE_SPLAY_TREE_H_

#include <iterator>
#include <type_traits>
#include <cstddef>
#include <cassert>
#include <iostream>

namespace splay_tree {

template <
    typename Key,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<Key>
>
class SplayTree {
public:
    typedef Key key_type;
    typedef key_type* pointer;
    typedef const key_type* const_pointer;
    typedef key_type& reference;
    typedef const key_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Allocator allocator_type;

private:
    struct SplayTreeNode {
        explicit SplayTreeNode(const key_type& key) :
            key(key),
            parent(nullptr),
            leftSon(nullptr),
            rightSon(nullptr) {
        }

        key_type key;
        SplayTreeNode* parent;
        SplayTreeNode* leftSon;
        SplayTreeNode* rightSon;
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

    SplayTreeNode* createNode(const key_type& key) {
        SplayTreeNode* newNode = allocateNode();
        try {
            nodeAllocator_.construct(newNode, key);
        } catch (...) {
            deallocateNode(newNode);
            throw;
        }
        // TODO: is it an appropriate place?
        ++numberOfNodes_;
        return newNode;
    }

    void destroyNode(SplayTreeNode* node) {
        nodeAllocator_.destroy(node);
        deallocateNode(node);
        --numberOfNodes_;
    }

    // TODO: get rid of recursion
    void destroy(SplayTreeNode* node) {
        assert(node);
        if (node->leftSon) {
            destroy(node->leftSon);
        }
        if (node->rightSon) {
            destroy(node->rightSon);
        }
        destroyNode(node);
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

    SplayTree() :
        root_(nullptr),
        numberOfNodes_(0) {
    }

    // TODO
    SplayTree(const SplayTree& rhs);

    // TODO
    SplayTree(SplayTree&& rhs);

    ~SplayTree() {
        clear();
    }

    // TODO
    SplayTree& operator=(const SplayTree& rhs);

    // TODO
    SplayTree& operator=(SplayTree&& rhs);

    iterator begin() {
        return iterator(getLeftMostNode(), root_);
    }

    const_iterator begin() const {
        return const_iterator(getLeftMostNode(), root_);
    }
    
    const_iterator cbegin() const {
        return const_iterator(getLeftMostNode(), root_);
    }

    iterator end() {
        return {nullptr, root_};
    }

    const_iterator end() const {
        return {nullptr, root_};
    }
    
    const_iterator cend() const {
        return {nullptr, root_};
    }

    iterator lower_bound(const key_type& key) {
        return iterator(innerLowerBound(key), root_);
    }

    const_iterator lower_bound(const key_type& key) const {
        return const_iterator(innerLowerBound(key), root_);
    }

    iterator find(const key_type& key) {
        return iterator(innerFind(key), root_);
    }

    const_iterator find(const key_type& key) const {
        return const_iterator(innerFind(key), root_);
    }

    void clear() {
        if (root_) {
            destroy(root_);
            root_ = nullptr;
        }
    }

    std::pair<iterator, bool> insert(const key_type& key);

    // TODO
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args);

    size_type count(const key_type& key) const;

    bool empty() const {
        return !root_;
    }

    size_type size() const {
        return numberOfNodes_;
    }

    void swap(SplayTree& rhs) {
        swap(root_, rhs.root_);
        swap(numberOfNodes_, rhs.numberOfNodes_);
    }

private:
    SplayTreeNode* splay(SplayTreeNode* node);

    SplayTreeNode* zigStep(SplayTreeNode* node);

    SplayTreeNode* zigZigStep(SplayTreeNode* node);

    SplayTreeNode* zigZagStep(SplayTreeNode* node);

    SplayTreeNode* leftRotation(SplayTreeNode* node);

    SplayTreeNode* rightRotation(SplayTreeNode* node);

    SplayTreeNode* getLeftMostNode() const;

    SplayTreeNode* getRightMostNode() const;

    SplayTreeNode* innerLowerBound(const key_type& key) const;

    SplayTreeNode* findPlaceToInsert(const key_type& key) const;

    SplayTreeNode* innerFind(const key_type& key) const;

    // For debugging purposes.
    void printTree(std::ostream& os, SplayTreeNode* node) const;

    SplayTreeNode* root_;
    size_type numberOfNodes_;
    NodeAllocator nodeAllocator_;
};

template <typename Key, typename Compare, typename Allocator>
template <bool IsConstIterator>
typename SplayTree<Key, Compare, Allocator>::template SplayTreeIterator<IsConstIterator>&
SplayTree<Key, Compare, Allocator>::SplayTreeIterator<IsConstIterator>::operator++() {
    if (node_) {
        SplayTreeNode* currentNode = node_;

        if (currentNode->rightSon) {
            currentNode = currentNode->rightSon;
            while (currentNode->leftSon) {
                currentNode = currentNode->leftSon;
            }
            node_ = currentNode;
        } else {
            while (currentNode->parent &&
                    currentNode->parent->rightSon == currentNode) {
                currentNode = currentNode->parent;
            }
            node_ = currentNode->parent;
        }
    }

    return *this;
}

template <typename Key, typename Compare, typename Allocator>
template <bool IsConstIterator>
typename SplayTree<Key, Compare, Allocator>::template SplayTreeIterator<IsConstIterator>&
SplayTree<Key, Compare, Allocator>::SplayTreeIterator<IsConstIterator>::operator--() {
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

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::splay(SplayTreeNode* node) {
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
    return node;
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::zigStep(SplayTreeNode* node) {
    if (node->parent->leftSon == node) {
        return rightRotation(node);
    } else {
        return leftRotation(node);
    }
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::zigZigStep(SplayTreeNode* node) {
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

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::zigZagStep(SplayTreeNode* node) {
    SplayTreeNode* parent = node->parent;
    if (parent->leftSon == node) {
        node = rightRotation(node);
        return leftRotation(node);
    } else {
        node = leftRotation(node);
        return rightRotation(node);
    }
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::leftRotation(SplayTreeNode* node) {
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

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::rightRotation(SplayTreeNode* node) {
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

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::getLeftMostNode() const {
    SplayTreeNode* currentNode = root_;
    while (currentNode && currentNode->leftSon) {
        currentNode = currentNode->leftSon;
    }
    return currentNode;
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::getRightMostNode() const {
    SplayTreeNode* currentNode = root_;
    while (currentNode && currentNode->rightSon) {
        currentNode = currentNode->rightSon;
    }
    return currentNode;
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::innerLowerBound(
        const SplayTree<Key, Compare, Allocator>::key_type& key) const {
    SplayTreeNode* currentNode = root_;
    SplayTreeNode* lowerBound = root_;
    while (currentNode) {
        if (currentNode->key == key) {
            return currentNode;
        }
        if (currentNode->key > key) {
            lowerBound = currentNode;
            currentNode = currentNode->leftSon;
        } else {
            currentNode = currentNode->rightSon;
        }
    }
    return lowerBound;
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::findPlaceToInsert(
        const SplayTree<Key, Compare, Allocator>::key_type& key) const {
    SplayTreeNode* currentNode = root_;
    while (currentNode) {
        if (currentNode->key == key) {
            return currentNode;
        }
        if (currentNode->key > key) {
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

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::SplayTreeNode*
SplayTree<Key, Compare, Allocator>::innerFind(
        const SplayTree<Key, Compare, Allocator>::key_type& key) const {
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    if (placeToInsert && placeToInsert->key == key) {
        return placeToInsert;
    } else {
        return nullptr;
    }
}

template <typename Key, typename Compare, typename Allocator>
std::pair<typename SplayTree<Key, Compare, Allocator>::iterator, bool>
SplayTree<Key, Compare, Allocator>::insert(
        const SplayTree<Key, Compare, Allocator>::key_type& key) {
    if (!root_) {
        root_ = createNode(key);
        return {iterator(root_, root_), true};
    }

    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    assert(placeToInsert);

    if (placeToInsert->key == key) {
        return {iterator(placeToInsert, root_), false};
    }

    SplayTreeNode* newNode = createNode(key);
    newNode->parent = placeToInsert;
    if (placeToInsert->key > key) {
        placeToInsert->leftSon = newNode;
    } else {
        placeToInsert->rightSon = newNode;
    }

    newNode = splay(newNode);
    assert(root_ == newNode);
    //printTree(std::cerr, root_);
    //std::cerr << "\n";
    return {iterator(newNode, root_), true};
}

template <typename Key, typename Compare, typename Allocator>
typename SplayTree<Key, Compare, Allocator>::size_type
SplayTree<Key, Compare, Allocator>::count(
        const SplayTree<Key, Compare, Allocator>::key_type& key) const {
    SplayTreeNode* placeToInsert = findPlaceToInsert(key);
    if (placeToInsert && placeToInsert->key == key) {
        return 1;
    } else {
        return 0;
    }
}

template <typename Key, typename Compare, typename Allocator>
void SplayTree<Key, Compare, Allocator>::printTree(
        std::ostream& os,
        SplayTree<Key, Compare, Allocator>::SplayTreeNode* node) const {
    if (!node) {
        return;
    }
    if (node != root_) {
        os << " ";
    }
    os << node->key << "[";
    if (node->leftSon) {
        assert(node->leftSon->parent == node);
        os << node->leftSon->key;
    }
    os << ",";
    if (node->rightSon) {
        assert(node->rightSon->parent == node);
        os << node->rightSon->key;
    }
    os << "]";
    printTree(os, node->leftSon);
    printTree(os, node->rightSon);
}

} // splay_tree

#endif // SPLAY_TREE_SPLAY_TREE_H_
