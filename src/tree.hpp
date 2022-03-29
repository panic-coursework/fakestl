#ifndef SJTU_TREE_HPP_
#define SJTU_TREE_HPP_

namespace panic {

class nullopt {};
template <typename T>
class Optional {
 public:
  Optional () : has(false) {}
  Optional (const nullopt &) : has(false) {}
  Optional (const T &value) : has(true), value_(new T(value)) {}
  ~Optional () { delete value_; }
  auto force () -> T & { return *value_; }
  auto force () const -> const T & { return *value_; }
  bool has;
 private:
  T *value = nullptr;
};

template <typename ValueType, typename Cmp>
class RbTree {
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
    using value_type = map::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;
    // If you are interested in type_traits, toy_traits_test provides a place to
    // practice. But the method used in that test is old and rarely used, so you
    // may explore on your own.

    iterator () {
      // TODO
    }
    iterator (const iterator &other) {
      // TODO
    }
    auto operator++ (int) -> iterator {}
    auto operator++ () -> iterator & {}
    auto operator-- (int) -> iterator {}
    auto operator-- () -> iterator & {}
    auto operator* () const -> value_type & {}
    auto operator== (const iterator &rhs) const -> bool {}
    auto operator== (const const_iterator &rhs) const -> bool {}
    auto operator!= (const iterator &rhs) const -> bool {}
    auto operator!= (const const_iterator &rhs) const -> bool {}

    auto operator-> () const noexcept -> value_type * {}
  };
  class const_iterator {
    // TODO
  };

  RbTree () {}
  RbTree (const RbTree &other) {}
  ~RbTree () { destroy_(); }

  auto operator= (const RbTree &other) -> RbTree & {
    if (this == &other) return *this;
  }
  auto begin () -> iterator {}
  auto cbegin () const -> const_iterator {}
  auto end () -> iterator {}
  auto cend () const -> const_iterator {}
  auto empty () const -> bool {}
  auto size () const -> size_t {}
  auto clear () -> void {}
  auto insert (const value_type &value) -> pair<iterator, bool> {}
  auto erase (iterator pos) -> void {}

 private:
  using Pointer = ValueType *;
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
    auto destroy () noexcept -> void {
      // should not destroy parent here.
      for (Node *&node : { left, right }) {
        if (node != nullptr) {
          node->destroy();
          delete node;
          node = nullptr;
        }
      }
    }
    auto isLeft () -> bool {
      return parent->left == this;
    }
    auto neighbor () -> Node * {
      return isLeft() ? node->parent->right ? node->parent->left;
    }
    auto replace (Node *replacement) -> void {
      (isLeft() ? parent->left : parent->right) = replacement;
    }

    auto insert (Node *newNode) -> void {
      Node *&next = lt_(newNode, this) ? left : right;
      if (next != nullptr) {
        return next->insert(newNode);
      }
      newNode->parent = this;
      next = newNode;
      // trick copied from libc++: if the left child of the
      // old leftmost node is not nullptr, then it must be
      // the new node.
      if (leftmost_->left != nullptr) leftmost_ = newNode;
      newNode->type = kRed;
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
  auto init_ () -> void {
    endNode_ = new Node();
  }
  auto destroy_ () noexcept -> void {
    if (endNode_ != nullptr) {
      endNode_->destroy();
      delete endNode_;
      endNode_ = nullptr;
    }
  }

  auto root_ () -> Node * { return endNode_->left; }
  auto root_ () const -> const Node * { return endNode_->left; }
  auto setRoot_ (Node *r) -> void {
    endNode_->left = r;
    r->parent = endNode_;
    r->type = Node::kBlack;
  }

  auto leftRotate_ (Node *x) -> void {
    Node *y = x->right;
    x->right = y->left;
    if (y->left != nullptr) y->left->parent = x;
    y->parent = x->parent;
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
    y->parent = x->parent;
    if (x == root_()) setRoot_(y);
    else x->replace(y);
    y->right = x;
    x->parent = y;
  }

  Cmp cmp_;
  auto lt_ (const Node *lhs, const Node *rhs) {
    return cmp_(lhs->value.force(), rhs->value.force());
  }
  auto insert_ (Node *newNode) -> void {
    if (root_() == nullptr) {
      setRoot_(newNode);
      leftmost_ = newNode;
      return;
    }
    root_().insert(newNode);
    fixupInsert_(newNode);
    root_().type = Node::kBlack;
  }
  auto fixupInsert_ (Node *node) -> void {
    if (node->parent->type != Node::kRed) return;
    bool parentIsLeft = node->parent->isLeft();
    Node grandParent = node->parent->parent;
    Node *uncle = parentIsLeft ? grandParent->right : grandParent->left;
    if (uncle->type == Node::kRed) {
      node->parent->type = Node::kBlack;
      grandParent->type = Node::kRed;
      uncle->type = Node::kBlack;
      return fixupInsert_(node->parent->parent);
    }
    if (parentIsLeft != node->isLeft()) {
      node = node->parent;
      if (parentIsLeft) leftRotate_(node);
      else rightRotate_(node);
    }
    node->parent->type = Node::kBlack;
    grandParent->type = Node::kRed;
    if (parentIsLeft) rightRotate_(node);
    else leftRotate_(node);
  }

  auto transplant_ (Node *from, Node *to) -> void {
    if (from == root_()) setRoot_(to);
    else from->replace(to);
    if (to != nullptr) to->parent = from->parent;
  }
  auto delete_ (Node *node) -> void {
    if (node == leftmost_) leftmost_ = /* TODO */;
    if (node->left == nullptr) {
      transplant_(node, node->right);
      if (node->type == Node::kBlack) fixupDelete_(node->right);
    } else if (node->right == nullptr) {
      transplant_(node, node->left);
      if (node->type == Node::kBlack) fixupDelete_(node->left);
    } else {
      Node *next = /* TODO */;
      bool shouldFixup = next->type == Node::kBlack;
      Node *fixupPoint = next->right;
      if (next->parent != node) {
        transplant_(next, next->right);
        next->right = node->right;
        next->right->parent = next;
      }
      transplant_(node, next);
      next->left = node->left;
      next->left->parent = next;
      next->left->type = node->type;
      if (shouldFixup) fixupDelete_(fixupPoint);
    }
  }
  auto fixupDelete_ (Node *node) -> void {
    if (node == nullptr) return;
    if (node == root_() || node.type == Node::kRed) {
      node->type = black;
      return;
    }
    // neighbor cannot be null here because of call site implications
    Node *neighbor = node->neighbor();
    if (neighbor->type == Node::kRed) {
      neighbor->type = Node::kBlack;
      node->parent->type = Node::kRed;
      leftRotate_(node->parent);
      neighbor = node->neighbor();
    }
    auto nullOrBlack = [] (Node *node) -> bool {
      return node == nullptr || node->type == Node::kBlack;
    }
    if (nullOrBlack(neighbor->left) && nullOrBlack(neighbor->right)) {
      neighbor->type = Node::kRed;
      return fixupDelete_(neighbor->parent);
    }
    bool isLeft = node->isLeft();
    auto left = isLeft ? &Node::left : &Node::right;
    auto right = isLeft ? &Node::right : &Node::left;
    if (nullOrBlack(neighbor->*right)) {
      Node *nl = neighbor->*left;
      if (nl != nullptr) nl->type = Node::kBlack;
      neighbor->type = Node::kRed;
    }
  }
};

} // namespace panic

#endif // SJTU_TREE_HPP_
