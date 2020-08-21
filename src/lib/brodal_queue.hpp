#ifndef MIX_DS_BRODAL_QUEUE_HPP
#define MIX_DS_BRODAL_QUEUE_HPP

#include <vector>
#include <cstdint>
#include <memory>
#include <string>
#include <stack>

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
     *  Result of delinkind a single Brodal node.
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
        Node of a brodal tree.
     */
    template<class T, class Compare>
    class brodal_node
    {
    public:
        using rank_t    = std::uint8_t;
        using num_t     = std::uint8_t;
        using node_map  = std::unordered_map<brodal_node const*, brodal_node*>;
        using data_uptr = std::unique_ptr<T>;
        using node_t    = brodal_node;

    public:
        template<class... Args>
        brodal_node  (std::piecewise_construct_t, Args&&... args);
        brodal_node  (brodal_node const& other);
        ~brodal_node ();

        auto operator*       ()       -> T&;
        auto operator*       () const -> T const&;
        auto operator<       (brodal_node const& other) const -> bool;
        auto addChild        (brodal_node* const newChild) -> void;
        auto removeChild     (brodal_node* const oldChild) -> void;
        auto addRightSibling (brodal_node* const sibling)  -> void;

        auto addToW (brodal_node* const node) -> void;
        auto addToV (brodal_node* const node) -> void;
        auto addRightViolationSibling (brodal_node* const sibling) -> void;

        auto disconnectSons  () -> brodal_node*;
        auto disconnect      () -> brodal_node*;
        auto setRank         () -> void;
        auto sameRankSibling () const -> brodal_node*;
        auto isSonOfRoot     () const -> bool;
        auto isViolating     () const -> bool;
        auto isInSet         () const -> bool;

        static auto areSiblings       (node_t* const first, node_t* const second) -> bool;
        static auto swapEntries       (node_t* const first, node_t* const second) -> void;
        static auto swapTreeNodes     (node_t* const first, node_t* const second) -> void;
        static auto makeSiblings      (node_t* const first, node_t* const second) -> void;
        static auto sameRankCount     (node_t const* const node) -> num_t;
        static auto sameRankViolation (node_t const* const node) -> num_t;
        static auto removeFromSet     (node_t* const node)  -> void;
        static auto copyList          (node_t* const first) -> node_t*;
        static auto copySet           (node_t* const first, node_map const& mapping) -> node_t*;
        static auto delinkNode        (node_t* const node)  -> delinked_nodes<T, Compare>;
        static auto linkNodes         (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*;
        static auto selectMaxPrio     (node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*;

        template<class UnaryFunction>
        static auto foldRight (node_t* const first, UnaryFunction func) -> void;

        template<class UnaryFunction>
        static auto foldLeft (node_t* const first, UnaryFunction func) -> void;

        template<class UnaryFunction>
        static auto foldNext (node_t* const first, UnaryFunction func) -> void;

        template<class BinaryFunction>
        static auto zipWith (node_t* const first1, node_t* const first2, BinaryFunction func) -> void;

    public:
        rank_t rank {0};

        data_uptr entry {nullptr};
        
        brodal_node* parent {nullptr};
        brodal_node* left   {nullptr};
        brodal_node* right  {nullptr};
        brodal_node* child  {nullptr};

        brodal_node* nextInSet {nullptr};
        brodal_node* prevInSet {nullptr};
        brodal_node* setW      {nullptr};
        brodal_node* setV      {nullptr};
    };

    /**
        Iterator of a single brodal tree.
     */
    template<class T>
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
        std::stack<node_t*> stack_;
    };

    /**
        Node handle that is returned after an insertion
        and can be used for decrease_key and erase.
     */
    template<class T, class Compare, class Allocator>
    class brodal_node_handle
    {
    public:
        auto operator*  ()       -> T&;
        auto operator*  () const -> T const&;
        auto operator-> ()       -> T*;
        auto operator-> () const -> T const*;

    private:
        using node_t = brodal_node<T, Compare>;
        template<class, class>
        friend class brodal_queue;
        brodal_node_handle(node_t* const node);
        node_t* node_;
    };

    
    /**
        Iterator for a single Brodal tree.
     */
    // template<class T, class Compare, class Allocator, class Val>
    // class brodal_tree_iterator
    // {
    // public:
    //     using difference_type   = std::ptrdiff_t;
    //     using value_type        = Val;
    //     using pointer           = value_type*;
    //     using reference         = value_type&;
    //     using iterator_category = std::forward_iterator_tag;
    //     using node_t            = brodal_node<T>;
    
    // public:
    //     brodal_tree_iterator () = default;
    //     brodal_tree_iterator (node_t* const root);

    //     auto operator++ ()       -> p_tree_iterator&;
    //     auto operator++ (int)    -> p_tree_iterator;
    //     auto operator*  () const -> reference;
    //     auto operator-> () const -> pointer;
    //     auto operator== (brodal_tree_iterator const&) const -> bool;
    //     auto operator!= (brodal_tree_iterator const&) const -> bool;

    // private:
    //     auto current () const -> node_t*;

    // private:
    //     std::queue<node_t*> queue_;
    // };


// brodal_queue declaration:

    // Ugly forward declarations:
    template<class T, class Compare>
    class brodal_queue;

    template<class T, class Compare, class Value, class Pointer, class Reference>
    using brodal_iterator = typename brodal_queue<T, Compare>::template BrodalQueueIterator<Value, Pointer, Reference>;

    template<class T, class Compare>
    auto swap(brodal_queue<T, Compare> & first, 
              brodal_queue<T, Compare> & second) noexcept -> void;

    template<class T, class Compare>
    auto meld(brodal_queue<T, Compare> & first, 
              brodal_queue<T, Compare> & second) -> brodal_queue<T, Compare>;

    template<class T, class Compare, class Value, class Pointer, class Reference>
    auto swap(brodal_iterator<T, Compare, Value, Pointer, Reference> & first, 
              brodal_iterator<T, Compare, Value, Pointer, Reference> & second) noexcept -> void;
    
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

        // using node_ptr       = brodal_node<T, Compare> *;
        // using node_cptr      = node_ptr const;
        // using cnode_ptr      = const brodal_node<T, Compare> *;
        // using cnode_cptr     = const brodal_node<T, Compare> * const;
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

            UpperReducer(const UpperReducer &) = default;
            UpperReducer(UpperReducer &&)      = default;
            ~UpperReducer()                    = default;

            UpperReducer(RootWrap * const root);

            auto operator= (UpperReducer && rhs) -> UpperReducer &;

            auto reduce (const index_t i)       -> void;
            auto get_num (const index_t i) const -> num_t;
        };

        struct LowerReducer
        {
            RootWrap * managedRoot {nullptr};

            LowerReducer(const LowerReducer &) = default;
            LowerReducer(LowerReducer &&)      = default;
            ~LowerReducer()                    = default;

            LowerReducer(RootWrap * const root);

            auto operator= (LowerReducer && rhs) -> LowerReducer &;

            auto reduce (const index_t i)       -> void;
            auto get_num (const index_t i) const -> num_t;
        };

        struct ViolationReducer
        {
            T1RootWrap * managedRoot {nullptr};

            // TODO nope, rule of zero...
            ViolationReducer(const ViolationReducer &) = default;
            ViolationReducer(ViolationReducer &&)      = default;
            ~ViolationReducer()                        = default;

            ViolationReducer(T1RootWrap * const root);

            auto operator= (ViolationReducer && rhs) -> ViolationReducer &;

            auto reduce (const index_t i)       -> void;
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

        struct IteratorNode
        {
            node_t const* node {nullptr};
            iter_node_uptr fallback {nullptr};
            bool childVisited {false};

            IteratorNode(IteratorNode && other) = default;

            IteratorNode(node_t const* node);
            IteratorNode(node_t const* node, iter_node_uptr && fallback);
            IteratorNode(const IteratorNode & other);
            auto operator= (IteratorNode && rhs) -> IteratorNode &;

            auto operator== (const IteratorNode & rhs) const -> bool;
            auto operator!= (const IteratorNode & rhs) const -> bool;
        };

        class TreeIterator
        {
        private:
			// TODO možno netreba alokovať dynamicky? stačil by objekt s operator=
            iter_node_uptr current {nullptr};            

        public:
            TreeIterator(node_t const* node);
            TreeIterator(const TreeIterator & other);
            TreeIterator(TreeIterator && other) = default;

            auto operator== (const TreeIterator & rhs) const -> bool;
            auto operator!= (const TreeIterator & rhs) const -> bool;
            auto operator*  () const -> node_t*;
            auto operator-> () const -> node_t*;
            auto operator++ ()       -> TreeIterator &;

            auto swap (TreeIterator & other) noexcept -> void;
        };

    public:  // Iterator: // TODO std::iterator deprecated remove it
        template<class Value, class Pointer, class Reference>
        class BrodalQueueIterator : public std::iterator<std::forward_iterator_tag, Value, std::ptrdiff_t, Pointer, Reference>
        {
        private:
            TreeIterator t1Iterator {nullptr};
            TreeIterator t2Iterator {nullptr};

        public:
            BrodalQueueIterator(const BrodalQueueIterator &) = default;
            BrodalQueueIterator(BrodalQueueIterator &&)      = default;
            BrodalQueueIterator(node_t const* t1, node_t const* t2);
            
            auto operator= (BrodalQueueIterator rhs) -> BrodalQueueIterator &;

            auto operator!= (const BrodalQueueIterator & rhs) const -> bool;
            auto operator== (const BrodalQueueIterator & rhs) const -> bool;

            auto operator*  () const -> Reference;
            auto operator-> () const -> Pointer;

            auto operator++ ()    -> BrodalQueueIterator &;
            auto operator++ (int) -> BrodalQueueIterator;

            friend auto swap<T, Compare, Value, Pointer, Reference>(BrodalQueueIterator & first, BrodalQueueIterator & second) noexcept -> void;

        private:
            auto activeTree ()       -> TreeIterator &;
            auto activeTree () const -> const TreeIterator &;
        };

    public:  // Member types:
        using value_type      = T;
        using pointer         = value_type *;
        using reference       = value_type &;
        using const_reference = const value_type &;
        using iterator        = BrodalQueueIterator<value_type, pointer, reference>;
        using const_iterator  = BrodalQueueIterator<const value_type, const pointer, const reference>;
        using difference_type = std::ptrdiff_t;
        using size_type       = size_t;
        using handle_t        = brodal_node_handle<T, Compare, int>;
        

    private: // BrodalQueue members:
        size_t queueSize {0};

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
        auto decrease_key (handle_t holder)         -> void;
        auto delete_min   ()                        -> value_type;
        auto find_min     ()                        -> reference;
        auto find_min     () const                  -> const_reference;
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

// brodal_node definition:

    template<class T, class Compare>
    template<class... Args>
    brodal_node<T, Compare>::brodal_node
        (std::piecewise_construct_t, Args&&... args) :
        entry {std::make_unique<T>(std::forward<Args>(args)...)}
    {
    }

    template<class T, class Compare>
    brodal_node<T, Compare>::brodal_node
        (brodal_node const& other) :
        rank  {other.rank},
        entry {std::make_unique<T>(*other.entry)},
        child {brodal_node::copyList(other.child)}
    {
        brodal_node::foldRight(this->child, [=](node_t const* n) {
            n->parent = this;
        });
    }

    template<class T, class Compare>
    brodal_node<T, Compare>::~brodal_node()
    {
        brodal_node::foldRight(this->child, [](brodal_node * n) { delete n; });
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator*
        () -> T&
    {
        return *entry;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator*
        () const -> T const&
    {
        return *entry;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::operator<
        (brodal_node const& other) const -> bool
    {
        return Compare {} (**this, *other);
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::addChild(node_t* const newChild) -> void
    {
        newChild->parent = this;

        if (this->child)
        {
            this->child->left = newChild;
            newChild->right   = this->child;
            this->child       = newChild;
        }
        else
        {
            this->child = newChild;
        }
        
        this->setRank();
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::removeChild(node_t* const oldChild) -> void
    {
        if (this->child == oldChild)
        {
            this->child = oldChild->right;
        }

        if (oldChild->left)
        {
            oldChild->left->right = oldChild->right;
        }

        if (oldChild->right)
        {
            oldChild->right->left = oldChild->left;
        }

        oldChild->parent = nullptr;
        oldChild->left   = nullptr;
        oldChild->right  = nullptr;

        this->setRank();
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::addRightSibling(node_t* const sibling) -> void
    {
        sibling->parent = this->parent;
        sibling->left   = this;
        sibling->right  = this->right;
        
        if (this->right) 
        {
            this->right->left = sibling;
        }

        this->right = sibling;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::addToW(node_t* const node) -> void
    {
        if (this->setW)
        {
            this->setW->prevInSet = node;
        }
        
        node->nextInSet = this->setW;
        node->prevInSet = this;
        this->setW = node;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::addToV(node_t* const node) -> void
    {
        if (this->setV)
        {
            this->setV->prevInSet = node;
        }

        node->nextInSet = this->setV;
        node->prevInSet = this;
        this->setV = node;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::addRightViolationSibling(node_t* const sibling) -> void
    {
        sibling->prevInSet = this;
        sibling->nextInSet = this->nextInSet;
        if ( this->nextInSet ) this->nextInSet->prevInSet = sibling;
        this->nextInSet = sibling; 
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::disconnectSons() -> node_t*
    {
        brodal_node::foldRight(this->child, [](node_t* const n) {
            n->parent = nullptr;
        });

        node_t* ret {this->child};
        this->child = nullptr;
        this->rank  = 0;

        return ret;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::disconnect() -> node_t*
    {
        this->left  = nullptr;
        this->right = nullptr;
        
        return this;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::setRank() -> void
    {
        this->rank = this->child ? this->child->rank + 1 : 0;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::sameRankSibling() const -> node_t*
    {
        if (this->left and this->left->rank == this->rank)
        {
            return this->left;
        }

        if (this->right and this->right->rank == this->rank)
        {
            return this->right;
        }

        return nullptr;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::isSonOfRoot() const -> bool
    {
        return this->parent and nullptr == this->parent->parent;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::isViolating() const -> bool
    {
        return this->parent and *this < *this->parent;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::isInSet() const -> bool
    {
        return this->prevInSet != nullptr;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::areSiblings
        (node_t* const first, node_t* const second) -> bool
    {
        return first->parent == second->parent;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::swapEntries(node_t* const first, node_t* const second) -> void
    {
        using std::swap;
        swap(first->entry, second->entry);
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::swapTreeNodes(node_t* const first, node_t* const second) -> void
    {
        // Swap parent pointers:
        if (first->parent->child  == first)  first->parent->child  = second;
        if (second->parent->child == second) second->parent->child = first;

        std::swap(first->parent, second->parent);

        // Swap left and right pointers:
        if (second->left)  second->left->right = first;
        if (second->right) second->right->left = first;
        if (first->left)   first->left->right = second;
        if (first->right)  first->right->left = second; 
        
        std::swap(first->left,  second->left);
        std::swap(first->right, second->right);
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::makeSiblings(node_t* const first, node_t* const second) -> void
    {
        if (*first->parent < *second->parent)
        {
            brodal_node::swapTreeNodes(first, second->sameRankSibling());
        }
        else
        {
            brodal_node::swapTreeNodes(first->sameRankSibling(), second);
        }
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::sameRankCount(node_t const* const node) -> num_t
    {
        num_t count {1};
        node_t const*  left  {node->left};
        node_t const*  right {node->right};

        while (left and left->rank == node->rank)
        {
            count++;
            left = left->left;
        }

        while (right and right->rank == node->rank)
        {
            count++;
            right = right->right;
        }

        return count;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::sameRankViolation(node_t const* const node) -> num_t
    {
        num_t count {1};

        node_t const*  prev {node->prevInSet};
        node_t const*  next {node->nextInSet};

        while (prev and prev->rank == node->rank)
        {
            count++;
            prev = prev->prevInSet;
        }

        while (next and next->rank == node->rank)
        {
            count++;
            next = next->nextInSet;
        }

        return count;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::removeFromSet(node_t* const node) -> void
    {
        if (node->prevInSet->setW == node)
        {
            node->prevInSet->setW = node->nextInSet;
            if (node->nextInSet) node->nextInSet->prevInSet = node->prevInSet;
        }
        else if (node->prevInSet->setV == node)
        {
            node->prevInSet->setV = node->nextInSet;
            if (node->nextInSet) node->nextInSet->prevInSet = node->prevInSet;
        }
        else
        {
            node->prevInSet->nextInSet = node->nextInSet;
            if (node->nextInSet) node->nextInSet->prevInSet = node->prevInSet;
        }

        node->prevInSet = nullptr;
        node->nextInSet = nullptr;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::copyList(node_t* const first) -> node_t*
    {
        if (not first) return nullptr;

        node_t*  newListLast  {new brodal_node(*first)};
        node_t const* newListFirst {newListLast};

        brodal_node::foldRight(first->right, [&](node_t const* n) {
            auto newNode {new brodal_node(*n)};

            newListLast->right = newNode;
            newNode->left      = newListLast;
            newListLast        = newNode;
        });

        return newListFirst;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::copySet(node_t* const first, node_map const& mapping) -> node_t*
    {
        if (not first) return nullptr;

        node_t* newSetLast   {mapping.at(first)};
        node_t const* newSetFirst {newSetLast};

        brodal_node::foldNext(first->nextInSet, [&](node_t* n){
            node_t* mappedNode {mapping.at(n)};

            newSetLast->nextInSet = mappedNode;
            mappedNode->prevInSet = newSetLast;
            newSetLast            = mappedNode;
        });

        return newSetFirst;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::delinkNode(node_t* const node) -> delinked_nodes<T, Compare>
    {
        const num_t sonCount   {brodal_node::sameRankCount(node->child)};
        const num_t removeSons {sonCount >= 4 ? static_cast<num_t>(2) : sonCount};
        
        node_t* const n1 {node->child};
        node_t* const n2 {n1->right};
        node_t* const n3 {removeSons > 2 ? n2->right : nullptr};

        node->removeChild(n1);
        node->removeChild(n2);
        if (n3) node->removeChild(n3);

        return {n1, n2, n3, node};
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::linkNodes(node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*
    {
        node_t* root {brodal_node::selectMaxPrio(n1, n2, n3)};

        if (n1 != root)
        {
            root->addChild(n1);
        }

        if (n2 != root)
        {
            root->addChild(n2);
        }

        if (n3 != root)
        {
            root->addChild(n3);
        }

        return root;
    }

    template<class T, class Compare>
    auto brodal_node<T, Compare>::selectMaxPrio(node_t* const n1, node_t* const n2, node_t* const n3) -> node_t*
    {
        node_t* max {n1};

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
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::foldRight(node_t* const first, UnaryFunction func) -> void
    {
        node_t* it {first};
        while (it)
        {
            node_t* const next {it->right};
            func(it);
            it = next;
        }
    }

    template<class T, class Compare>
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::foldLeft(node_t* const first, UnaryFunction func) -> void
    {
        node_t* it {first};
        while (it)
        {
            node_t* const next {it->left};
            func(it);
            it = next;
        }
    }

    template<class T, class Compare>
    template<class UnaryFunction>
    auto brodal_node<T, Compare>::foldNext(node_t* const first, UnaryFunction func) -> void
    {
        node_t* it {first};
        while (it)
        {
            node_t* const next {it->nextInSet};
            func(it);
            it = next;
        }
    }

    template<class T, class Compare>
    template<class BinaryFunction>
    auto brodal_node<T, Compare>::zipWith(node_t* const fold1, node_t* const fold2, BinaryFunction func) -> void
    {
        node_t* it1 {fold1};
        node_t* it2 {fold2};

        while (it1 and it2)
        {
            node_t* const next1 {it1->right};
            node_t* const next2 {it2->right};
            
            func(it1, it2);

            it1 = next1;
            it2 = next2;
        }
    }

// brodal_tree_iterator definition:

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
        return (stack_.empty() && rhs.stack_.empty())
            || (stack_.size()  == rhs.stack_.size()
            &&  stack_.front() == rhs.stack_.front());
    }

    template<class T, class Compare, class MergeMode, class Allocator, class Val>
    auto p_tree_iterator<T, Compare, MergeMode, Allocator, Val>::operator!=
        (p_tree_iterator const& rhs) const -> bool
    {
        return !(*this == rhs);
    }

// brodal_node_handle definition:

    template<class T, class Compare, class Allocator>
    brodal_node_handle<T, Compare, Allocator>::brodal_node_handle
        (node_t* const node) :
        node_ {node}
    {
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node_handle<T, Compare, Allocator>::operator*
        () -> T&
    {
        return **node_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node_handle<T, Compare, Allocator>::operator*
        () const -> T const&
    {
        return **node_;
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node_handle<T, Compare, Allocator>::operator->
        () -> T*
    {
        return std::addressof(**node_);
    }

    template<class T, class Compare, class Allocator>
    auto brodal_node_handle<T, Compare, Allocator>::operator->
        () const -> T const*
    {
        return std::addressof(**node_);
    }

// brodal_queue definition:
    

    // BrodalQueue::UpperReducer implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::UpperReducer::UpperReducer(RootWrap * const root) :
        managedRoot {root}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::UpperReducer::operator=(UpperReducer && rhs) -> UpperReducer &
    {
        this->managedRoot = rhs.managedRoot;
        rhs.managedRoot   = nullptr;
        
        return *this;
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
            node_t::sameRankCount(this->managedRoot->sons[i])
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
    auto brodal_queue<T, Compare>::LowerReducer::operator=(LowerReducer && rhs) -> LowerReducer &
    {
        this->managedRoot = rhs.managedRoot;
        rhs.managedRoot   = nullptr;
        
        return *this;
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
            node_t::sameRankCount(this->managedRoot->sons[i])
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
    auto brodal_queue<T, Compare>::ViolationReducer::operator=(ViolationReducer && rhs) -> ViolationReducer &
    {
        this->managedRoot = rhs.managedRoot;
        rhs.managedRoot   = nullptr;
        
        return *this;
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
            node_t::sameRankViolation(this->managedRoot->auxW[i])
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
        TreeIterator otherIt  {other.root};
        TreeIterator otherEnd {nullptr};
        TreeIterator thisIt   {this->root};

        while (otherIt != otherEnd)
        {
            node_t const* otherNode {*otherIt};
            node_t const* thisNode  {*thisIt};
            node_t*  setWCopy  {node_t::copySet(otherNode->setW, mapping)};
            node_t*  setVCopy  {node_t::copySet(otherNode->setV, mapping)};

            if (setWCopy)
            {
                setWCopy->prevInSet = thisNode;
                thisNode->setW      = setWCopy;
            }

            if (setVCopy)
            {
                setVCopy->prevInSet = thisNode;
                thisNode->setV      = setVCopy;
            }

            ++otherIt;
            ++thisIt;
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addChild(node_t* const child) -> void
    {
        if (child->isInSet())
        {
            this->queue->T1.removeViolation(child);
        }

        this->sons[child->rank]->addRightSibling(child);
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
        if (this->sons[child->rank] == child)
        {
            this->sons[child->rank] = child->right;
        }

        this->root->removeChild(child);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::addChildChecked(node_t* const child) -> void
    {
        this->addChild(child);
        this->upperCheck(child->rank);
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
        if (this->root->rank > 2 and rank < this->root->rank - 2)
        {
            this->upper.inc(rank);
        }

        this->upperCheckNMinus2();
        this->upperCheckNMinus1();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheck(const rank_t rank) -> void
    {
        if (this->root->rank > 2 and rank < this->root->rank - 2)
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
        this->root->addChild(n1);
        this->root->addChild(n2);
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
        if ( this->root->rank < 3 ) return;

        this->upper.increase_domain();
        this->lower.increase_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::decrease_domain() -> void
    {
        if ( this->root->rank < 3 ) return;

        this->upper.decrease_domain();
        this->lower.decrease_domain();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::upperCheckNMinus1() -> void
    {
        const rank_t rank {static_cast<rank_t>(this->root->rank - 1)};
        num_t count {node_t::sameRankCount(this->sons[rank])};

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
        if ( this->root->rank < 2 ) return;
        
        const rank_t rank {static_cast<rank_t>(this->root->rank - 2)};
        const num_t count {node_t::sameRankCount(this->sons[rank])};
        
        if ( count <= 7 ) return;

        this->reduceUpper(rank);
    }
    
    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheckNMinus1() -> void
    {
        const rank_t rank {static_cast<rank_t>(this->root->rank - 1)};
        const num_t count {node_t::sameRankCount(this->sons[rank])};

        if ( count >= 2 ) return;

        // Case 1:
        if (this->root->rank > 1)
        {
            const num_t countNMinus2 {
                node_t::sameRankCount(this->sons[rank - 1])
            };
            if (countNMinus2 >= 5)
            {
                this->reduceUpper(rank - 1);
                return;
            }
        }

        // Case 2:
        node_t* const node {this->sons[rank]};
        const num_t sonsCount {node_t::sameRankCount(node->child)};
        if (sonsCount >= 5)
        {
            node_t* const n1 {node->child};
            node_t* const n2 {n1->right};
            node_t* const n3 {n2->right};
            if (n1->isInSet()) this->queue->T1.removeViolation(n1);
            if (n2->isInSet()) this->queue->T1.removeViolation(n2);
            if (n3->isInSet()) this->queue->T1.removeViolation(n3);
            node->removeChild(n1);
            node->removeChild(n2);
            node->removeChild(n3);
            node_t* const linked {node_t::linkNodes(n1, n2, n3)};
            this->addChild(linked);
            return;
        }

        // Case 3:
        this->decrease_domain();
        this->removeChild(node);
        this->decreaseRank();

        const delinked_nodes delinked1 {node_t::delinkNode(node)};
        this->addDelinkedNodes(delinked1);
        if (node->rank == rank)
        {
            const delinked_nodes delinked2 {node_t::delinkNode(node)};
            this->addDelinkedNodes(delinked2);
        }
        this->addChildChecked(node);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::lowerCheckNMinus2(const num_t bound) -> num_t
    {
        if ( this->root->rank < 2 ) return 0;

        const rank_t rank {static_cast<rank_t>(this->root->rank - 2)};
        const num_t count {node_t::sameRankCount(this->sons[rank])};

        if ( count >= bound ) return 0;

        const delinked_nodes delinked {this->delinkChild(rank + 1)};

        this->addDelinkedNodes(delinked);
        this->addChild(delinked.extra);
        const rank_t extraRank {delinked.extra->rank};
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
        node_t* const n2 {n1->right};
        node_t* const n3 {n2->right};

        this->removeChild(n3);
        this->removeChild(n2);
        this->removeChild(n1);

        return node_t::linkNodes(n1, n2, n3);
    } 

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::delinkChild(const rank_t rank) -> delinked_nodes<T, Compare>
    {
        node_t* const toDelink {this->sons[rank]};

        this->removeChild(toDelink);

        return node_t::delinkNode(toDelink);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::RootWrap::swap(RootWrap & first, RootWrap & second) -> void
    {
        std::swap(first.root, second.root);
        guide<UpperReducer>::swap(first.upper, second.upper);
        guide<LowerReducer>::swap(first.lower, second.lower);
        std::swap(first.sons, second.sons);
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
        if (node->isInSet())
        {
            this->removeViolation(node);
        }

        if (node->rank >= this->root->rank)
        {
            this->root->addToV(node);
        }
        else if (this->auxW[node->rank])
        {
            this->auxW[node->rank]->addRightViolationSibling(node);
        }
        else
        {
            this->auxW[node->rank] = node;
            this->root->addToW(node);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T1RootWrap::removeViolation(node_t* const node) -> void
    {
        if (node->rank < this->auxW.size())
        {
            if (this->auxW[node->rank] == node)
            {
                node_t* const next {node->nextInSet};
                this->auxW[node->rank] = (next and next->rank == node->rank ? next : nullptr);
            }
        }

        node_t::removeFromSet(node);
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

            const rank_t rank {node->rank};
            num_t count {node_t::sameRankViolation(node)};
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
        node_t* const n2    {n1->right};
        node_t* const extra {n2->right};

        this->increaseRank(n1->disconnect(), n2->disconnect());
        this->increase_domain();

        node_t::foldRight(extra, [=](node_t* const n) {
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

        while (it and it->rank == rank)
        {
            if (not it->isSonOfRoot())
            {
                if      (not first)  first  = it;
                else if (not second) second = it;
                else return std::make_pair(first, second);
            }
            
            it = it->nextInSet;
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

        while (it and it->rank == rank)
        {
            if (it->isSonOfRoot())
            {
                count++;
                if (count > 4)
                {
                    if (not first) first = it;
                    else if (not second) second = it;
                }
            }
            
            it = it->nextInSet;
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

        if (not node->isViolating())
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
        node_t* const n2 {second ? second : n1->sameRankSibling()};

        if (not n1->isViolating())
        {
            this->removeViolation(n1);
            return 1;
        }

        const num_t removed {
            static_cast<num_t>(n2 != nullptr ? 2 : 1)
        };

        if (not node_t::areSiblings(n1, n2))
        {
            node_t::makeSiblings(n1, n2);
        }

        const num_t siblingCount {node_t::sameRankCount(n1)};
        node_t* const parent{n1->parent};
        if (siblingCount > 2)
        {
            parent->removeChild(n1);
            this->addChildChecked(n1);
            return 1;
        }

        const bool removeParent {parent->rank == n1->rank + 1};
        if (removeParent)
        {
            if (parent->parent != this->root)
            {
                node_t* const replacement {this->sons[parent->rank]->right};
                node_t::swapTreeNodes(parent, replacement);
                if (replacement->isViolating())
                {
                    this->addViolation(replacement);
                }
            }
            if (parent->isInSet())
            {
                // Parent will no longer be violating and his rank will change.
                this->removeViolation(parent);
            }
            this->removeChild(parent);
            this->lowerCheck(parent->rank);
            this->queue->addExtraNodes();
        }
        
        parent->removeChild(n1);
        parent->removeChild(n2);
        
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

        std::swap(first.auxW, second.auxW);
        guide<ViolationReducer>::swap(first.violation, second.violation);
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
        if (child->isViolating())
        {
            this->queue->violations.push(child);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::removeChild(node_t* const child) -> void
    {
        if (child->isInSet())
        {
            this->queue->T1.removeViolation(child);
        }

        RootWrap::removeChild(child);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::T2RootWrap::removeLargeSons() -> node_t*
    {
        const rank_t newRootsRank {static_cast<rank_t>(this->root->rank - 1)};
        node_t* const ret {this->sons[this->root->rank - 1]};

        if (0 == newRootsRank)
        {
            this->root->child = nullptr;
            this->root->rank  = 0; 
        }
        else
        {
            this->root->child       = this->sons[this->root->rank - 2];
            this->root->child->left->right = nullptr;
            this->root->child->left        = nullptr;
            this->root->setRank();
        }

        node_t::foldRight(ret, [=](node_t* n) {
            if (n->isInSet())
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

    // BrodalQueue::IteratorNode implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::IteratorNode::IteratorNode(node_t const* node) :
        node {node}
    {
    }

    template<class T, class Compare>
    brodal_queue<T, Compare>::IteratorNode::IteratorNode(node_t const* node, iter_node_uptr && fallback) :
        node     {node}, 
        fallback {std::move(fallback)}
    {
    }

    template<class T, class Compare>
    brodal_queue<T, Compare>::IteratorNode::IteratorNode(const IteratorNode & other) :
        node         {other.node}, 
        fallback     {other.fallback 
                        ? std::make_unique<IteratorNode>(*other.fallback)
                        : nullptr
                     },
        childVisited {other.childVisited}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::IteratorNode::operator=(IteratorNode && rhs) -> IteratorNode &
    {
        this->node         = rhs.node;
        this->childVisited = rhs.childVisited;
        this->fallback     = std::move(rhs.fallback);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::IteratorNode::operator==(const IteratorNode & rhs) const -> bool
    {
        return this->node == rhs.node
           and this->childVisited == rhs.childVisited;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::IteratorNode::operator!=(const IteratorNode & rhs) const -> bool
    {
        return not (*this == rhs);
    }

    // BrodalQueue::TreeIterator implementation:

    template<class T, class Compare>
    brodal_queue<T, Compare>::TreeIterator::TreeIterator(node_t const* node) :
        current {nullptr == node 
                      // This represents end iterator.
                    ? std::make_unique<IteratorNode>(nullptr, nullptr) 
                      // Fallback will be end iterator.
                    : std::make_unique<IteratorNode> (
                        node, 
                        std::make_unique<IteratorNode>(nullptr, nullptr)
                    )
                }
    {
    }

    template<class T, class Compare>
    brodal_queue<T, Compare>::TreeIterator::TreeIterator(const TreeIterator & other) :
        current {std::make_unique<IteratorNode>(*other.current)}
    {
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::operator==(const TreeIterator & rhs) const -> bool
    {
        return *this->current == *rhs.current;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::operator!=(const TreeIterator & rhs) const -> bool
    {
        return not (*this == rhs);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::operator*() const -> node_t*
    {
        return this->current->node;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::operator->() const -> node_t*
    {
        return this->current->node;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::operator++() -> TreeIterator &
    {
        // If this is end iterator there is nowhere to advance.
        if (not this->current->node)
        {
            return *this;
        }
        
        // Move down if possible.
        if (not this->current->childVisited 
            and this->current->node->child)
        {
            this->current->childVisited = true;
            this->current = std::make_unique<IteratorNode> (
                this->current->node->child,
                std::move(this->current)
            );

            return *this;
        }

        // Move right if possible.
        if (this->current->node->right)
        {
            this->current = std::make_unique<IteratorNode> (
                this->current->node->right,
                std::move(this->current->fallback)
            );

            return *this;
        }

        // Fallback up if possible.
        if (this->current->fallback)
        {
            this->current = std::move(this->current->fallback);
            return ++(*this);
        }

        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::TreeIterator::swap(TreeIterator & other) noexcept -> void
    {
        this->current.swap(other.current);
    }

    // BrodalQueue::BrodalQueueIterator implementation:

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::BrodalQueueIterator(node_t const* t1, node_t const* t2) :
        t1Iterator {t1},
        t2Iterator {t2}
    {
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator=(BrodalQueueIterator other) -> BrodalQueueIterator &
    {
        swap(*this, other);
        return *this;
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator==(const BrodalQueueIterator & rhs) const -> bool
    {
        return this->t1Iterator == rhs.t1Iterator
           and this->t2Iterator == rhs.t2Iterator;
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator!=(const BrodalQueueIterator & rhs) const -> bool
    {
        return not (*this == rhs);
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator*() const -> Reference
    {
        return *(*this->activeTree())->entry;
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator->() const -> Pointer
    {
        return std::addressof(*(*this->activeTree())->entry);
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator++() -> BrodalQueueIterator &
    {
        ++(this->activeTree());
        return *this;
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::operator++(int) -> BrodalQueueIterator
    {
        BrodalQueueIterator ret {*this};
        ++(*this);
        return ret;
    }

    template<class T, class Compare, class Value, class Pointer, class Reference>
    auto swap(
        typename brodal_queue<T, Compare>::template BrodalQueueIterator<Value, Pointer, Reference> & first, 
        typename brodal_queue<T, Compare>::template BrodalQueueIterator<Value, Pointer, Reference> & second
    ) noexcept -> void
    {
        first.t1Iterator.swap(second.t1Iterator);
        first.t2Iterator.swap(second.t2Iterator);
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::activeTree() -> TreeIterator &
    {
        return nullptr != *this->t1Iterator 
                ? this->t1Iterator
                : this->t2Iterator; 
    }

    template<class T, class Compare>
    template<class Value, class Pointer, class Reference>
    auto brodal_queue<T, Compare>::BrodalQueueIterator<Value, Pointer, Reference>::activeTree() const -> const TreeIterator &
    {
        return nullptr != *this->t1Iterator 
               ? this->t1Iterator
               : this->t2Iterator; 
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
    auto brodal_queue<T, Compare>::operator=(brodal_queue rhs) -> brodal_queue &
    {
        swap(*this, rhs);
        return *this;
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::decrease_key(handle_t holder) -> void
    {
        // TODO wtf?
        // this->moveToT1(); 

        node_t* const node {holder.node_};

        if (*node < *this->T1.root)
        {
            node_t::swapEntries(node, this->T1.root);
        }

        if (node->isViolating() and not node->isInSet())
        {
            this->T1.addViolation(node);
            this->T1.violationCheck(node->rank);
        }
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
        if (newRoot->isInSet())
        {
            this->T1.removeViolation(newRoot.get());
        }
        
        if (not newRoot->isSonOfRoot())
        {
            this->makeSonOfRoot(newRoot.get());
        }

        this->T1.removeChild(newRoot.get());
        this->T1.lowerCheck(newRoot->rank);
        this->addExtraNodes();

        node_t* const sonsToAdd {newRoot->disconnectSons()};
        node_t::swapEntries(newRoot.get(), oldRoot);
        node_t::foldRight(sonsToAdd, [=](node_t* const n) {
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
        
        this->queueSize++;

        if (*node < *this->T1.root)
        {
            node_t::swapEntries(node, this->T1.root);
        }

        this->T1.addChildChecked(node);

        this->moveToT1();

        return handle_t(node);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::insert_special_impl
        (node_t* const node) -> handle_t
    {
        this->queueSize++;

        if (1 == this->size())
        {
            this->T1.root = node;
            return handle_t(node);
        }
        else if (*node < *this->T1.root)
        {
            node_t::swapEntries(node, this->T1.root);
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

        return handle_t(node);
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
            node_t* const firstChild  {this->T1.root->disconnectSons()};
            node_t* const secondChild {firstChild->right};

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
        return iterator {nullptr, nullptr};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::begin() const -> const_iterator
    {
        return const_iterator {this->T1.root, this->T2.root};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::end() const -> const_iterator
    {
        return const_iterator {nullptr, nullptr};
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::cbegin() const -> const_iterator
    {
        return const_cast<const brodal_queue *>(this)->begin();
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::cend() const -> const_iterator
    {
        return const_cast<const brodal_queue *>(this)->end();
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
    auto operator==(const brodal_queue<T, Compare> & lhs, const brodal_queue<T, Compare> & rhs) -> bool
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T, class Compare>
    auto operator!=(const brodal_queue<T, Compare> & lhs, const brodal_queue<T, Compare> & rhs) -> bool
    {
        return not (lhs == rhs);
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::addExtraNodes() -> void
    {
        while (not this->extraNodes.empty())
        {
            node_t* const node {this->extraNodes.top()};
            this->extraNodes.pop();
            if (node->rank < this->T1.root->rank)
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
            this->T1.violationCheck(violation->rank);
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
        node_t* const leftFoldable  {rightFoldable->left};
        oldt2->disconnectSons();
        // Reset t2 wrap to defaults.
        this->T2 = T2RootWrap {this};

        // Add sons with rank 0 under t1.
        node_t::foldRight(rightFoldable, [=](node_t* const n) {
            this->T1.addChildChecked(n->disconnect());
        });

        // Add other sons with ranks > 0 under t1.
        node_t::foldLeft(leftFoldable, [=](node_t* const n) {
            n->disconnect();
            
            while (n->rank >= this->T1.root->rank)
            {
                const delinked_nodes delinked {node_t::delinkNode(n)};
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

        if (this->T2.root->rank <= this->T1.root->rank + 2) // r(t2) is (r(t1) + 1) or (r(t1) + 2)
        {
            node_t* const sons1 {this->T2.removeLargeSons()};
            if (this->T2.root->rank > this->T1.root->rank)
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
            const rank_t t1rank {this->T1.root->rank};
            node_t* const toDelink  {this->T2.sons[t1rank + 1]};
            this->T2.removeChild(toDelink);
            this->T2.lowerCheck(toDelink->rank);

            const delinked_nodes delinked {node_t::delinkNode(toDelink)};
            this->T1.increaseRank(delinked.first, delinked.second);
            this->T1.increase_domain();
            if (delinked.third)
            {
                this->T1.addChildChecked(delinked.third);
            }
            
            // In case toDelink had more than 3 sons of rank (n-1).
            while (toDelink->rank == this->T1.root->rank)
            {
                const delinked_nodes delinked {node_t::delinkNode(toDelink)};
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
        node_t* newRoot {this->T1.root->child};

        node_t::foldRight(newRoot, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::foldNext(this->T1.root->setW, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        node_t::foldNext(this->T1.root->setV, [&](node_t* const n) {
            if (*n < *newRoot) newRoot = n;
        });

        return newRoot;
    } 

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::makeSonOfRoot(node_t* const newRoot) -> void
    {
        node_t* const swapped {this->T1.sons[newRoot->rank]->right};
        node_t::swapTreeNodes(newRoot, swapped);
        if (swapped->isViolating())
        {
            this->T1.addViolation(swapped);
        }
    }

    template<class T, class Compare>
    auto brodal_queue<T, Compare>::mergeSets(node_t* const newRoot) -> void
    {
        node_t::foldNext(this->T1.root->setV, [=](node_t* const n) {
            this->T1.addViolation(n);
        });

        node_t::foldNext(newRoot->setW, [=](node_t* const n) {
            this->T1.addViolation(n);
        });

        node_t::foldNext(newRoot->setV, [=](node_t* const n) {
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
        TreeIterator itOriginal  {original.root};
        TreeIterator originalEnd {nullptr};
        TreeIterator itCopy      {copy.root};

        // Put violating nodes into the map.
        while (itOriginal != originalEnd)
        {
            if (itOriginal->isInSet())
            {
                map[*itOriginal] = *itCopy;
            }

            ++itOriginal;
            ++itCopy;
        }

        // Put nodes that are in sons vector into the map.
        // These nodes are 'first of their rank'.
        node_t::zipWith(original.root->child, copy.root->child, 
            [&](node_t const* n1, node_t const* n2) {
                if (not n1->left
                    or  n1->left->rank != n1->rank)
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

        while (node->rank == root.root->rank)
        {
            const delinked_nodes delinked {node_t::delinkNode(node)};
            root.addDelinkedNodesChecked(delinked);
        }

        root.addChildChecked(node);
    }

}

#endif