#ifndef SPLAY_TREE_SPLAY_TREE_H_
#define SPLAY_TREE_SPLAY_TREE_H_

#include <iterator>
#include <type_traits>
#include <cstddef>
#include <cassert>

namespace splay_tree {

template <typename Key>
class SplayTree {
public:
    typedef Key key_type;

private:
    struct SplayTreeNode {
        explicit SplayTreeNode(const Key& key) :
            key(key),
            parent(nullptr),
            leftSon(nullptr),
            rightSon(nullptr) {
        }

        Key key;
        SplayTreeNode* parent;
        SplayTreeNode* leftSon;
        SplayTreeNode* rightSon;
    };

    static SplayTreeNode* createNode(const Key& key) {
        return new SplayTreeNode(key);
    }

    template<bool IsConstIterator>
    class SplayTreeIterator {
    public:
        typedef Key value_type;
        typedef
            typename std::conditional<IsConstIterator, const Key&, Key&>::type
            reference; 
        typedef
            typename std::conditional<IsConstIterator, const Key*, Key*>::type
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

        SplayTreeIterator& operator++() {
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

        SplayTreeIterator operator++(int) {
            const SplayTreeIterator old(*this);
            ++(*this);
            return old;
        }

        SplayTreeIterator& operator--() {
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

    // TODO: get rid of recursion
    void destroy(SplayTreeNode* node) {
        assert(node);
        if (node->leftSon) {
            destroy(node->leftSon);
        }
        if (node->rightSon) {
            destroy(node->rightSon);
        }
        delete node;
    }

    SplayTreeNode* getLeftMostNode() const {
        SplayTreeNode* currentNode = root_;
        while (currentNode && currentNode->leftSon) {
            currentNode = currentNode->leftSon;
        }
        return currentNode;
    }

    SplayTreeNode* getRightMostNode() const {
        SplayTreeNode* currentNode = root_;
        while (currentNode && currentNode->rightSon) {
            currentNode = currentNode->rightSon;
        }
        return currentNode;
    }


public:
    typedef SplayTreeIterator<false> iterator;
    typedef SplayTreeIterator<true> const_iterator;

    SplayTree() :
        root_(nullptr) {
    }

    ~SplayTree() {
        if (root_) {
            destroy(root_);
        }
    }

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

private:
    SplayTreeNode* innerLowerBound(const Key& key) const {
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

    SplayTreeNode* findPlaceToInsert(const Key& key) const {
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

    SplayTreeNode* innerFind(const Key& key) const {
        SplayTreeNode* placeToInsert = findPlaceToInsert(key);
        if (placeToInsert && placeToInsert->key == key) {
            return placeToInsert;
        } else {
            return nullptr;
        }
    }

public:
    iterator lower_bound(const Key& key) {
        return iterator(innerLowerBound(key), root_);
    }

    const_iterator lower_bound(const Key& key) const {
        return const_iterator(innerLowerBound(key), root_);
    }

    iterator find(const Key& key) {
        return iterator(innerFind(key), root_);
    }

    const_iterator find(const Key& key) const {
        return const_iterator(innerFind(key), root_);
    }

    std::pair<iterator, bool> insert(const Key& key) {
        if (!root_) {
            root_ = createNode(key);
            return {iterator(root_, root_), true};
        }
        // Find appropriate place for the new key.
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
        return {iterator(newNode, root_), true};
        // TODO Run splay
    }

    size_t count(const Key& key) const {
        SplayTreeNode* placeToInsert = findPlaceToInsert(key);
        if (placeToInsert && placeToInsert->key == key) {
            return 1;
        } else {
            return 0;
        }
    }

    bool empty() const {
        return !root_;
    }

    void swap(SplayTree& rhs) {
        swap(root_, rhs.root_);
    }

private:
    SplayTreeNode* root_;
};

} // splay_tree

#endif // SPLAY_TREE_SPLAY_TREE_H_
