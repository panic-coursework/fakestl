#ifndef SJTU_TREE_HPP_
#define SJTU_TREE_HPP_

#include "utility.hpp"
#include "exceptions.hpp"

namespace panic {

class nullopt {};
template <typename T>
class Optional {
 public:
  bool has = false;
 private:
  T *value_ = nullptr;
 public:
  Optional () = default;
  Optional (const nullopt &/* unused */) {}
  Optional (const T &value) : has(true), value_(new T(value)) {}
  Optional (const Optional &other) { *this = other; }
  ~Optional () { delete value_; }
  auto operator= (const Optional &other) -> Optional & {
    if (this == &other) return *this;
    has = other.has;
    delete value_;
    value_ = nullptr;
    if (has) value_ = new T(*other.value_);
    return *this;
  }
  auto force () -> T & { return *value_; }
  auto force () const -> const T & { return *value_; }
};

/**
 * An implementation of the red-black tree, allowing no
 * duplicate keys.
 *
 * The algorithms are derived from those listed in Cormen
 * et. al, Introduction to Algorithms, Third Ed. Differences
 * are as follows:
 *
 * - We use nullptr for leafs, not a faux node (aka T.nil).
 * - We have a ``end node'' that has the real root node as
 *   its left child. End node is, in terms of memory
 *   structure, a full node, in contrary to being only a
 *   smaller subset in libc++ and libstdc++. This design
 *   offers a great reduction in class hierachy (so we don't
 *   need __parent_unsafe()'s), at the expense of slightly
 *   more memory consumption.
 * - We use tail recursions when possible for better code
 *   readability without sacrificing efficiency (when we
 *   have -O1 or above).
 * - We make abstractions over common patterns in the
 *   original algorithms, e.g. isLeft.
 *
 * The overall structure is based on libc++'s.
 */
template <typename ValueType, typename Cmp>
class RbTree {
 private:
  using Pointer = ValueType *;
  class Node;
 public:
  using value_type = ValueType;
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
   private:
    Node *node_;
    RbTree *home_;
    friend class RbTree;
    friend class const_iterator;
   public:
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a type.
    // For instance, for an iterator, iterator::value_type is the type that the
    // iterator points to.
    // STL algorithms and containers may use these type_traits (e.g. the following
    // typedef) to work properly.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
    // About iterator_category: https://en.cppreference.com/w/cpp/iterator
    using difference_type = std::ptrdiff_t;
    using value_type = RbTree::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;
    // If you are interested in type_traits, toy_traits_test provides a place to
    // practice. But the method used in that test is old and rarely used, so you
    // may explore on your own.

