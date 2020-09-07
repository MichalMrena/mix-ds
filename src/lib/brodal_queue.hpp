#ifndef MIX_DS_BRODAL_QUEUE_HPP
#define MIX_DS_BRODAL_QUEUE_HPP

#include <vector>
#include <cstdint>
#include <memory>
#include <string>
#include <stack>
#include <type_traits>
#include <functional>
#include <algorithm>

// TODO tmp
#include <iterator>
#include <iostream>

namespace mix::ds
{
    /**
     *  Guide structure that is used to maintain invariants.
     */
    template<class Reducer>
    class guide
    {
    public:
        using num_t   = std::uint8_t;
        using index_t = std::uint8_t;

    public:
        guide (guide&& other) noexcept;
        guide (Reducer reducer, guide const& other);
        guide (Reducer reducer, guide&& other) noexcept;
        guide (Reducer reducer);
        guide (Reducer reducer, std::vector<std::shared_ptr<index_t>> blocks);

        // TODO nope?
        guide (guide const&)  = delete;
        auto operator= (guide rhs) -> guide&;
        auto swap      (guide& rhs) noexcept -> void;

        auto inc (index_t const i) -> void;

        auto increase_domain () -> void;
        auto decrease_domain () -> void;

        auto to_string () const -> std::string;

    private:
        auto inc_in_block           (index_t const i) -> void;
        auto inc_out_block          (index_t const i) -> void;
        auto cancel_block           (index_t const i) -> void;
        auto is_in_block            (index_t const i) -> bool;
        auto is_first_in_block      (index_t const i) -> bool;
        auto is_last_in_block       (index_t const i) -> bool;
        auto is_valid_non_block_num (num_t const num) -> bool;
        auto is_valid_block_num     (num_t const num, index_t const i) -> bool;

    private:
        inline static constexpr auto NULL_BLOCK = std::numeric_limits<index_t>::max();

    private:
        Reducer reducer_;
        std::vector<std::shared_ptr<index_t>> blocks_;
    };

    template<class Reducer>
    auto swap (guide<Reducer>& lhs, guide<Reducer>& rhs) noexcept -> void;

    // Forward declaration of the tree node.
    template<class T, class Compare, class Allocator>
    class brodal_node;

    // Forward declarations of the queue:
    template<class T, class Compare, class Allocator>
    class brodal_queue;

    /**
        Result of delinking a single Brodal node.
     */
    template<class T, class Compare, class Allocator>
    struct delinked_nodes
    {
        using node_t = brodal_node<T, Compare, Allocator>;

        node_t* first  {nullptr};
        node_t* second {nullptr};
        node_t* third  {nullptr};
        node_t* extra  {nullptr};
    };

    /**
        Entry that wraps data inserted into the queue.
     */
    template<class T, class Compare, class Allocator>
    class brodal_entry
    {
    public:
        using node_t = brodal_node<T, Compare, Allocator>;

    public:
        template<class... Args>
        brodal_entry (Args&&... args);

        auto operator* ()       -> T&;
        auto operator* () const -> T const&;

    public:
        T       data_;
        node_t* node_;
    };

    /**
        Node of a brodal tree.
     */
    template<class T, class Compare, class Allocator>
    class brodal_node
    {
    public:
        using rank_t     = std::uint8_t;
        using num_t      = std::uint8_t;
        using node_map   = std::unordered_map<brodal_node const*, brodal_node*>;
        using entry_t    = brodal_entry<T, Compare, Allocator>;
        using node_t     = brodal_node;
        using delinked_t = delinked_nodes<T, Compare, Allocator>;

    public:
        brodal_node  (entry_t* entry);
        brodal_node  (entry_t* entry, brodal_node const& other);
        ~brodal_node ();
        
        auto operator<         (node_t const& other) const -> bool;
        auto add_child         (node_t* const newChild)    -> void;
        auto remove_child      (node_t* const oldChild)    -> void;
        auto add_right_sibling (node_t* const sibling)     -> void;
        auto add_to_W          (node_t* const node)        -> void;
        auto add_to_V          (node_t* const node)        -> void;
        auto add_set_sibling   (node_t* const sibling)     -> void;

        auto operator*         ()       -> T&;
        auto operator*         () const -> T const&;
        auto disconnect_sons   ()       -> node_t*;
        auto disconnect        ()       -> node_t*;
        auto set_rank          ()       -> void;
        auto same_rank_sibling () const -> node_t*;
        auto is_son_of_root    () const -> bool;
        auto is_violating      () const -> bool;
        auto is_in_set         () const -> bool;
        auto is_first_in_W_set () const -> bool;
        auto is_first_in_V_set () const -> bool;

