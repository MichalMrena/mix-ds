#ifndef MIX_DS_BRODAL_TEST_HPP
#define MIX_DS_BRODAL_TEST_HPP

#include "test_commons.hpp"
#include "queue_test.hpp"
#include "../lib/brodal_queue.hpp"
#include "../utils/string_utils.hpp"
#include "../compare/boost_pairing_heap.hpp"

#include <vector>
#include <functional>
#include <initializer_list>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <utility>
#include <limits>
#include <iostream>

namespace mix::ds
{

// declarations:

    struct test_reducer
    {
        using uint = unsigned;

        std::vector<uint> ns_;

        test_reducer (std::vector<uint> ns);

        auto get_num (std::size_t const i) const -> uint;
        auto reduce  (std::size_t const i)       -> void;
    };

    struct reducer_wrap
    {
        std::reference_wrapper<test_reducer> reducer_;

        auto get_num (std::size_t const i) const -> uint;
        auto reduce  (std::size_t const i)       -> void;
    };

    struct guide_tester
    {
        using uint = unsigned;

        guide_tester (std::initializer_list<uint> const ns);
        auto do_inc  (std::initializer_list<uint> const is) -> guide_tester&;
        auto expect  (std::initializer_list<uint> const ns) -> void;

        test_reducer        reducer_;
        guide<reducer_wrap> guide_;
    };

// utils:
    inline auto make_blocks
        (std::vector<unsigned> const& ns)
    {
        auto bis = std::vector<std::pair<unsigned, unsigned>>();

        auto it  = std::rbegin(ns);
        auto end = std::rend(ns);
        auto i   = ns.size();
        --i;

        while (it != end)
        {
            if (2 != *it)
            {
                ++it;
                --i;
            }
            else
            {
                auto const beginIndex = i;

                ++it;
                --i;

                while (1 == *it)
                {
                    ++it;
                    --i;
                }

                auto const endIndex = i;

                ++it;
                --i;

                bis.emplace_back(endIndex, beginIndex);
            }
        }

        auto constexpr NULL_BLOCK = std::numeric_limits<std::uint8_t>::max();
        auto blocks = std::vector<std::shared_ptr<std::uint8_t>>(ns.size());
        std::fill(std::begin(blocks), std::end(blocks), std::make_shared<std::uint8_t>(NULL_BLOCK));

        for (auto&& [first, last] : bis)
        {
            auto blockPtr = std::make_shared<std::uint8_t>(last);
            for (auto j = first; j <= last; ++j)
            {
                blocks.at(j) = blockPtr;
            }
        }

        return blocks;
    }

// test_reduer:

    inline test_reducer::test_reducer
        (std::vector<uint> ns) :
        ns_ {std::move(ns)}
    {        
    }

    inline auto test_reducer::get_num
        (std::size_t const i) const -> uint
    {
        return ns_.at(i);
    }

    inline auto test_reducer::reduce
        (std::size_t const i) -> void
    {
        ns_.at(i) -= 2;
        if (i < ns_.size() - 1)
        {
            ns_.at(i + 1) += 1;
        }
    }

// reducer_wrap:

    inline auto reducer_wrap::get_num
        (std::size_t const i) const -> uint
    {
        return reducer_.get().get_num(i);
    }

    inline auto reducer_wrap::reduce
        (std::size_t const i) -> void
    {
        reducer_.get().reduce(i);
    }

// guide_tester:

    inline guide_tester::guide_tester
        (std::initializer_list<uint> const ns) :
        reducer_ {std::vector(std::rbegin(ns), std::rend(ns))},
        guide_   {reducer_wrap {reducer_}, make_blocks(reducer_.ns_)}
    {
    }

    inline auto guide_tester::do_inc
        (std::initializer_list<uint> const is) -> guide_tester&
    {
        for (auto i : is)
        {
            ++reducer_.ns_.at(i);
            guide_.inc(i);
        }

        return *this;
    }

    inline auto guide_tester::expect
        (std::initializer_list<uint> const ns) -> void
    {
        auto const isSame = std::equal(std::rbegin(ns), std::rend(ns) , std::begin(reducer_.ns_));
        auto const expStr = utils::concat_range(ns, "");
        auto const gotStr = utils::concat_range(std::rbegin(reducer_.ns_), std::rend(reducer_.ns_), "");
        std::cout << (isSame ? "Guide test passed." : "!!! Guide test failed.")
                  << " Expected " << expStr << " got " << gotStr << std::endl;
    }

// tests:

    inline auto test_guide()
    {
        auto t1 = guide_tester({2, 0, 1, 2, 0});
        auto t2 = guide_tester(t1);
        auto t3 = t2;

        // std::cout << guide_tester({0, 0, 0, 0}).guide_.to_string() << std::endl; 
        // std::cout << guide_tester({2, 0, 2, 0}).guide_.to_string() << std::endl; 
        // std::cout << guide_tester({2, 1, 0, 0}).guide_.to_string() << std::endl; 
        // std::cout << guide_tester({2, 1, 1, 0}).guide_.to_string() << std::endl; 
        
        std::cout << t1.guide_.to_string() << std::endl; 
        std::cout << t2.guide_.to_string() << std::endl; 
        std::cout << t3.guide_.to_string() << std::endl; 

        // guide_tester({2, 1, 1, 0, 0}).do_inc({4}).expect({1, 1, 1, 0, 0});
        // guide_tester({2, 1, 0, 2, 0}).do_inc({1}).expect({0, 1, 1, 1, 0});
        // guide_tester({0, 1, 1, 1, 0}).do_inc({1}).expect({0, 1, 2, 0, 0});
    }

    inline auto real_test_brodal_queue()
    {
        auto constexpr seed = 256843512.1;
        auto constexpr n    = 3'000'000;
        // auto constexpr n    = 100'000;

        // queue_test_insert<brodal_queue>(n, seed);
        // queue_test_delete<brodal_queue>(n, seed);
        // queue_test_decrease<brodal_queue>(n, seed);
        // queue_test_other<brodal_queue>(n, seed);
        queue_test_random_all<brodal_queue>(n, seed);
        // queue_test_random_all<pairing_heap>(n, seed);
        // queue_test_random_all<boost_pairing_heap>(n, seed);
    }
}


#endif