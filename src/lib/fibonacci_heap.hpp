#ifndef MIX_DS_FIBONACCI_HEAP_HPP
#define MIX_DS_FIBONACCI_HEAP_HPP

#include <functional>
#include <utility>
#include <stack>
#include <memory>
#include <limits>
#include <unordered_map>
#include <cmath>

namespace mix::ds
{
    /**
        Fibonacci tree node.
     */
    template<class T, class Compare, class Allocator>
    class fib_node
    {
    public:
        using rank_t = unsigned int;
        using node_t = fib_node<T, Compare, Allocator>;

    public:
        template<class... Args>
        fib_node (std::piecewise_construct_t, Args&&... args);

        auto operator* ()       -> T&;
        auto operator* () const -> T const&;

        auto add_child           (node_t* node) -> void;
        auto remove_child        (node_t* node) -> void;
        auto disconnect_children ()             -> node_t*;
        auto to_looped_list      ()             -> void;
        auto is_last_in_list     () const       -> bool;
        auto is_root             () const       -> bool;
        auto is_not_root         () const       -> bool;

        static auto merge_roots      (node_t* first, node_t* second) -> node_t*;
        static auto merge_lists      (node_t* first, node_t* second) -> node_t*;
        static auto remove_from_list (node_t* node) -> void;

        template<class UnaryFunction>
        static auto fold_list (node_t* node, UnaryFunction f) -> void;

    public:
        rank_t  rank_;
        node_t* parent_;
        node_t* left_;
        node_t* right_;
        node_t* child_;
        bool    mark_;
        T       data_;
    };

    /**
        Fibonacci heap forward declaration.
     */
    template< class T
            , class Compare   = std::less<T>
            , class Allocator = std::allocator<T>>
    class fibonacci_heap;

    /**
        Fibonacci tree iterator.
     */
    template<class T, class Compare, class Allocator, bool IsConst>
    class fib_heap_iterator
    {
    public:
        using node_t            = fib_node<T, Compare, Allocator>;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::conditional_t<IsConst, T const, T>;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;

    public:
        fib_heap_iterator () = default;
        fib_heap_iterator (node_t* roots);
        fib_heap_iterator (fib_heap_iterator const& other);
        fib_heap_iterator (fib_heap_iterator&& other) noexcept;
        
        auto swap       (fib_heap_iterator& other) noexcept    -> void;
        auto operator=  (fib_heap_iterator other)              -> fib_heap_iterator&;
        auto operator!= (fib_heap_iterator const& ohter) const -> bool;
        auto operator== (fib_heap_iterator const& ohter) const -> bool;
        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator++ ()       -> fib_heap_iterator&;
        auto operator++ (int)    -> fib_heap_iterator;

    private:
        friend class fibonacci_heap<T, Compare, Allocator>;
        auto current () const -> node_t*;

    private:
        std::stack<node_t*> nodes_;
    };

    template<class T, class Compare, class Allocator, bool IsConst>
    auto swap ( fib_heap_iterator<T, Compare, Allocator, IsConst>& lhs
              , fib_heap_iterator<T, Compare, Allocator, IsConst>& rhs ) noexcept -> void;

    /**
        Node handle that is returned after an insertion
        and can be used for decrease_key and erase.
     */
    template<class T, class Compare, class Allocator>
    class fib_node_handle
    {
    public:
        auto operator*  ()       -> T&;
        auto operator*  () const -> T const&;
        auto operator-> ()       -> T*;
        auto operator-> () const -> T const*;

    private:
        using node_t = fib_node<T, Compare, Allocator>;
        friend class fibonacci_heap<T, Compare, Allocator>;
        fib_node_handle(node_t* const node);
        node_t* node_;
    };