        static auto remove_from_set     (node_t* const node)       -> void;
        static auto copy_list           (node_t* const first)      -> node_t*;
        static auto delink_node         (node_t* const node)       -> delinked_t;
        static auto same_rank_count     (node_t const* const node) -> num_t;
        static auto same_rank_violation (node_t const* const node) -> num_t;
        static auto are_siblings        (node_t* const first, node_t* const second)    -> bool;
        static auto swap_entries        (node_t* const first, node_t* const second)    -> void;
        static auto swap_tree_nodes     (node_t* const first, node_t* const second)    -> void;
        static auto make_siblings       (node_t* const first, node_t* const second)    -> void;
        static auto copy_set            (node_t* const first, node_map const& mapping) -> node_t*;
        static auto link_nodes          (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*;
        static auto max_prio_node       (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*;

        template<class PrevFucntion, class NextFunction>
        static auto same_rank_impl (node_t const* const node, PrevFucntion prev, NextFunction next) -> num_t;

        template<class UnaryFunction>
        static auto fold_right (node_t* const first, UnaryFunction func) -> void;

        template<class UnaryFunction>
        static auto fold_left (node_t* const first, UnaryFunction func) -> void;

        template<class UnaryFunction>
        static auto fold_next (node_t* const first, UnaryFunction func) -> void;

        template<class UnaryFunction, class NextFunction>
        static auto fold_impl (node_t* const first, UnaryFunction func, NextFunction get_next) -> void;
        
        template<class BinaryFunction>
        static auto zip_with (node_t* const first1, node_t* const first2, BinaryFunction func) -> void;
        
    public:
        rank_t   rank_      {0};
        entry_t* entry_     {nullptr};
        node_t*  parent_    {nullptr};
        node_t*  left_      {nullptr};
        node_t*  right_     {nullptr};
        node_t*  child_     {nullptr};
        node_t*  nextInSet_ {nullptr};
        node_t*  prevInSet_ {nullptr};
        node_t*  setW_      {nullptr};
        node_t*  setV_      {nullptr};
    };

    /**
        Reducer for the upper bound on number of sons.
     */
    template<class Reducible, class T, class Compare, class Allocator>
    struct upper_reducer
    {
        using index_t = std::uint8_t;
        using num_t   = std::uint8_t;
        using node_t  = brodal_node<T, Compare, Allocator>;

        upper_reducer (Reducible* const root);
        auto reduce   (index_t const i)       -> void;
        auto get_num  (index_t const i) const -> num_t;

        Reducible* managedRoot_ {nullptr};
    };

    /**
        Reducer for the lower bound on number of sons.
     */
    template<class Reducible, class T, class Compare, class Allocator>
    struct lower_reducer
    {
        using index_t = std::uint8_t;
        using num_t   = std::uint8_t;
        using node_t  = brodal_node<T, Compare, Allocator>;

        lower_reducer (Reducible* const root);
        auto reduce   (index_t const i)       -> void;
        auto get_num  (index_t const i) const -> num_t;
        
        Reducible* managedRoot_ {nullptr};
    };

    /**
        Reducer for the number of violation.
     */
    template<class Reducible, class T, class Compare, class Allocator>
    struct violation_reducer
    {
        using index_t = std::uint8_t;
        using num_t   = std::uint8_t;
        using node_t  = brodal_node<T, Compare, Allocator>;

        violation_reducer (Reducible* const root);
        auto reduce       (index_t const i)       -> void;
        auto get_num      (index_t const i) const -> num_t;
        
        Reducible* managedRoot_ {nullptr};
    };

    /// Temporary hack. Don't judge me please.
    template<class T, class Compare, class Allocator>
    class root_wrap_holder;

    /**
        Wrapper for the root of a tree.
     */
    template<class Tree, class T, class Compare, class Allocator>
    class root_wrap
    {
    public:
        using node_t        = brodal_node<T, Compare, Allocator>;
        using queue_t       = brodal_queue<T, Compare, Allocator>;
        using num_t         = std::uint8_t;
        using rank_t        = std::uint8_t;
        using delinked_t    = delinked_nodes<T, Compare, Allocator>;
        using node_map      = std::unordered_map<node_t const*, node_t*>;
        using up_reducer_t  = upper_reducer<root_wrap, T, Compare, Allocator>;
        using low_reducer_t = lower_reducer<root_wrap, T, Compare, Allocator>;
        using wrap_h        = root_wrap_holder<T, Compare, Allocator>;

    public:
        root_wrap  (queue_t* const queue);
        root_wrap  (queue_t* const queue, root_wrap const& other);
        root_wrap  (queue_t* const queue, root_wrap&& other) noexcept;
        
        auto operator= (root_wrap&& other) -> root_wrap&; // TODO nope
        auto operator= (wrap_h other)      -> root_wrap&;

        auto add_child                  (node_t* const child)     -> void;
        auto remove_child               (node_t* const child)     -> void;
        auto add_child_base             (node_t* const child)     -> void;
        auto remove_child_base          (node_t* const child)     -> void;
        auto add_child_checked          (node_t* const child)     -> void;
        auto remove_child_checked       (node_t* const child)     -> void;
        auto add_delinked_nodes         (delinked_t const& nodes) -> void;
        auto add_delinked_nodes_checked (delinked_t const& nodes) -> void;
        auto upper_check                (rank_t const rank)       -> void;
        auto lower_check                (rank_t const rank)       -> void;
        auto reduce_upper               (rank_t const rank)       -> void;
        auto reduce_lower               (rank_t const rank)       -> void;
        auto increase_rank              (node_t* const n1, node_t* const n2) -> void;
        auto increase_rank_base         (node_t* const n1, node_t* const n2) -> void;
        auto decrease_rank              () -> void;
        auto decrease_rank_base         () -> void;
        auto release_root               () -> node_t*;
        auto increase_domain            () -> void;
        auto decrease_domain            () -> void;
        auto increase_domain_base       () -> void;
        auto decrease_domain_base       () -> void;

        auto swap (root_wrap& rhs) noexcept -> void;

    public:
        auto upper_check_n_minus_1 () -> void;
        auto upper_check_n_minus_2 () -> void;
        auto lower_check_n_minus_1 () -> void;
        auto lower_check_n_minus_2 (num_t const bound) -> num_t;
        auto link_children         (rank_t const rank) -> node_t*;
        auto delink_child          (rank_t const rank) -> delinked_t;

    public:
        queue_t*             queue_ {nullptr};
        node_t*              root_  {nullptr};
        guide<up_reducer_t>  upper_ {up_reducer_t  {this}};
        guide<low_reducer_t> lower_ {low_reducer_t {this}};
        std::vector<node_t*> sons_;
    };

    template<class T, class Compare, class Allocator>
    class t1_wrap : public root_wrap<t1_wrap<T, Compare, Allocator>, T, Compare, Allocator>
    {
    public:
        using node_t         = brodal_node<T, Compare, Allocator>;
        using queue_t        = brodal_queue<T, Compare, Allocator>;
        using num_t          = std::uint8_t;
        using rank_t         = std::uint8_t;
        using delinked_t     = delinked_nodes<T, Compare, Allocator>;
        using node_map       = std::unordered_map<node_t const*, node_t*>;
        using node_ptr_pair  = std::pair<node_t*, node_t*>;
        using viol_reducer_t = violation_reducer<t1_wrap, T, Compare, Allocator>;
        using base_t         = root_wrap<t1_wrap, T, Compare, Allocator>;

    public:
        t1_wrap (queue_t* const queue);
        t1_wrap (queue_t* const queue, t1_wrap const& other);
        t1_wrap (queue_t* const queue, t1_wrap&& other) noexcept;

        // auto operator= (t1_wrap const& other) -> t1_wrap&;
        auto operator= (t1_wrap&& other) -> t1_wrap&;

        auto add_child             (node_t* const child)  -> void;
        auto remove_child          (node_t* const child)  -> void;
        auto add_violation         (node_t* const node)   -> void;
        auto remove_violation      (node_t* const node)   -> void;
        auto violation_check       (rank_t const rank)    -> void;
        auto reduce_violation      (rank_t const rank)    -> void;
        auto increase_rank         (node_t* const linked) -> void;
        auto increase_rank         (node_t* const n1, node_t* const n2) -> void;
        auto reduce_all_violations () -> void;
        auto decrease_rank         () -> void;
        auto increase_domain       () -> void;
        auto decrease_domain       () -> void;

        auto swap (t1_wrap& second) noexcept -> void;

    public:
        /// Removes at least one violation of given rank.
        /// Can create one violation of rank (rank+1),
        /// but in that case it removes two violations.
        /// @return Number of violations removed.
        auto reduce_violations (rank_t const rank) -> num_t;

        auto pick_normal_violations (rank_t const rank) -> node_ptr_pair;

        /// Pick violations of given rank that are sons of t2. 
        /// First 4 violations are ignored, i.e. if there are 
        /// 5 violations, only one is returned.
        auto pick_t2_son_violations (rank_t const rank) -> node_ptr_pair;

        /// @return 1 If violating node was removed.
        ///         0 If violating node was not removed,
        ///           i.e. node was nullptr.  
        auto remove_t2_violation (node_t* const node) -> num_t;

        auto remove_normal_violations (node_t* const first, node_t* const second) -> num_t;

    public:
        guide<viol_reducer_t> violation_ {viol_reducer_t {this}};
        std::vector<node_t*>  auxW_;
    };

    template<class T, class Compare, class Allocator>
    auto swap ( t1_wrap<T, Compare, Allocator>& lhs 
              , t1_wrap<T, Compare, Allocator>& rhs ) noexcept -> void;

    template<class T, class Compare, class Allocator>
    class t2_wrap : public root_wrap<t2_wrap<T, Compare, Allocator>, T, Compare, Allocator>
    {
    public:
        using node_t  = brodal_node<T, Compare, Allocator>;
        using queue_t = brodal_queue<T, Compare, Allocator>;
        using num_t   = std::uint8_t;
        using rank_t  = std::uint8_t;
        using base_t  = root_wrap<t2_wrap<T, Compare, Allocator>, T, Compare, Allocator>;
        using wrap_h  = root_wrap_holder<T, Compare, Allocator>;

    public:
        t2_wrap (queue_t * const queue);
        t2_wrap (queue_t * const queue, t2_wrap const& other);
        t2_wrap (queue_t * const queue, t2_wrap&& other) noexcept;

        auto operator= (wrap_h other)    -> t2_wrap&;
        auto operator= (t2_wrap&& other) -> t2_wrap&;

        auto add_child    (node_t* const child) -> void;
        auto remove_child (node_t* const child) -> void;

        auto increase_rank (node_t* const n1, node_t* const n2) -> void;
        auto decrease_rank () -> void;

        auto increase_domain () -> void;
        auto decrease_domain () -> void;

        /// Does NOT perform any guide adjustments.
        /// Leaves wrap in an incorrect state.
        /// Use only if wrap will be discarded soon afer.
        auto removeLargeSons () -> node_t*; 

        auto swap (t2_wrap& rhs) noexcept -> void;
    };

    template<class T, class Compare, class Allocator>
    auto swap ( t2_wrap<T, Compare, Allocator>& lhs 
              , t2_wrap<T, Compare, Allocator>& rhs ) noexcept -> void;

    /// Temporary hack. Don't judge me please.
    template<class T, class Compare, class Allocator>
    class root_wrap_holder
    {
    private:
        t1_wrap<T, Compare, Allocator>* t1_ {nullptr};
        t2_wrap<T, Compare, Allocator>* t2_ {nullptr};

    public:
        auto operator= (t1_wrap<T, Compare, Allocator>& t)
        {
            t1_ = &t;
        }

        auto operator= (t2_wrap<T, Compare, Allocator>& t)
        {
            t2_ = &t;
        }

        auto root ()
        {
            return t1_ ? t1_->root_ : t2_->root_;
        }

        auto upper () -> decltype(t1_->upper_)&&
        {
            return t1_ ? t1_->upper_ : t2_->upper_;
        }

        auto lower () -> decltype(t1_->lower_)&&
        {
            return t1_ ? t1_->lower_ : t2_->lower_;
        }

        auto sons () -> decltype(t1_->sons_)&&
        {
            return t1_ ? t1_->sons_ : t2_->sons_;
        }
    };

    /**
        Iterator of a single brodal tree.
     */
    template<class T, class Compare, class Allocator>
    class brodal_tree_iterator
    {
    public:
        using node_t            = brodal_node<T, Compare, Allocator>;
        using difference_type   = std::ptrdiff_t;
        using value_type        = node_t;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;

    public:
        brodal_tree_iterator () = default;
        brodal_tree_iterator (node_t* const root);

        auto operator++ ()       -> brodal_tree_iterator&;
        auto operator++ (int)    -> brodal_tree_iterator;
        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator== (brodal_tree_iterator const&) const -> bool;
        auto operator!= (brodal_tree_iterator const&) const -> bool;

    public:
        auto current () const -> node_t*;

    public:
        std::stack<node_t*> stack_;
    };

    /**
        Iterator for the Brodal queue.
     */
    template<class T, class Compare, class Allocator, bool IsConst>
    class brodal_queue_iterator
    {
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::conditional_t<IsConst, T const, T>;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;
        using node_t            = brodal_node<T, Compare, Allocator>;
        using tree_iterator_t   = brodal_tree_iterator<T, Compare, Allocator>;

    public:
        brodal_queue_iterator ();
        brodal_queue_iterator (node_t* const t1, node_t* const t2);

        auto operator++ ()       -> brodal_queue_iterator&;
        auto operator++ (int)    -> brodal_queue_iterator;
        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator== (brodal_queue_iterator const&) const -> bool;
        auto operator!= (brodal_queue_iterator const&) const -> bool;

    public:
        auto current () const -> node_t*;
        auto active  ()       -> tree_iterator_t&; 
        auto active  () const -> tree_iterator_t const&; 

    private:
        enum class active_it {first, second};

    private:
        tree_iterator_t  T1Iterator_;
        tree_iterator_t  T2Iterator_;
        active_it        activeIterator_;
    };

    /**
        Node handle that is returned after an insertion
        and can be used for decrease_key and erase.
     */
    template<class T, class Compare, class Allocator>
    class brodal_entry_handle
    {
    public:
        auto operator*  ()       -> T&;
        auto operator*  () const -> T const&;
        auto operator-> ()       -> T*;
        auto operator-> () const -> T const*;

    public:
        using entry_t = brodal_entry<T, Compare, Allocator>;
        brodal_entry_handle(entry_t* const node);
        entry_t* entry_;
    };    
    
    // TODO nope
    template<class T, class Compare, class Allocator>
    auto meld(brodal_queue<T, Compare, Allocator>& first, 
              brodal_queue<T, Compare, Allocator>& second) -> brodal_queue<T, Compare, Allocator>;

    /**
        Brodal queue.
     */
    template<class T, class Compare = std::less<T>, class Allocator = std::allocator<T>>
    class brodal_queue
    {
    public:
        using value_type      = T;
        using pointer         = T*;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = brodal_queue_iterator<T, Compare, Allocator, false>;
        using const_iterator  = brodal_queue_iterator<T, Compare, Allocator, true>;
        using difference_type = std::ptrdiff_t;
        using size_type       = std::size_t;
        using handle_t        = brodal_entry_handle<T, Compare, Allocator>;
        
        using num_t              = std::uint8_t;
        using index_t            = std::uint8_t;
        using rank_t             = index_t;
        using node_t             = brodal_node<T, Compare, Allocator>;
        using entry_t            = brodal_entry<T, Compare, Allocator>;
        using type_alloc_traits  = std::allocator_traits<Allocator>;
        using entry_alloc_traits = typename type_alloc_traits::template rebind_traits<entry_t>;
        using node_alloc_traits  = typename type_alloc_traits::template rebind_traits<node_t>;
        using node_alloc_t       = typename type_alloc_traits::template rebind_alloc<node_t>;
        using entry_alloc_t      = typename type_alloc_traits::template rebind_alloc<entry_t>;
        using node_map           = std::unordered_map<node_t const*, node_t*>;
        using node_ptr_pair      = std::pair<node_t*, node_t*>;
        using node_stack_t       = std::stack<node_t*>;
        using t1_wrap_t          = t1_wrap<T, Compare, Allocator>;
        using t2_wrap_t          = t2_wrap<T, Compare, Allocator>;
        using wrap_h             = root_wrap_holder<T, Compare, Allocator>;

    public:
        brodal_queue  (Allocator const& alloc = Allocator());
        brodal_queue  (brodal_queue const&);
        brodal_queue  (brodal_queue&&) noexcept;
        ~brodal_queue ();

        auto operator= (brodal_queue rhs) -> brodal_queue&;

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
        auto erase        (handle_t const handle)      -> void;
        auto erase        (iterator pos)               -> void;
        auto erase        (const_iterator pos)         -> void;
        auto swap         (brodal_queue& rhs) noexcept -> void;
        auto size         () const                     -> size_type;
        auto max_size     () const                     -> size_type;
        auto empty        () const                     -> bool;
        auto clear        ()                           -> void;
        auto begin        ()                           -> iterator;
        auto end          ()                           -> iterator;
        auto begin        () const                     -> const_iterator;
        auto end          () const                     -> const_iterator;
        auto cbegin       () const                     -> const_iterator;
        auto cend         () const                     -> const_iterator;

        friend auto meld<T, Compare, Allocator>(brodal_queue & first, 
                                     brodal_queue & second) -> brodal_queue;
    public:
        template<class... Args>
        auto new_node (Args&&... args) -> node_t*;
        
        template<class... Args>
        auto new_entry (Args&&... args) -> entry_t*;

        template<class Cmp = Compare>
        auto dec_key_impl (node_t* const node) -> void;

        auto delete_min_special   ()       -> void;
        auto add_extra_nodes      ()       -> void;
        auto add_violations       ()       -> void;
        auto is_empty_check       ()       -> void;
        auto move_all_to_T1       ()       -> void;
        auto move_to_T1           ()       -> void;
        auto find_new_root        () const -> node_t*;
        auto shallow_copy_node    (node_t* const node)        -> node_t*;
        auto delete_node          (node_t* const node)        -> void;
        auto delete_entry         (entry_t* const entry)      -> void;
        auto insert_impl          (node_t* const node)        -> handle_t;
        auto insert_special_impl  (node_t* const node)        -> handle_t;
        auto erase_impl           (node_t* const node)        -> void;
        auto make_son_of_root     (node_t* const newRoot)     -> void;
        auto merge_sets           (node_t* const newRoot)     -> void;
        auto shallow_copy_nodes   (brodal_queue const& other) -> node_map;
        auto deep_copy_tree       (node_map const& map)       -> void;
        auto deep_copy_violations (node_map const& map)       -> void;
        auto deep_copy_wraps      (brodal_queue const& other, node_map const& map) -> void;

        static auto dumbMeld  (brodal_queue& first, brodal_queue& second) -> brodal_queue;
        static auto findNewT1 (brodal_queue& first, brodal_queue& second) -> t1_wrap_t&;
        static auto findNewT2 (brodal_queue& first, brodal_queue& second, t1_wrap_t& t1) -> wrap_h;
        
        template<class RootWrap>
        static auto addUnderRoot (RootWrap& root, RootWrap& toAdd) -> void;

    public:
        std::size_t   size_;
        t1_wrap_t     T1_;
        t2_wrap_t     T2_;
        node_alloc_t  nodeAllocator_;
        entry_alloc_t entryAllocator_;
        node_stack_t  extraNodes_;
        node_stack_t  violations_;
    };

    template<class T, class Compare, class Allocator>
    auto swap( brodal_queue<T, Compare, Allocator>& first 
             , brodal_queue<T, Compare, Allocator>& second ) noexcept -> void;

    template<class T, class Compare, class Allocator>
    auto operator== ( brodal_queue<T, Compare, Allocator> const& lhs
                    , brodal_queue<T, Compare, Allocator> const& rhs ) -> bool;

    template<class T, class Compare, class Allocator>
    auto operator!= ( brodal_queue<T, Compare, Allocator> const& lhs
                    , brodal_queue<T, Compare, Allocator> const& rhs ) -> bool;

// guide definition:

    namespace guide_impl
    {
        template<class Vector, class Index>
        auto copy_blocks (Vector const& vs, Index const null)
        {
            using val_t = typename Vector::value_type::element_type;

            auto newVs = Vector();
            newVs.reserve(vs.size());

            auto it  = std::begin(vs);
            auto end = std::end(vs);

            while (it != end)
            {
                auto const block = **it;

                if (null == block)
                {
                    newVs.push_back(std::make_shared<val_t>(block));
                    ++it;
                    continue;
                }

                auto const newBlock = std::make_shared<val_t>(block);
                while (it != end and **it == *newBlock)
                {
                    newVs.push_back(newBlock);
                    ++it;
                }
            }

            return newVs;
        }
    }

    template<class Reducer>
    guide<Reducer>::guide
        (guide&& other) noexcept :
        reducer_ {std::move(other.reducer_)},
        blocks_  {std::move(other.blocks_)}
    {
    }

    template<class Reducer>
    guide<Reducer>::guide
        (Reducer reducer, guide const& other) :
        reducer_ {std::move(reducer)},
        blocks_  {guide_impl::copy_blocks(other.blocks_, NULL_BLOCK)}
    {
    }

    template<class Reducer>
    guide<Reducer>::guide
        (Reducer reducer, guide&& other) noexcept :
        reducer_ {std::move(reducer)},
        blocks_  {std::move(other.blocks_)}
    {
    }

    template<class Reducer>
    guide<Reducer>::guide
        (Reducer reducer) :
        reducer_ {std::move(reducer)}
    {
    }

    template<class Reducer>
    guide<Reducer>::guide
        (Reducer reducer, std::vector<std::shared_ptr<index_t>> blocks) :
        reducer_ {std::move(reducer)},
        blocks_  {std::move(blocks)}
    {
    }

    template<class Reducer>
    auto guide<Reducer>::operator=
        (guide rhs) -> guide&
    {
        using std::swap;
        swap(*this, rhs);
        return *this;
    }

    template<class Reducer>
    auto guide<Reducer>::inc
        (index_t const i) -> void
    {
        if (this->is_in_block(i))
        {
            this->inc_in_block(i);
        }
        else
        {
            this->inc_out_block(i);
        }
    }

    template<class Reducer>
    auto guide<Reducer>::increase_domain
        () -> void
    {
        blocks_.push_back(std::make_shared<index_t>(NULL_BLOCK));
        this->inc(blocks_.size() - 1);
    }

    template<class Reducer>
    auto guide<Reducer>::decrease_domain
        () -> void
    {
        this->cancel_block(blocks_.size() - 1);
        blocks_.pop_back();
    }

    template<class Reducer>
    auto guide<Reducer>::to_string
        () const -> std::string
    {
        auto out = std::string();
        out.reserve(2 + 2 * blocks_.size());

        for (auto i = blocks_.size(); i > 0;)
        {
            --i;
            out += std::to_string(reducer_.get_num(i));
        }
        out += '\n';

        auto it  = std::rbegin(blocks_);
        auto end = std::rend(blocks_);
        while (it != end)
        {
            auto const bn = **it++;
            out += bn == NULL_BLOCK ? "-" : std::to_string(bn);
        }
        out += '\n';

        return out;
    }

    template<class Reducer>
    auto guide<Reducer>::swap
        (guide& rhs) noexcept -> void
    {
        using std::swap;
        swap(blocks_, rhs.blocks_);
    }

    template<class Reducer>
    auto guide<Reducer>::inc_in_block
        (index_t const i) -> void
    {
        auto const num = reducer_.get_num(i);

        if (this->is_valid_block_num(num, i))
        {
            return;
        }

        if (this->is_last_in_block(i))
        {
            auto const blockBeginIndex = *blocks_[i];
            this->cancel_block(blockBeginIndex);
            this->inc_out_block(blockBeginIndex);
        }
        else if (this->is_first_in_block(i))
        {
            this->cancel_block(i);
            reducer_.reduce(i);
            if (i + 1u < blocks_.size())
            {
                this->inc(i + 1);
            }
        }
        else
        {
            auto const blockBeginIndex = *blocks_[i];
            this->cancel_block(i);
            this->inc_out_block(blockBeginIndex);
            this->inc_out_block(i);
        }
    }

    template<class Reducer>
    auto guide<Reducer>::inc_out_block
        (index_t const i) -> void
    {
        auto const num = reducer_.get_num(i);

        if (this->is_valid_non_block_num(num))
        {
            return;
        }

        reducer_.reduce(i);

        if (this->is_in_block(i + 1) && reducer_.get_num(i + 1) == 1u)
        {
            blocks_[i] = blocks_[i + 1];
        }
        else if (i + 1u < blocks_.size() && reducer_.get_num(i + 1) == 2)
        {
            blocks_[i]     = std::make_shared<index_t>(i + 1);
            blocks_[i + 1] = blocks_[i];
        }
    }

    template<class Reducer>
    auto guide<Reducer>::cancel_block
        (index_t const i) -> void
    {
        *blocks_[i] = NULL_BLOCK;
    }

    template<class Reducer>
    auto guide<Reducer>::is_in_block
        (index_t const i) -> bool
    {
        return i < blocks_.size() 
            && *blocks_[i] != NULL_BLOCK;
    }

    template<class Reducer>
    auto guide<Reducer>::is_first_in_block
        (index_t const i) -> bool
    {
        return i + 1u == blocks_.size()
            || *blocks_[i] != *blocks_[i + 1];
    }

    template<class Reducer>
    auto guide<Reducer>::is_last_in_block
        (index_t const i) -> bool
    {
        return 0 == i 
            || *blocks_[i] != *blocks_[i - 1];
    }

    template<class Reducer>
    auto guide<Reducer>::is_valid_block_num
        (num_t const num, index_t const i) -> bool
    {
        if (this->is_last_in_block(i))
        {
            return num <= 0;
        }
        
        if (this->is_first_in_block(i))
        {
            return num <= 2;
        }

        return num <= 1;
    }

    template<class Reducer>
    auto guide<Reducer>::is_valid_non_block_num
        (num_t const num) -> bool
    {
        return num <= 1;
    }
    
    template<class Reducer>
    auto swap 
        (guide<Reducer>& lhs, guide<Reducer>& rhs) noexcept -> void
    {
        lhs.swap(rhs);
    }

// brodal_entry definition:

    template<class T, class Compare, class Allocator>
    template<class... Args>
    brodal_entry<T, Compare, Allocator>::brodal_entry
        (Args&&... args) :
        data_ {std::forward<Args>(args)...},
        node_ {nullptr}
    {
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry<T, Compare, Allocator>::operator*
        () -> T&
    {
        return data_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return data_;
    }

// brodal_node definition:

    template<class T, class Compare, class Allocator>
    brodal_node<T, Compare, Allocator>::brodal_node
        (entry_t* entry) :
        entry_ {entry}
    {
        entry->node_ = this;
    }

    template<class T, class Compare, class Allocator>
    brodal_node<T, Compare, Allocator>::brodal_node
        (entry_t* entry, brodal_node const& other) :
        rank_      {other.rank_},
        entry_     {entry},
        parent_    {other.parent_},
        left_      {other.left_},
        right_     {other.right_},
        child_     {other.child_},
        nextInSet_ {other.nextInSet_},
        prevInSet_ {other.prevInSet_},
        setW_      {other.setW_},
        setV_      {other.setV_}
    {
    }

    template<class T, class Compare, class Allocator>
    brodal_node<T, Compare, Allocator>::~brodal_node()
    {
        // brodal_node::fold_right(child_, [](auto const n) { delete n; });
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::operator*
        () -> T&
    {
        return **entry_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return **entry_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::operator<
        (brodal_node const& other) const -> bool
    {
        return Compare {} (**this, *other);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::add_child
        (node_t* const newChild) -> void
    {
        newChild->parent_ = this;

        if (child_)
        {
            child_->left_ = newChild;
            newChild->right_   = child_;
            child_       = newChild;
        }
        else
        {
            child_ = newChild;
        }
        
        this->set_rank();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::remove_child
        (node_t* const oldChild) -> void
    {
        if (child_ == oldChild)
        {
            child_ = oldChild->right_;
        }

        if (oldChild->left_)
        {
            oldChild->left_->right_ = oldChild->right_;
        }

        if (oldChild->right_)
        {
            oldChild->right_->left_ = oldChild->left_;
        }

        oldChild->parent_ = nullptr;
        oldChild->left_   = nullptr;
        oldChild->right_  = nullptr;

        this->set_rank();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::add_right_sibling
        (node_t* const sibling) -> void
    {
        sibling->parent_ = parent_;
        sibling->left_   = this;
        sibling->right_  = right_;
        
        if (right_) 
        {
            right_->left_ = sibling;
        }

        right_ = sibling;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::add_to_W
        (node_t* const node) -> void
    {
        if (setW_)
        {
            setW_->prevInSet_ = node;
        }
        
        node->nextInSet_ = setW_;
        node->prevInSet_ = this;
        setW_ = node;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::add_to_V
        (node_t* const node) -> void
    {
        if (setV_)
        {
            setV_->prevInSet_ = node;
        }

        node->nextInSet_ = setV_;
        node->prevInSet_ = this;
        setV_ = node;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::add_set_sibling
        (node_t* const sibling) -> void
    {
        sibling->prevInSet_ = this;
        sibling->nextInSet_ = nextInSet_;
        
        if (nextInSet_)
        {
            nextInSet_->prevInSet_ = sibling;
        }
        
        nextInSet_ = sibling; 
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::disconnect_sons
        () -> node_t*
    {
        node_t::fold_right(child_, [](auto const n)
        {
            n->parent_ = nullptr;
        });

        auto ret = child_;
        child_ = nullptr;
        rank_  = 0;

        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::disconnect
        () -> node_t*
    {
        left_  = nullptr;
        right_ = nullptr;
        
        return this;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::set_rank
        () -> void
    {
        rank_ = child_ ? child_->rank_ + 1 : 0;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::same_rank_sibling
        () const -> node_t*
    {
        if (left_ && left_->rank_ == rank_)
        {
            return left_;
        }

        if (right_ && right_->rank_ == rank_)
        {
            return right_;
        }

        return nullptr;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::is_son_of_root
        () const -> bool
    {
        return parent_ && !parent_->parent_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::is_violating
        () const -> bool
    {
        return parent_ && *this < *parent_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::is_in_set
        () const -> bool
    {
        return nullptr != prevInSet_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::is_first_in_W_set
        () const -> bool
    {
        return prevInSet_ && prevInSet_->setW_ == this;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::is_first_in_V_set
        () const -> bool
    {
        return prevInSet_ && prevInSet_->setV_ == this;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::are_siblings
        (node_t* const first, node_t* const second) -> bool
    {
        return first->parent_ == second->parent_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::swap_entries
        (node_t* const first, node_t* const second) -> void
    {
        using std::swap;
        swap(first->entry_, second->entry_);
        swap(first->entry_->node_, second->entry_->node_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::swap_tree_nodes
        (node_t* const first, node_t* const second) -> void
    {
        using std::swap;

        // Swap parent_ pointers:
        if (first->parent_->child_ == first)
        {
            first->parent_->child_ = second;
        }
        
        if (second->parent_->child_ == second)
        {
            second->parent_->child_ = first;
        }

        swap(first->parent_, second->parent_);

        // Swap left_ and right_ pointers:
        if (second->left_)
        {
            second->left_->right_ = first;
        }

        if (second->right_)
        {
            second->right_->left_ = first;
        }

        if (first->left_)
        {
            first->left_->right_ = second;
        }
        
        if (first->right_)
        {
            first->right_->left_ = second; 
        }
        
        swap(first->left_,  second->left_);
        swap(first->right_, second->right_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::make_siblings
        (node_t* const first, node_t* const second) -> void
    {
        if (*first->parent_ < *second->parent_)
        {
            node_t::swap_tree_nodes(first, second->same_rank_sibling());
        }
        else
        {
            node_t::swap_tree_nodes(first->same_rank_sibling(), second);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::same_rank_count
        (node_t const* const node) -> num_t
    {
        
        return node_t::same_rank_impl( node
                                     , [](auto const n){ return n->left_; }
                                     , [](auto const n){ return n->right_; });
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::same_rank_violation
        (node_t const* const node) -> num_t
    {
        return node_t::same_rank_impl( node
                                     , [](auto const n){ return n->prevInSet_; }
                                     , [](auto const n){ return n->nextInSet_; });
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::remove_from_set
        (node_t* const node) -> void
    {
        if (node->is_first_in_W_set())
        {
            node->prevInSet_->setW_ = node->nextInSet_;
            if (node->nextInSet_)
            {
                node->nextInSet_->prevInSet_ = node->prevInSet_;
            }
        }
        else if (node->is_first_in_V_set())
        {
            node->prevInSet_->setV_ = node->nextInSet_;
            if (node->nextInSet_)
            {
                node->nextInSet_->prevInSet_ = node->prevInSet_;
            }
        }
        else
        {
            node->prevInSet_->nextInSet_ = node->nextInSet_;
            if (node->nextInSet_)
            {
                node->nextInSet_->prevInSet_ = node->prevInSet_;
            }
        }

        node->prevInSet_ = nullptr;
        node->nextInSet_ = nullptr;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::copy_list
        (node_t* const first) -> node_t*
    {
        if (!first)
        {
            return nullptr;
        }

        auto newListLast = new brodal_node(*first);
        auto const newListFirst = newListLast;

        node_t::fold_right(first->right_, [&](auto const n)
        {
            auto const newNode = new brodal_node(*n);
            newListLast->right_ = newNode;
            newNode->left_      = newListLast;
            newListLast        = newNode;
        });

        return newListFirst;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::copy_set
        (node_t* const first, node_map const& mapping) -> node_t*
    {
        if (!first)
        {
            return nullptr;
        }

        auto newSetLast = mapping.at(first);
        auto const newSetFirst = newSetLast;

        node_t::fold_next(first->nextInSet_, [&](auto const n)
        {
            auto const mappedNode = mapping.at(n);
            newSetLast->nextInSet_ = mappedNode;
            mappedNode->prevInSet_ = newSetLast;
            newSetLast            = mappedNode;
        });

        return newSetFirst;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::delink_node
        (node_t* const node) -> delinked_t
    {
        auto const sonCount   = node_t::same_rank_count(node->child_);
        auto const removeSons = sonCount >= 4 ? num_t(2) : sonCount;
        
        auto const n1 = node->child_;
        auto const n2 = n1->right_;
        auto const n3 = removeSons > 2 ? n2->right_ : nullptr;

        node->remove_child(n1);
        node->remove_child(n2);
        if (n3)
        {
            node->remove_child(n3);
        }

        return {n1, n2, n3, node};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::link_nodes
        (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*
    {
        auto const root = node_t::max_prio_node(n1, n2, n3);

        if (n1 != root)
        {
            root->add_child(n1);
        }

        if (n2 != root)
        {
            root->add_child(n2);
        }

        if (n3 != root)
        {
            root->add_child(n3);
        }

        return root;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node<T, Compare, Allocator>::max_prio_node
        (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*
    {
        auto max = n1;

        if (*n2 < *max)
        {
            max = n2;
        }

        if (*n3 < *max)
        {
            max = n3;
        }

        return max;
    }

    template<class T, class Compare, class Allocator>
    template<class PrevFucntion, class NextFunction>
    auto brodal_node<T, Compare, Allocator>::same_rank_impl
        (node_t const* const node, PrevFucntion prev, NextFunction next) -> num_t
    {
        auto count = num_t(1);

        auto prevNode = prev(node);
        auto nextNode = next(node);

        while (prevNode && prevNode->rank_ == node->rank_)
        {
            ++count;
            prevNode = prev(prevNode);
        }

        while (nextNode && nextNode->rank_ == node->rank_)
        {
            ++count;
            nextNode = next(nextNode);
        }

        return count;
    }

    template<class T, class Compare, class Allocator>
    template<class UnaryFunction>
    auto brodal_node<T, Compare, Allocator>::fold_right
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->right_; });
    }

    template<class T, class Compare, class Allocator>
    template<class UnaryFunction>
    auto brodal_node<T, Compare, Allocator>::fold_left
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->left_; });
    }

    template<class T, class Compare, class Allocator>
    template<class UnaryFunction>
    auto brodal_node<T, Compare, Allocator>::fold_next
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->nextInSet_; });
    }

    template<class T, class Compare, class Allocator>
    template<class UnaryFunction, class NextFunction>
    auto brodal_node<T, Compare, Allocator>::fold_impl
        (node_t* const first, UnaryFunction func, NextFunction get_next) -> void
    {
        auto it = first;
        while (it)
        {
            auto const next = get_next(it);
            func(it);
            it = next;
        }
    }

    template<class T, class Compare, class Allocator>
    template<class BinaryFunction>
    auto brodal_node<T, Compare, Allocator>::zip_with
        (node_t* const fold1, node_t* const fold2, BinaryFunction func) -> void
    {
        auto it1 = fold1;
        auto it2 = fold2;

        while (it1 && it2)
        {
            auto const next1 = it1->right_;
            auto const next2 = it2->right_;
            
            func(it1, it2);

            it1 = next1;
            it2 = next2;
        }
    }

// brodal_tree_iterator definition:

    template<class T, class Compare, class Allocator>
    brodal_tree_iterator<T, Compare, Allocator>::brodal_tree_iterator
        (node_t* const root) :
        stack_ {root ? std::deque {root} : std::deque<node_t*> {}}
    {
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator++
        () -> brodal_tree_iterator&
    {
        // TODO foldr
        auto next = stack_.top()->child_;
        stack_.pop();
        while (next)
        {
            stack_.push(next);
            next = next->right_;
        }

        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator++
        (int) -> brodal_tree_iterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator*
        () const -> reference
    {
        return *stack_.top();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator->
        () const -> pointer
    {
        return stack_.top();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator==
        (brodal_tree_iterator const& rhs) const -> bool
    {
        return (stack_.empty() && rhs.stack_.empty())
            || (stack_.size()  == rhs.stack_.size()
            &&  stack_.top()   == rhs.stack_.top());
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::operator!=
        (brodal_tree_iterator const& rhs) const -> bool
    {
        return !(*this == rhs);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_tree_iterator<T, Compare, Allocator>::current
        () const -> node_t*
    {
        return this->operator->();
    }

// brodal_queue_iterator definition:

    template<class T, class Compare, class Allocator, bool IsConst>
    brodal_queue_iterator<T, Compare, Allocator, IsConst>::brodal_queue_iterator
        () :
        brodal_queue_iterator<T, Compare, Allocator, IsConst> {nullptr, nullptr}
    {
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    brodal_queue_iterator<T, Compare, Allocator, IsConst>::brodal_queue_iterator
        (node_t* const t1, node_t* const t2) :
        T1Iterator_     {t1},
        T2Iterator_     {t2},
        activeIterator_ {active_it::first}
    {
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator++
        () -> brodal_queue_iterator& 
    {
        ++this->active();
        if (tree_iterator_t {} == this->active())
        {
            activeIterator_ = active_it::second;
        }

        return *this;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator++
        (int) -> brodal_queue_iterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator*
        () const -> reference
    {
        return **this->active();
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator->
        () const -> pointer
    {
        return std::addressof(**this->active());
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator==
        (brodal_queue_iterator const& rhs) const -> bool
    {
        return T1Iterator_ == rhs.T1Iterator_
            && T2Iterator_ == rhs.T2Iterator_;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::operator!=
        (brodal_queue_iterator const& rhs) const -> bool
    {
        return ! (*this == rhs);
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::current
        () const -> node_t* 
    {
        return this->active().current();
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::active
        () -> tree_iterator_t&
    {
        return active_it::first == activeIterator_ ? T1Iterator_ : T2Iterator_;
    }

    template<class T, class Compare, class Allocator, bool IsConst>
    auto brodal_queue_iterator<T, Compare, Allocator, IsConst>::active
        () const -> tree_iterator_t const&
    {
        return active_it::first == activeIterator_ ? T1Iterator_ : T2Iterator_;
    }

// brodal_entry_handle definition:

    template<class T, class Compare, class Allocator>
    brodal_entry_handle<T, Compare, Allocator>::brodal_entry_handle
        (entry_t* const entry) :
        entry_ {entry}
    {
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry_handle<T, Compare, Allocator>::operator*
        () -> T&
    {
        return **entry_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry_handle<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return **entry_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry_handle<T, Compare, Allocator>::operator->
        () -> T*
    {
        return std::addressof(**entry_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_entry_handle<T, Compare, Allocator>::operator->
        () const -> T const*
    {
        return std::addressof(**entry_);
    }

// root_wrap definition:

    template<class Tree, class T, class Compare, class Allocator>
    root_wrap<Tree, T, Compare, Allocator>::root_wrap
        (queue_t* const queue) :
        queue_ {queue}
    {
    }

    template<class Tree, class T, class Compare, class Allocator>
    root_wrap<Tree, T, Compare, Allocator>::root_wrap
        (queue_t* queue, root_wrap const& other) :
        queue_ {queue},
        upper_ {up_reducer_t {this}, other.upper_},
        lower_ {low_reducer_t {this}, other.lower_},
        sons_  {other.sons_}
    {
    }

    template<class Tree, class T, class Compare, class Allocator>
    root_wrap<Tree, T, Compare, Allocator>::root_wrap
        (queue_t* queue, root_wrap&& other) noexcept:
        queue_ {queue},
        upper_ {up_reducer_t {this}, std::move(other.upper_)},
        lower_ {low_reducer_t {this}, std::move(other.lower_)},
        sons_  {std::move(other.sons_)}
    {
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::operator=
        (root_wrap&& other) -> root_wrap&
    {
        root_ = other.root_;
        other.root_ = nullptr;

        upper_ = std::move(other.upper_);
        lower_ = std::move(other.lower_);
        sons_  = std::move(other.sons_);
 
        return *this;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::operator=
        (wrap_h other) -> root_wrap&
    {
        root_ = other.root_;
        other.root_ = nullptr;

        upper_ = other.upper();
        lower_ = other.lower();
        sons_  = other.sons();
 
        return *this;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_child
        (node_t* const child) -> void
    {
        return static_cast<Tree*>(this)->add_child(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::remove_child
        (node_t* const child) -> void
    {
        return static_cast<Tree*>(this)->remove_child(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_child_base
        (node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            queue_->T1_.remove_violation(child);
        }

        sons_[child->rank_]->add_right_sibling(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_delinked_nodes
        (delinked_nodes<T, Compare, Allocator> const& nodes) -> void
    {
        this->add_child(nodes.first);
        this->add_child(nodes.second);
        if (nodes.third)
        {
            this->add_child(nodes.third);
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_delinked_nodes_checked
        (delinked_nodes<T, Compare, Allocator> const& nodes) -> void
    {
        this->add_child_checked(nodes.first);
        this->add_child_checked(nodes.second);
        if (nodes.third)
        {
            this->add_child_checked(nodes.third);
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::remove_child_base
        (node_t* const child) -> void
    {
        if (sons_[child->rank_] == child)
        {
            sons_[child->rank_] = child->right_;
        }

        root_->remove_child(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_child_checked
        (node_t* const child) -> void
    {
        this->add_child(child);
        this->upper_check(child->rank_);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::release_root
        () -> node_t*
    {
        auto const ret = root_;
        root_ = nullptr;
        return ret;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check
        (rank_t const rank) -> void
    {
        if (root_->rank_ > 2 && rank < root_->rank_ - 2)
        {
            this->upper_.inc(rank);
        }

        this->upper_check_n_minus_2();
        this->upper_check_n_minus_1();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check
        (rank_t const rank) -> void
    {
        if (root_->rank_ > 2 && rank < root_->rank_ - 2)
        {
            lower_.inc(rank);
        }

        this->lower_check_n_minus_2(2);
        this->lower_check_n_minus_1();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::reduce_upper
        (rank_t const rank) -> void
    {
        this->add_child(this->link_children(rank));
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::reduce_lower
        (rank_t const rank) -> void
    {
        auto const delinked = this->delink_child(rank + 1);
        this->add_delinked_nodes(delinked);
        queue_->extraNodes_.push(delinked.extra);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::increase_rank
        (node_t* const n1, node_t* const n2) -> void
    {
        static_cast<Tree*>(this)->increase_rank(n1, n2);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_rank
        () -> void
    {
        static_cast<Tree*>(this)->decrease_rank();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::increase_rank_base
        (node_t* const n1, node_t* const n2) -> void
    {
        root_->add_child(n1);
        root_->add_child(n2);
        sons_.push_back(n2);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_rank_base
        () -> void
    {
        sons_.pop_back();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::increase_domain
        () -> void
    {
        static_cast<Tree*>(this)->increase_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_domain
        () -> void
    {
        static_cast<Tree*>(this)->decrease_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::increase_domain_base
        () -> void
    {
        if (root_->rank_ < 3)
        {
            return;
        }

        upper_.increase_domain();
        lower_.increase_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_domain_base() -> void
    {
        if (root_->rank_ < 3)
        {
            return;
        }

        upper_.decrease_domain();
        lower_.decrease_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check_n_minus_1
        () -> void
    {
        auto const rank = static_cast<rank_t>(root_->rank_ - 1);
        auto count      = node_t::same_rank_count(sons_[rank]);

        if (count <= 7)
        {
            return;
        }

        count -= this->lower_check_n_minus_2(3);

        if (count <= 7)
        {
            return;
        }

        auto const firstLinekd  = this->link_children(rank);
        auto const secondLinked = this->link_children(rank);

        this->increase_rank(firstLinekd, secondLinked);
        this->increase_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check_n_minus_2
        () -> void
    {
        if (root_->rank_ < 2)
        {
            return;
        }
        
        auto const rank  = static_cast<rank_t>(root_->rank_ - 2);
        auto const count = node_t::same_rank_count(sons_[rank]);
        
        if (count <= 7)
        {
            return;
        }

        this->reduce_upper(rank);
    }
    
    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check_n_minus_1
        () -> void
    {
        auto const rank  = static_cast<rank_t>(root_->rank_ - 1);
        auto const count = node_t::same_rank_count(sons_[rank]);

        if (count >= 2)
        {
            return;
        }

        // Case 1:
        if (root_->rank_ > 1)
        {
            auto const countNMinus2 = node_t::same_rank_count(sons_[rank - 1]);
            if (countNMinus2 >= 5)
            {
                this->reduce_upper(rank - 1);
                return;
            }
        }

        // Case 2:
        auto const node      = sons_[rank];
        auto const sonsCount = node_t::same_rank_count(node->child_);
        if (sonsCount >= 5)
        {
            auto const n1 = node->child_;
            auto const n2 = n1->right_;
            auto const n3 = n2->right_;
            if (n1->is_in_set()) queue_->T1_.remove_violation(n1);
            if (n2->is_in_set()) queue_->T1_.remove_violation(n2);
            if (n3->is_in_set()) queue_->T1_.remove_violation(n3);
            node->remove_child(n1);
            node->remove_child(n2);
            node->remove_child(n3);
            auto const linked = node_t::link_nodes(n1, n2, n3);
            this->add_child(linked);
            return;
        }

        // Case 3:
        this->decrease_domain();
        this->remove_child(node);
        this->decrease_rank();

        auto const delinked1 = node_t::delink_node(node);
        this->add_delinked_nodes(delinked1);
        if (node->rank_ == rank)
        {
            auto const delinked2 = node_t::delink_node(node);
            this->add_delinked_nodes(delinked2);
        }
        this->add_child_checked(node);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check_n_minus_2
        (num_t const bound) -> num_t
    {
        if (root_->rank_ < 2)
        {
            return 0;
        }

        auto const rank  = static_cast<rank_t>(root_->rank_ - 2);
        auto const count = node_t::same_rank_count(sons_[rank]);

        if (count >= bound)
        {
            return 0;
        }

        auto const delinked = this->delink_child(rank + 1);

        this->add_delinked_nodes(delinked);
        this->add_child(delinked.extra);
        auto const extraRank = delinked.extra->rank_;
        if (extraRank < rank)
        {
            upper_.inc(extraRank);
        }

        return extraRank != rank + 1 ? 1 : 0;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::link_children
        (rank_t const rank) -> node_t*
    {
        auto const n1 = sons_[rank];
        auto const n2 = n1->right_;
        auto const n3 = n2->right_;

        this->remove_child(n3);
        this->remove_child(n2);
        this->remove_child(n1);

        return node_t::link_nodes(n1, n2, n3);
    } 

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::delink_child
        (rank_t const rank) -> delinked_t
    {
        auto const toDelink = sons_[rank];
        this->remove_child(toDelink);
        return node_t::delink_node(toDelink);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::swap
        (root_wrap& rhs) noexcept -> void
    {
        using std::swap;
        swap(root_,  rhs.root_);
        swap(upper_, rhs.upper_);
        swap(lower_, rhs.lower_);
        swap(sons_,  rhs.sons_);
    }

// t1_wrap definition:

    template<class T, class Compare, class Allocator>
    t1_wrap<T, Compare, Allocator>::t1_wrap
        (queue_t* const queue) :
        base_t {queue}
    {
    }

    template<class T, class Compare, class Allocator>
    t1_wrap<T, Compare, Allocator>::t1_wrap
        (queue_t* const queue, t1_wrap const& other) :
        base_t     {queue, other},
        violation_ {viol_reducer_t {this}, other.violation_},
        auxW_      {other.auxW_}
    {
    }

    template<class T, class Compare, class Allocator>
    t1_wrap<T, Compare, Allocator>::t1_wrap
        (queue_t* const queue, t1_wrap&& other) noexcept :
        base_t     {queue, std::move(other)},
        violation_ {viol_reducer_t {this}, std::move(other.violation_)},
        auxW_      {std::move(other.auxW_)}
    {
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::operator=
        (t1_wrap&& other) -> t1_wrap&
    {
        base_t::operator=(std::move(other));

        violation_ = std::move(other.violation_);
        auxW_      = std::move(other.auxW_);

        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::add_child
        (node_t* const child) -> void
    {
        base_t::add_child_base(child);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_child
        (node_t* const child) -> void
    {
        base_t::remove_child_base(child);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::add_violation
        (node_t* const node) -> void
    {
        if (node->is_in_set())
        {
            this->remove_violation(node);
        }

        if (node->rank_ >= base_t::root_->rank_)
        {
            base_t::root_->add_to_V(node);
        }
        else if (auxW_[node->rank_])
        {
            auxW_[node->rank_]->add_set_sibling(node);
        }
        else
        {
            auxW_[node->rank_] = node;
            base_t::root_->add_to_W(node);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_violation
        (node_t* const node) -> void
    {
        if (node->rank_ < auxW_.size())
        {
            if (auxW_[node->rank_] == node)
            {
                node_t* const next {node->nextInSet_};
                auxW_[node->rank_] = (next and next->rank_ == node->rank_ ? next : nullptr);
            }
        }

        node_t::remove_from_set(node);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::violation_check
        (rank_t const rank) -> void
    {
        if (rank < auxW_.size())
        {
            violation_.inc(rank);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_violation
        (rank_t const rank) -> void
    {
        auto removed = num_t {0};
        while (removed < 2)
        {
            removed += this->reduce_violations(rank);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_all_violations
        () -> void
    {
        for (auto const node : auxW_)
        {
            if (!node)
            {
                continue;
            }

            auto const rank = node->rank_;
            auto count      = node_t::same_rank_violation(node);
            while (count > 1)
            {
                count -= this->reduce_violations(rank);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::increase_rank
        (node_t* const linked) -> void
    {
        node_t* const n1    {linked};
        node_t* const n2    {n1->right_};
        node_t* const extra {n2->right_};

        this->increase_rank(n1->disconnect(), n2->disconnect());
        this->increase_domain();

        node_t::fold_right(extra, [=](node_t* const n) {
            this->add_child_checked(n->disconnect());
        });
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::increase_rank
        (node_t* const n1, node_t* const n2) -> void
    {
        base_t::increase_rank_base(n1, n2);
        auxW_.push_back(nullptr);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::decrease_rank
        () -> void
    {
        base_t::decrease_rank_base();
        auxW_.pop_back();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::increase_domain
        () -> void
    {
        base_t::increase_domain_base();
        violation_.increase_domain();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::decrease_domain
        () -> void
    {
        base_t::decrease_domain_base();
        violation_.decrease_domain();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_violations
        (rank_t const rank) -> num_t
    {
        auto const normal = this->pick_normal_violations(rank);
        auto const t2sons = this->pick_t2_son_violations(rank);

        auto const t2SonsRemoved 
            = static_cast<num_t>( this->remove_t2_violation(t2sons.first) 
                                + this->remove_t2_violation(t2sons.second) );

        if (2 == t2SonsRemoved)
        {
            return 2;
        }

        return this->remove_normal_violations(normal.first, normal.second);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::pick_normal_violations
        (rank_t const rank) -> node_ptr_pair
    {
        auto it     = auxW_[rank];
        auto first  = static_cast<node_t*>(nullptr);
        auto second = static_cast<node_t*>(nullptr);

        while (it && it->rank_ == rank)
        {
            if (!it->is_son_of_root())
            {
                if      (!first)  first  = it;
                else if (!second) second = it;
                else return std::make_pair(first, second);
            }
            
            it = it->nextInSet_;
        }

        return std::make_pair(first, second);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::pick_t2_son_violations
        (rank_t const rank) -> node_ptr_pair
    {
        auto count = num_t {0};
        auto it     = auxW_[rank];
        auto first  = static_cast<node_t*>(nullptr);
        auto second = static_cast<node_t*>(nullptr);

        while (it && it->rank_ == rank)
        {
            if (it->is_son_of_root())
            {
                count++;
                if (count > 4)
                {
                    if (not first) first = it;
                    else if (not second) second = it;
                }
            }
            
            it = it->nextInSet_;
        }

        return std::make_pair(first, second);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_t2_violation
        (node_t* const node) -> num_t
    {
        if (!node)
        {
            return 0;
        }

        if (!node->is_violating())
        {
            this->remove_violation(node);
            return 1;
        }

        this->remove_violation(node);
        base_t::queue_->T2_.remove_child(node);
        // No need for lower check since this transformation is 
        // done only if there are more than 4 sons with given rank.
        // So removing the node won't affect lower guide.
        base_t::queue_->T1_.add_child_checked(node);

        return 1;
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_normal_violations
        (node_t* const first, node_t* const second) -> num_t
    {
        auto const n1 = first;
        auto const n2 = second ? second : n1->same_rank_sibling();

        if (!n1->is_violating())
        {
            this->remove_violation(n1);
            return 1;
        }

        auto const removed = static_cast<num_t>(n2 != nullptr ? 2 : 1);

        if (!node_t::are_siblings(n1, n2))
        {
            node_t::make_siblings(n1, n2);
        }

        auto const siblingCount = node_t::same_rank_count(n1);
        auto const parent       = n1->parent_;
        if (siblingCount > 2)
        {
            parent->remove_child(n1);
            this->add_child_checked(n1);
            return 1;
        }

        auto const removeParent = parent->rank_ == n1->rank_ + 1;
        if (removeParent)
        {
            if (parent->parent_ != base_t::root_)
            {
                auto const replacement = base_t::sons_[parent->rank_]->right_;
                node_t::swap_tree_nodes(parent, replacement);
                if (replacement->is_violating())
                {
                    this->add_violation(replacement);
                }
            }
            if (parent->is_in_set())
            {
                // Parent will no longer be violating and his rank will change.
                this->remove_violation(parent);
            }
            this->remove_child(parent);
            this->lower_check(parent->rank_);
            base_t::queue_->add_extra_nodes();
        }
        
        parent->remove_child(n1);
        parent->remove_child(n2);
        
        this->add_child_checked(n1);
        this->add_child_checked(n2);

        if (removeParent)
        {
            this->add_child_checked(parent);
        }

        return removed;
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::swap
        (t1_wrap& rhs) noexcept -> void
    {
        using std::swap;
        base_t::swap(rhs);
        swap(auxW_,      rhs.auxW_);
        swap(violation_, rhs.violation_);
    }

    template<class T, class Compare, class Allocator>
    auto swap
        ( t1_wrap<T, Compare, Allocator>& lhs
        , t1_wrap<T, Compare, Allocator>& rhs ) noexcept -> void
    {
        lhs.swap(rhs);
    }

// t2_wrap definition:

    template<class T, class Compare, class Allocator>
    t2_wrap<T, Compare, Allocator>::t2_wrap
        (queue_t* const queue) :
        base_t {queue}
    {
    }

    template<class T, class Compare, class Allocator>
    t2_wrap<T, Compare, Allocator>::t2_wrap
        (queue_t* const queue, t2_wrap const& other) :
        base_t {queue, other}
    {
    }

    template<class T, class Compare, class Allocator>
    t2_wrap<T, Compare, Allocator>::t2_wrap
        (queue_t* const queue, t2_wrap&& other) noexcept :
        base_t {queue, std::move(other)}
    {
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::operator=
        (t2_wrap&& other) -> t2_wrap&
    {
        base_t::operator=(std::move(other));
        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::operator=
        (wrap_h other) -> t2_wrap&
    {
        base_t::operator=(other);
        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::add_child
        (node_t* const child) -> void
    {
        base_t::add_child_base(child);
        if (child->is_violating())
        {
            base_t::queue_->violations_.push(child);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::remove_child
        (node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            base_t::queue_->T1_.remove_violation(child);
        }

        base_t::remove_child_base(child);
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::increase_rank
        (node_t* const n1, node_t* const n2) -> void
    {
        base_t::increase_rank_base(n1, n2);
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::decrease_rank
        () -> void
    {
        base_t::decrease_rank_base();
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::increase_domain
        () -> void
    {
        base_t::increase_domain_base();
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::decrease_domain
        () -> void
    {
        base_t::decrease_domain_base();
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::removeLargeSons
        () -> node_t*
    {
        auto const newRootsRank = static_cast<rank_t>(base_t::root_->rank_ - 1);
        auto const ret          = base_t::sons_[base_t::root_->rank_ - 1];

        if (0 == newRootsRank)
        {
            base_t::root_->child_ = nullptr;
            base_t::root_->rank_  = 0; 
        }
        else
        {
            base_t::root_->child_                = base_t::sons_[base_t::root_->rank_ - 2];
            base_t::root_->child_->left_->right_ = nullptr;
            base_t::root_->child_->left_         = nullptr;
            base_t::root_->set_rank();
        }

        node_t::fold_right(ret, [=](auto const n)
        {
            if (n->is_in_set())
            {
                base_t::queue_->T1_.remove_violation(n);
            }
        });

        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::swap
        (t2_wrap& rhs) noexcept -> void
    {
        base_t::swap(rhs);
    }

    template<class T, class Compare, class Allocator>
    auto swap
        ( t2_wrap<T, Compare, Allocator>& lhs
        , t2_wrap<T, Compare, Allocator>& rhs ) noexcept -> void
    {
        lhs.swap(rhs);
    }

// upper_reducer definition:

    template<class Reducible, class T, class Compare, class Allocator>
    upper_reducer<Reducible, T, Compare, Allocator>::upper_reducer
        (Reducible* const root) :
        managedRoot_ {root}
    {
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto upper_reducer<Reducible, T, Compare, Allocator>::reduce
        (index_t i) -> void
    {
        managedRoot_->reduce_upper(i);
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto upper_reducer<Reducible, T, Compare, Allocator>::get_num
        (index_t i) const -> num_t
    {
        auto const count = node_t::same_rank_count(managedRoot_->sons_[i]);
        return count < 6 ? 0 : count - 5;
    }

// lower_reducer definition:

    template<class Reducible, class T, class Compare, class Allocator>
    lower_reducer<Reducible, T, Compare, Allocator>::lower_reducer
        (Reducible* const root) :
        managedRoot_ {root}
    {
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto lower_reducer<Reducible, T, Compare, Allocator>::reduce
        (index_t const i) -> void
    {
        managedRoot_->reduce_lower(i);
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto lower_reducer<Reducible, T, Compare, Allocator>::get_num
        (index_t const i) const -> num_t
    {
        auto const count = node_t::same_rank_count(managedRoot_->sons_[i]);
        return count >= 4 ? 0 : 4 - count;
    }

// violation_reducer definition:

    template<class Reducible, class T, class Compare, class Allocator>
    violation_reducer<Reducible, T, Compare, Allocator>::violation_reducer
        (Reducible* const root) :
        managedRoot_ {root}
    {
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto violation_reducer<Reducible, T, Compare, Allocator>::reduce
        (index_t const i) -> void
    {
        managedRoot_->reduce_violation(i);
    }

    template<class Reducible, class T, class Compare, class Allocator>
    auto violation_reducer<Reducible, T, Compare, Allocator>::get_num
        (index_t const i) const -> num_t
    {
        if (!managedRoot_->auxW_[i]) 
        {
            return 0;
        }

        auto const count = node_t::same_rank_violation(managedRoot_->auxW_[i]);
        return count < 5 ? 0 : count - 4;
    }

// brodal_queue definition:

    template<class T, class Compare, class Allocator>
    brodal_queue<T, Compare, Allocator>::brodal_queue
        (Allocator const& alloc) :
        size_      {0},
        T1_             {this},
        T2_             {this},
        nodeAllocator_  {alloc},
        entryAllocator_ {alloc}
    {
    }

    template<class T, class Compare, class Allocator>
    brodal_queue<T, Compare, Allocator>::brodal_queue
        (brodal_queue const& other) :
        size_      {other.size_},
        T1_             {this, other.T1_},
        T2_             {this, other.T2_},
        nodeAllocator_  {other.nodeAllocator_},
        entryAllocator_ {other.entryAllocator_}
    {
        auto const map = this->shallow_copy_nodes(other);
        this->deep_copy_tree(map);
        this->deep_copy_violations(map);
        this->deep_copy_wraps(other, map);
    }

    template<class T, class Compare, class Allocator>
    brodal_queue<T, Compare, Allocator>::brodal_queue
        (brodal_queue&& other) noexcept :
        size_      {std::exchange(other.size_, 0)},
        T1_             {this, std::move(other.T1_)},
        T2_             {this, std::move(other.T2_)},
        nodeAllocator_  {std::move(other.nodeAllocator_)},
        entryAllocator_ {std::move(other.entryAllocator_)}
    {
    }

    template<class T, class Compare, class Allocator>
    brodal_queue<T, Compare, Allocator>::~brodal_queue
        ()
    {
        auto it  = this->begin();
        auto end = this->end();

        while (it != end)
        {
            this->delete_node(it.current());
            ++it;
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::operator=
        (brodal_queue rhs) -> brodal_queue&
    {
        swap(*this, rhs);
        return *this;
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto brodal_queue<T, Compare, Allocator>::emplace
        (Args&&... args) -> handle_t
    {
        return this->insert_impl(this->new_node(std::forward<Args>(args)...));
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert
        (value_type const& value) -> handle_t
    {
        return this->insert_impl(this->new_node(value));
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert
        (value_type&& value) -> handle_t
    {
        return this->insert_impl(this->new_node(std::move(value)));
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::delete_min
        () -> void
    {
        if (this->size() < 4)
        {
            this->delete_min_special();
            return;
        }

        this->move_all_to_T1();

        auto const oldRoot = T1_.root_;
        auto const newRoot = this->find_new_root(); 
        if (newRoot->is_in_set())
        {
            T1_.remove_violation(newRoot);
        }
        
        if (!newRoot->is_son_of_root())
        {
            this->make_son_of_root(newRoot);
        }

        T1_.remove_child(newRoot);
        T1_.lower_check(newRoot->rank_);
        this->add_extra_nodes();

        auto const sonsToAdd = newRoot->disconnect_sons();
        node_t::swap_entries(newRoot, oldRoot);
        node_t::fold_right(sonsToAdd, [this](auto const n) 
        {
            this->T1_.add_child_checked(n->disconnect());
        });

        this->merge_sets(newRoot);
        T1_.reduce_all_violations();
        this->delete_node(newRoot);
        --size_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::find_min
        () -> reference
    {
        this->is_empty_check();
        this->move_to_T1();
        return **T1_.root_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::find_min
        () const -> const_reference
    {
        this->is_empty_check();
        return **T1_.root_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::decrease_key
        (handle_t const handle) -> void
    {
        this->dec_key_impl(handle.entry_->node_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::decrease_key
        (iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::decrease_key
        (const_iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::erase
        (handle_t const handle) -> void
    {
        this->erase_impl(handle.entry_->node_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::erase
        (iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::erase
        (const_iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::swap
        (brodal_queue& rhs) noexcept -> void
    {
        using std::swap;
        swap(size_, rhs.size_);
        swap(T1_,   rhs.T1_);
        swap(T2_,   rhs.T2_);

        if constexpr (node_alloc_traits::propagate_on_container_swap::value)
        {
            swap(nodeAllocator_, rhs.nodeAllocator_);
        }

        if constexpr (entry_alloc_traits::propagate_on_container_swap::value)
        {
            swap(entryAllocator_, rhs.entryAllocator_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::size
        () const -> size_type
    {
        return size_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::max_size
        () const -> size_type
    {
        return std::numeric_limits<size_type>::max();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::empty
        () const -> bool
    {
        return 0 == this->size();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::clear
        () -> void
    {
        *this = brodal_queue {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::begin
        () -> iterator
    {
        return iterator {T1_.root_, T2_.root_};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::end() -> iterator
    {
        return iterator {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::begin
        () const -> const_iterator
    {
        return const_iterator {T1_.root_, T2_.root_};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::end
        () const -> const_iterator
    {
        return const_iterator {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::cbegin
        () const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->begin();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::cend
        () const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->end();
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto brodal_queue<T, Compare, Allocator>::new_node
        (Args&&... args) -> node_t*
    {
        auto const entry = this->new_entry(std::forward<Args>(args)...) ;
        auto const node  = node_alloc_traits::allocate(nodeAllocator_, 1);
        node_alloc_traits::construct(nodeAllocator_, node, entry);
        return node;
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto brodal_queue<T, Compare, Allocator>::new_entry
        (Args&&... args) -> entry_t*
    {
        auto entry = entry_alloc_traits::allocate(entryAllocator_, 1);
        entry_alloc_traits::construct(entryAllocator_, entry, std::forward<Args>(args)...);
        return entry;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::shallow_copy_node
        (node_t* const node) -> node_t*
    {
        auto const entry = this->new_entry(**node) ;
        auto const cnode = node_alloc_traits::allocate(nodeAllocator_, 1);
        node_alloc_traits::construct(nodeAllocator_, cnode, entry, *node);
        return cnode;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::delete_node
        (node_t* const node) -> void
    {
        this->delete_entry(node->entry_);
        node_alloc_traits::destroy(nodeAllocator_, node);
        node_alloc_traits::deallocate(nodeAllocator_, node, 1);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::delete_entry
        (entry_t* const entry) -> void
    {
        entry_alloc_traits::destroy(entryAllocator_, entry);
        entry_alloc_traits::deallocate(entryAllocator_, entry, 1);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert_impl
        (node_t* const node) -> handle_t
    {
        if (this->size() < 3)
        {
            return this->insert_special_impl(node);
        }
        
        auto const entry = node->entry_;

        ++size_;

        if (*node < *T1_.root_)
        {
            node_t::swap_entries(node, T1_.root_);
        }

        T1_.add_child_checked(node);

        this->move_to_T1();

        return handle_t(entry);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert_special_impl
        (node_t* const node) -> handle_t
    {
        ++size_;

        auto const entry = node->entry_;

        if (1 == this->size())
        {
            T1_.root_ = node;
            return handle_t(entry);
        }
        else if (*node < *T1_.root_)
        {
            node_t::swap_entries(node, T1_.root_);
        }

        if (2 == this->size())
        {
            T2_.root_ = node;
        }
        else
        {
            auto const oldT2 = T2_.root_;
            T2_.root_ = nullptr;
            T1_.increase_rank(node, oldT2);
            T1_.increase_domain();
        }

        return handle_t(entry);
    }

    template<class T, class Compare, class Allocator>
    template<class Cmp>
    auto brodal_queue<T, Compare, Allocator>::dec_key_impl
        (node_t* const node) -> void
    {
        this->move_to_T1(); 

        if (Cmp {} (**node, **T1_.root_))
        {
            node_t::swap_entries(node, T1_.root_);
        }

        if (node->is_violating() and not node->is_in_set())
        {
            T1_.add_violation(node);
            T1_.violation_check(node->rank_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::erase_impl
        (node_t* const node) -> void
    {
        node_t::swap_entries(node, T1_.root_);

        if (node->parent_ && T1_.root_ != node->parent_ && !node->is_in_set())
        {
            // At this point node can be violating even if it is son of t1.
            // If that is the case we can't add it to violation set.
            // It would break delete_min since sons of t1 cannot be violating.
            T1_.add_violation(node);
        }

        this->delete_min();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::delete_min_special
        () -> void
    {
        auto const oldRoot = T1_.root_;

        if (1 == size_)
        {
            T1_.root_ = nullptr;
        }
        else if (2 == size_)
        {
            T1_.root_ = T2_.root_;
            T2_.root_ = nullptr;
        }
        else
        {
            auto const firstChild  = T1_.root_->disconnect_sons();
            auto const secondChild = firstChild->right_;

            firstChild->disconnect();
            secondChild->disconnect();

            if (*firstChild < *secondChild)
            {
                T1_.root_ = firstChild;
                T2_.root_ = secondChild;
            }
            else
            {
                T1_.root_ = secondChild;
                T2_.root_ = firstChild;
            }

            T1_.decrease_rank();
            T1_.decrease_domain();
        }

        this->delete_node(oldRoot);
        --size_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::add_extra_nodes
        () -> void
    {
        while (!extraNodes_.empty())
        {
            auto const node = extraNodes_.top();
            extraNodes_.pop();
            if (node->rank_ < T1_.root_->rank_)
            {
                T1_.add_child_checked(node);
            }
            else
            {
                T2_.add_child_checked(node);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::add_violations
        () -> void
    {
        while (!violations_.empty())
        {
            auto const violation = violations_.top();
            violations_.pop();
            T1_.add_violation(violation);
            T1_.violation_check(violation->rank_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::is_empty_check
        () -> void
    {
        if (this->empty())
        {
            throw std::out_of_range("Priority queue is empty.");
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::move_all_to_T1
        () -> void
    {
        if (!T2_.root_)
        {
            return;
        }

        // Disconnect sons of t2.
        auto const oldt2         = T2_.root_;
        auto const rightFoldable = T2_.sons_[0];
        auto const leftFoldable  = rightFoldable->left_;
        oldt2->disconnect_sons();
        // Reset t2 wrap to defaults.
        T2_ = t2_wrap {this};

        // Add sons with rank 0 under t1.
        node_t::fold_right(rightFoldable, [=](auto const n)
        {
            T1_.add_child_checked(n->disconnect());
        });

        // Add other sons with ranks > 0 under t1.
        node_t::fold_left(leftFoldable, [=](auto const n)
        {
            n->disconnect();
            
            while (n->rank_ >= T1_.root_->rank_)
            {
                auto const delinked = node_t::delink_node(n);
                T1_.add_delinked_nodes_checked(delinked);
            }
            
            T1_.add_child_checked(n);
        });

        // Finally add t2 as son of t1.
        T1_.add_child_checked(oldt2);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::move_to_T1
        () -> void
    {
        if (!T2_.root_ || this->size() < 4)
        {
            return;
        }

        if (T2_.root_->rank_ <= T1_.root_->rank_ + 2)
        {
            // r(t2) is (r(t1) + 1) or (r(t1) + 2)
            auto const sons1 = T2_.removeLargeSons();
            if (T2_.root_->rank_ > T1_.root_->rank_)
            {
                auto const sons2 = T2_.removeLargeSons();
                T1_.increase_rank(sons2);
            }
            T1_.increase_rank(sons1);

            auto const oldt2 = T2_.release_root();
            T1_.add_child_checked(oldt2);

            // Reset t2 wrap to defaults. 
            T2_ = t2_wrap {this};
        }
        else
        {
            auto const t1rank   = T1_.root_->rank_;
            auto const toDelink = T2_.sons_[t1rank + 1];
            T2_.remove_child(toDelink);
            T2_.lower_check(toDelink->rank_);

            auto const delinked = node_t::delink_node(toDelink);
            T1_.increase_rank(delinked.first, delinked.second);
            T1_.increase_domain();
            if (delinked.third)
            {
                T1_.add_child_checked(delinked.third);
            }
            
            // In case toDelink had more than 3 sons of rank (n-1).
            while (toDelink->rank_ == T1_.root_->rank_)
            {
                auto const delinked = node_t::delink_node(toDelink);
                T1_.add_delinked_nodes_checked(delinked);
            }

            T1_.add_child_checked(toDelink);

            // These might have been created while doing
            // delinking under t2.
            this->add_extra_nodes();
            this->add_violations();
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::find_new_root
        () const -> node_t*
    {
        auto newRoot = T1_.root_->child_;

        node_t::fold_right(newRoot, [&](auto const n)
        {
            if (*n < *newRoot)
            {
                newRoot = n;
            }
        });

        node_t::fold_next(T1_.root_->setW_, [&](auto const n)
        {
            if (*n < *newRoot)
            {
                newRoot = n;
            }
        });

        node_t::fold_next(T1_.root_->setV_, [&](auto const n)
        {
            if (*n < *newRoot)
            {
                newRoot = n;
            }
        });

        return newRoot;
    } 

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::make_son_of_root
        (node_t* const newRoot) -> void
    {
        auto const swapped = T1_.sons_[newRoot->rank_]->right_;
        node_t::swap_tree_nodes(newRoot, swapped);
        if (swapped->is_violating())
        {
            T1_.add_violation(swapped);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::merge_sets
        (node_t* const newRoot) -> void
    {
        node_t::fold_next(T1_.root_->setV_, [=](auto const n)
        {
            this->T1_.add_violation(n);
        });

        node_t::fold_next(newRoot->setW_, [=](auto const n)
        {
            this->T1_.add_violation(n);
        });

        node_t::fold_next(newRoot->setV_, [=](auto const n)
        {
            this->T1_.add_violation(n);
        });
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::shallow_copy_nodes
        (brodal_queue const& other) -> node_map
    {
        auto map = node_map {};
        auto it  = std::begin(other);
        auto end = std::end(other);

        while (it != end)
        {
            auto const node     = it.current();
            auto const nodeCopy = this->shallow_copy_node(node);
            map.emplace(node, nodeCopy);
            ++it;
        }
        map.emplace(nullptr, nullptr);

        return map;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::deep_copy_tree
        (node_map const& map) -> void
    {
        for (auto [original, copy] : map)
        {
            if (copy) [[likely]]
            {
                copy->parent_ = map.at(original->parent_);
                copy->left_   = map.at(original->left_);
                copy->right_  = map.at(original->right_);
                copy->child_  = map.at(original->child_);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::deep_copy_violations
        (node_map const& map) -> void
    {
        for (auto [original, copy] : map)
        {
            if (copy) [[likely]]
            {
                copy->nextInSet_ = map.at(original->nextInSet_);
                copy->prevInSet_ = map.at(original->prevInSet_);
                copy->setW_      = map.at(original->setW_);
                copy->setV_      = map.at(original->setV_);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::deep_copy_wraps
        (brodal_queue const& other, node_map const& map) -> void
    {
        T1_.root_ = map.at(other.T1_.root_);
        T2_.root_ = map.at(other.T2_.root_);

        for (auto& son : T1_.sons_)
        {
            son = map.at(son);
        }

        for (auto& son : T2_.sons_)
        {
            son = map.at(son);
        }

        for (auto& violation : T1_.auxW_)
        {
            violation = map.at(violation);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::dumbMeld(brodal_queue & first, brodal_queue & second) -> brodal_queue
    {
        brodal_queue<T, Compare, Allocator> newQueue {};

        while (not first.empty())
        {
            newQueue.insert(first.delete_min());
        }

        while (not second.empty())
        {
            newQueue.insert(second.delete_min());
        }

        return newQueue;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::findNewT1(brodal_queue & first, brodal_queue & second) -> t1_wrap_t&
    {
        if (not first.T1_.root_)  return second.T1_;
        if (not second.T1_.root_) return first.T1_;

        return *first.T1_.root_ < *second.T1_.root_ ? first.T1_ : second.T1_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::findNewT2(brodal_queue& first, brodal_queue& second, t1_wrap_t& t1) -> wrap_h
    {
        auto newT2 = wrap_h {};
        newT2 = t1;

        if (first.T1_.root_ and first.T1_.root_->rank > newT2->root()->rank)
        {
            newT2 = first.T1_;
        }

        if (first.T2_.root_ and first.T2_.root_->rank > newT2->root()->rank)
        {
            newT2 = first.T2_;
        }

        if (second.T1_.root_ and second.T1_.root_->rank > newT2->root()->rank)
        {
            newT2 = second.T1_;
        }

        if (second.T2_.root_ and second.T2_.root_->rank > newT2->root()->rank)
        {
            newT2 = second.T2_;
        }

        return newT2;
    }

    template<class T, class Compare, class Allocator>
    template<class RootWrap>
    auto brodal_queue<T, Compare, Allocator>::addUnderRoot(RootWrap& root, RootWrap& toAdd) -> void
    {
        if (not toAdd.root_)
        {
            return;
        }
        
        node_t const* node {toAdd.release_root()};

        while (node->rank_ == root.root_->rank)
        {
            const delinked_nodes delinked {node_t::delink_node(node)};
            root.add_delinked_nodes_checked(delinked);
        }

        root.add_child_checked(node);
    }

    template<class T, class Compare, class Allocator>
    auto meld(brodal_queue<T, Compare, Allocator>& first, brodal_queue<T, Compare, Allocator>& second) -> brodal_queue<T, Compare, Allocator>
    {
        using brodal_queue_t = brodal_queue<T, Compare, Allocator>;
        
        if (first.empty() and second.empty())
        {
            return brodal_queue_t {};
        }

        auto& newt1 {brodal_queue_t::findNewT1(first, second)};
        auto& newt2 {brodal_queue_t::findNewT2(first, second, newt1)};

        if (0 == newt2.root_->rank)
        {
            return brodal_queue_t::dumbMeld(first, second);
        }

        brodal_queue_t newQueue {};
        newQueue.size_ = first.size_ + second.size_;
        newQueue.T1_        = std::move(newt1);

        if (&newt1 == &newt2)
        {
            brodal_queue_t::addUnderRoot(newQueue.T1_, first.T1_);
            brodal_queue_t::addUnderRoot(newQueue.T1_, first.T2_);
            brodal_queue_t::addUnderRoot(newQueue.T1_, second.T1_);
            brodal_queue_t::addUnderRoot(newQueue.T1_, second.T2_);
        }
        else
        {
            newQueue.T2_ = std::move(newt2);
            brodal_queue_t::addUnderRoot(newQueue.T2_, first.T1_);
            brodal_queue_t::addUnderRoot(newQueue.T2_, first.T2_);
            brodal_queue_t::addUnderRoot(newQueue.T2_, second.T1_);
            brodal_queue_t::addUnderRoot(newQueue.T2_, second.T2_);
            newQueue.add_violations();
        }

        first  = brodal_queue_t {};
        second = brodal_queue_t {};

        return newQueue;
    }

    template<class T, class Compare, class Allocator>
    auto swap
        ( brodal_queue<T, Compare, Allocator>& first
        , brodal_queue<T, Compare, Allocator>& second) noexcept -> void
    {
        first.swap(second);
    }

    template<class T, class Compare, class Allocator>
    auto operator==
        ( brodal_queue<T, Compare, Allocator> const& lhs
        , brodal_queue<T, Compare, Allocator> const& rhs) -> bool
    {
        return lhs.size() == rhs.size()
            && std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    template<class T, class Compare, class Allocator>
    auto operator!=
        ( brodal_queue<T, Compare, Allocator> const& lhs
        , brodal_queue<T, Compare, Allocator> const& rhs) -> bool
    {
        return ! (lhs == rhs);
    }

}

#endif