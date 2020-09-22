#ifndef MIX_DS_QUEUE_TEST_HPP
#define MIX_DS_QUEUE_TEST_HPP

#include "test_commons.hpp"
#include "../utils/random_wrap.hpp"
#include "../compare/dijkstra.hpp"

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

    auto operator== (test_data const& lhs, test_data const& rhs)
    {
        return lhs.data == rhs.data;
    }

    auto operator!= (test_data const& lhs, test_data const& rhs)
    {
        return ! (lhs == rhs);
    }

    template<class Handles>
    auto erase_handle(Handles& handles, std::size_t const index)
    {
        std::swap(handles.at(index), handles.back());
        (*handles.at(index)).index = index;
        handles.pop_back();
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

    template<class Queue>
    auto queue_test_size(Queue const& queue)
    {
        return queue.size() == real_size(queue);
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
        using queue_t   = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto queueFirst = queue_t();
        auto rngFirst   = make_rng<test_t>(0u, n, seed);
        auto handles    = queue_insert_n(1 + n, queueFirst, rngFirst);
        queueFirst.delete_min();

        auto queueSecond = queue_t(queueFirst);

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

        auto const is_odd = [](auto const n)
        {
            return n & 1;
        };

        auto const is_even = [&](auto const n)
        {
            return ! is_odd(n);
        };

        auto const to_even = [](auto const n)
        {
            return n & (~1u);
        };

        auto const to_odd = [](auto const n)
        {
            return n | 1;
        };

        auto const insert = [&]()
        {
            auto const data   = to_odd(rngData.next_int());
            auto const handle = queue.insert(test_data {data, handles.size()});
            handles.emplace_back(handle);
        };

        auto prevPrimordial = test_t(0);
        auto const delete_min = [&]()
        {
            auto const poppedData = queue.find_min().data;
            erase_handle(handles, queue.find_min().index);
            queue.delete_min();

            // Order must hold for primordial numbers.
            if (is_even(poppedData))
            {
                if (poppedData < prevPrimordial)
                {
                    std::cout << "!!! Test all failed. Invalid order of primordial keys: " 
                              << poppedData << " < " << prevPrimordial << std::endl;
                }
                prevPrimordial = poppedData;
            }
        };

        auto const decrease_key = [&]()
        {
            auto handle        = handles[rngIndex.next_int() % handles.size()];
            auto const newData = to_odd(rngNew.next_int() % (*handle).data);
            (*handle).data     = newData;
            queue.decrease_key(handle);
        };

        auto const erase = [&]()
        {
            auto const index  = rngIndex.next_int() % handles.size();
            auto const handle = handles.at(index);
            erase_handle(handles, index);
            queue.erase(handle);
        };

        for (auto i = 0u; i < n; ++i)
        {
            // Only primordial numbers are even.
            auto const data = to_even(rngData.next_int());
            handles.emplace_back(queue.insert(test_data {data, handles.size()}));
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

        auto queueCopy = queue_t(queue);

        ASSERT(queue_test_size(queue), "Test all [internal test size]");
        ASSERT(queue_test_size(queueCopy), "Test all [internal test size]");
        ASSERT(queue_test_delete(queue), "Test all [internal test delete]");
        ASSERT(queue_test_delete(queueCopy), "Test all [internal test delete]");
    }

    template<template<class, class, class...> class Queue>
    auto test_dijkstra_to_all(std::size_t const n, unsigned long const seed)
    {
        using namespace mix::ds;
        auto vs = load_road_graph("/mnt/c/Users/mrena/Downloads/USA-road-d.NY.gr");

        auto rngSrc = make_rng<std::size_t>(0ul, vs.vertices.size(), seed);

        auto totalDist = 0.0;
        for (auto i = 0ul; i < n; ++i)
        {
            find_point_to_all<Queue>(vs, rngSrc.next_int());
            for (auto const& v : vs.vertices)
            {
                if (dijkstra_max_dist() != v.distAprox)
                {
                    totalDist += v.distAprox;
                }
            }
        }

        std::cout << std::fixed << totalDist << '\n';
    }

    template<template<class, class, class...> class Queue>
    auto test_dijkstra_to_point(std::size_t const n, unsigned long const seed)
    {
        using namespace mix::ds;
        auto vs = load_road_graph("/mnt/c/Users/mrena/Downloads/USA-road-d.NY.gr");

        auto rngSeed = make_seeder(seed);
        auto rngSrc  = make_rng<std::size_t>(0ul, vs.vertices.size(), rngSeed.next_int());
        auto rngDst  = make_rng<std::size_t>(0ul, vs.vertices.size(), rngSeed.next_int());

        auto totalDist = 0.0;
        for (auto i = 0ul; i < n; ++i)
        {
            auto const path = find_point_to_point<Queue>(vs, rngSrc.next_int(), rngDst.next_int());
            if (dijkstra_max_dist() != path.cost)
            {
                totalDist += path.cost;
            }
        }

        std::cout << std::fixed << totalDist << '\n';
    }

    template<template<class, class...> class TestedQueue, class... Options>
    auto queue_test_test(unsigned long const seed)
    {
        using queue_t  = TestedQueue<test_t, std::less<test_t>, Options...>;
        auto constexpr n = 20u;

        auto rngSeed    = make_seeder(seed);
        auto rngLow     = make_rng<test_t>(0, 19'000, rngSeed.next_int());
        auto rngHigh    = make_rng<test_t>(20'000, 1'000'000, rngSeed.next_int());
        auto queueSmall1 = queue_t();
        auto queueBig1   = queue_t();
        auto queueSmall2 = queue_t();
        auto queueBig2   = queue_t();

        queue_insert_n(1'000, queueSmall1, rngLow);
        queue_insert_n(10'000, queueBig1, rngHigh);
        queue_insert_n(1'000, queueSmall2, rngLow);
        queue_insert_n(5'000, queueBig2, rngHigh);

        auto queueMelded1 = meld(queueSmall1, queueBig1);
        auto queueMelded2 = meld(queueSmall2, queueBig2);
        auto queueMelded3 = meld(queueMelded2, queueMelded1);

        // ASSERT(queue_test_delete(queueMelded1), "Test meld.");
        // ASSERT(queue_test_delete(queueMelded2), "Test meld.");
        ASSERT(queue_test_size(queueMelded3), "Test size.");
        ASSERT(queue_test_delete(queueMelded3), "Test meld.");
    }
}

#endif