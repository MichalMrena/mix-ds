#ifndef MIX_DS_PAIRING_HEAP_HPP
#define MIX_DS_PAIRING_HEAP_HPP

#include <functional>
#include <utility>
#include <limits>
#include <memory>
#include <stdexcept>
#include <queue>
#include <tuple>
#include <algorithm>
#include <unordered_map>

namespace mix::ds
{
    // Types that specify different merge modes.
    namespace merge_modes
    {
        struct two_pass   {};
        struct fifo_queue {};
    }

    // Forward declaration of the tree node.
    template<class T, class Compare>
    class p_node;

    // Forward declaration of the iterator.
    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    class p_tree_iterator;

    // Forward declaration of the pairing heap.
    template< class T
            , class Compare   = std::less<T>
            , class MergeMode = merge_modes::two_pass
            , class Allocator = std::allocator<p_node<T, Compare>> >
    class pairing_heap;

    /**
        Node of a binary tree.
     */
    template<class T, class Compare>
    class p_node
    {
    public:
        template<class... Args> 
        p_node (std::piecewise_construct_t, Args&&... args);
        
        auto operator* ()       -> T&;
        auto operator* () const -> T const&;
        
        template<class, class, class, class> 
        friend class pairing_heap;
    
        template<class, class, class, class, class>
        friend class p_tree_iterator; 

    private:
        T       data_;
        p_node* parent_;
        p_node* left_;
        p_node* right_;
    };
    
    /**
        Node handle that is returned after an insertion
        and can be used for decrease_key and erase.
     */
    template<class T, class Compare, class MergeMode, class Allocator>
    class p_node_handle
    {
    public:
        auto operator*  ()       -> T&;
        auto operator*  () const -> T const&;
        auto operator-> ()       -> T*;
        auto operator-> () const -> T const*;

    private:
        using node_t = p_node<T, Compare>;
        friend class pairing_heap<T, Compare, MergeMode, Allocator>;
        p_node_handle(node_t* const node);
        node_t* node_;
    };

    /**
        Iterator of a binary tree.
     */
    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    class p_tree_iterator
    {
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = Val;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;
        using node_t            = p_node<T, Compare>;
    
    public:
        p_tree_iterator () = default;
        p_tree_iterator (node_t* const root);

        auto operator++ ()       -> p_tree_iterator&;
        auto operator++ (int)    -> p_tree_iterator;
        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator== (p_tree_iterator const&) const -> bool;
        auto operator!= (p_tree_iterator const&) const -> bool;

    private:
        friend class pairing_heap<T, Compare, MergeMode, Allocator>;
        auto current () const -> node_t*;

    private:
        std::queue<node_t*> queue_;
    };

    /**
        Pairing heap represented by a binary tree.
        Unlike std::priority_queue in this heap an element "which is less"
        has a higher priority. This behaviour can be changed by providing an appropriate
        Compare e.g. std::greater.

        @tparam T           The type of the stored elements.
        @tparam Compare     Type providing a strict weak ordering.
                            See https://en.cppreference.com/w/cpp/named_req/Compare  
        @tparam MergeMode   See the merge_modes namespace above.
        @tparam Allocator   Allocator. See https://en.cppreference.com/w/cpp/named_req/Allocator  
     */
    template<class T, class Compare, class MergeMode, class Allocator>
    class pairing_heap
    {
    public:
        using node_t          = p_node<T, Compare>;
        using handle_t        = p_node_handle<T, Compare, MergeMode, Allocator>;
        using value_type      = T;
        using reference       = T&;
        using const_reference = T const&;
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator        = p_tree_iterator<T, Compare, MergeMode, Allocator, T>;
        using const_iterator  = p_tree_iterator<T, Compare, MergeMode, Allocator, T const>;

    public:
        pairing_heap  (Allocator const& alloc = Allocator());
        pairing_heap  (pairing_heap const& other);
        pairing_heap  (pairing_heap&& other) noexcept;
        ~pairing_heap ();

        auto operator= (pairing_heap other) -> pairing_heap&;

