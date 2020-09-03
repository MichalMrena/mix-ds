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
        guide (guide const& other);
        guide (guide&& other);
        guide (Reducer reducer);
        guide (Reducer reducer, std::vector<std::shared_ptr<index_t>> blocks);

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
        brodal_entry (node_t* const node, Args&&... args);

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
        using entry_uptr = std::unique_ptr<entry_t>;
        using node_t     = brodal_node;
        using delinked_t = delinked_nodes<T, Compare, Allocator>;

    public:
        template<class... Args>
        brodal_node  (std::piecewise_construct_t, Args&&... args);
        brodal_node  (brodal_node const& other);
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
        rank_t     rank_      {0};
        entry_uptr entry_     {nullptr};
        node_t*    parent_    {nullptr};
        node_t*    left_      {nullptr};
        node_t*    right_     {nullptr};
        node_t*    child_     {nullptr};
        node_t*    nextInSet_ {nullptr};
        node_t*    prevInSet_ {nullptr};
        node_t*    setW_      {nullptr};
        node_t*    setV_      {nullptr};
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
        ~root_wrap ();
        root_wrap  (queue_t* const queue);
        auto operator= (root_wrap&& other) -> root_wrap&;
        auto operator= (wrap_h other)      -> root_wrap&;

        auto copyTree       (root_wrap const& other) -> void;
        auto copySons       (root_wrap const& other, node_map const& mapping) -> void;
        auto copyViolations (root_wrap const& other, node_map const& mapping) -> void;

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

        static auto swap (root_wrap& first, root_wrap& second) -> void;

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
        t1_wrap(queue_t* const queue);

        auto operator= (t1_wrap const& other) -> t1_wrap&;
        auto operator= (t1_wrap&& other)      -> t1_wrap&;

        auto copy_aux_violations (t1_wrap const& other, node_map const& mapping) -> void;

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

        static auto swap (t1_wrap& first, t1_wrap& second) -> void;

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

        static auto swap (t2_wrap & first, t2_wrap & second) -> void;
    };

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
    
// brodal_queue declaration:
   

    template<class T, class Compare, class Allocator>
    auto swap(brodal_queue<T, Compare, Allocator>& first, 
              brodal_queue<T, Compare, Allocator>& second) noexcept -> void;

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
        
        using num_t         = std::uint8_t;
        using index_t       = std::uint8_t;
        using rank_t        = index_t;
        using node_t        = brodal_node<T, Compare, Allocator>;
        using entry_t       = brodal_entry<T, Compare, Allocator>;
        using node_uptr     = std::unique_ptr<node_t>;
        using node_ptr_pair = std::pair<node_t*, node_t*>;
        using node_map      = std::unordered_map<node_t const*, node_t*>;
        using t1_wrap_t     = t1_wrap<T, Compare, Allocator>;
        using wrap_h        = root_wrap_holder<T, Compare, Allocator>;
        using type_alloc_traits  = std::allocator_traits<Allocator>;
        using node_alloc_traits  = typename type_alloc_traits::template rebind_traits<node_t>;
        using entry_alloc_traits = typename type_alloc_traits::template rebind_traits<entry_t>;
        
    public:
        brodal_queue  () = default;
        ~brodal_queue () = default;

        brodal_queue (brodal_queue const&);
        brodal_queue (brodal_queue &&);

        auto operator= (brodal_queue rhs) -> brodal_queue&;

        auto insert       (value_type const& value) -> handle_t;
        auto insert       (value_type&& value)      -> handle_t;
        auto delete_min   ()                        -> value_type;
        auto find_min     ()                        -> reference;
        auto find_min     () const                  -> const_reference;
        auto decrease_key (handle_t const handle)   -> void;
        auto decrease_key (iterator pos)            -> void;
        auto decrease_key (const_iterator pos)      -> void;
        auto erase        (handle_t const handle)   -> void;
        auto erase        (iterator pos)            -> void;
        auto erase        (const_iterator pos)      -> void;
        auto size         () const                  -> size_type;
        auto max_size     () const                  -> size_type;
        auto empty        () const                  -> bool;
        auto clear        ()                        -> void;

        auto begin  () -> iterator;
        auto end    () -> iterator;
        auto begin  () const -> const_iterator;
        auto end    () const -> const_iterator;
        auto cbegin () const -> const_iterator;
        auto cend   () const -> const_iterator;

        friend auto meld<T, Compare, Allocator>(brodal_queue & first, 
                                     brodal_queue & second) -> brodal_queue;
        friend auto swap<T, Compare, Allocator>(brodal_queue<T, Compare, Allocator> & first, 
                                     brodal_queue<T, Compare, Allocator> & second) noexcept -> void;

    public:
        template<class... Args>
        auto new_node (Args&&... args) -> node_t*;

        template<class... Args>
        auto new_node_impl (Args&&... args) -> node_t*;

        auto insert_impl (node_t* const node) -> handle_t;
        auto insert_special_impl (node_t* const node) -> handle_t;

        template<class Cmp = Compare>
        auto dec_key_impl (node_t* const node) -> void;

        auto erase_impl (node_t* const node) -> void;

        auto deleteMinSpecial () -> value_type;

        auto addExtraNodes () -> void;
        auto addViolations () -> void;

        auto isEmptyCheck  () -> void;
        auto moveAllToT1   () -> void;
        auto moveToT1      () -> void;
        auto findNewRoot   () const -> node_t*;
        auto makeSonOfRoot (node_t* const newRoot) -> void;
        auto mergeSets     (node_t* const newRoot) -> void;

        static auto makeNodeMapping (brodal_queue const& original, brodal_queue const& copy) -> node_map;
        
        template<class RootWrap>
        static auto nodeMappingOfTree (RootWrap const& original, RootWrap const& copy, node_map& map) -> void;
        
        static auto dumbMeld  (brodal_queue& first, brodal_queue& second) -> brodal_queue;
        static auto findNewT1 (brodal_queue& first, brodal_queue& second) -> t1_wrap_t&;
        static auto findNewT2 (brodal_queue& first, brodal_queue& second, t1_wrap_t& t1) -> wrap_h;
        
        template<class RootWrap>
        static auto addUnderRoot (RootWrap& root, RootWrap& toAdd) -> void;

    public:
        std::size_t         queueSize {0};
        t1_wrap<T, Compare, Allocator> T1        {this};
        t2_wrap<T, Compare, Allocator> T2        {this};
        std::stack<node_t*> extraNodes;
        std::stack<node_t*> violations;
    };

    template<class T, class Compare, class Allocator>
    auto operator== (const brodal_queue<T, Compare, Allocator> & lhs, 
                     const brodal_queue<T, Compare, Allocator> & rhs) -> bool;

    template<class T, class Compare, class Allocator>
    auto operator!= (const brodal_queue<T, Compare, Allocator> & lhs, 
                     const brodal_queue<T, Compare, Allocator> & rhs) -> bool;