    /**
        Fibonacci heap.
        Unlike std::priority_queue in this heap an element "which is less"
        has a higher priority. This behaviour can be changed by providing an appropriate
        Compare e.g. std::greater.

        @tparam T           The type of the stored elements.
        @tparam Compare     Type providing a strict weak ordering.
                            See https://en.cppreference.com/w/cpp/named_req/Compare
        @tparam Allocator   Allocator. See https://en.cppreference.com/w/cpp/named_req/Allocator
     */
    template<class T, class Compare, class Allocator>
    class fibonacci_heap
    {
    public:
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        using const_reference   = T const&;
        using iterator          = fib_heap_iterator<T, Compare, Allocator, false>;
        using const_iterator    = fib_heap_iterator<T, Compare, Allocator, true>;
        using difference_type   = std::ptrdiff_t;
        using size_type         = std::size_t;
        using node_t            = fib_node<T, Compare, Allocator>;
        using handle_t          = fib_node_handle<T, Compare, Allocator>;
        using rank_t            = unsigned int;
        using type_alloc_traits = std::allocator_traits<Allocator>;
        using node_alloc_traits = typename type_alloc_traits::template rebind_traits<node_t>;
        using node_allocator    = typename type_alloc_traits::template rebind_alloc<node_t>;
        using node_map          = std::unordered_map<node_t const*, node_t*>;

    public:
        fibonacci_heap  (Allocator const& alloc = Allocator());
        fibonacci_heap  (fibonacci_heap const& other);
        fibonacci_heap  (fibonacci_heap&& other) noexcept;
        ~fibonacci_heap ();

        auto operator= (fibonacci_heap other) noexcept -> fibonacci_heap&;

        template<class... Args>
        auto emplace      (Args&&... args)               -> handle_t;
        auto insert       (value_type const& value)      -> handle_t;
        auto insert       (value_type&& value)           -> handle_t;
        auto delete_min   ()                             -> void;
        auto find_min     ()                             -> reference;
        auto find_min     () const                       -> const_reference;
        auto decrease_key (handle_t const handle)        -> void;
        auto decrease_key (iterator pos)                 -> void;
        auto decrease_key (const_iterator pos)           -> void;
        auto meld         (fibonacci_heap rhs)           -> fibonacci_heap&;
        auto erase        (handle_t const handle)        -> void;
        auto erase        (iterator pos)                 -> void;
        auto erase        (const_iterator pos)           -> void;
        auto swap         (fibonacci_heap& rhs) noexcept -> void;
        auto empty        () const                       -> bool;
        auto size         () const                       -> size_type;
        auto max_size     () const                       -> size_type;
        auto clear        ()                             -> void;
        auto begin        ()                             -> iterator;
        auto end          ()                             -> iterator;
        auto begin        () const                       -> const_iterator;
        auto end          () const                       -> const_iterator;
        auto cbegin       () const                       -> const_iterator;
        auto cend         () const                       -> const_iterator;

    private:
        template<class... Args>
        auto new_node (Args&&... args) -> node_t*;

        template<class... Args>
        auto new_node_impl (Args&&... args) -> node_t*;

        template<class NodeOp>
        auto for_each_node (NodeOp op) const -> void;

        auto deep_copy         (fibonacci_heap const& other) -> node_t*;
        auto shallow_copy      (fibonacci_heap const& other) -> node_map;
        auto erase_impl        (node_t* const node)          -> void;
        auto decrease_key_impl (node_t* const node)          -> void;
        auto insert_impl       (node_t* const node)          -> handle_t;
        auto copy_node         (node_t* const node)          -> node_t*;
        auto delete_node       (node_t* const node)          -> void;
        auto cut_node          (node_t* node)                -> void;
        auto consolidate_roots ()       -> void;
        auto is_empty_check    () const -> void;
        auto max_possible_rank () const -> rank_t;

    private:
        inline static constexpr auto PHI = 1.61803398875;

    private:
        node_allocator alloc_;
        node_t*        root_;
        size_t         size_;
    };

    template<class T, class Compare, class Allocator>
    auto meld ( fibonacci_heap<T, Compare, Allocator>
              , fibonacci_heap<T, Compare, Allocator> ) noexcept -> fibonacci_heap<T, Compare, Allocator>;

