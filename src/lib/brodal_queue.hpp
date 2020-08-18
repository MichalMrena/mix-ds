#ifndef MIX_DS_BRODAL_QUEUE_HPP
#define MIX_DS_BRODAL_QUEUE_HPP

#include <vector>
#include <cstdint>
#include <memory>
#include <string>

namespace mix::ds
{
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

// guide definitions:

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
}


#endif