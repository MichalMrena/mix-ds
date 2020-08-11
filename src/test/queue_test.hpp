#ifndef MIX_DS_QUEUE_TEST_HPP
#define MIX_DS_QUEUE_TEST_HPP

#include "test_commons.hpp"
#include "../utils/random_wrap.hpp"

#include <vector>
#include <iterator>
#include <unordered_map>
#include <algorithm>
#include <string_view>
#include <limits>

namespace mix::ds
{
    using test_t = unsigned int;
    
    struct test_data
    {
        test_t      data;
        std::size_t index;
    };

    auto operator< (test_data const& lhs, test_data const& rhs)
    {
        return lhs.data < rhs.data;
    }

    auto operator> (test_data const& lhs, test_data const& rhs)
    {
        return lhs.data > rhs.data;
    }

    template<class Handles>
    auto erase_handle(Handles& handles, std::size_t const index)
    {
        std::swap(handles[index], handles.back());
        handles.pop_back();
        (*handles[index]).index = index;
    }

    template<class Queue>
    auto queue_insert_n (std::size_t const n, Queue& queue, mix::utils::random_uniform_int<test_t>& rng)
    {
        auto hs = std::vector<typename Queue::handle_t>();

        for (auto i = 0u; i < n; ++i)
        {
            hs.emplace_back(queue.insert(rng.next_int()));
        }

        return hs;
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_insert(std::size_t const n, unsigned long const seed)
    {
        using queue_t = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queue    = queue_t();
        auto rng      = make_rng<test_t>(seed);

        queue_insert_n(n, queue, rng);

        ASSERT(queue.size() == n, "Test insert");
    }

    template<class Queue>
    auto queue_test_delete_n(Queue& queue, std::size_t const n)
    {
        auto prev = queue.find_min();
        queue.delete_min();

        for (auto i = 0ul; i < n - 1; ++i)
        {
            auto const curr = queue.find_min();
            queue.delete_min();
            if (curr < prev)
            {
                return false;
            }

            prev = curr;
        }

        return true;
    }

    template<class Queue>
    auto queue_test_delete(Queue& queue)
    {
        return queue_test_delete_n(queue, queue.size());
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_delete(std::size_t const n, unsigned long const seed)
    {
        using queue_t = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queue    = queue_t();
        auto rng      = make_rng<test_t>(seed);

        queue_insert_n(n, queue, rng);
        ASSERT(queue_test_delete(queue), "Test delete");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_decrease(std::size_t const n, unsigned long const seed)
    {
        using queue_t = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queue    = queue_t();
        auto rng      = make_rng<test_t>(1u, n, seed);
        auto handles  = queue_insert_n(n, queue, rng);

        for (auto handle : handles)
        {
            *handle = rng.next_int() % *handle;
            queue.decrease_key(handle);
        }
        std::cout << "Test decrease partialy passed. Keys were decreased." << std::endl;
        
        ASSERT(queue_test_delete(queue), "Test decrease [internal test delete].");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_copy(std::size_t const n, unsigned long const seed)
    {
        using queue_t = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queue    = queue_t();
        auto rng      = make_rng<test_t>(1u, n, seed);
        
        queue_insert_n(n, queue, rng);
        queue.delete_min();

        auto copyQueue = queue_t(queue);
        ASSERT(copyQueue == queue, "Test copy");
        ASSERT(queue_test_delete(copyQueue), "Test copy [internal test delete]");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_erase(std::size_t const n, unsigned long const seed)
    {
        using queue_t = TestedQueue<test_data, std::less<test_data>, Options...>;
        auto queue    = queue_t();
        auto rngSeed  = make_seeder(seed);
        auto rngData  = make_rng<test_t>(0u, n, rngSeed.next_int());
        auto rngIndex = make_rng<std::size_t>(rngSeed.next_int());
        auto handles  = std::vector<typename queue_t::handle_t>();
        handles.reserve(n);

        for (auto i = 0u; i < n; ++i)
        {
            handles.emplace_back(queue.insert(test_data {rngData.next_int(), handles.size()}));
        }

        for (auto i = 0u; i < n / 2; ++i)
        {
            auto const index = rngIndex.next_int() % handles.size();
            queue.erase(handles[index]);
            erase_handle(handles, index);
        }

        ASSERT(queue_test_delete(queue), "Test erase [internal test delete]");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_meld(std::size_t const n, unsigned long const seed)
    {
        using queue_t    = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queueFirst  = queue_t();
        auto queueSecond = queue_t();
        auto rngSeed     = make_seeder(seed);
        auto rngFirst    = make_rng<test_t>(0u, n, rngSeed.next_int());
        auto rngSecond   = make_rng<test_t>(0u, n, rngSeed.next_int());

        queue_insert_n(n, queueFirst, rngFirst);
        queue_insert_n(n, queueSecond, rngSecond);

        auto melded = meld(std::move(queueFirst), std::move(queueSecond));

        ASSERT(queue_test_delete(melded), "Test meld [internal test delete]");
        ASSERT(queueFirst.empty() && queueSecond.empty(), "Test meld");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_other(std::size_t const n, unsigned long const seed)
    {
        using queue_t    = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queueFirst  = queue_t();
        auto queueSecond = queue_t();
        auto rngFirst    = make_rng<test_t>(0u, n, seed);
        auto rngSecond   = make_rng<test_t>(0u, n, seed);

        queue_insert_n(1 + n, queueFirst, rngFirst);
        queue_insert_n(1 + n, queueSecond, rngSecond);

        queueFirst.delete_min();
        queueSecond.delete_min();

        ASSERT(queueFirst == queueSecond, "Test equal");
        ASSERT(queueFirst.size() == real_size(queueFirst), "Test size");

        queue_test_delete_n(queueFirst, queueFirst.size() / 2);
        swap(queueFirst, queueSecond);

        ASSERT(queue_test_delete_n(queueSecond, n / 2), "Test swap");
        ASSERT(queue_test_delete_n(queueFirst, n), "Test swap");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_random_all(std::size_t const n, unsigned long const seed)
    {
        auto constexpr OpInsert      = 0u;
        auto constexpr OpDeleteMin   = 1u;
        auto constexpr OpDecreaseKey = 2u;
        auto constexpr OpErase       = 3u;

        using queue_t = TestedQueue<test_data, std::less<test_data>, Options...>;
        auto queue    = queue_t();
        auto rngSeed  = make_seeder(seed);
        auto rngData  = make_rng<test_t>(3u, n, rngSeed.next_int());
        auto rngNew   = make_rng<test_t>(3u, n, rngSeed.next_int());
        auto rngOp    = make_rng<decltype(OpInsert)>(0u, 3, rngSeed.next_int());
        auto rngIndex = make_rng<std::size_t>(0u, 2 * n, rngSeed.next_int());
        auto handles  = std::vector<typename queue_t::handle_t>();
        handles.reserve(2 * n);

        auto prevOdd = test_t(0);

        auto const is_odd = [](auto const n)
        {
            return n & 1;
        };

        auto const insert = [&]()
        {
            handles.emplace_back(queue.insert(test_data {std::max(2u, rngData.next_int() & (~1u)), handles.size()}));
        };

        auto const delete_min = [&]()
        {
            auto const poppedData  = queue.find_min().data;
            erase_handle(handles, queue.find_min().index);
            queue.delete_min();

            if (is_odd(poppedData))
            {
                if (poppedData < prevOdd)
                {
                    std::cerr << "!!! Test all failed. Invalid order of primodial keys." << std::endl;
                }
                prevOdd = poppedData;
            }
        };

        auto const decrease_key = [&]()
        {
            auto handle = handles[rngIndex.next_int() % handles.size()];
            (*handle).data = std::max(2u, (rngNew.next_int() % (*handle).data) & (~1u));
            queue.decrease_key(handle);
        };

        auto const erase = [&]()
        {
            auto const index  = rngIndex.next_int() % handles.size();
            auto const handle = handles[index];
            erase_handle(handles, index);
            queue.erase(handle);
        };

        for (auto i = 0u; i < n; ++i)
        {
            handles.emplace_back(queue.insert(test_data {std::max(2u, rngData.next_int() & (~1u)), handles.size()}));
        }

        for (auto i = 0u; i < n; ++i)
        {
            auto const opcode = rngOp.next_int();
            switch (opcode)
            {
                case OpInsert:      insert();       break;
                case OpDeleteMin:   delete_min();   break;
                case OpDecreaseKey: decrease_key(); break;
                case OpErase:       erase();        break;
                default: throw "This should not have happened.";
            }
        }

        ASSERT(queue_test_delete(queue), "Test all [internal test delete]");
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_test(unsigned long const seed)
    {
        auto constexpr n = 20u;

        using queue_t = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queue    = queue_t();
        auto rng      = make_rng<test_t>(0u, n, seed);
        auto handles  = queue_insert_n(n, queue, rng);

        while (!queue.empty())
        {
            std::cout << queue.find_min() << std::endl;
            queue.delete_min();
        }
    }
}

#endif