        template<class... Args>
        auto emplace      (Args&&... args)             -> handle_t;
        auto insert       (value_type const& value)    -> handle_t;
        auto insert       (value_type&& value)         -> handle_t;
        auto delete_min   ()                           -> void;
        auto find_min     ()                           -> reference;
        auto find_min     () const                     -> const_reference;
        auto decrease_key (handle_t const handle)      -> void;
        auto decrease_key (iterator pos)               -> void;
        auto decrease_key (const_iterator pos)         -> void;
        auto meld         (pairing_heap rhs)           -> pairing_heap&;
        auto erase        (handle_t const handle)      -> void;
        auto erase        (iterator pos)               -> void;
        auto erase        (const_iterator pos)         -> void;
        auto swap         (pairing_heap& rhs) noexcept -> void;
        auto empty        () const                     -> bool;
        auto size         () const                     -> size_type;
        auto max_size     () const                     -> size_type;
        auto clear        ()                           -> void;
        auto begin        ()                           -> iterator;
        auto end          ()                           -> iterator;
        auto begin        () const                     -> const_iterator;
        auto end          () const                     -> const_iterator;
        auto cbegin       () const                     -> const_iterator;
        auto cend         () const                     -> const_iterator;

    private:
        template<class... Args>
        auto new_node      (Args&&... args)            -> node_t*;
        auto copy_node     (node_t* const node)        -> node_t*;
        auto delete_node   (node_t* const node)        -> void;
        auto insert_impl   (node_t* const node)        -> handle_t;
        auto empty_check   () const                    -> void;
        auto fill_map      (pairing_heap const& other) -> std::unordered_map<node_t*, node_t*>;
        auto deep_copy     (pairing_heap const& other) -> node_t*;
        auto erase_impl    (node_t* const node)        -> void;

        template<class Cmp = Compare>
        auto dec_key_impl  (node_t* const node) -> void;

        template<class NodeOp>
        auto for_each_node (NodeOp op) const -> void;

        template<class... Args>
        auto new_node_impl (Args&&... args) -> node_t*;

        template<class Cmp = Compare>
        static auto pair         (node_t* const lhs, node_t* const rhs)         -> node_t*;
        static auto merge        (node_t* const first)                          -> node_t*;
        static auto merge        (node_t* const first, merge_modes::two_pass)   -> node_t*;
        static auto merge        (node_t* const first, merge_modes::fifo_queue) -> node_t*;
        static auto first_pass   (node_t* first)                                -> node_t*;
        static auto second_pass  (node_t* last)                                 -> node_t*;
        static auto fill_queue   (node_t* first)                                -> std::queue<node_t*>;
        static auto is_left_son  (node_t* const node)                           -> bool;

    private:
        Allocator alloc_;
        node_t*   root_;
        size_type size_;
    };

    template<class T, class Compare, class MergeMode, class Allocator>
    auto meld ( pairing_heap<T, Compare, MergeMode, Allocator>
              , pairing_heap<T, Compare, MergeMode, Allocator> ) noexcept 
              -> pairing_heap<T, Compare, MergeMode, Allocator>;

    template<class T, class Compare, class MergeMode, class Allocator>
    auto swap ( pairing_heap<T, Compare, MergeMode, Allocator>&
              , pairing_heap<T, Compare, MergeMode, Allocator>& ) noexcept -> void;

    template<class T, class Compare, class MergeMode, class Allocator>
    auto operator== ( pairing_heap<T, Compare, MergeMode, Allocator> const&
                    , pairing_heap<T, Compare, MergeMode, Allocator> const& ) -> bool;

    template<class T, class Compare, class MergeMode, class Allocator>
    auto operator!= ( pairing_heap<T, Compare, MergeMode, Allocator> const&
                    , pairing_heap<T, Compare, MergeMode, Allocator> const& ) -> bool;

/// definitions:

    namespace aux_impl
    {
        struct always_true_cmp
        {
            template<class T>
            auto operator() (T const&, T const&) -> bool
            {
                return true;
            }
        };    
    }

// p_node:

    template<class T, class Compare>
    template<class... Args>
    p_node<T, Compare>::p_node(std::piecewise_construct_t, Args&&... args) :
        data_   (std::forward<Args>(args)...),
        parent_ (nullptr),
        left_   (nullptr),
        right_  (nullptr)
    {
    }

    template<class T, class Compare>
    auto p_node<T, Compare>::operator*
        () -> T&
    {
        return data_;
    }

