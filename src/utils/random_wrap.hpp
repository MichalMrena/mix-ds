#ifndef MIX_UTILS_RANDOM_WRAP_HPP
#define MIX_UTILS_RANDOM_WRAP_HPP

#include <random>

namespace mix::utils
{
    namespace aux_impl
    {
        class random_base
        {
        public:
            explicit random_base(unsigned long seed);

        protected:
            std::mt19937 generator_;
        };  

        inline random_base::random_base(unsigned long seed) :
            generator_ {seed}
        {
        }
    }

    template<class IntType>
    class random_uniform_int : private aux_impl::random_base
    {
    public:
        random_uniform_int();
        random_uniform_int(unsigned long seed);
        random_uniform_int(IntType const min, IntType const max);
        random_uniform_int(IntType const min, IntType const max, unsigned long const seed);
        
        /**
            @return uniformly distributed integral type from [min, max] (!!! inclusive)
         */
        auto next_int () -> IntType;

    private:
        std::uniform_int_distribution<IntType> distribution_;
    };

    template<class IntType>
    random_uniform_int<IntType>::random_uniform_int () :
        random_uniform_int<IntType> { std::numeric_limits<IntType>::min() 
                                    , std::numeric_limits<IntType>::max()
                                    , std::random_device {} () }
    {
    }

    template<class IntType>
    random_uniform_int<IntType>::random_uniform_int (unsigned long const seed) :
        random_uniform_int<IntType> { std::numeric_limits<IntType>::min() 
                                    , std::numeric_limits<IntType>::max()
                                    , seed }
    {
    }

    template<class IntType>
    random_uniform_int<IntType>::random_uniform_int (IntType const min, IntType const max) :
        random_uniform_int<IntType> {std::random_device {} (), min , max}
    {
    }

    template<class IntType>
    random_uniform_int<IntType>::random_uniform_int (IntType const min, IntType const max, unsigned long const seed) :
        random_base   {seed},
        distribution_ {min, max}
    {
    }
    
    template<class IntType>
    auto random_uniform_int<IntType>::next_int
        () -> IntType
    {
        return distribution_(random_base::generator_);
    }
}

#endif