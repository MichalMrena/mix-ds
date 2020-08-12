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

    enum class FindMethod
    {
        At,
        Find,
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

    template<class Map>
    auto map_insert_n(std::size_t const n, Map& map, utils::random_uniform_int<unsigned long>& seeder)
    {
        auto rngMethod = make_rng(0ul, static_cast<unsigned long>(InsertMethod::Count) - 1, seeder.next_int());
        auto rngKey    = make_rng<map_test_key_t>(seeder.next_int());
        auto keys      = std::unordered_set<map_test_key_t>();
        keys.reserve(1.1 * n);

        auto const methods = std::array { InsertMethod::Insert,  InsertMethod::InsertAssign
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

        return std::vector<map_test_key_t>(std::begin(keys), std::end(keys));
    }

    template<template<class, class, class...> class Map>
    auto map_test_insert(std::size_t const n, unsigned long const seed)
    {
        using map_t  = Map<map_test_key_t, map_test_key_t>;
        auto map     = map_t();
        auto rngSeed = make_seeder(seed);
        auto keys    = map_insert_n(n, map, rngSeed);

        ASSERT(has_keys(map, keys), "Test has keys");
        ASSERT(has_unique_keys(map), "Test unique keys.");
    }

    template<template<class, class, class...> class Map>
    auto map_test_find_erase(std::size_t const n, unsigned long const seed)
    {
        auto map         = Map<map_test_key_t, map_test_key_t>();
        auto rngSeed     = make_seeder(seed);
        auto keys        = map_insert_n(n, map, rngSeed);
        auto rngMethod   = make_rng(0ul, static_cast<unsigned long>(FindMethod::Count) - 1, rngSeed.next_int());
        auto rngInIndex  = make_rng(0ul, keys.size(), rngSeed.next_int());
        auto rngOutIndex = make_rng(0ul, keys.size(), rngSeed.next_int());
        auto erasedKeys  = std::vector<map_test_key_t>();
        erasedKeys.reserve(n);

        auto const methods = std::array {FindMethod::At, FindMethod::Find};

        erasedKeys.emplace_back(keys.back());
        keys.pop_back();
        map.erase(erasedKeys.front());

        for (auto i = 0u; i < map.size(); ++i)
        {
            auto const method      = rngMethod.next_int();
            auto const inKeyIndex  = rngInIndex.next_int() % keys.size();
            auto const outKeyIndex = rngOutIndex.next_int() % erasedKeys.size();
            auto const key         = keys[inKeyIndex];

            auto wasFound  = false;
            auto mappedVal = map_test_key_t();

            switch (methods[method])
            {
            case FindMethod::At:
                try 
                {
                    mappedVal = map.at(key);
                    wasFound  = true;
                }
                catch (...) { }
                break;

            case FindMethod::Find:
                auto const it = map.find(key);
                if (map.end() != it)
                {
                    mappedVal = it->second;
                    wasFound  = true;
                }
                break;
            }

            CHECK(wasFound, "Test find key");
            CHECK(key == mappedVal, "Test mapped val");

            erasedKeys.emplace_back(keys[inKeyIndex]);
            std::swap(keys[inKeyIndex], keys.back());
            keys.pop_back();

            auto const erasedKey = erasedKeys[outKeyIndex];
            map.erase(erasedKey);
            CHECK(map.end() == map.find(erasedKey), "Test erase key");
        }
    }
}

#endif