    template<class T, class Compare>
    auto p_node<T, Compare>::operator*
        () const -> T const&
    {
        return data_;
    }

// p_node_handle:

    template<class T, class Compare, class MergeMode, class Allocator>
    p_node_handle<T, Compare, MergeMode, Allocator>::p_node_handle(node_t* const node) :
        node_ {node}
    {
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto p_node_handle<T, Compare, MergeMode, Allocator>::operator*
        () -> T&
    {
        return **node_;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto p_node_handle<T, Compare, MergeMode, Allocator>::operator*
        () const -> T const&
    {
        return **node_;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto p_node_handle<T, Compare, MergeMode, Allocator>::operator->
        () -> T*
    {
        return std::addressof(*node_);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto p_node_handle<T, Compare, MergeMode, Allocator>::operator->
        () const -> T const*
    {
        return std::addressof(*node_);
    }

// p_tree_iterator:

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::p_tree_iterator
        (node_t* const root) : 
        // queue_ (std::deque<node_t*> {root})
        queue_ (std::deque {root})
    {
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator++
        () -> p_tree_iterator&
    {
        if (queue_.front()->left_)
        {
            queue_.push(queue_.front()->left_);
        }

        if (queue_.front()->right_)
        {
            queue_.push(queue_.front()->right_);
        }

        queue_.pop();
        return *this;
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator++
        (int) -> p_tree_iterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator*
        () const -> reference
    {
        return **this->current();
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator->
        () const -> pointer
    {
        return std::addressof(**this);
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator==
        (p_tree_iterator const& rhs) const -> bool
    {
        return (queue_.empty() && rhs.queue_.empty())
            || (queue_.size()  == rhs.queue_.size()
            &&  queue_.front() == rhs.queue_.front());
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator!=
        (p_tree_iterator const& rhs) const -> bool
    {
        return !(*this == rhs);
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::current
        () const -> node_t*
    {
        return queue_.front();
    }

// pairing_heap:

    template<class T, class Compare, class MergeMode, class Allocator>
    pairing_heap<T, Compare, MergeMode, Allocator>::pairing_heap
        (Allocator const& alloc) :
        alloc_ (alloc),
        root_  (nullptr),
        size_  (0)
    {
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    pairing_heap<T, Compare, MergeMode, Allocator>::pairing_heap
        (pairing_heap const& other) :
        alloc_ (other.alloc_),
        root_  (this->deep_copy(other)),
        size_  (other.size_)
    {
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    pairing_heap<T, Compare, MergeMode, Allocator>::pairing_heap
        (pairing_heap&& other) noexcept :
        alloc_ (std::move(other.alloc_)),
        root_  (std::exchange(other.root_, nullptr)),
        size_  (std::exchange(other.size_, 0))
    {
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    pairing_heap<T, Compare, MergeMode, Allocator>::~pairing_heap()
    {
        this->clear();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::operator=
        (pairing_heap other) -> pairing_heap&
    {
        swap(*this, other);
        return *this;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class... Args>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::emplace
        (Args&&... args) -> handle_t
    {
        return this->insert_impl(this->new_node(std::forward<Args>(args)...));
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::insert
        (value_type const& value) -> handle_t
    {
        return this->insert_impl(this->new_node(value));
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::insert
        (value_type&& value) -> handle_t
    {
        return this->insert_impl(this->new_node(std::move(value)));
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::delete_min
        () -> void
    {
        this->empty_check();
        auto const oldRoot = root_;

        if (1 == this->size())
        {
            root_ = nullptr;
        }
        else
        {
            root_->left_->parent_ = nullptr;
            root_ = pairing_heap::merge(root_->left_);
        }

        --size_;
        this->delete_node(oldRoot);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::find_min
        () -> reference
    {
        this->empty_check();
        return **root_;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::find_min
        () const -> const_reference
    {
        this->empty_check();
        return **root_;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::decrease_key
        (handle_t const handle) -> void
    {
        this->dec_key_impl(handle.node_);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::decrease_key
        (iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::decrease_key
        (const_iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::meld
        (pairing_heap rhs) -> pairing_heap&
    {
        if (!root_ && !rhs.root_)
        {
            return *this;
        }

        auto const otherroot = std::exchange(rhs.root_, nullptr);

        if (root_ && otherroot)
        {
            root_ = pairing_heap::pair(root_, otherroot);
        }
        else if (!root_)
        {
            root_ = otherroot;
        }

        size_ += std::exchange(rhs.size_, 0);
        return *this;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::erase
        (handle_t const handle) -> void
    {
        this->erase_impl(handle.node_);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::erase
        (iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::erase
        (const_iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::swap
        (pairing_heap& rhs) noexcept -> void
    {
        using std::swap;
        swap(root_, rhs.root_);
        swap(size_, rhs.size_);

        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
        {
            swap(alloc_, rhs.alloc_);
        }
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::empty
        () const -> bool
    {
        return 0 == this->size();
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::size
        () const -> size_type
    {
        return size_;
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::max_size
        () const -> size_type
    {
        return std::numeric_limits<size_type>::max();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::clear
        () -> void
    {
        this->for_each_node([this](auto const node)
        {
            this->delete_node(node);
        });

        root_ = nullptr;
        size_ = 0;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::begin
        () -> iterator
    {
        return root_ ? iterator(root_) : iterator();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::end
        () -> iterator
    {
        return iterator();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::begin
        () const -> const_iterator
    {
        return this->cbegin();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::end
        () const -> const_iterator
    {
        return this->cend();
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::cbegin
        () const -> const_iterator
    {
        return root_ ? const_iterator(root_) : const_iterator();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::cend
        () const -> const_iterator
    {
        return const_iterator();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class... Args>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::new_node
        (Args&&... args) -> node_t*
    {
        return this->new_node_impl(std::piecewise_construct, std::forward<Args>(args)...);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::copy_node
        (node_t* const node) -> node_t*
    {
        return this->new_node_impl(*node);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::delete_node
        (node_t* const node) -> void
    {
        std::allocator_traits<Allocator>::destroy(alloc_, node);
        std::allocator_traits<Allocator>::deallocate(alloc_, node, 1);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::insert_impl
        (node_t* const node) -> handle_t
    {
        root_ = this->empty() ? node : pairing_heap::pair(root_, node);
        ++size_;
        return handle_t(node);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::empty_check
        () const -> void
    {
        if (this->empty())
        {
            throw std::out_of_range("Heap is empty!");
        }
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::fill_map
        (pairing_heap const& other) -> std::unordered_map<node_t*, node_t*>
    {
        auto map = std::unordered_map<node_t*, node_t*>();
        map.reserve(other.size());

        other.for_each_node([&map, this](auto const node)
        {
            map.emplace(node, this->copy_node(node));
        });

        return map;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::deep_copy
        (pairing_heap const& other) -> node_t*
    {
        if (other.empty())
        {
            return nullptr;
        }

        auto const map = this->fill_map(other);
        
        for (auto&& [oldnode, newnode] : map)
        {
            if (newnode->parent_)
            {
                newnode->parent_ = map.at(newnode->parent_);
            }

            if (newnode->left_)
            {
                newnode->left_ = map.at(newnode->left_);
            }

            if (newnode->right_)
            {
                newnode->right_ = map.at(newnode->right_);
            }
        }

        return map.at(other.root_);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::erase_impl
        (node_t* const node) -> void
    {
        this->dec_key_impl<aux_impl::always_true_cmp>(node);
        this->delete_min();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class Cmp>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::dec_key_impl
        (node_t* const node) -> void
    {
        if (node == root_)
        {
            return;
        }

        if (pairing_heap::is_left_son(node))
        {
            node->parent_->left_ = node->right_;
        }
        else
        {
            node->parent_->right_ = node->right_;
        }

        if (node->right_)
        {
            node->right_->parent_ = node->parent_;
        }

        node->parent_ = nullptr;
        node->right_  = nullptr;

        root_ = pairing_heap::pair<Cmp>(node, root_);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class NodeOp>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::for_each_node
        (NodeOp op) const -> void
    {
        auto it  = this->begin();
        auto end = this->end();

        while (it != end)
        {
            op(it.current());
            ++it;
        }
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class... Args>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::new_node_impl
        (Args&&... args) -> node_t*
    {
        auto const p = std::allocator_traits<Allocator>::allocate(alloc_, 1);
        std::allocator_traits<Allocator>::construct(alloc_, p, std::forward<Args>(args)...);
        return p;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    template<class Cmp>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::pair
        (node_t* const lhs, node_t* const rhs) -> node_t*
    {
        auto const areOrdered = Cmp () (**lhs, **rhs);
        auto const parent     = areOrdered ? lhs : rhs;
        auto const son        = areOrdered ? rhs : lhs;
        auto const oldLeftSon = parent->left_;

        son->parent_  = parent;
        son->right_   = oldLeftSon;
        parent->left_ = son;
        
        if (oldLeftSon)
        {
            oldLeftSon->parent_ = son;
        }

        return parent;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::merge
        (node_t* const first) -> node_t*
    {
        return pairing_heap::merge(first, MergeMode());
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::merge
        (node_t* const first, merge_modes::two_pass) -> node_t*
    {
        return pairing_heap::second_pass(pairing_heap::first_pass(first));
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::merge
        (node_t* first, merge_modes::fifo_queue) -> node_t*
    {
        auto fifo = pairing_heap::fill_queue(first);

        while (fifo.size() > 1)
        {
            auto const lhs = fifo.front();
            fifo.pop();
            auto const rhs = fifo.front();
            fifo.pop();
            fifo.push(pairing_heap::pair(lhs, rhs));
        }
        
        return fifo.front();
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::first_pass
        (node_t* first) -> node_t*
    {
        auto prev   = static_cast<node_t*>(nullptr);
        auto paired = static_cast<node_t*>(nullptr);
        auto next   = first;
        auto second = first->right_;

        for (;;)
        {
            first = next;
            if (!first)
            {
                return prev;
            }

            second = first->right_;
            if (second)
            {
                next            = second->right_;
                first->right_   = nullptr;
                first->parent_  = nullptr;
                second->right_  = nullptr;
                second->parent_ = nullptr;
                paired          = pairing_heap::pair(first, second);
            }
            else
            {
                paired = first;
                next   = nullptr;
            }

            if (prev)
            {
                prev->right_    = paired;
                paired->parent_ = prev;
            }
            prev = paired;

            if (next)
            {
                next->parent_ = nullptr;
            }
            else
            {
                return prev;
            }
        }

        return prev;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::second_pass
        (node_t* last) -> node_t*
    {
        auto parent   = last->parent_;
        last->parent_ = nullptr;

        while (parent)
        {
            auto const next = parent->parent_;
            parent->right_  = nullptr;
            parent->parent_ = nullptr;
            last            = pairing_heap::pair(last, parent);
            parent          = next;
        }

        return last;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::fill_queue
        (node_t* first) -> std::queue<node_t*>
    {
        auto next = first;
        auto qs   = std::queue<node_t*>();

        while (next)
        {
            next           = first->right_;
            first->right_  = nullptr;
            first->parent_ = nullptr;
            qs.push(first);
            first          = next;
        }

        return qs;
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto pairing_heap<T, Compare, MergeMode, Allocator>::is_left_son
        (node_t* const node) -> bool
    {
        return node->parent_ && node == node->parent_->left_;
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto meld ( pairing_heap<T, Compare, MergeMode, Allocator> lhs
              , pairing_heap<T, Compare, MergeMode, Allocator> rhs ) noexcept 
              -> pairing_heap<T, Compare, MergeMode, Allocator>
    {
        lhs.meld(std::move(rhs));
        return pairing_heap<T, Compare, MergeMode, Allocator>(std::move(lhs));
    }
    
    template<class T, class Compare, class MergeMode, class Allocator>
    auto swap ( pairing_heap<T, Compare, MergeMode, Allocator>& lhs
              , pairing_heap<T, Compare, MergeMode, Allocator>& rhs ) noexcept -> void
    {
        lhs.swap(rhs);
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto operator== ( pairing_heap<T, Compare, MergeMode, Allocator> const& lhs
                    , pairing_heap<T, Compare, MergeMode, Allocator> const& rhs ) -> bool
    {
        return lhs.size() == rhs.size()
            && std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    template<class T, class Compare, class MergeMode, class Allocator>
    auto operator!= ( pairing_heap<T, Compare, MergeMode, Allocator> const& lhs
                    , pairing_heap<T, Compare, MergeMode, Allocator> const& rhs ) -> bool
    {
        return ! (lhs == rhs);
    }
} 

#endif