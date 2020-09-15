#ifndef MIX_DS_FIBONACCI_HEAP_HPP
#define MIX_DS_FIBONACCI_HEAP_HPP

#include <functional>
#include <utility>
#include <stack>
#include <memory>
#include <limits>

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
        using node_t = fib_node<T>;

    public:
        template<class... Args>
        fib_node (std::piecewise_construct_t, Args&&... args);
        
        auto operator* ()       -> T&;
        auto operator* () const -> T const&;
        auto operator< (node_t const& other) const -> bool;

        auto add_child           (node_t* node) -> void;
        auto remove_child        (node_t* node) -> void;
        auto disconnect_children ()             -> node_t*;
        auto start_cycled_list   ()             -> void;
        auto is_violating        () const       -> bool;
        auto is_last_in_list     () const       -> bool;
        auto is_root             () const       -> bool;
        auto is_not_root         () const       -> bool;

        static auto merge_roots      (node_t* first, node_t* second) -> node_t*;
        static auto merge_lists      (node_t* first, node_t* second) -> node_t*;
        static auto remove_from_list (node_t * node)    -> void;

        template<class UnaryFunction>
        static auto fold_list (node_t * node, UnaryFunction f) -> void;

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
        Fibonacci tree iterator.
     */
    template<class T, class Compare, class Allocator, bool IsConst>
    class fib_heap_iterator
    {
    public:
        using node_t            = fib_node<T, Compare, Allocator>
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
        auto current () const -> node_t*;

    private:
        std::stack<node_t*> nodes_;
    };

    template<class T, class Compare, class Allocator, bool IsConst>
    auto swap ( fib_heap_iterator<T, Compare, Allocator, IsConst>& lhs
              , fib_heap_iterator<T, Compare, Allocator, IsConst>& rhs ) noexcept -> void;

    /**
        Fibonacci heap forward declaration.
     */
    template< class T
            , class Compare   = std::less<T>
            , class Allocator = std::allocator<T>>
    class fibonacci_heap;

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
     */
    template<class T, class Compare, class Allocator>
    class fibonacci_heap
    {
    public:
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        using const_reference   = T const&;
        using iterator          = fib_heap_iterator<T, Compare, false>;
        using const_iterator    = fib_heap_iterator<T, Compare, true>;
        using difference_type   = std::ptrdiff_t;
        using size_type         = std::size_t;
        using node_t            = fib_node<T, Compare, Allocator>;
        using handle_t          = fib_node_handle<T, Compare, Allocator>;
        using rank_t            = unsigned int;
        using type_alloc_traits = std::allocator_traits<Allocator>;
        using node_alloc_traits = typename type_alloc_traits::template rebind_traits<node_t>;
        using node_allocator    = typename type_alloc_traits::template rebind_alloc<node_t>;

    public:
        fibonacci_heap  (Allocator const& alloc = Allocator());
        fibonacci_heap  (fibonacci_heap const& other);
        fibonacci_heap  (fibonacci_heap && other) noexcept;
        ~fibonacci_heap ();

        auto operator=(fibonacci_heap other) noexcept -> fibonacci_heap&;

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
        auto increase_key (handle_t const handle)        -> void;
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

        template<class Cmp>
        auto decrease_key_impl (node_t* const node) -> void;

        auto deep_copy         (fibonacci_heap const& other) -> node_t*;
        auto insert_impl       (node_t* const node)          -> handle_t;
        auto copy_node         (node_t* const node)          -> node_t*;
        auto delete_node       (node_t* const node)          -> void;
        auto cut_node          (node_t* node)                -> void;
        auto consolidate_roots ()       -> void;
        auto is_empty_check    () const -> void;
        auto max_possible_rank () const -> rank_t;

    private:
        inline static constexpr auto PHI {1.61803398875};

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
    auto fib_node<T, Compare, Allocator>::operator<
        (node_t const& other) const -> bool
    {
        return Compare () (**this, *other);
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::add_child
        (node_t* node) -> void
    {
        newChild->parent_ = this;
        newChild->start_cycled_list();

        if (child_)
        {
            node_t::merge_lists(child_, node);
        }
        else
        {
            child_ = node;
        }

        ++rank_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::remove_child
        (node_t* node) -> void
    {
        node->parent_ = nullptr;
        
        if (node->is_last_in_list())
        {
            child_ = nullptr;
        }
        else if (node == child_)
        {
            child_ = child_->right_;
        }
        
        node_t::remove_from_list(node);
        
        --rank_;
        oldChild->mark_ = false;
        
        if (this->is_not_root())
        {
            mark_ = true;
        }
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::disconnect_children
        () -> node_t*
    {
        if (!child_)
        {
            return nullptr;
        }

        node_t::fold_list(child_, [](auto const n)
        {
            n->parent_ = nullptr;
        });
        auto ret = child_;
        child_   = nullptr;
        rank_    = 0;
        
        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::start_cycled_list
        () -> void
    {
        left_  = this;
        right_ = this;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::is_violating
        () const -> bool
    {
        return parent_ && *this < *parent_;
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
        return parent_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::is_not_root
        () const -> bool
    {
        return !parent_;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node<T, Compare, Allocator>::merge_roots
        (node_t* first, node_t* second) -> node_t*
    {
        if (*second < *first)
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
        stack_ (fib_impl::roots_to_dequeue(roots))
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
        (fib_heap_iterator const& other) const -> bool
    {
        return (stack_.empty() && rhs.stack_.empty())
            || (stack_.size()  == rhs.stack_.size()
               && stack_.top() == rhs.stack_.top());
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator!=
        (fib_heap_iterator const& other) const -> bool
    {
        return ! (*this == other);
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator*
        () const -> reference
    {
        return *nodes_.top();
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto fib_heap_iterator<T, Compare, Allocator, IsConst>::operator->
        () const -> pointer
    {
        return std::addressof(*nodes_.top());
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
        return **node;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return **node;
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator->
        () -> T*
    {
        return std::addressof(**node);
    }

    template<class T, class Compare, class Allocator>
    auto fib_node_handle<T, Compare, Allocator>::operator->
        () const -> T const*
    {
        return std::addressof(**node);
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
        this->clear();
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::operator=
        (fibonacci_heap<T, Compare, Allocator> other) noexcept -> fibonacci_heap<T, Compare, Allocator>&
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
            if (root_)
            {
                root_ = node_t::merge_lists(root_, children);
            }
            else
            {
                root_ = children;
            }
        }

        --size_;

        // Consolidate root nodes so that each rank will be unique.
        if (root_)
        {
            this->consolidate_roots();
        }

        this->delete_node(oldRoot);
        return data;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::find_min
        () -> reference
    {
        this->is_empty_check();
        return *root_;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::find_min
        () const -> const_reference
    {
        this->is_empty_check();
        return *root_;
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
    auto fibonacci_heap<T, Compare, Allocator>::increase_key
        (handle_t const handle) -> void
    {
        auto const node  = handle.node_;

        node_t::fold_list(node->child_, [this](auto const n)
        {
            if (n->is_violating())
            {
                this->cut_node(n);
            }
        });

        this->consolidate_roots();
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
        root_ =  *root_ < *otherroot ? root_ : otherroot;
        size_ += std::exchange(rhs.size_, 0);

        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (handle_t const handle) -> void
    {
        // TODO
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (iterator pos) -> void
    {
        // TODO
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::erase
        (const_iterator pos) -> void
    {
        // TODO
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
        auto it  = this->begin();
        auto end = this->end();

        while (it != end)
        {
            this->delete_node(*it);
            ++it;
        }
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
    template<class Cmp>
    auto fibonacci_heap<T, Compare, Allocator>::decrease_key_impl
        (node_t* const node) -> void
    {
        if (node->is_root())
        {
            return;
        }

        if (Cmp () (*node, *node->parent_))
        {
            this->cut_node(node);
        }

        if (Cmp () (*node, *root_))
        {
            root_ = fibNode;
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::insert_impl
        (node_t* const node) -> handle_t
    {
        node->start_cycled_list();

        if (root_)
        {
            node_t::merge_lists(root_, node);    
        }
        else
        {
            root_ = node;
        }

        if (*node < *this->root_)
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
        for (;;)
        {
            auto const parent  = node->parent_;
            auto const cutMore = node->is_not_root() && parent_->mark_;

            parent_->remove_child(node);
            node->start_cycled_list();
            node_t::merge_lists(root_, node);
            
            if (!cutMore)
            {
                return;
            }
            
            node = parent_;
        }
    }

    template<class T, class Compare, class Allocator>
    auto fibonacci_heap<T, Compare, Allocator>::consolidate_roots
        () -> void
    {
        auto const maxRank = this->max_possible_rank();
        auto aux = std::vector<fib_node*>(maxRank, nullptr);

        auto rootNode     = root_;
        auto lastRoot     = root_->left_;
        auto iterateRoots = true;

        while (iterateRoots)
        {
            auto const nextRootNode = rootNode->right_;
            iterateRoots = rootNode != lastRoot;

            for (;;)
            {
                if (aux[rootNode->rank_])
                {
                    rootNode = node_t::merge_roots(aux[rootNode->rank_], rootNode);
                    aux[rootNode->rank_ - 1] = nullptr;
                }
                else
                {
                    aux[rootNode->rank_] = rootNode;
                    break;
                }
            }

            rootNode = nextRootNode;
        }

        // Before this loop root_ holds pointer to some
        // element in the heap.
        // This pointer must be replaced with pointer from the
        // new list of roots even when they have same priorities.
        // That is why the condition in the if statement 
        // is written as it is. Of course this could have been solved
        // differently but I like the code this way.
        for (auto const root : aux)
        {
            if (!root)
            {
                continue;
            }
            
            if (!(*root_ < *root))
            {
                root_ = root;
            }
        }
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
        auto const goldration = fibonacci_heap::PHI;
        auto const rank = 1 + std::ceil(std::log(size_) / std::log(goldration));
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