    iterator () = default;
    iterator (Node *node, RbTree *home) : node_(node), home_(home) {}
    auto operator++ (int) -> iterator {
      if (node_ == home_->endNode_) throw sjtu::invalid_iterator();
      iterator retval = *this;
      node_ = node_->next();
      return retval;
    }
    auto operator++ () -> iterator & {
      if (node_ == home_->endNode_) throw sjtu::invalid_iterator();
      node_ = node_->next();
      return *this;
    }
    auto operator-- (int) -> iterator {
      if (node_ == home_->leftmost_) throw sjtu::invalid_iterator();
      iterator retval = *this;
      node_ = node_->prev();
      return retval;
    }
    auto operator-- () -> iterator & {
      if (node_ == home_->leftmost_) throw sjtu::invalid_iterator();
      node_ = node_->prev();
      return *this;
    }
    auto operator* () const -> value_type & {
      return node_->value.force();
    }
    auto operator== (const iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator== (const const_iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator!= (const iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator!= (const const_iterator &rhs) const -> bool {
      return !(*this == rhs);
    }

    auto operator-> () const noexcept -> value_type * {
      return &node_->value.force();
    }
  };
  class const_iterator {
   private:
    Node *node_;
    RbTree *home_;
    friend class RbTree;
    friend class iterator;
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = const RbTree::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;
    const_iterator () = default;
    const_iterator (const Node *node, const RbTree *home) : node_(const_cast<Node *>(node)), home_(const_cast<RbTree *>(home)) {}
    const_iterator (const iterator &it) : node_(it.node_), home_(it.home_) {}
    auto operator++ (int) -> const_iterator {
      if (node_ == home_->endNode_) throw sjtu::invalid_iterator();
      const_iterator retval = *this;
      node_ = node_->next();
      return retval;
    }
    auto operator++ () -> const_iterator & {
      if (node_ == home_->endNode_) throw sjtu::invalid_iterator();
      node_ = node_->next();
      return *this;
    }
    auto operator-- (int) -> const_iterator {
      if (node_ == home_->leftmost_) throw sjtu::invalid_iterator();
      const_iterator retval = *this;
      node_ = node_->prev();
      return retval;
    }
    auto operator-- () -> const_iterator & {
      if (node_ == home_->leftmost_) throw sjtu::invalid_iterator();
      node_ = node_->prev();
      return *this;
    }
    auto operator* () const -> value_type & {
      return node_->value.force();
    }
    auto operator== (const const_iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator== (const iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator!= (const const_iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator!= (const iterator &rhs) const -> bool {
      return !(*this == rhs);
    }

    auto operator-> () const noexcept -> value_type * {
      return &node_->value.force();
    }
  };

  RbTree () { init_(); }
  RbTree (const RbTree &other) { *this = other; }
  ~RbTree () { destroy_(); }

  auto operator= (const RbTree &other) -> RbTree & {
    if (this == &other) return *this;
    destroy_();
    size_ = other.size_;
    endNode_ = other.endNode_->clone(nullptr);
    leftmost_ = endNode_->min();
    return *this;
  }
  auto begin () -> iterator {
    return iterator(leftmost_, this);
  }
  auto cbegin () const -> const_iterator {
    return const_iterator(leftmost_, this);
  }
  auto end () -> iterator {
    return iterator(endNode_, this);
  }
  auto cend () const -> const_iterator {
    return const_iterator(endNode_, this);
  }
  auto empty () const -> bool {
    return size_ == 0;
  }
  auto size () const -> size_t {
    return size_;
  }
  auto clear () -> void {
    destroy_();
    init_();
  }
  auto insert (const value_type &value) -> sjtu::pair<iterator, bool> {
    Node *node = new Node(value);
    auto res = insert_(node);
    if (res.has) {
      delete node;
      return sjtu::pair(iterator(res.force(), this), false);
    }
    ++size_;
    return sjtu::pair(iterator(node, this), true);
  }
  auto erase (iterator pos) -> void {
    if (pos.node_ == endNode_ || pos.home_ != this) throw sjtu::invalid_iterator();
    delete_(pos.node_);
    delete pos.node_;
    --size_;
  }
  template <typename K>
  auto find (const K &key) -> iterator {
    if (empty()) return end();
    auto node = endNode_->left->find(key);
    if (!node.has) return end();
    return iterator(const_cast<Node *>(node.force()), this);
  }
  template <typename K>
  auto find (const K &key) const -> const_iterator {
    if (empty()) return cend();
    auto node = endNode_->left->find(key);
    if (!node.has) return cend();
    return const_iterator(node.force(), this);
  }

 private:
  class Node {
   public:
    Node *parent = nullptr;
    Node *left = nullptr;
    Node *right = nullptr;
    enum Type { kRed, kBlack };
    Type type;
    Optional<ValueType> value;
    Node () = default;
    Node (const ValueType &value) : value(value) {}
    /// Destroys all the descendants, but not its parent.
    auto destroy () noexcept -> void {
      // should not destroy parent here.
      if (left != nullptr) {
        left->destroy();
        delete left;
        left = nullptr;
      }
      if (right != nullptr) {
        right->destroy();
        delete right;
        right = nullptr;
      }
    }
    /// Is the node a left child?
    auto isLeft () -> bool {
      return parent->left == this;
    }
    /// The neighbor aka sibling of the node.
    /// @nullable
    auto neighbor () -> Node * {
      return isLeft() ? parent->right : parent->left;
    }
    /// Unplugs the node from its parent, and plugs the replacement in.
    auto replace (Node *replacement) -> void {
      (isLeft() ? parent->left : parent->right) = replacement;
      if (replacement != nullptr) replacement->parent = parent;
    }

    /**
     * Inserts the new node into the tree hierachy. It takes
     * care of the order, but does not attempt to repair
     * balance. Therefore, it is possible to get an invalid
     * tree after calling.
     *
     * @returns nullopt if successful, Node * if a duplicate
     *   is found, the duplicate node.
     */
    auto insert (Node *newNode) -> Optional<Node *> {
      if (!lt_(newNode, this) && !lt_(this, newNode)) {
        return this;
      }
      Node *&next = lt_(newNode, this) ? left : right;
      if (next != nullptr) {
        return next->insert(newNode);
      }
      newNode->parent = this;
      next = newNode;
      newNode->type = kRed;
      return nullopt();
    }

    /// The minimal node of the tree.
    auto min () -> Node * {
      return left == nullptr ? this : left->min();
    }
    /// The maximal node of the tree.
    auto max () -> Node * {
      return right == nullptr ? this : right->max();
    }
    /// The node that immediately follows this node in ascending order.
    auto next () -> Node * {
      if (right != nullptr) return right->min();
      Node *node = this;
      while (!node->isLeft()) node = node->parent;
      return node->parent;
    }
    /// The node that immediately precedes this node in ascending order.
    auto prev () -> Node * {
      if (left != nullptr) return left->max();
      Node *node = this;
      while (node->isLeft()) node = node->parent;
      return node->parent;
    }

    /// Makes a clean clone of the tree hierachy.
    auto clone (Node *parent) -> Node * {
      Node *newNode = new Node(*this);
      newNode->parent = parent;
      if (left != nullptr) newNode->left = left->clone(newNode);
      if (right != nullptr) newNode->right = right->clone(newNode);
      return newNode;
    }

    /**
     * Finds the Node of the exact given key.
     *
     * @returns nullopt if not found, Node * if found, the node.
     */
    template <typename K>
    auto find (const K &key) const -> Optional<const Node *> {
      Cmp cmp_;
      if (!cmp_(key, value.force()) && !cmp_(value.force(), key)) {
        return this;
      }
      if (cmp_(key, value.force())) {
        if (left == nullptr) return nullopt();
        return left->find(key);
      }
      if (right == nullptr) return nullopt();
      return right->find(key);
    }
   private:
    auto lt_ (const Node *lhs, const Node *rhs) {
      return Cmp()(lhs->value.force(), rhs->value.force());
    }
  };
  /**
   * The ``end node'' is a special node. The only defined
   * property is its left child, which should be the root
   * node of the tree.
   */
  Node *endNode_ = nullptr;
  // for O(1) begin() and cbegin().
  Node *leftmost_ = nullptr;
  size_t size_ = 0;
  auto init_ () -> void {
    leftmost_ = endNode_ = new Node();
    size_ = 0;
  }
  auto destroy_ () noexcept -> void {
    if (endNode_ != nullptr) {
      endNode_->destroy();
      delete endNode_;
      endNode_ = nullptr;
      leftmost_ = nullptr;
    }
    size_ = 0;
  }

  /**
   * The ``root'' is the left child of the end node.
   * Do NOT use this to change the root node; use setRoot_
   * instead.
   */
  auto root_ () -> Node * { return endNode_->left; }
  auto root_ () const -> const Node * { return endNode_->left; }
  /// Sets the root and performs fixups.
  auto setRoot_ (Node *root) -> void {
    endNode_->left = root;
    root->parent = endNode_;
    root->type = Node::kBlack;
  }

  auto leftRotate_ (Node *x) -> void {
    Node *y = x->right;
    x->right = y->left;
    if (y->left != nullptr) y->left->parent = x;
    if (x == root_()) setRoot_(y);
    else x->replace(y);
    y->left = x;
    x->parent = y;
  }
  // TODO: dedupe the code
  auto rightRotate_ (Node *x) -> void {
    Node *y = x->left;
    x->left = y->right;
    if (y->right != nullptr) y->right->parent = x;
    if (x == root_()) setRoot_(y);
    else x->replace(y);
    y->right = x;
    x->parent = y;
  }

  /**
   * Inserts a new node and performs fixups if necessary.
   * Note that the caller needs to construct the new node
   * before calling, and destruct the new node if the insert
   * was unsuccessful. It updates leftmost_ but not size_.
   *
   * @returns nullopt if successful, Node * if a duplicate
   *   is found, the duplicate node.
   */
  auto insert_ (Node *newNode) -> Optional<Node *> {
    if (root_() == nullptr) {
      setRoot_(newNode);
      leftmost_ = newNode;
      return nullopt();
    }
    auto dup = root_()->insert(newNode);
    if (dup.has) return dup;
    newNode->type = newNode == root_() ? Node::kBlack : Node::kRed;
    // trick copied from libc++: if the left child of the
    // old leftmost node is not nullptr, then it must be
    // the new node.
    if (leftmost_->left != nullptr) leftmost_ = newNode;
    fixupInsert_(newNode);
    root_()->type = Node::kBlack;
    return nullopt();
  }
  /// Performs fixups after an insert.
  auto fixupInsert_ (Node *node) -> void {
    if (node->parent->type != Node::kRed) return;
    if (node == root_()) return;
    bool parentIsLeft = node->parent->isLeft();
    Node *grandParent = node->parent->parent;
    Node *uncle = parentIsLeft ? grandParent->right : grandParent->left;
    if (uncle != nullptr && uncle->type == Node::kRed) {
      node->parent->type = Node::kBlack;
      grandParent->type = grandParent == root_() ? Node::kBlack : Node::kRed;
      uncle->type = Node::kBlack;
      // we have a bad uncle here, calling grandparent for help.
      return fixupInsert_(node->parent->parent);
    }
    if (parentIsLeft != node->isLeft()) {
      node = node->parent;
      if (parentIsLeft) leftRotate_(node);
      else rightRotate_(node);
    }
    node->parent->type = Node::kBlack;
    grandParent->type = Node::kRed;
    if (parentIsLeft) rightRotate_(grandParent);
    else leftRotate_(grandParent);
  }

  /**
   * Deletes the node and performs fixups if necessary.
   * It does not destruct the node, so the caller needs to
   * do so by itself. It updates leftmost_ but not size_.
   */
  auto delete_ (Node *node) -> void {
    if (node == leftmost_) leftmost_ = node->next();
    // y is the node to be removed, and would have at most one child.
    bool notFull = node->left == nullptr || node->right == nullptr;
    Node *y = notFull ? node : node->right->min();
    // must be red if not null.
    // @nullable
    Node *childY = y->left != nullptr ? y->left : y->right;
    // will become childY's neighbor
    // @nullable
    Node *neighborY = y == root_() ? nullptr : y->neighbor();
    y->replace(childY);
    bool shouldFixup = y->type == Node::kBlack && root_() != nullptr;
    if (node != y) {
      node->replace(y);
      y->left = node->left;
      y->left->parent = y;
      y->right = node->right;
      if (y->right != nullptr) y->right->parent = y;
      y->type = node->type;
    }
    if (shouldFixup) {
      if (childY != nullptr) childY->type = Node::kBlack;
      else fixupDelete_(neighborY);
    }
  }
  /// Performs fixups after a delete operation.
  auto fixupDelete_ (Node *neighbor) -> void {
    auto nullOrBlack = [] (Node *node) -> bool {
      return node == nullptr || node->type == Node::kBlack;
    };
    // node is left child, so neighbor is right
    bool isLeft = !neighbor->isLeft();
    auto left = isLeft ? &Node::left : &Node::right;
    auto right = isLeft ? &Node::right : &Node::left;
    if (neighbor->type == Node::kRed) {
      neighbor->type = Node::kBlack;
      neighbor->parent->type = Node::kRed;
      if (isLeft) leftRotate_(neighbor->parent);
      else rightRotate_(neighbor->parent);
      neighbor = neighbor->*left->*right;
    }
    if (nullOrBlack(neighbor->left) && nullOrBlack(neighbor->right)) {
      neighbor->type = Node::kRed;
      Node *parent = neighbor->parent;
      if (parent == root_() || parent->type == Node::kRed) {
        parent->type = Node::kBlack;
        return;
      }
      // We have doubly black children and a bad parent.
      // Too bad we have to find our ancestors for help.
      return fixupDelete_(parent->neighbor());
    }
    // neighbor must have at least one red child at this point.
    if (nullOrBlack(neighbor->*right)) {
      (neighbor->*left)->type = Node::kBlack;
      neighbor->type = Node::kRed;
      if (isLeft) rightRotate_(neighbor);
      else leftRotate_(neighbor);
      neighbor = neighbor->parent;
    }
    neighbor->type = neighbor->parent->type;
    neighbor->parent->type = Node::kBlack;
    (neighbor->*right)->type = Node::kBlack;
    if (isLeft) leftRotate_(neighbor->parent);
    else rightRotate_(neighbor->parent);
  }
};

} // namespace panic

#endif // SJTU_TREE_HPP_