    template<class T, class Compare, class Allocator>
    auto swap ( fibonacci_heap<T, Compare, Allocator>&
              , fibonacci_heap<T, Compare, Allocator>& ) noexcept -> void;

    template<class T, class Compare, class Allocator>
    auto operator== ( fibonacci_heap<T, Compare, Allocator> const&
                    , fibonacci_heap<T, Compare, Allocator> const& ) -> bool;

    template<class T, class Compare, class Allocator>
    auto operator!= ( fibonacci_heap<T, Compare, Allocator> const&
                    , fibonacci_heap<T, Compare, Allocator> const& ) -> bool;

// fib_node definition:

    template<class T, class Compare, class Allocator>
    template<class... Args>
    fib_node<T, Compare, Allocator>::fib_node
        (std::piecewise_construct_t, Args&&... args) :
        rank_   (0),
        parent_ (nullptr),
        left_   (nullptr),
        right_  (nullptr),
        child_  (nullptr),
        mark_   (false),
        data_   (std::forward<Args>(args)...)
    {
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::operator*
        () -> T&
    {
        return data_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return data_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::add_child
        (node_t* node) -> void
    {
        node->parent_ = this;
        node->to_looped_list();
        child_ = child_ ? node_t::merge_lists(child_, node) : node;
        ++rank_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::remove_child
        (node_t* node) -> void
    {
        node->parent_ = nullptr;
        child_        = node->right_;
        --rank_;

        if (node->is_last_in_list())
        {
            child_ = nullptr;
        }
        node_t::remove_from_list(node);

        // node->mark_ = false;

        // if (this->is_not_root())
        // {
        //     mark_ = true;
        // }
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::disconnect_children
        () -> node_t*
    {
        node_t::fold_list(child_, [](auto const n)
        {
            n->parent_ = nullptr;
        });

        rank_ = 0;
        return std::exchange(child_, nullptr);
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::to_looped_list
        () -> void
    {
        left_  = this;
        right_ = this;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::is_last_in_list
        () const -> bool
    {
        return this == right_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::is_root
        () const -> bool
    {
        return !parent_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::is_not_root
        () const -> bool
    {
        return !this->is_root();
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::merge_roots
        (node_t* first, node_t* second) -> node_t*
    {
        if (Compare () (**second, **first))
        {
            std::swap(first, second);
        }

        node_t::remove_from_list(second);
        first->add_child(second);
        return first;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::merge_lists
        (node_t* first, node_t* second) -> node_t*
    {
        auto const firstLast  = first->left_;
        auto const secondLast = second->left_;

        firstLast->right_  = second;
        second->left_      = firstLast;
        secondLast->right_ = first;
        first->left_       = secondLast;

        return first;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::remove_from_list
        (node_t* node) -> void
    {
        auto const left_  = node->left_;
        auto const right_ = node->right_;

        left_->right_ = right_;
        right_->left_ = left_;

        node->left_  = nullptr;
        node->right_ = nullptr;
    }
    
    template<class T, class Compare, class Allocator>
    template<class UnaryFunction>
    auto fib_node<T, Compare, Allocator>::fold_list
        (node_t* node, UnaryFunction f) -> void
    {
        auto const last = node ? node->left_ : nullptr;

        while (node)
        {
            auto const next = node->right_;
            f(node);

            if (node == last)
            {
                break;
            }

            node = next;
        }
    }

// fib_heap_iterator definition:

    namespace fib_impl
    {
        template<class T, class Compare, class Allocator>
        auto roots_to_dequeue
            (fib_node<T, Compare, Allocator>* roots) -> std::deque<fib_node<T, Compare, Allocator>*>
        {
            using node_t = fib_node<T, Compare, Allocator>;
            auto deq = std::deque<node_t*>();

            node_t::fold_list(roots, [&deq](auto const n)
            {
                deq.push_back(n);
            });

            return deq;
        }
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    fib_heap_iterator<T, Compare, Allocator, IsConst>::fib_heap_iterator
        (node_t* roots) : 
        nodes_ (fib_impl::roots_to_dequeue(roots))
    {
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    fib_heap_iterator<T, Compare, Allocator, IsConst>::fib_heap_iterator
        (fib_heap_iterator const& other) :
        nodes_ (other.nodes_)
    {
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    fib_heap_iterator<T, Compare, Allocator, IsConst>::fib_heap_iterator
        (fib_heap_iterator && other) noexcept :
        nodes_ (std::move(other.nodes_))
    {
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::swap
        (fib_heap_iterator& other) noexcept -> void
    {
        using std::swap;
        swap(nodes_, other.nodes_);
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator=
        (fib_heap_iterator other) -> fib_heap_iterator&
    {
        swap(*this, other);
        return *this;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator==
        (fib_heap_iterator const& rhs) const -> bool
    {
        return (nodes_.empty() && rhs.nodes_.empty())
            || (nodes_.size()  == rhs.nodes_.size()
               && nodes_.top() == rhs.nodes_.top());
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator!=
        (fib_heap_iterator const& rhs) const -> bool
    {
        return ! (*this == rhs);
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator*
        () const -> reference
    {
        return **nodes_.top();
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator->
        () const -> pointer
    {
        return std::addressof(**this);
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator++
        () -> fib_heap_iterator &
    {
        auto const topschild = nodes_.top()->child_;
        nodes_.pop();

        node_t::fold_list(topschild, [this](auto const n)
        {
            this->nodes_.push(n);
        });

        return *this;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator++
        (int) -> fib_heap_iterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::current
        () const -> node_t*
    {
        return nodes_.top();
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto swap ( fib_heap_iterator<T, Compare, Allocator, IsConst>& lhs
              , fib_heap_iterator<T, Compare, Allocator, IsConst>& rhs ) noexcept -> void
    {
        lhs.swap(rhs);
    }

// fib_node_handle definition:

    template<class T, class Compare, class Allocator>
    fib_node_handle<T, Compare, Allocator>::fib_node_handle
        (node_t* const node) :
        node_ (node)
    {
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator*
        () -> T&
    {
        return **node_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return **node_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator->
        () -> T*
    {
        return std::addressof(**node_);
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator->
        () const -> T const*
    {
        return std::addressof(**node_);
    }

// fibonacci_heap definition:

    template<class T, class Compare, class Allocator>
    fibonacci_heap<T, Compare, Allocator>::fibonacci_heap
        (Allocator const& alloc) :
        alloc_ (alloc),
        root_  (nullptr),
        size_  (0)
    {
    }

    template<class T, class Compare, class Allocator>
    fibonacci_heap<T, Compare, Allocator>::fibonacci_heap
        (fibonacci_heap<T, Compare, Allocator> const& other) :
        alloc_ (other.alloc_),
        root_  (this->deep_copy(other)),
        size_  (other.size_)
    {
    }

    template<class T, class Compare, class Allocator>
    fibonacci_heap<T, Compare, Allocator>::fibonacci_heap
        (fibonacci_heap<T, Compare, Allocator>&& other) noexcept :
        alloc_ (std::move(other.alloc_)),
        root_  (std::exchange(other.root_, nullptr)),
        size_  (std::exchange(other.size_, 0))
    {
    }

    template<class T, class Compare, class Allocator>
    fibonacci_heap<T, Compare, Allocator>::~fibonacci_heap
        ()
    {
        this->for_each_node([this](auto const node)
        {
            this->delete_node(node);
        });

        root_ = nullptr;
        size_ = 0;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::operator=
        (fibonacci_heap<T, Compare, Allocator> other) noexcept -> fibonacci_heap&
    {
        swap(*this, other);
        return *this;
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto fibonacci_heap<T, Compare, Allocator>::emplace
        (Args&&... args) -> handle_t
    {
        return this->insert_impl(this->new_node(std::forward<Args>(args)...));
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::insert
        (value_type const& data) -> handle_t
    {
        return this->insert_impl(this->new_node(data));
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::insert
        (value_type&& data) -> handle_t
    {
        return this->insert_impl(this->new_node(std::move(data)));
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::delete_min
        () -> void
    {
        this->is_empty_check();
        auto const oldRoot = root_;

        // Remove max prio root from the list of roots.
        if (oldRoot->is_last_in_list())
        {
            root_ = nullptr;
        }
        else
        {
            root_ = root_->right_;
            node_t::remove_from_list(oldRoot);
        }

        // Disconnect children from former max prio root and add them
        // into the list of roots.
        auto const children = oldRoot->disconnect_children();
        if (children)
        {
            root_ = root_ ? node_t::merge_lists(root_, children) : children;
        }

        --size_;

        // Consolidate root nodes so that each rank will be unique.
        if (root_)
        {
            this->consolidate_roots();
        }

        this->delete_node(oldRoot);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::find_min
        () -> reference
    {
        this->is_empty_check();
        return **root_;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::find_min
        () const -> const_reference
    {
        this->is_empty_check();
        return **root_;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::decrease_key
        (handle_t const handle) -> void
    {
        this->decrease_key_impl(handle.node_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::decrease_key
        (iterator pos) -> void
    {
        this->decrease_key_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::decrease_key
        (const_iterator pos) -> void
    {
        this->decrease_key_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::meld
        (fibonacci_heap rhs) -> fibonacci_heap&
    {
        if (rhs.empty())
        {
            return *this;
        }

        if (this->empty())
        {
            *this = std::move(rhs);
            return *this;
        }

        auto const otherroot = std::exchange(rhs.root_, nullptr);
        node_t::merge_lists(root_, otherroot);
        root_ =  Compare () (**root_, **otherroot) ? root_ : otherroot;
        size_ += std::exchange(rhs.size_, 0);

        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (handle_t const handle) -> void
    {
        this->erase_impl(handle.node_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (const_iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::swap
        (fibonacci_heap& rhs) noexcept -> void
    {
        using std::swap;
        swap(root_, rhs.root_);
        swap(size_, rhs.size_);

        if constexpr (node_alloc_traits::propagate_on_container_swap::value)
        {
            swap(alloc_, rhs.alloc_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::empty
        () const -> bool
    {
        return 0 == this->size();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::size
        () const -> size_type
    {
        return size_;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::max_size
        () const -> size_type
    {
        return std::numeric_limits<size_type>::max();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::clear
        () -> void
    {
        *this = fibonacci_heap();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::begin
        () -> iterator
    {
        return iterator(root_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::end
        () -> iterator
    {
        return iterator();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::begin
        () const -> const_iterator
    {
        return const_iterator(root_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::end
        () const -> const_iterator
    {
        return const_iterator();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::cbegin
        () const -> const_iterator
    {
        return iterator(root_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::cend
        () const -> const_iterator
    {
        return iterator();
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto fibonacci_heap<T, Compare, Allocator>::new_node
        (Args&&... args) -> node_t*
    {
        return this->new_node_impl( std::piecewise_construct
                                  , std::forward<Args>(args)... );
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto fibonacci_heap<T, Compare, Allocator>::new_node_impl
        (Args&&... args) -> node_t*
    {
        auto const p = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, p, std::forward<Args>(args)...);
        return p;
    }

    template<class T, class Compare, class Allocator>
    template<class NodeOp>
    auto fibonacci_heap<T, Compare, Allocator>::for_each_node
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

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::deep_copy
        (fibonacci_heap const& other) -> node_t*
    {
        auto const map = this->shallow_copy(other);

        for (auto [original, copy] : map)
        {
            if (copy)
            {
                copy->parent_ = map.at(original->parent_);
                copy->left_   = map.at(original->left_);
                copy->right_  = map.at(original->right_);
                copy->child_  = map.at(original->child_);
            }
        }

        return map.at(other.root_);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::shallow_copy
        (fibonacci_heap const& other) -> node_map
    {
        auto map = node_map();
        map.reserve(other.size());

        other.for_each_node([&map, this](auto const node)
        {
            map.emplace(node, this->copy_node(node));
        });
        map.emplace(nullptr, nullptr);

        return map;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase_impl
        (node_t* const node) -> void
    {
        if (node->is_not_root())
        {
            this->cut_node(node);
        }

        root_ = node;
        this->delete_min();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::decrease_key_impl
        (node_t* const node) -> void
    {
        if (node->is_not_root() && Compare () (**node, **node->parent_))
        {
            this->cut_node(node);
        }

        if (Compare () (**node, **root_))
        {
            root_ = node;
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::insert_impl
        (node_t* const node) -> handle_t
    {
        node->to_looped_list();

        if (root_)
        {
            node_t::merge_lists(root_, node);
        }
        else
        {
            root_ = node;
        }

        if (Compare () (**node, **root_))
        {
            root_ = node;
        }

        ++size_;
        return handle_t(node);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::copy_node
        (node_t* const node) -> node_t*
    {
        return this->new_node_impl(*node);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::delete_node
        (node_t* const node) -> void
    {
        node_alloc_traits::destroy(alloc_, node);
        node_alloc_traits::deallocate(alloc_, node, 1);
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::cut_node
        (node_t* node) -> void
    {
        auto cut = true;
        while (cut)
        {
            auto const parent = node->parent_;
            parent->remove_child(node);
            node->to_looped_list();
            node_t::merge_lists(root_, node);
            node->mark_ = false;
            cut  = parent->parent_ && std::exchange(parent->mark_, true);
            node = parent;
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::consolidate_roots
        () -> void
    {
        auto const maxRank = this->max_possible_rank();
        auto aux = std::vector<node_t*>(maxRank, nullptr);

        node_t::fold_list(root_, [&aux, this](auto root)
        {
            while (aux[root->rank_])
            {
                auto const auxroot = std::exchange(aux[root->rank_], nullptr);
                root = node_t::merge_roots(auxroot, root);
            }

            aux[root->rank_] = root;
            root->mark_      = false;

            if (!Compare () (**root_, **root))
            {
                root_ = root;
            }
        });
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::is_empty_check
        () const -> void
    {
        if (this->empty())
        {
            throw std::out_of_range("Priority queue is empty.");
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::max_possible_rank
        () const -> rank_t
    {
        auto const goldrat = fibonacci_heap::PHI;
        auto const rank = 1 + std::ceil(std::log(size_) / std::log(goldrat));
        return static_cast<rank_t>(rank);
    }

    template<class T, class Compare, class Allocator>
    auto meld ( fibonacci_heap<T, Compare, Allocator> lhs
              , fibonacci_heap<T, Compare, Allocator> rhs ) noexcept -> fibonacci_heap<T, Compare, Allocator>
    {
        using heap_t = fibonacci_heap<T, Compare, Allocator>;
        lhs.meld(std::move(rhs));
        return heap_t(std::move(lhs));
    }

    template<class T, class Compare, class Allocator>
    auto swap ( fibonacci_heap<T, Compare, Allocator>& lhs
              , fibonacci_heap<T, Compare, Allocator>& rhs ) noexcept -> void
    {
        lhs.swap(rhs);
    }

    template<class T, class Compare, class Allocator>
    auto operator== ( fibonacci_heap<T, Compare, Allocator> const& lhs
                    , fibonacci_heap<T, Compare, Allocator> const& rhs ) -> bool
    {
        return lhs.size() == rhs.size()
            && std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    template<class T, class Compare, class Allocator>
    auto operator!= ( fibonacci_heap<T, Compare, Allocator> const& lhs
                    , fibonacci_heap<T, Compare, Allocator> const& rhs ) -> bool
    {
        return ! (lhs == rhs);
    }
}

#endif