// guide definition:

    namespace aux_impl
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
        (guide const& other) :
        reducer_ {other.reducer_},
        blocks_  {aux_impl::copy_blocks(other.blocks_, NULL_BLOCK)}
    {
    }

    template<class Reducer>
    guide<Reducer>::guide
        (guide&& other) :
        reducer_ {std::move(other.reducer_)},
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
    auto guide<Reducer>::operator=(guide rhs) -> guide &
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
        std::swap(blocks_, rhs.blocks_);
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
    auto swap (guide<Reducer>& lhs, guide<Reducer>& rhs) noexcept -> void
    {
        lhs.swap(rhs);
    }

// brodal_entry definition:

    template<class T, class Compare, class Allocator>
    template<class... Args>
    brodal_entry<T, Compare, Allocator>::brodal_entry
        (node_t* const node, Args&&... args) :
        data_ {std::forward<Args>(args)...},
        node_ {node}
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
    template<class... Args>
    brodal_node<T, Compare, Allocator>::brodal_node
        (std::piecewise_construct_t, Args&&... args) :
        entry_ {std::make_unique<entry_t>(this, std::forward<Args>(args)...)}
    {
    }

    template<class T, class Compare, class Allocator>
    brodal_node<T, Compare, Allocator>::brodal_node
        (brodal_node const& other) :
        rank_  {other.rank_},
        entry_ {std::make_unique<entry_t>(this, **other.entry_)},
        child_ {node_t::copy_list(other.child_)}
    {
        node_t::fold_right(child_, [=](node_t* const n)
        {
            n->parent_ = this;
        });
    }

    template<class T, class Compare, class Allocator>
    brodal_node<T, Compare, Allocator>::~brodal_node()
    {
        brodal_node::fold_right(child_, [](auto const n) { delete n; });
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
    root_wrap<Tree, T, Compare, Allocator>::root_wrap(brodal_queue<T, Compare, Allocator> * const queue) :
        queue_ {queue}
    {
    }

    template<class Tree, class T, class Compare, class Allocator>
    root_wrap<Tree, T, Compare, Allocator>::~root_wrap()
    {
        delete root_;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::operator=(root_wrap&& other) -> root_wrap&
    {
        root_ = other.root_;
        other.root_ = nullptr;

        this->upper_ = std::move(other.upper_);
        this->lower_ = std::move(other.lower_);
        this->sons_  = std::move(other.sons_);
 
        return *this;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::operator=(wrap_h other) -> root_wrap&
    {
        root_ = other.root_;
        other.root_ = nullptr;

        this->upper_ = other.upper();
        this->lower_ = other.lower();
        this->sons_  = other.sons();
 
        return *this;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::copyTree(root_wrap const& other) -> void
    {
        this->upper_ = other.upper_;
        this->lower_ = other.lower_;

        if (other.root_)
        {
            root_ = new brodal_node(*other.root_);
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::copySons(const root_wrap & other, const node_map & mapping) -> void
    {
        this->sons_.reserve(other.sons_.size());

        for (node_t const* otherSon : other.sons_)
        {
            this->sons_.push_back(mapping.at(otherSon));
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::copyViolations(const root_wrap & other, const node_map & mapping) -> void
    {
        using tree_iterator_t = brodal_tree_iterator<T, Compare, Allocator>;

        tree_iterator_t otherIt  {other.root_};
        tree_iterator_t otherEnd {nullptr};
        tree_iterator_t thisIt   {root_};

        while (otherIt != otherEnd)
        {
            node_t* const otherNode {std::addressof(*otherIt)};
            node_t* const thisNode  {std::addressof(*thisIt)};
            node_t*  setWCopy  {node_t::copy_set(otherNode->setW_, mapping)};
            node_t*  setVCopy  {node_t::copy_set(otherNode->setV_, mapping)};

            if (setWCopy)
            {
                setWCopy->prevInSet_ = thisNode;
                thisNode->setW_      = setWCopy;
            }

            if (setVCopy)
            {
                setVCopy->prevInSet_ = thisNode;
                thisNode->setV_      = setVCopy;
            }

            ++otherIt;
            ++thisIt;
        }
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
    auto root_wrap<Tree, T, Compare, Allocator>::add_child_base(node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            queue_->T1.remove_violation(child);
        }

        this->sons_[child->rank_]->add_right_sibling(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_delinked_nodes(const delinked_nodes<T, Compare, Allocator> & nodes) -> void
    {
        this->add_child(nodes.first);
        this->add_child(nodes.second);
        if (nodes.third)
        {
            this->add_child(nodes.third);
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_delinked_nodes_checked(const delinked_nodes<T, Compare, Allocator> & nodes) -> void
    {
        this->add_child_checked(nodes.first);
        this->add_child_checked(nodes.second);
        if (nodes.third)
        {
            this->add_child_checked(nodes.third);
        }
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::remove_child_base(node_t* const child) -> void
    {
        if (this->sons_[child->rank_] == child)
        {
            this->sons_[child->rank_] = child->right_;
        }

        root_->remove_child(child);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::add_child_checked(node_t* const child) -> void
    {
        this->add_child(child);
        this->upper_check(child->rank_);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::release_root() -> node_t*
    {
        node_t* ret {root_};
        root_ = nullptr;
        return ret;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check(const rank_t rank) -> void
    {
        if (root_->rank_ > 2 and rank < root_->rank_ - 2)
        {
            this->upper_.inc(rank);
        }

        this->upper_check_n_minus_2();
        this->upper_check_n_minus_1();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check(const rank_t rank) -> void
    {
        if (root_->rank_ > 2 and rank < root_->rank_ - 2)
        {
            this->lower_.inc(rank);
        }

        this->lower_check_n_minus_2(2);
        this->lower_check_n_minus_1();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::reduce_upper(const rank_t rank) -> void
    {
        this->add_child(this->link_children(rank));
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::reduce_lower(const rank_t rank) -> void
    {
        const delinked_nodes delinked {this->delink_child(rank + 1)};
        this->add_delinked_nodes(delinked);
        queue_->extraNodes.push(delinked.extra);
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
    auto root_wrap<Tree, T, Compare, Allocator>::increase_rank_base(node_t* const n1, node_t* const n2) -> void
    {
        root_->add_child(n1);
        root_->add_child(n2);
        this->sons_.push_back(n2);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_rank_base() -> void
    {
        this->sons_.pop_back();
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
    auto root_wrap<Tree, T, Compare, Allocator>::increase_domain_base() -> void
    {
        if ( root_->rank_ < 3 ) return;

        this->upper_.increase_domain();
        this->lower_.increase_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::decrease_domain_base() -> void
    {
        if ( root_->rank_ < 3 ) return;

        this->upper_.decrease_domain();
        this->lower_.decrease_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check_n_minus_1() -> void
    {
        const rank_t rank {static_cast<rank_t>(root_->rank_ - 1)};
        num_t count {node_t::same_rank_count(this->sons_[rank])};

        if ( count <= 7 ) return;

        count -= this->lower_check_n_minus_2(3);

        if ( count <= 7 ) return;

        node_t* const firstLinekd  {this->link_children(rank)};
        node_t* const secondLinked {this->link_children(rank)};

        this->increase_rank(firstLinekd, secondLinked);
        this->increase_domain();
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::upper_check_n_minus_2() -> void
    {
        if ( root_->rank_ < 2 ) return;
        
        const rank_t rank {static_cast<rank_t>(root_->rank_ - 2)};
        const num_t count {node_t::same_rank_count(this->sons_[rank])};
        
        if ( count <= 7 ) return;

        this->reduce_upper(rank);
    }
    
    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check_n_minus_1() -> void
    {
        const rank_t rank {static_cast<rank_t>(root_->rank_ - 1)};
        const num_t count {node_t::same_rank_count(this->sons_[rank])};

        if ( count >= 2 ) return;

        // Case 1:
        if (root_->rank_ > 1)
        {
            const num_t countNMinus2 {
                node_t::same_rank_count(this->sons_[rank - 1])
            };
            if (countNMinus2 >= 5)
            {
                this->reduce_upper(rank - 1);
                return;
            }
        }

        // Case 2:
        node_t* const node {this->sons_[rank]};
        const num_t sonsCount {node_t::same_rank_count(node->child_)};
        if (sonsCount >= 5)
        {
            node_t* const n1 {node->child_};
            node_t* const n2 {n1->right_};
            node_t* const n3 {n2->right_};
            if (n1->is_in_set()) queue_->T1.remove_violation(n1);
            if (n2->is_in_set()) queue_->T1.remove_violation(n2);
            if (n3->is_in_set()) queue_->T1.remove_violation(n3);
            node->remove_child(n1);
            node->remove_child(n2);
            node->remove_child(n3);
            node_t* const linked {node_t::link_nodes(n1, n2, n3)};
            this->add_child(linked);
            return;
        }

        // Case 3:
        this->decrease_domain();
        this->remove_child(node);
        this->decrease_rank();

        const delinked_nodes delinked1 {node_t::delink_node(node)};
        this->add_delinked_nodes(delinked1);
        if (node->rank_ == rank)
        {
            const delinked_nodes delinked2 {node_t::delink_node(node)};
            this->add_delinked_nodes(delinked2);
        }
        this->add_child_checked(node);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::lower_check_n_minus_2(const num_t bound) -> num_t
    {
        if ( root_->rank_ < 2 ) return 0;

        const rank_t rank {static_cast<rank_t>(root_->rank_ - 2)};
        const num_t count {node_t::same_rank_count(this->sons_[rank])};

        if ( count >= bound ) return 0;

        const delinked_nodes delinked {this->delink_child(rank + 1)};

        this->add_delinked_nodes(delinked);
        this->add_child(delinked.extra);
        const rank_t extraRank {delinked.extra->rank_};
        if (extraRank < rank)
        {
            this->upper_.inc(extraRank);
        }

        return extraRank != rank + 1 ? 1 : 0;
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::link_children(const rank_t rank) -> node_t*
    {
        node_t* const n1 {this->sons_[rank]};
        node_t* const n2 {n1->right_};
        node_t* const n3 {n2->right_};

        this->remove_child(n3);
        this->remove_child(n2);
        this->remove_child(n1);

        return node_t::link_nodes(n1, n2, n3);
    } 

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::delink_child(const rank_t rank) -> delinked_nodes<T, Compare, Allocator>
    {
        node_t* const toDelink {this->sons_[rank]};

        this->remove_child(toDelink);

        return node_t::delink_node(toDelink);
    }

    template<class Tree, class T, class Compare, class Allocator>
    auto root_wrap<Tree, T, Compare, Allocator>::swap(root_wrap & first, root_wrap & second) -> void
    {
        using std::swap;
        swap(first.root_,  second.root_);
        swap(first.upper_, second.upper_);
        swap(first.lower_, second.lower_);
        swap(first.sons_,  second.sons_);
    }

// t1_wrap definition:

    template<class T, class Compare, class Allocator>
    t1_wrap<T, Compare, Allocator>::t1_wrap(brodal_queue<T, Compare, Allocator>* const queue) :
        base_t {queue}
    {
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::operator=(t1_wrap&& other) -> t1_wrap &
    {
        base_t::operator=(std::move(other));

        this->violation_ = std::move(other.violation_);
        this->auxW_      = std::move(other.auxW_);

        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::copy_aux_violations(t1_wrap const& other, node_map const& mapping) -> void
    {
        this->violation_ = other.violation_;
        
        for (node_t* node : other.auxW_)
        {
            this->auxW_.push_back(
                node ? mapping.at(node) : nullptr
            );
        }
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
    auto t1_wrap<T, Compare, Allocator>::add_violation(node_t* const node) -> void
    {
        if (node->is_in_set())
        {
            this->remove_violation(node);
        }

        if (node->rank_ >= base_t::root_->rank_)
        {
            base_t::root_->add_to_V(node);
        }
        else if (this->auxW_[node->rank_])
        {
            this->auxW_[node->rank_]->add_set_sibling(node);
        }
        else
        {
            this->auxW_[node->rank_] = node;
            base_t::root_->add_to_W(node);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_violation(node_t* const node) -> void
    {
        if (node->rank_ < this->auxW_.size())
        {
            if (this->auxW_[node->rank_] == node)
            {
                node_t* const next {node->nextInSet_};
                this->auxW_[node->rank_] = (next and next->rank_ == node->rank_ ? next : nullptr);
            }
        }

        node_t::remove_from_set(node);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::violation_check(const rank_t rank) -> void
    {
        if (rank < this->auxW_.size())
        {
            this->violation_.inc(rank);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_violation(const rank_t rank) -> void
    {
        num_t removed {0};
        while (removed < 2)
        {
            removed += this->reduce_violations(rank);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_all_violations() -> void
    {
        for (node_t const* node : this->auxW_)
        {
            if (not node) continue;

            const rank_t rank {node->rank_};
            num_t count {node_t::same_rank_violation(node)};
            while (count > 1)
            {
                count -= this->reduce_violations(rank);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::increase_rank(node_t* const linked) -> void
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
    auto t1_wrap<T, Compare, Allocator>::increase_rank(node_t* const n1, node_t* const n2) -> void
    {
        base_t::increase_rank_base(n1, n2);
        this->auxW_.push_back(nullptr);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::decrease_rank() -> void
    {
        base_t::decrease_rank_base();
        this->auxW_.pop_back();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::increase_domain() -> void
    {
        base_t::increase_domain_base();
        this->violation_.increase_domain();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::decrease_domain() -> void
    {
        base_t::decrease_domain_base();
        this->violation_.decrease_domain();
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::reduce_violations(const rank_t rank) -> num_t
    {
        const node_ptr_pair normal {this->pick_normal_violations(rank)};
        const node_ptr_pair t2sons {this->pick_t2_son_violations(rank)};

        const num_t t2SonsRemoved {
            static_cast<num_t>(
                this->remove_t2_violation(t2sons.first) + 
                this->remove_t2_violation(t2sons.second)
            )
        };

        if (2 == t2SonsRemoved)
        {
            return 2;
        }

        return this->remove_normal_violations(
            normal.first,
            normal.second
        );
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::pick_normal_violations(const rank_t rank) -> node_ptr_pair
    {
        node_t* it {this->auxW_[rank]};

        node_t* first  {nullptr};
        node_t* second {nullptr};

        while (it and it->rank_ == rank)
        {
            if (not it->is_son_of_root())
            {
                if      (not first)  first  = it;
                else if (not second) second = it;
                else return std::make_pair(first, second);
            }
            
            it = it->nextInSet_;
        }

        return std::make_pair(first, second);
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::pick_t2_son_violations(const rank_t rank) -> node_ptr_pair
    {
        node_t* it {this->auxW_[rank]};

        num_t count {0};
        node_t* first  {nullptr};
        node_t* second {nullptr};

        while (it and it->rank_ == rank)
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
    auto t1_wrap<T, Compare, Allocator>::remove_t2_violation(node_t* const node) -> num_t
    {
        if (not node)
        {
            return 0;
        }

        if (not node->is_violating())
        {
            this->remove_violation(node);
            return 1;
        }

        this->remove_violation(node);
        base_t::queue_->T2.remove_child(node);
        // No need for lower check since this transformation is 
        // done only if there are more than 4 sons with given rank.
        // So removing the node won't affect lower guide.
        base_t::queue_->T1.add_child_checked(node);

        return 1;
    }

    template<class T, class Compare, class Allocator>
    auto t1_wrap<T, Compare, Allocator>::remove_normal_violations(node_t* const first, node_t* const second) -> num_t
    {
        node_t* const n1 {first};
        node_t* const n2 {second ? second : n1->same_rank_sibling()};

        if (not n1->is_violating())
        {
            this->remove_violation(n1);
            return 1;
        }

        const num_t removed {
            static_cast<num_t>(n2 != nullptr ? 2 : 1)
        };

        if (not node_t::are_siblings(n1, n2))
        {
            node_t::make_siblings(n1, n2);
        }

        const num_t siblingCount {node_t::same_rank_count(n1)};
        node_t* const parent {n1->parent_};
        if (siblingCount > 2)
        {
            parent->remove_child(n1);
            this->add_child_checked(n1);
            return 1;
        }

        const bool removeParent {parent->rank_ == n1->rank_ + 1};
        if (removeParent)
        {
            if (parent->parent_ != base_t::root_)
            {
                node_t* const replacement {this->sons_[parent->rank_]->right_};
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
            base_t::queue_->addExtraNodes();
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
    auto t1_wrap<T, Compare, Allocator>::swap(t1_wrap & first, t1_wrap & second) -> void
    {
        base_t::swap(first, second);

        using std::swap;
        swap(first.auxW_, second.auxW_);
        swap(first.violation_, second.violation_);
    }

// t2_wrap definition:

    template<class T, class Compare, class Allocator>
    t2_wrap<T, Compare, Allocator>::t2_wrap(brodal_queue<T, Compare, Allocator> * const queue) :
        base_t {queue}
    {
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::operator=(t2_wrap&& other) -> t2_wrap&
    {
        base_t::operator=(std::move(other));
        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::operator=(wrap_h other) -> t2_wrap&
    {
        base_t::operator=(other);
        return *this;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::add_child(node_t* const child) -> void
    {
        base_t::add_child_base(child);
        if (child->is_violating())
        {
            base_t::queue_->violations.push(child);
        }
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::remove_child(node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            base_t::queue_->T1.remove_violation(child);
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
    auto t2_wrap<T, Compare, Allocator>::removeLargeSons() -> node_t*
    {
        const rank_t newRootsRank {static_cast<rank_t>(base_t::root_->rank_ - 1)};
        node_t* const ret {this->sons_[base_t::root_->rank_ - 1]};

        if (0 == newRootsRank)
        {
            base_t::root_->child_ = nullptr;
            base_t::root_->rank_  = 0; 
        }
        else
        {
            base_t::root_->child_       = this->sons_[base_t::root_->rank_ - 2];
            base_t::root_->child_->left_->right_ = nullptr;
            base_t::root_->child_->left_        = nullptr;
            base_t::root_->set_rank();
        }

        node_t::fold_right(ret, [=](node_t* n) {
            if (n->is_in_set())
            {
                base_t::queue_->T1.remove_violation(n);
            }
        });

        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto t2_wrap<T, Compare, Allocator>::swap(t2_wrap & first, t2_wrap & second) -> void
    {
        base_t::swap(first, second);
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
    brodal_queue<T, Compare, Allocator>::brodal_queue(const brodal_queue & other) :
        queueSize {other.queueSize}
    {
        // First copy the structure of the tree.
        this->T1.copyTree(other.T1);
        this->T2.copyTree(other.T2);

        // Map the original nodes to their corresponding copies.
        node_map mapping {brodal_queue::makeNodeMapping(other, *this)};

        // Now copy auxiliary arrays.
        this->T1.copySons(other.T1, mapping);
        this->T2.copySons(other.T2, mapping);

        // And finally copy violation sets and auxW.
        this->T1.copyViolations(other.T1, mapping);
        this->T2.copyViolations(other.T2, mapping);
        this->T1.copy_aux_violations(other.T1, mapping);
    }

    template<class T, class Compare, class Allocator>
    brodal_queue<T, Compare, Allocator>::brodal_queue(brodal_queue && other) :
        queueSize {other.queueSize}
    {
        other.queueSize = 0;
        this->T1 = std::move(other.T1);
        this->T2 = std::move(other.T2);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert(value_type const& value) -> handle_t
    {
        return this->insert_impl(this->new_node(value));
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert(value_type&& value) -> handle_t
    {
        return this->insert_impl(this->new_node(std::move(value)));
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::operator=
        (brodal_queue rhs) -> brodal_queue&
    {
        swap(*this, rhs);
        return *this;
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
    auto brodal_queue<T, Compare, Allocator>::delete_min() -> value_type
    {
        if (this->size() < 4)
        {
            return this->deleteMinSpecial();
        }

        this->moveAllToT1();

        T ret {this->find_min()};
        node_t* const oldRoot {this->T1.root_};
        node_uptr newRoot {this->findNewRoot()}; 
        if (newRoot->is_in_set())
        {
            // TODO if it is violating we need to remove it from set and auxW
            this->T1.remove_violation(newRoot.get());
        }
        
        if (not newRoot->is_son_of_root())
        {
            this->makeSonOfRoot(newRoot.get());
        }

        this->T1.remove_child(newRoot.get());
        this->T1.lower_check(newRoot->rank_);
        this->addExtraNodes();

        node_t* const sonsToAdd {newRoot->disconnect_sons()};
        node_t::swap_entries(newRoot.get(), oldRoot);
        node_t::fold_right(sonsToAdd, [=](node_t* const n) {
            this->T1.add_child_checked(n->disconnect());
        });

        this->mergeSets(newRoot.get());
        this->T1.reduce_all_violations();

        this->queueSize--;

        return ret;
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto brodal_queue<T, Compare, Allocator>::new_node
        (Args&&... args) -> node_t*
    {
        return this->new_node_impl(std::piecewise_construct, std::forward<Args>(args)...);
    }

    template<class T, class Compare, class Allocator>
    template<class... Args>
    auto brodal_queue<T, Compare, Allocator>::new_node_impl
        (Args&&... args) -> node_t*
    {
        return new node_t(std::forward<Args>(args)...);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert_impl
        (node_t* const node) -> handle_t
    {
        if (this->size() < 3)
        {
            return this->insert_special_impl(node);
        }
        
        auto const entry = node->entry_.get();

        this->queueSize++;

        if (*node < *this->T1.root_)
        {
            node_t::swap_entries(node, this->T1.root_);
        }

        this->T1.add_child_checked(node);

        this->moveToT1();

        return handle_t(entry);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::insert_special_impl
        (node_t* const node) -> handle_t
    {
        this->queueSize++;

        auto const entry = node->entry_.get();

        if (1 == this->size())
        {
            this->T1.root_ = node;
            return handle_t(entry);
        }
        else if (*node < *this->T1.root_)
        {
            node_t::swap_entries(node, this->T1.root_);
        }

        if (2 == this->size())
        {
            this->T2.root_ = node;
        }
        else
        {
            node_t* const oldT2 {this->T2.root_};
            this->T2.root_ = nullptr;
            this->T1.increase_rank(node, oldT2);
            this->T1.increase_domain();
        }

        return handle_t(entry);
    }

    template<class T, class Compare, class Allocator>
    template<class Cmp>
    auto brodal_queue<T, Compare, Allocator>::dec_key_impl
        (node_t* const node) -> void
    {
        // TODO wtf?
        // this->moveToT1(); 

        if (Cmp {} (**node, **this->T1.root_))
        {
            node_t::swap_entries(node, this->T1.root_);
        }

        if (node->is_violating() and not node->is_in_set())
        {
            this->T1.add_violation(node);
            this->T1.violation_check(node->rank_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::erase_impl
        (node_t* const node) -> void
    {
        node_t::swap_entries(node, this->T1.root_);

        if (node->parent_ && T1.root_ != node->parent_ && !node->is_in_set())
        {
            // At this point node can be violating even if it is son of t1.
            // If that is the case we can't add it to violation set.
            // It would break delete_min since sons of t1 cannot be violating.
            this->T1.add_violation(node);
        }

        this->delete_min();
    }



    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::deleteMinSpecial() -> value_type
    {
        T ret {this->find_min()};
        node_uptr oldRoot {this->T1.root_};

        if (1 == this->queueSize)
        {
            this->T1.root_ = nullptr;
        }
        else if (2 == this->queueSize)
        {
            this->T1.root_ = this->T2.root_;
            this->T2.root_ = nullptr;
        }
        else
        {
            node_t* const firstChild  {this->T1.root_->disconnect_sons()};
            node_t* const secondChild {firstChild->right_};

            firstChild->disconnect();
            secondChild->disconnect();

            if (*firstChild < *secondChild)
            {
                this->T1.root_ = firstChild;
                this->T2.root_ = secondChild;
            }
            else
            {
                this->T1.root_ = secondChild;
                this->T2.root_ = firstChild;
            }

            this->T1.decrease_rank();
            this->T1.decrease_domain();
        }

        this->queueSize--;

        return ret;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::find_min() -> reference
    {
        this->isEmptyCheck();
        this->moveToT1();

        return **this->T1.root_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::find_min() const -> const_reference
    {
        this->isEmptyCheck();

        return **this->T1.root_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::size() const -> size_type
    {
        return this->queueSize;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::max_size() const -> size_type
    {
        return std::numeric_limits<size_type>::max();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::empty() const -> bool
    {
        return 0 == this->size();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::clear() -> void
    {
        *this = brodal_queue {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::begin() -> iterator
    {
        return iterator {this->T1.root_, this->T2.root_};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::end() -> iterator
    {
        return iterator {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::begin() const -> const_iterator
    {
        return const_iterator {this->T1.root_, this->T2.root_};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::end() const -> const_iterator
    {
        return const_iterator {};
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::cbegin() const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->begin();
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::cend() const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->end();
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
        newQueue.queueSize = first.queueSize + second.queueSize;
        newQueue.T1        = std::move(newt1);

        if (&newt1 == &newt2)
        {
            brodal_queue_t::addUnderRoot(newQueue.T1, first.T1);
            brodal_queue_t::addUnderRoot(newQueue.T1, first.T2);
            brodal_queue_t::addUnderRoot(newQueue.T1, second.T1);
            brodal_queue_t::addUnderRoot(newQueue.T1, second.T2);
        }
        else
        {
            newQueue.T2 = std::move(newt2);
            brodal_queue_t::addUnderRoot(newQueue.T2, first.T1);
            brodal_queue_t::addUnderRoot(newQueue.T2, first.T2);
            brodal_queue_t::addUnderRoot(newQueue.T2, second.T1);
            brodal_queue_t::addUnderRoot(newQueue.T2, second.T2);
            newQueue.addViolations();
        }

        first  = brodal_queue_t {};
        second = brodal_queue_t {};

        return newQueue;
    }

    template<class T, class Compare, class Allocator>
    auto swap(brodal_queue<T, Compare, Allocator> & first, brodal_queue<T, Compare, Allocator> & second) noexcept -> void
    {
        std::swap(first.queueSize, second.queueSize);
        t1_wrap<T, Compare, Allocator>::swap(first.T1, second.T1);
        t2_wrap<T, Compare, Allocator>::swap(first.T2, second.T2);
    }

    template<class T, class Compare, class Allocator>
    auto operator==
        (brodal_queue<T, Compare, Allocator> const& lhs, brodal_queue<T, Compare, Allocator> const& rhs) -> bool
    {
        return lhs.size() == rhs.size()
            && std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    template<class T, class Compare, class Allocator>
    auto operator!=
        (brodal_queue<T, Compare, Allocator> const& lhs, brodal_queue<T, Compare, Allocator> const& rhs) -> bool
    {
        return ! (lhs == rhs);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::addExtraNodes() -> void
    {
        while (not this->extraNodes.empty())
        {
            node_t* const node {this->extraNodes.top()};
            this->extraNodes.pop();
            if (node->rank_ < this->T1.root_->rank_)
            {
                this->T1.add_child_checked(node);
            }
            else
            {
                this->T2.add_child_checked(node);
            }
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::addViolations() -> void
    {
        while (not this->violations.empty())
        {
            node_t* const violation {this->violations.top()};
            this->violations.pop();
            this->T1.add_violation(violation);
            this->T1.violation_check(violation->rank_);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::isEmptyCheck() -> void
    {
        if (this->empty())
        {
            throw std::out_of_range("Priority queue is empty.");
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::moveAllToT1() -> void
    {
        if (not this->T2.root_)
        {
            return;
        }

        // Disconnect sons of t2.
        node_t* const oldt2         {this->T2.root_};
        node_t* const rightFoldable {this->T2.sons_[0]};
        node_t* const leftFoldable  {rightFoldable->left_};
        oldt2->disconnect_sons();
        // Reset t2 wrap to defaults.
        this->T2 = t2_wrap {this};

        // Add sons with rank 0 under t1.
        node_t::fold_right(rightFoldable, [=](node_t* const n) {
            this->T1.add_child_checked(n->disconnect());
        });

        // Add other sons with ranks > 0 under t1.
        node_t::fold_left(leftFoldable, [=](node_t* const n) {
            n->disconnect();
            
            while (n->rank_ >= this->T1.root_->rank_)
            {
                const delinked_nodes delinked {node_t::delink_node(n)};
                this->T1.add_delinked_nodes_checked(delinked);
            }
            
            this->T1.add_child_checked(n);
        });

        // Finally add t2 as son of t1.
        this->T1.add_child_checked(oldt2);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::moveToT1() -> void
    {
        if (not this->T2.root_ or this->size() < 4)
        {
            return;
        }

        if (this->T2.root_->rank_ <= this->T1.root_->rank_ + 2) // r(t2) is (r(t1) + 1) or (r(t1) + 2)
        {
            node_t* const sons1 {this->T2.removeLargeSons()};
            if (this->T2.root_->rank_ > this->T1.root_->rank_)
            {
                node_t* const sons2 {this->T2.removeLargeSons()};
                this->T1.increase_rank(sons2);
            }
            this->T1.increase_rank(sons1);

            node_t* const oldt2 {this->T2.release_root()};
            this->T1.add_child_checked(oldt2);

            // Reset t2 wrap to defaults. 
            this->T2 = t2_wrap {this};
        }
        else
        {
            const rank_t t1rank {this->T1.root_->rank_};
            node_t* const toDelink  {this->T2.sons_[t1rank + 1]};
            this->T2.remove_child(toDelink);
            this->T2.lower_check(toDelink->rank_);

            const delinked_nodes delinked {node_t::delink_node(toDelink)};
            this->T1.increase_rank(delinked.first, delinked.second);
            this->T1.increase_domain();
            if (delinked.third)
            {
                this->T1.add_child_checked(delinked.third);
            }
            
            // In case toDelink had more than 3 sons of rank (n-1).
            while (toDelink->rank_ == this->T1.root_->rank_)
            {
                const delinked_nodes delinked {node_t::delink_node(toDelink)};
                this->T1.add_delinked_nodes_checked(delinked);
            }

            this->T1.add_child_checked(toDelink);

            // These might have been created while doing
            // delinking under t2.
            this->addExtraNodes();
            this->addViolations();
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::findNewRoot() const -> node_t*
    {
        node_t* newRoot {this->T1.root_->child_};

        node_t::fold_right(newRoot, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::fold_next(this->T1.root_->setW_, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::fold_next(this->T1.root_->setV_, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        return newRoot;
    } 

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::makeSonOfRoot(node_t* const newRoot) -> void
    {
        node_t* const swapped {this->T1.sons_[newRoot->rank_]->right_};
        node_t::swap_tree_nodes(newRoot, swapped);
        if (swapped->is_violating())
        {
            this->T1.add_violation(swapped);
        }
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::mergeSets(node_t* const newRoot) -> void
    {
        node_t::fold_next(this->T1.root_->setV_, [=](node_t* const n) {
            this->T1.add_violation(n);
        });

        node_t::fold_next(newRoot->setW_, [=](node_t* const n) {
            this->T1.add_violation(n);
        });

        node_t::fold_next(newRoot->setV_, [=](node_t* const n) {
            this->T1.add_violation(n);
        });
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::makeNodeMapping(const brodal_queue & original, const brodal_queue & copy) -> node_map
    {
        node_map map;
        
        if (original.T1.root_)
        {
            brodal_queue::nodeMappingOfTree(original.T1, copy.T1, map);
        }

        if (original.T2.root_)
        {
            brodal_queue::nodeMappingOfTree(original.T2, copy.T2, map);
        }

        return map;
    }

    template<class T, class Compare, class Allocator>
    template<class RootWrap>
    auto brodal_queue<T, Compare, Allocator>::nodeMappingOfTree(RootWrap const& original, RootWrap const& copy, node_map & map) -> void
    {
        using tree_iterator_t = brodal_tree_iterator<T, Compare, Allocator>;
        
        tree_iterator_t itOriginal  {original.root_};
        tree_iterator_t originalEnd {nullptr};
        tree_iterator_t itCopy      {copy.root_};

        // Put violating nodes into the map.
        while (itOriginal != originalEnd)
        {
            if (itOriginal->is_in_set())
            {
                // TODO emplace
                map[std::addressof(*itOriginal)] = std::addressof(*itCopy);
            }

            ++itOriginal;
            ++itCopy;
        }

        // Put nodes that are in sons vector into the map.
        // These nodes are 'first of their rank'.
        node_t::zip_with(original.root_->child_, copy.root_->child_, 
            [&](node_t* const n1, node_t* const n2) {
                if (not n1->left_
                    or  n1->left_->rank_ != n1->rank_)
                {
                    map[n1] = n2;
                }
            }
        );
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
        if (not first.T1.root_)  return second.T1;
        if (not second.T1.root_) return first.T1;

        return *first.T1.root_ < *second.T1.root_ ? first.T1 : second.T1;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_queue<T, Compare, Allocator>::findNewT2(brodal_queue& first, brodal_queue& second, t1_wrap_t& t1) -> wrap_h
    {
        auto newT2 = wrap_h {};
        newT2 = t1;

        if (first.T1.root_ and first.T1.root_->rank > newT2->root()->rank)
        {
            newT2 = first.T1;
        }

        if (first.T2.root_ and first.T2.root_->rank > newT2->root()->rank)
        {
            newT2 = first.T2;
        }

        if (second.T1.root_ and second.T1.root_->rank > newT2->root()->rank)
        {
            newT2 = second.T1;
        }

        if (second.T2.root_ and second.T2.root_->rank > newT2->root()->rank)
        {
            newT2 = second.T2;
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

}

#endif