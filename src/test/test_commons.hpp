#ifndef MIX_DS_TEST_COMMONS_HPP
#define MIX_DS_TEST_COMMONS_HPP

#include "../utils/random_wrap.hpp"

#include <iostream>
#include <string_view>

namespace mix::ds
{
    inline auto ASSERT(bool const b, std::string_view testName)
    {
        if (b)
        {
            std::cout << testName << " passed." << std::endl;
        }
        else
        {
            std::cout << "!!! " << testName << " failed." << std::endl;
        }
    }

    inline auto CHECK(bool const b, std::string_view testName)
    {
        if (!b)
        {
            std::cout << "!!! " << testName << " failed." << std::endl;
        }
    }

    inline auto make_seeder(unsigned long const seed)
    {
        using seed_t = unsigned long;
        auto constexpr maxseed = std::numeric_limits<seed_t>::max();
        return utils::random_uniform_int<seed_t>(10, maxseed, seed);
    }

    template<class T>
    auto make_rng(T const min, T const max, unsigned long seed)
    {
        return utils::random_uniform_int<T>(min, max, seed);
    }

    template<class T>
    auto make_rng(unsigned long seed)
    {
        return utils::random_uniform_int<T>(seed);
    }

    template<class Range>
    auto real_size (Range const& r)
    {
        return static_cast<std::size_t>(std::distance(std::begin(r), std::end(r)));
    }
}

#endif