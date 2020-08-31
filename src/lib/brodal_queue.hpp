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
    template<class T, class Compare>
    class brodal_node;

    /**
        Result of delinking a single Brodal node.
     */
    template<class T, class Compare>
    struct delinked_nodes
    {
        using node_t = brodal_node<T, Compare>;

        node_t* first  {nullptr};
        node_t* second {nullptr};
        node_t* third  {nullptr};
        node_t* extra  {nullptr};
    };

    /**
        Entry that wraps data inserted into the queue.
     */
    template<class T, class Compare>
    class brodal_entry
    {
    public:
        using node_t = brodal_node<T, Compare>;

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
    template<class T, class Compare>
    class brodal_node
    {
    public:
        using rank_t     = std::uint8_t;
        using num_t      = std::uint8_t;
        using node_map   = std::unordered_map<brodal_node const*, brodal_node*>;
        using entry_t    = brodal_entry<T, Compare>;
        using entry_uptr = std::unique_ptr<entry_t>;
        using node_t     = brodal_node;
        using delinked_t = delinked_nodes<T, Compare>;

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

        static auto are_siblings        (node_t* const first, node_t* const second) -> bool;
        static auto swap_entries        (node_t* const first, node_t* const second) -> void;
        static auto swap_tree_nodes     (node_t* const first, node_t* const second) -> void;
        static auto make_siblings       (node_t* const first, node_t* const second) -> void;
        static auto same_rank_count     (node_t const* const node) -> num_t;
        static auto same_rank_violation (node_t const* const node) -> num_t;
        static auto remove_from_set     (node_t* const node)  -> void;
        static auto copy_list           (node_t* const first) -> node_t*;
        static auto delink_node         (node_t* const node)  -> delinked_t;
        static auto copy_set            (node_t* const first, node_map const& mapping)         -> node_t*;
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
        rank_t rank_ {0};

        entry_uptr entry_ {nullptr};
        
        node_t* parent_ {nullptr};
        node_t* left_   {nullptr};
        node_t* right_  {nullptr};
        node_t* child_  {nullptr};

        node_t* nextInSet_ {nullptr};
        node_t* prevInSet_ {nullptr};
        node_t* setW_      {nullptr};
        node_t* setV_      {nullptr};
    };

    /**
        Iterator of a single brodal tree.
     */
    template<class T, class Compare>
    class brodal_tree_iterator
    {
    public:
        using node_t            = brodal_node<T, Compare>;
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
        using node_t            = brodal_node<T, Compare>;
        using tree_iterator_t   = brodal_tree_iterator<T, Compare>;

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
        using entry_t = brodal_entry<T, Compare>;
        template<class, class>
        friend class brodal_queue;
        brodal_entry_handle(entry_t* const node);
        entry_t* entry_;
    };    
    
// brodal_queue declaration:

    // Ugly forward declarations:
    template<class T, class Compare>
    class brodal_queue;

    template<class T, class Compare>
    auto swap(brodal_queue<T, Compare> & first, 
              brodal_queue<T, Compare> & second) noexcept -> void;

    template<class T, class Compare>
    auto meld(brodal_queue<T, Compare> & first, 
              brodal_queue<T, Compare> & second) -> brodal_queue<T, Compare>;

    // BrodalQueue declaration:
    template<class T, class Compare = std::less<T>>
    class brodal_queue
    {
    private: // Forward declarations:
        struct RootWrap;
        struct T1RootWrap;
        struct IteratorNode;

    private: // Member types:
        using num_t   = std::uint8_t;
        using index_t = std::uint8_t;
        using rank_t  = index_t;

        using node_t         = brodal_node<T, Compare>;
        using node_uptr      = std::unique_ptr<node_t>;
        using node_ptr_pair  = std::pair<node_t*, node_t*>;
        using wrap_ref       = RootWrap &;
        using cwrap_ref      = const RootWrap &;
        using wrap_ptr       = RootWrap *;
        using t1_wrap_ref    = T1RootWrap &;
        using iter_node_uptr = std::unique_ptr<IteratorNode>;
        using node_map       = std::unordered_map<node_t const*, node_t*>;

    private: // Nested classes:
        struct UpperReducer
        {
            RootWrap * managedRoot {nullptr};

            UpperReducer(RootWrap * const root);

            auto reduce  (const index_t i)       -> void;
            auto get_num (const index_t i) const -> num_t;
        };

        struct LowerReducer
        {
            RootWrap * managedRoot {nullptr};

            LowerReducer(RootWrap * const root);

            auto reduce  (const index_t i)       -> void;
            auto get_num (const index_t i) const -> num_t;
        };

        struct ViolationReducer
        {
            T1RootWrap * managedRoot {nullptr};

            ViolationReducer(T1RootWrap * const root);

            auto reduce  (const index_t i)       -> void;
            auto get_num (const index_t i) const -> num_t;
        };

        struct RootWrap
        {
            brodal_queue*        queue {nullptr};
            node_t*              root  {nullptr};
            guide<UpperReducer>  upper {UpperReducer {this}};
            guide<LowerReducer>  lower {LowerReducer {this}};
            std::vector<node_t*> sons;

            RootWrap(brodal_queue * const queue);
            virtual ~RootWrap();
            auto operator= (RootWrap && other)      -> RootWrap &;

            auto copyTree       (const RootWrap & other) -> void;
            auto copySons       (const RootWrap & other, const node_map & mapping) -> void;
            auto copyViolations (const RootWrap & other, const node_map & mapping) -> void;

            // TODO nie virtuálne ale nejako cez napr. constexpr if
            virtual auto addChild    (node_t* const child) -> void;
            virtual auto removeChild (node_t* const child) -> void;
            auto addChildChecked     (node_t* const child) -> void;
            auto removeChildChecked  (node_t* const child) -> void;
            auto addDelinkedNodes        (delinked_nodes<T, Compare> const& nodes) -> void;
            auto addDelinkedNodesChecked (delinked_nodes<T, Compare> const& nodes) -> void;

            auto releaseRoot () -> node_t*;

            auto upperCheck (const rank_t rank) -> void;
            auto lowerCheck (const rank_t rank) -> void;

            auto reduceUpper (const rank_t rank) -> void;
            auto reduceLower (const rank_t rank) -> void;

            virtual auto increaseRank (node_t* const n1, node_t* const n2) -> void;
            virtual auto decreaseRank ()                           -> void;

            virtual auto increase_domain () -> void;
            virtual auto decrease_domain () -> void;

            static auto swap(RootWrap & first, RootWrap & second) -> void;

        private:
            auto upperCheckNMinus1 () -> void;
            auto upperCheckNMinus2 () -> void;
            auto lowerCheckNMinus1 () -> void;
            auto lowerCheckNMinus2 (const num_t bound) -> num_t;

            auto linkChildren (const rank_t rank) -> node_t*;
            auto delinkChild  (const rank_t rank) -> delinked_nodes<T, Compare>;
        };

        struct T1RootWrap : public RootWrap
        {
            guide<ViolationReducer> violation {ViolationReducer {this}};
            std::vector<node_t*> auxW;

            T1RootWrap(brodal_queue * const queue);
            virtual ~T1RootWrap() = default;

            auto operator= (const T1RootWrap & other) -> T1RootWrap &;
            auto operator= (T1RootWrap && other)      -> T1RootWrap &;

            auto copyAuxViolations (const T1RootWrap & other, const node_map & mapping) -> void;

            auto addViolation    (node_t* const node) -> void;
            auto removeViolation (node_t* const node) -> void;

            auto violationCheck  (const rank_t rank) -> void;
            auto reduceViolation (const rank_t rank) -> void;

            auto fullyReduceViolations () -> void;

            auto increaseRank (node_t* const linked) -> void;
            auto increaseRank (node_t* const n1, node_t* const n2) -> void override;
            auto decreaseRank ()                                   -> void override;

            auto increase_domain () -> void override;
            auto decrease_domain () -> void override;

            static auto swap(T1RootWrap & first, T1RootWrap & second) -> void;

        private:
            /// Removes at least one violation of given rank.
            /// Can create one violation of rank (rank+1),
            /// but in that case it removes two violations.
            /// @return Number of violations removed.
            auto reduceViolations (const rank_t rank) -> num_t;

            auto pickNormalViolations (const rank_t rank) -> node_ptr_pair;

            /// Pick violations of given rank that are sons of t2. 
            /// First 4 violations are ignored, i.e. if there are 
            /// 5 violations, only one is returned.
            auto pickT2SonViolations  (const rank_t rank) -> node_ptr_pair;

            /// @return 1 If violating node was removed.
            ///         0 If violating node was not removed,
            ///           i.e. node was nullptr.  
            auto removeT2Violation (node_t* const node) -> num_t;

            auto removeNormalViolations (node_t* const first, node_t* const second) -> num_t;
        };

        struct T2RootWrap : public RootWrap
        {
            T2RootWrap(brodal_queue * const queue);
            virtual ~T2RootWrap() = default;

            auto operator= (RootWrap   && other)      -> T2RootWrap &;
            auto operator= (const T2RootWrap & other) -> T2RootWrap &;
            auto operator= (T2RootWrap && other)      -> T2RootWrap &;

            auto addChild    (node_t* const child) -> void override;
            auto removeChild (node_t* const child) -> void override;

            /// Does NOT perform any guide adjustments.
            /// Leaves wrap in an incorrect state.
            /// Use only if wrap will be discarded soon afer.
            auto removeLargeSons () -> node_t*; 

            static auto swap(T2RootWrap & first, T2RootWrap & second) -> void;
        };

    public:  // Member types:
        using value_type      = T;
        using pointer         = value_type*;
        using reference       = value_type&;
        using const_reference = value_type const&;
        using iterator        = brodal_queue_iterator<T, Compare, int, false>;
        using const_iterator  = brodal_queue_iterator<T, Compare, int, true>;
        using difference_type = std::ptrdiff_t;
        using size_type       = std::size_t;
        using handle_t        = brodal_entry_handle<T, Compare, int>;

    public: // BrodalQueue members:
        std::size_t queueSize {0};

        T1RootWrap T1 {this};
        T2RootWrap T2 {this};

        std::stack<node_t*> extraNodes;
        std::stack<node_t*> violations;
        
    public:  // BrodalQueue interface:
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

        friend auto meld<T, Compare>(brodal_queue & first, 
                                     brodal_queue & second) -> brodal_queue;
        friend auto swap<T, Compare>(brodal_queue<T, Compare> & first, 
                                     brodal_queue<T, Compare> & second) noexcept -> void;

    private: // Auxiliary methods:
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

        static auto makeNodeMapping   (const brodal_queue & original, const brodal_queue & copy)  -> node_map;
        static auto nodeMappingOfTree (cwrap_ref original, cwrap_ref copy, node_map & map) -> void;
        
        static auto dumbMeld     (brodal_queue & first, brodal_queue & second) -> brodal_queue;
        static auto findNewT1    (brodal_queue & first, brodal_queue & second) -> t1_wrap_ref;
        static auto findNewT2    (brodal_queue & first, brodal_queue & second, t1_wrap_ref t1) -> wrap_ref;
        static auto addUnderRoot (wrap_ref root, wrap_ref toAdd) -> void;
    };

    // BrodalQueue interface:

    template<class T, class Compare>
    auto operator== (const brodal_queue<T, Compare> & lhs, 
                     const brodal_queue<T, Compare> & rhs) -> bool;

    template<class T, class Compare>
    auto operator!= (const brodal_queue<T, Compare> & lhs, 
                     const brodal_queue<T, Compare> & rhs) -> bool;

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

    template<class T, class Compare>
    template<class... Args>
    brodal_entry<T, Compare>::brodal_entry
        (node_t* const node, Args&&... args) :
        data_ {std::forward<Args>(args)...},
        node_ {node}
    {
    }

    template<class T, class Compare>
    auto brodal_entry<T, Compare>::operator*
        () -> T&
    {
        return data_;
    }

    template<class T, class Compare>
    auto brodal_entry<T, Compare>::operator*
        () const -> T const&
    {
        return data_;
    }

// brodal_node definition:

    template<class T, class Compare>
    template<class... Args>
    brodal_node<T, Compare>::brodal_node
        (std::piecewise_construct_t, Args&&... args) :
        entry_ {std::make_unique<entry_t>(this, std::forward<Args>(args)...)}
    {
    }

    template<class T, class Compare>
    brodal_node<T, Compare>::brodal_node
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

    template<class T, class Compare>
    brodal_node<T, Compare>::~brodal_node()
    {
        brodal_node::fold_right(child_, [](auto const n) { delete n; });
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator*
        () -> T&
    {
        return **entry_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator*
        () const -> T const&
    {
        return **entry_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator<
        (brodal_node const& other) const -> bool
    {
        return Compare {} (**this, *other);
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::add_child
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::remove_child
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::add_right_sibling
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::add_to_W
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::add_to_V
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::add_set_sibling
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::disconnect_sons
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::disconnect
        () -> node_t*
    {
        left_  = nullptr;
        right_ = nullptr;
        
        return this;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::set_rank
        () -> void
    {
        rank_ = child_ ? child_->rank_ + 1 : 0;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::same_rank_sibling
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::is_son_of_root
        () const -> bool
    {
        return parent_ && !parent_->parent_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::is_violating
        () const -> bool
    {
        return parent_ && *this < *parent_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::is_in_set
        () const -> bool
    {
        return nullptr != prevInSet_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::is_first_in_W_set
        () const -> bool
    {
        return prevInSet_ && prevInSet_->setW_ == this;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::is_first_in_V_set
        () const -> bool
    {
        return prevInSet_ && prevInSet_->setV_ == this;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::are_siblings
        (node_t* const first, node_t* const second) -> bool
    {
        return first->parent_ == second->parent_;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::swap_entries
        (node_t* const first, node_t* const second) -> void
    {
        using std::swap;
        swap(first->entry_, second->entry_);
        swap(first->entry_->node_, second->entry_->node_);
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::swap_tree_nodes
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::make_siblings
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::same_rank_count
        (node_t const* const node) -> num_t
    {
        
        return node_t::same_rank_impl( node
                                     , [](auto const n){ return n->left_; }
                                     , [](auto const n){ return n->right_; });
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::same_rank_violation
        (node_t const* const node) -> num_t
    {
        return node_t::same_rank_impl( node
                                     , [](auto const n){ return n->prevInSet_; }
                                     , [](auto const n){ return n->nextInSet_; });
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::remove_from_set
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::copy_list
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::copy_set
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::delink_node
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::link_nodes
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

    template<class T, class Compare>
    auto brodal_node<T, Compare>::max_prio_node
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

    template<class T, class Compare>
    template<class PrevFucntion, class NextFunction>
    auto brodal_node<T, Compare>::same_rank_impl
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

    template<class T, class Compare>
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::fold_right
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->right_; });
    }

    template<class T, class Compare>
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::fold_left
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->left_; });
    }

    template<class T, class Compare>
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::fold_next
        (node_t* const first, UnaryFunction func) -> void
    {
        node_t::fold_impl(first, func, [](auto const n){ return n->nextInSet_; });
    }

    template<class T, class Compare>
    template<class UnaryFunction, class NextFunction>
    auto brodal_node<T, Compare>::fold_impl
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

    template<class T, class Compare>
    template<class BinaryFunction>
    auto brodal_node<T, Compare>::zip_with
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

    template<class T, class Compare>
    brodal_tree_iterator<T, Compare>::brodal_tree_iterator
        (node_t* const root) : 
        stack_ {root ? std::deque {root} : std::deque<node_t*> {}}
    {
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator++
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

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator++
        (int) -> brodal_tree_iterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator*
        () const -> reference
    {
        return *stack_.top();
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator->
        () const -> pointer
    {
        return stack_.top();
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator==
        (brodal_tree_iterator const& rhs) const -> bool
    {
        return (stack_.empty() && rhs.stack_.empty())
            || (stack_.size()  == rhs.stack_.size()
            &&  stack_.top() == rhs.stack_.top());
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::operator!=
        (brodal_tree_iterator const& rhs) const -> bool
    {
        return !(*this == rhs);
    }

    template<class T, class Compare>
    auto brodal_tree_iterator<T, Compare>::current
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

// brodal_queue definition:
    

    // BrodalQueue::UpperReducer implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::UpperReducer::UpperReducer(RootWrap * const root) :
        managedRoot {root}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::UpperReducer::reduce(const index_t i) -> void
    {
        this->managedRoot->reduceUpper(i);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::UpperReducer::get_num(const index_t i) const -> num_t
    {
        const num_t count {
            node_t::same_rank_count(this->managedRoot->sons[i])
        };

        return count < 6 ? 0 : count - 5;
    }

    // BrodalQueue::LowerReducer implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::LowerReducer::LowerReducer(RootWrap * const root) :
        managedRoot {root}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::LowerReducer::reduce(const index_t i) -> void
    {
        this->managedRoot->reduceLower(i);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::LowerReducer::get_num(const index_t i) const -> num_t
    {
        const num_t count {
            node_t::same_rank_count(this->managedRoot->sons[i])
        };

        return count >= 4 ? 0 : 4 - count;
    }

    // BrodalQueue::ViolationReducer implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::ViolationReducer::ViolationReducer(T1RootWrap * const root) :
        managedRoot {root}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::ViolationReducer::reduce(const index_t i) -> void
    {
        this->managedRoot->reduceViolation(i);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::ViolationReducer::get_num(const index_t i) const -> num_t
    {
        if (not this->managedRoot->auxW[i]) 
        {
            return 0;
        }

        const num_t count {
            node_t::same_rank_violation(this->managedRoot->auxW[i])
        };

        return count < 5 ? 0 : count - 4;
    } 

    // BrodalQueue::RootWrap implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::RootWrap::RootWrap(brodal_queue * const queue) :
        queue {queue}
    {
    }

    template<class T, class Compare>
    brodal_queue<T, Compare>::RootWrap::~RootWrap()
    {
        delete this->root;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::operator=(RootWrap && other) -> RootWrap &
    {
        this->root = other.root;
        other.root = nullptr;

        this->upper = std::move(other.upper);
        this->lower = std::move(other.lower);
        this->sons  = std::move(other.sons);
 
        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::copyTree(const RootWrap & other) -> void
    {
        this->upper = other.upper;
        this->lower = other.lower;

        if (other.root)
        {
            this->root = new brodal_node(*other.root);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::copySons(const RootWrap & other, const node_map & mapping) -> void
    {
        this->sons.reserve(other.sons.size());

        for (node_t const* otherSon : other.sons)
        {
            this->sons.push_back(mapping.at(otherSon));
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::copyViolations(const RootWrap & other, const node_map & mapping) -> void
    {
        using tree_iterator_t = brodal_tree_iterator<T, Compare>;

        tree_iterator_t otherIt  {other.root};
        tree_iterator_t otherEnd {nullptr};
        tree_iterator_t thisIt   {this->root};

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

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addChild(node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            this->queue->T1.removeViolation(child);
        }

        this->sons[child->rank_]->add_right_sibling(child);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addDelinkedNodes(const delinked_nodes<T, Compare> & nodes) -> void
    {
        this->addChild(nodes.first);
        this->addChild(nodes.second);
        if (nodes.third)
        {
            this->addChild(nodes.third);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addDelinkedNodesChecked(const delinked_nodes<T, Compare> & nodes) -> void
    {
        this->addChildChecked(nodes.first);
        this->addChildChecked(nodes.second);
        if (nodes.third)
        {
            this->addChildChecked(nodes.third);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::removeChild(node_t* const child) -> void
    {
        if (this->sons[child->rank_] == child)
        {
            this->sons[child->rank_] = child->right_;
        }

        this->root->remove_child(child);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addChildChecked(node_t* const child) -> void
    {
        this->addChild(child);
        this->upperCheck(child->rank_);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::releaseRoot() -> node_t*
    {
        node_t* ret {this->root};
        this->root = nullptr;
        return ret;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::upperCheck(const rank_t rank) -> void
    {
        if (this->root->rank_ > 2 and rank < this->root->rank_ - 2)
        {
            this->upper.inc(rank);
        }

        this->upperCheckNMinus2();
        this->upperCheckNMinus1();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheck(const rank_t rank) -> void
    {
        if (this->root->rank_ > 2 and rank < this->root->rank_ - 2)
        {
            this->lower.inc(rank);
        }

        this->lowerCheckNMinus2(2);
        this->lowerCheckNMinus1();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::reduceUpper(const rank_t rank) -> void
    {
        this->addChild(this->linkChildren(rank));
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::reduceLower(const rank_t rank) -> void
    {
        const delinked_nodes delinked {this->delinkChild(rank + 1)};
        this->addDelinkedNodes(delinked);
        queue->extraNodes.push(delinked.extra);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::increaseRank(node_t* const n1, node_t* const n2) -> void
    {
        this->root->add_child(n1);
        this->root->add_child(n2);
        this->sons.push_back(n2);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::decreaseRank() -> void
    {
        this->sons.pop_back();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::increase_domain() -> void
    {
        if ( this->root->rank_ < 3 ) return;

        this->upper.increase_domain();
        this->lower.increase_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::decrease_domain() -> void
    {
        if ( this->root->rank_ < 3 ) return;

        this->upper.decrease_domain();
        this->lower.decrease_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::upperCheckNMinus1() -> void
    {
        const rank_t rank {static_cast<rank_t>(this->root->rank_ - 1)};
        num_t count {node_t::same_rank_count(this->sons[rank])};

        if ( count <= 7 ) return;

        count -= this->lowerCheckNMinus2(3);

        if ( count <= 7 ) return;

        node_t* const firstLinekd  {this->linkChildren(rank)};
        node_t* const secondLinked {this->linkChildren(rank)};

        this->increaseRank(firstLinekd, secondLinked);
        this->increase_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::upperCheckNMinus2() -> void
    {
        if ( this->root->rank_ < 2 ) return;
        
        const rank_t rank {static_cast<rank_t>(this->root->rank_ - 2)};
        const num_t count {node_t::same_rank_count(this->sons[rank])};
        
        if ( count <= 7 ) return;

        this->reduceUpper(rank);
    }
    
    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheckNMinus1() -> void
    {
        const rank_t rank {static_cast<rank_t>(this->root->rank_ - 1)};
        const num_t count {node_t::same_rank_count(this->sons[rank])};

        if ( count >= 2 ) return;

        // Case 1:
        if (this->root->rank_ > 1)
        {
            const num_t countNMinus2 {
                node_t::same_rank_count(this->sons[rank - 1])
            };
            if (countNMinus2 >= 5)
            {
                this->reduceUpper(rank - 1);
                return;
            }
        }

        // Case 2:
        node_t* const node {this->sons[rank]};
        const num_t sonsCount {node_t::same_rank_count(node->child_)};
        if (sonsCount >= 5)
        {
            node_t* const n1 {node->child_};
            node_t* const n2 {n1->right_};
            node_t* const n3 {n2->right_};
            if (n1->is_in_set()) this->queue->T1.removeViolation(n1);
            if (n2->is_in_set()) this->queue->T1.removeViolation(n2);
            if (n3->is_in_set()) this->queue->T1.removeViolation(n3);
            node->remove_child(n1);
            node->remove_child(n2);
            node->remove_child(n3);
            node_t* const linked {node_t::link_nodes(n1, n2, n3)};
            this->addChild(linked);
            return;
        }

        // Case 3:
        this->decrease_domain();
        this->removeChild(node);
        this->decreaseRank();

        const delinked_nodes delinked1 {node_t::delink_node(node)};
        this->addDelinkedNodes(delinked1);
        if (node->rank_ == rank)
        {
            const delinked_nodes delinked2 {node_t::delink_node(node)};
            this->addDelinkedNodes(delinked2);
        }
        this->addChildChecked(node);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheckNMinus2(const num_t bound) -> num_t
    {
        if ( this->root->rank_ < 2 ) return 0;

        const rank_t rank {static_cast<rank_t>(this->root->rank_ - 2)};
        const num_t count {node_t::same_rank_count(this->sons[rank])};

        if ( count >= bound ) return 0;

        const delinked_nodes delinked {this->delinkChild(rank + 1)};

        this->addDelinkedNodes(delinked);
        this->addChild(delinked.extra);
        const rank_t extraRank {delinked.extra->rank_};
        if (extraRank < rank)
        {
            this->upper.inc(extraRank);
        }

        return extraRank != rank + 1 ? 1 : 0;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::linkChildren(const rank_t rank) -> node_t*
    {
        node_t* const n1 {this->sons[rank]};
        node_t* const n2 {n1->right_};
        node_t* const n3 {n2->right_};

        this->removeChild(n3);
        this->removeChild(n2);
        this->removeChild(n1);

        return node_t::link_nodes(n1, n2, n3);
    } 

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::delinkChild(const rank_t rank) -> delinked_nodes<T, Compare>
    {
        node_t* const toDelink {this->sons[rank]};

        this->removeChild(toDelink);

        return node_t::delink_node(toDelink);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::swap(RootWrap & first, RootWrap & second) -> void
    {
        using std::swap;
        swap(first.root,  second.root);
        swap(first.upper, second.upper);
        swap(first.lower, second.lower);
        swap(first.sons,  second.sons);
    }

    // BrodalQueue::T1RootWrap implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::T1RootWrap::T1RootWrap(brodal_queue * const queue) :
        RootWrap {queue}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::operator=(T1RootWrap && other) -> T1RootWrap &
    {
        RootWrap::operator=(std::move(other));

        this->violation = std::move(other.violation);
        this->auxW      = std::move(other.auxW);

        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::copyAuxViolations(const T1RootWrap & other, const node_map & mapping) -> void
    {
        this->violation = other.violation;
        
        for (node_t* node : other.auxW)
        {
            this->auxW.push_back(
                node ? mapping.at(node) : nullptr
            );
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::addViolation(node_t* const node) -> void
    {
        if (node->is_in_set())
        {
            this->removeViolation(node);
        }

        if (node->rank_ >= this->root->rank_)
        {
            this->root->add_to_V(node);
        }
        else if (this->auxW[node->rank_])
        {
            this->auxW[node->rank_]->add_set_sibling(node);
        }
        else
        {
            this->auxW[node->rank_] = node;
            this->root->add_to_W(node);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::removeViolation(node_t* const node) -> void
    {
        if (node->rank_ < this->auxW.size())
        {
            if (this->auxW[node->rank_] == node)
            {
                node_t* const next {node->nextInSet_};
                this->auxW[node->rank_] = (next and next->rank_ == node->rank_ ? next : nullptr);
            }
        }

        node_t::remove_from_set(node);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::violationCheck(const rank_t rank) -> void
    {
        if (rank < this->auxW.size())
        {
            this->violation.inc(rank);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::reduceViolation(const rank_t rank) -> void
    {
        num_t removed {0};
        while (removed < 2)
        {
            removed += this->reduceViolations(rank);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::fullyReduceViolations() -> void
    {
        for (node_t const* node : this->auxW)
        {
            if (not node) continue;

            const rank_t rank {node->rank_};
            num_t count {node_t::same_rank_violation(node)};
            while (count > 1)
            {
                count -= this->reduceViolations(rank);
            }
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::increaseRank(node_t* const linked) -> void
    {
        node_t* const n1    {linked};
        node_t* const n2    {n1->right_};
        node_t* const extra {n2->right_};

        this->increaseRank(n1->disconnect(), n2->disconnect());
        this->increase_domain();

        node_t::fold_right(extra, [=](node_t* const n) {
            this->addChildChecked(n->disconnect());
        });
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::increaseRank(node_t* const n1, node_t* const n2) -> void
    {
        RootWrap::increaseRank(n1, n2);
        this->auxW.push_back(nullptr);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::decreaseRank() -> void
    {
        RootWrap::decreaseRank();
        this->auxW.pop_back();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::increase_domain() -> void
    {
        RootWrap::increase_domain();
        this->violation.increase_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::decrease_domain() -> void
    {
        RootWrap::decrease_domain();
        this->violation.decrease_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::reduceViolations(const rank_t rank) -> num_t
    {
        const node_ptr_pair normal {this->pickNormalViolations(rank)};
        const node_ptr_pair t2sons {this->pickT2SonViolations(rank)};

        const num_t t2SonsRemoved {
            static_cast<num_t>(
                this->removeT2Violation(t2sons.first) + 
                this->removeT2Violation(t2sons.second)
            )
        };

        if (2 == t2SonsRemoved)
        {
            return 2;
        }

        return this->removeNormalViolations(
            normal.first,
            normal.second
        );
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::pickNormalViolations(const rank_t rank) -> node_ptr_pair
    {
        node_t* it {this->auxW[rank]};

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

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::pickT2SonViolations(const rank_t rank) -> node_ptr_pair
    {
        node_t* it {this->auxW[rank]};

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

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::removeT2Violation(node_t* const node) -> num_t
    {
        if (not node)
        {
            return 0;
        }

        if (not node->is_violating())
        {
            this->removeViolation(node);
            return 1;
        }

        this->removeViolation(node);
        this->queue->T2.removeChild(node);
        // No need for lower check since this transformation is 
        // done only if there are more than 4 sons with given rank.
        // So removing the node won't affect lower guide.
        this->queue->T1.addChildChecked(node);

        return 1;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::removeNormalViolations(node_t* const first, node_t* const second) -> num_t
    {
        node_t* const n1 {first};
        node_t* const n2 {second ? second : n1->same_rank_sibling()};

        if (not n1->is_violating())
        {
            this->removeViolation(n1);
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
            this->addChildChecked(n1);
            return 1;
        }

        const bool removeParent {parent->rank_ == n1->rank_ + 1};
        if (removeParent)
        {
            if (parent->parent_ != this->root)
            {
                node_t* const replacement {this->sons[parent->rank_]->right_};
                node_t::swap_tree_nodes(parent, replacement);
                if (replacement->is_violating())
                {
                    this->addViolation(replacement);
                }
            }
            if (parent->is_in_set())
            {
                // Parent will no longer be violating and his rank will change.
                this->removeViolation(parent);
            }
            this->removeChild(parent);
            this->lowerCheck(parent->rank_);
            this->queue->addExtraNodes();
        }
        
        parent->remove_child(n1);
        parent->remove_child(n2);
        
        this->addChildChecked(n1);
        this->addChildChecked(n2);

        if (removeParent)
        {
            this->addChildChecked(parent);
        }

        return removed;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::swap(T1RootWrap & first, T1RootWrap & second) -> void
    {
        RootWrap::swap(first, second);

        using std::swap;
        swap(first.auxW, second.auxW);
        swap(first.violation, second.violation);
    }

    // BrodalQueue::T2RootWrap implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::T2RootWrap::T2RootWrap(brodal_queue * const queue) :
        RootWrap {queue}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::operator=(RootWrap && other) -> T2RootWrap &
    {
        RootWrap::operator=(std::move(other));
        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::operator=(T2RootWrap && other) -> T2RootWrap &
    {
        RootWrap::operator=(std::move(other));
        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::addChild(node_t* const child) -> void
    {
        RootWrap::addChild(child);
        if (child->is_violating())
        {
            this->queue->violations.push(child);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::removeChild(node_t* const child) -> void
    {
        if (child->is_in_set())
        {
            this->queue->T1.removeViolation(child);
        }

        RootWrap::removeChild(child);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::removeLargeSons() -> node_t*
    {
        const rank_t newRootsRank {static_cast<rank_t>(this->root->rank_ - 1)};
        node_t* const ret {this->sons[this->root->rank_ - 1]};

        if (0 == newRootsRank)
        {
            this->root->child_ = nullptr;
            this->root->rank_  = 0; 
        }
        else
        {
            this->root->child_       = this->sons[this->root->rank_ - 2];
            this->root->child_->left_->right_ = nullptr;
            this->root->child_->left_        = nullptr;
            this->root->set_rank();
        }

        node_t::fold_right(ret, [=](node_t* n) {
            if (n->is_in_set())
            {
                this->queue->T1.removeViolation(n);
            }
        });

        return ret;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::swap(T2RootWrap & first, T2RootWrap & second) -> void
    {
        RootWrap::swap(first, second);
    }

    

    
    

    // BrodalQueue implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::brodal_queue(const brodal_queue & other) :
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
        this->T1.copyAuxViolations(other.T1, mapping);
    }

    template<class T, class Compare>
    brodal_queue<T, Compare>::brodal_queue(brodal_queue && other) :
        queueSize {other.queueSize}
    {
        other.queueSize = 0;
        this->T1 = std::move(other.T1);
        this->T2 = std::move(other.T2);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::insert(value_type const& value) -> handle_t
    {
        return this->insert_impl(this->new_node(value));
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::insert(value_type&& value) -> handle_t
    {
        return this->insert_impl(this->new_node(std::move(value)));
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::operator=
        (brodal_queue rhs) -> brodal_queue&
    {
        swap(*this, rhs);
        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::decrease_key
        (handle_t const handle) -> void
    {
        this->dec_key_impl(handle.entry_->node_);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::decrease_key
        (iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::decrease_key
        (const_iterator pos) -> void
    {
        this->dec_key_impl(pos.current());
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::erase
        (handle_t const handle) -> void
    {
        this->erase_impl(handle.entry_->node_);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::erase
        (iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::erase
        (const_iterator pos) -> void
    {
        this->erase_impl(pos.current());
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::delete_min() -> value_type
    {
        if (this->size() < 4)
        {
            return this->deleteMinSpecial();
        }

        this->moveAllToT1();

        T ret {this->find_min()};
        node_t* const oldRoot {this->T1.root};
        node_uptr newRoot {this->findNewRoot()}; 
        if (newRoot->is_in_set())
        {
            // TODO if it is violating we need to remove it from set and auxW
            this->T1.removeViolation(newRoot.get());
        }
        
        if (not newRoot->is_son_of_root())
        {
            this->makeSonOfRoot(newRoot.get());
        }

        this->T1.removeChild(newRoot.get());
        this->T1.lowerCheck(newRoot->rank_);
        this->addExtraNodes();

        node_t* const sonsToAdd {newRoot->disconnect_sons()};
        node_t::swap_entries(newRoot.get(), oldRoot);
        node_t::fold_right(sonsToAdd, [=](node_t* const n) {
            this->T1.addChildChecked(n->disconnect());
        });

        this->mergeSets(newRoot.get());
        this->T1.fullyReduceViolations();

        this->queueSize--;

        return ret;
    }

    template<class T, class Compare>
    template<class... Args>
    auto brodal_queue<T, Compare>::new_node
        (Args&&... args) -> node_t*
    {
        return this->new_node_impl(std::piecewise_construct, std::forward<Args>(args)...);
    }

    template<class T, class Compare>
    template<class... Args>
    auto brodal_queue<T, Compare>::new_node_impl
        (Args&&... args) -> node_t*
    {
        return new node_t(std::forward<Args>(args)...);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::insert_impl
        (node_t* const node) -> handle_t
    {
        if (this->size() < 3)
        {
            return this->insert_special_impl(node);
        }
        
        auto const entry = node->entry_.get();

        this->queueSize++;

        if (*node < *this->T1.root)
        {
            node_t::swap_entries(node, this->T1.root);
        }

        this->T1.addChildChecked(node);

        this->moveToT1();

        return handle_t(entry);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::insert_special_impl
        (node_t* const node) -> handle_t
    {
        this->queueSize++;

        auto const entry = node->entry_.get();

        if (1 == this->size())
        {
            this->T1.root = node;
            return handle_t(entry);
        }
        else if (*node < *this->T1.root)
        {
            node_t::swap_entries(node, this->T1.root);
        }

        if (2 == this->size())
        {
            this->T2.root = node;
        }
        else
        {
            node_t* const oldT2 {this->T2.root};
            this->T2.root = nullptr;
            this->T1.increaseRank(node, oldT2);
            this->T1.increase_domain();
        }

        return handle_t(entry);
    }

    template<class T, class Compare>
    template<class Cmp>
    auto brodal_queue<T, Compare>::dec_key_impl
        (node_t* const node) -> void
    {
        // TODO wtf?
        // this->moveToT1(); 

        if (Cmp {} (**node, **this->T1.root))
        {
            node_t::swap_entries(node, this->T1.root);
        }

        if (node->is_violating() and not node->is_in_set())
        {
            this->T1.addViolation(node);
            this->T1.violationCheck(node->rank_);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::erase_impl
        (node_t* const node) -> void
    {
        node_t::swap_entries(node, this->T1.root);

        if (node->parent_ && T1.root != node->parent_ && !node->is_in_set())
        {
            // At this point node can be violating even if it is son of t1.
            // If that is the case we can't add it to violation set.
            // It would break delete_min since sons of t1 cannot be violating.
            this->T1.addViolation(node);
        }

        this->delete_min();
    }



    template<class T, class Compare>
    auto brodal_queue<T, Compare>::deleteMinSpecial() -> value_type
    {
        T ret {this->find_min()};
        node_uptr oldRoot {this->T1.root};

        if (1 == this->queueSize)
        {
            this->T1.root = nullptr;
        }
        else if (2 == this->queueSize)
        {
            this->T1.root = this->T2.root;
            this->T2.root = nullptr;
        }
        else
        {
            node_t* const firstChild  {this->T1.root->disconnect_sons()};
            node_t* const secondChild {firstChild->right_};

            firstChild->disconnect();
            secondChild->disconnect();

            if (*firstChild < *secondChild)
            {
                this->T1.root = firstChild;
                this->T2.root = secondChild;
            }
            else
            {
                this->T1.root = secondChild;
                this->T2.root = firstChild;
            }

            this->T1.decreaseRank();
            this->T1.decrease_domain();
        }

        this->queueSize--;

        return ret;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::find_min() -> reference
    {
        this->isEmptyCheck();
        this->moveToT1();

        return **this->T1.root;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::find_min() const -> const_reference
    {
        this->isEmptyCheck();

        return **this->T1.root;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::size() const -> size_type
    {
        return this->queueSize;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::max_size() const -> size_type
    {
        return std::numeric_limits<size_type>::max();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::empty() const -> bool
    {
        return 0 == this->size();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::clear() -> void
    {
        *this = brodal_queue {};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::begin() -> iterator
    {
        return iterator {this->T1.root, this->T2.root};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::end() -> iterator
    {
        return iterator {};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::begin() const -> const_iterator
    {
        return const_iterator {this->T1.root, this->T2.root};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::end() const -> const_iterator
    {
        return const_iterator {};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::cbegin() const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->begin();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::cend() const -> const_iterator
    {
        return const_cast<brodal_queue const*>(this)->end();
    }

    template<class T, class Compare>
    auto meld(brodal_queue<T, Compare> & first, brodal_queue<T, Compare> & second) -> brodal_queue<T, Compare>
    {
        using brodal_queue_t = brodal_queue<T, Compare>;
        using t1_wrap_ref  = typename brodal_queue<T, Compare>::t1_wrap_ref;
        using wrap_ref     = typename brodal_queue<T, Compare>::wrap_ref;
        
        if (first.empty() and second.empty())
        {
            return brodal_queue_t {};
        }

        t1_wrap_ref newt1 {brodal_queue_t::findNewT1(first, second)};
        wrap_ref    newt2 {brodal_queue_t::findNewT2(first, second, newt1)};

        if (0 == newt2.root->rank)
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

    template<class T, class Compare>
    auto swap(brodal_queue<T, Compare> & first, brodal_queue<T, Compare> & second) noexcept -> void
    {
        std::swap(first.queueSize, second.queueSize);

        using t1_wrap = typename brodal_queue<T, Compare>::T1RootWrap;
        using t2_wrap = typename brodal_queue<T, Compare>::T2RootWrap;

        t1_wrap::swap(first.T1, second.T1);
        t2_wrap::swap(first.T2, second.T2);
    }

    template<class T, class Compare>
    auto operator==
        (brodal_queue<T, Compare> const& lhs, brodal_queue<T, Compare> const& rhs) -> bool
    {
        return lhs.size() == rhs.size()
            && std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    template<class T, class Compare>
    auto operator!=
        (brodal_queue<T, Compare> const& lhs, brodal_queue<T, Compare> const& rhs) -> bool
    {
        return ! (lhs == rhs);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::addExtraNodes() -> void
    {
        while (not this->extraNodes.empty())
        {
            node_t* const node {this->extraNodes.top()};
            this->extraNodes.pop();
            if (node->rank_ < this->T1.root->rank_)
            {
                this->T1.addChildChecked(node);
            }
            else
            {
                this->T2.addChildChecked(node);
            }
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::addViolations() -> void
    {
        while (not this->violations.empty())
        {
            node_t* const violation {this->violations.top()};
            this->violations.pop();
            this->T1.addViolation(violation);
            this->T1.violationCheck(violation->rank_);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::isEmptyCheck() -> void
    {
        if (this->empty())
        {
            throw std::out_of_range("Priority queue is empty.");
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::moveAllToT1() -> void
    {
        if (not this->T2.root)
        {
            return;
        }

        // Disconnect sons of t2.
        node_t* const oldt2         {this->T2.root};
        node_t* const rightFoldable {this->T2.sons[0]};
        node_t* const leftFoldable  {rightFoldable->left_};
        oldt2->disconnect_sons();
        // Reset t2 wrap to defaults.
        this->T2 = T2RootWrap {this};

        // Add sons with rank 0 under t1.
        node_t::fold_right(rightFoldable, [=](node_t* const n) {
            this->T1.addChildChecked(n->disconnect());
        });

        // Add other sons with ranks > 0 under t1.
        node_t::fold_left(leftFoldable, [=](node_t* const n) {
            n->disconnect();
            
            while (n->rank_ >= this->T1.root->rank_)
            {
                const delinked_nodes delinked {node_t::delink_node(n)};
                this->T1.addDelinkedNodesChecked(delinked);
            }
            
            this->T1.addChildChecked(n);
        });

        // Finally add t2 as son of t1.
        this->T1.addChildChecked(oldt2);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::moveToT1() -> void
    {
        if (not this->T2.root or this->size() < 4)
        {
            return;
        }

        if (this->T2.root->rank_ <= this->T1.root->rank_ + 2) // r(t2) is (r(t1) + 1) or (r(t1) + 2)
        {
            node_t* const sons1 {this->T2.removeLargeSons()};
            if (this->T2.root->rank_ > this->T1.root->rank_)
            {
                node_t* const sons2 {this->T2.removeLargeSons()};
                this->T1.increaseRank(sons2);
            }
            this->T1.increaseRank(sons1);

            node_t* const oldt2 {this->T2.releaseRoot()};
            this->T1.addChildChecked(oldt2);

            // Reset t2 wrap to defaults. 
            this->T2 = T2RootWrap {this};
        }
        else
        {
            const rank_t t1rank {this->T1.root->rank_};
            node_t* const toDelink  {this->T2.sons[t1rank + 1]};
            this->T2.removeChild(toDelink);
            this->T2.lowerCheck(toDelink->rank_);

            const delinked_nodes delinked {node_t::delink_node(toDelink)};
            this->T1.increaseRank(delinked.first, delinked.second);
            this->T1.increase_domain();
            if (delinked.third)
            {
                this->T1.addChildChecked(delinked.third);
            }
            
            // In case toDelink had more than 3 sons of rank (n-1).
            while (toDelink->rank_ == this->T1.root->rank_)
            {
                const delinked_nodes delinked {node_t::delink_node(toDelink)};
                this->T1.addDelinkedNodesChecked(delinked);
            }

            this->T1.addChildChecked(toDelink);

            // These might have been created while doing
            // delinking under t2.
            this->addExtraNodes();
            this->addViolations();
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::findNewRoot() const -> node_t*
    {
        node_t* newRoot {this->T1.root->child_};

        node_t::fold_right(newRoot, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::fold_next(this->T1.root->setW_, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::fold_next(this->T1.root->setV_, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        return newRoot;
    } 

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::makeSonOfRoot(node_t* const newRoot) -> void
    {
        node_t* const swapped {this->T1.sons[newRoot->rank_]->right_};
        node_t::swap_tree_nodes(newRoot, swapped);
        if (swapped->is_violating())
        {
            this->T1.addViolation(swapped);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::mergeSets(node_t* const newRoot) -> void
    {
        node_t::fold_next(this->T1.root->setV_, [=](node_t* const n) {
            this->T1.addViolation(n);
        });

        node_t::fold_next(newRoot->setW_, [=](node_t* const n) {
            this->T1.addViolation(n);
        });

        node_t::fold_next(newRoot->setV_, [=](node_t* const n) {
            this->T1.addViolation(n);
        });
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::makeNodeMapping(const brodal_queue & original, const brodal_queue & copy) -> node_map
    {
        node_map map;
        
        if (original.T1.root)
        {
            brodal_queue::nodeMappingOfTree(original.T1, copy.T1, map);
        }

        if (original.T2.root)
        {
            brodal_queue::nodeMappingOfTree(original.T2, copy.T2, map);
        }

        return map;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::nodeMappingOfTree(cwrap_ref original, cwrap_ref copy, node_map & map) -> void
    {
        using tree_iterator_t = brodal_tree_iterator<T, Compare>;
        
        tree_iterator_t itOriginal  {original.root};
        tree_iterator_t originalEnd {nullptr};
        tree_iterator_t itCopy      {copy.root};

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
        node_t::zip_with(original.root->child_, copy.root->child_, 
            [&](node_t* const n1, node_t* const n2) {
                if (not n1->left_
                    or  n1->left_->rank_ != n1->rank_)
                {
                    map[n1] = n2;
                }
            }
        );
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::dumbMeld(brodal_queue & first, brodal_queue & second) -> brodal_queue
    {
        brodal_queue<T, Compare> newQueue {};

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

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::findNewT1(brodal_queue & first, brodal_queue & second) -> t1_wrap_ref
    {
        if (not first.T1.root)  return second.T1;
        if (not second.T1.root) return first.T1;

        return *first.T1.root < *second.T1.root ? first.T1 : second.T1;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::findNewT2(brodal_queue & first, brodal_queue & second, t1_wrap_ref t1) -> wrap_ref
    {
        wrap_ptr newT2 {&t1};

        if (first.T1.root and first.T1.root->rank > newT2->root->rank)
        {
            newT2 = &first.T1;
        }

        if (first.T2.root and first.T2.root->rank > newT2->root->rank)
        {
            newT2 = &first.T2;
        }

        if (second.T1.root and second.T1.root->rank > newT2->root->rank)
        {
            newT2 = &second.T1;
        }

        if (second.T2.root and second.T2.root->rank > newT2->root->rank)
        {
            newT2 = &second.T2;
        }

        return *newT2;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::addUnderRoot(wrap_ref root, wrap_ref toAdd) -> void
    {
        if (not toAdd.root)
        {
            return;
        }
        
        node_t const* node {toAdd.releaseRoot()};

        while (node->rank_ == root.root->rank)
        {
            const delinked_nodes delinked {node_t::delink_node(node)};
            root.addDelinkedNodesChecked(delinked);
        }

        root.addChildChecked(node);
    }

}

#endif