#ifndef MIX_DS_BOOST_PAIRING_HEAP_HPP
#define MIX_DS_BOOST_PAIRING_HEAP_HPP

#include <boost/heap/pairing_heap.hpp>

namespace mix::ds
{
    template<class Cmp>
    struct reverse_compare
    {
        template<class T>
        auto operator() (T const& lhs, T const& rhs) const -> bool
        {
            return ! (Cmp () (lhs, rhs));
        }
    };

    template<class T, class Compare>
    class boost_pairing_heap
    {
    private:
        using compare_t    = boost::heap::compare<reverse_compare<Compare>>;
        using boost_heap_t = boost::heap::pairing_heap<T, compare_t>;

    private:
        boost_heap_t heap_;

    public:
        using handle_t = typename boost_heap_t::handle_type;

    public:
        auto insert (T const& t) -> handle_t
        {
            return heap_.push(t);
        }

        auto delete_min () -> void
        {
            heap_.pop();
        }

        auto find_min () const -> T const&
        {
            return heap_.top();
        }

        auto empty () const -> bool
        {
            return heap_.empty();
        }

        auto decrease_key (handle_t const handle) -> void
        {
            heap_.increase(handle);
        }

        auto erase (handle_t const handle) -> void
        {
            heap_.erase(handle);
        }

        auto size () const -> std::size_t
        {
            return heap_.size();
        }
    };
}

#endif