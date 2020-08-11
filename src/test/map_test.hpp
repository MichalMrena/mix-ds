#ifndef MIX_DS_MAP_TEST_HPP
#define MIX_DS_MAP_TEST_HPP

#include "test_commons.hpp"
#include "../utils/random_wrap.hpp"

#include <algorithm>
#include <unordered_set>
#include <array>

namespace mix::ds
{
    using map_test_key_t = int;

    enum class InsertMethod
    {
        Insert,
        InsertAssign,
        Emplace,
        TryEmplace,
        Bracket,
        Count
    };

    template<class Map, class Keys>
    auto has_keys(Map const& map, Keys const& keys)
    {
        return std::all_of( std::begin(keys), std::end(keys)
                          , [&map](auto const& k) { return map.find(k) != map.end(); } );
    }

    template<class Map>
    auto has_unique_keys(Map const& map)
    {
        auto uniqueKeys = std::unordered_set<typename Map::key_type>();
        std::transform( std::begin(map), std::end(map), std::inserter(uniqueKeys, std::end(uniqueKeys)) 
                      , [](auto const& p) { return p.first; } );
        return uniqueKeys.size() == real_size(map);
    }

    template<template<class, class, class...> class Map>
    auto map_test_insert(std::size_t const n, unsigned long const seed)
    {
        using map_t    = Map<map_test_key_t, map_test_key_t>;
        auto map       = map_t();
        auto rngSeed   = make_seeder(seed);
        auto rngMethod = make_rng(0ul, static_cast<unsigned long>(InsertMethod::Count), rngSeed.next_int());
        auto rngKey    = make_rng<map_test_key_t>(seed);
        auto keys      = std::unordered_set<map_test_key_t>();
        keys.reserve(1.1 * n);

        auto const methods = std::array { InsertMethod::Insert, InsertMethod::InsertAssign
                                        , InsertMethod::Emplace, InsertMethod::TryEmplace
                                        , InsertMethod::Bracket };
        static_assert( methods.size() == static_cast<std::size_t>(InsertMethod::Count)
                     , "Missing insert method in the array." );

        for (auto i = 0u; i < n; ++i)
        {
            auto const method = rngMethod.next_int();
            auto const key    = rngKey.next_int();

            switch (methods[method])
            {
            case InsertMethod::Insert:
                map.insert(std::make_pair(key, key));
                break;
            
            case InsertMethod::InsertAssign:
                map.insert_or_assign(key, key);
                break;

            case InsertMethod::Emplace:
                map.emplace(key, key);
                break;
            
            case InsertMethod::TryEmplace:
                map.emplace(key, key);
                break;
            
            case InsertMethod::Bracket:
                map[key] = key;
                break;

            default:
                throw "This should not have happened.";
            }
            
            keys.emplace(key);
        }

        ASSERT(has_keys(map, keys), "Test has keys");
        ASSERT(has_unique_keys(map), "Test unique keys.");
    }
}

#endif