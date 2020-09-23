#include "lib/pairing_heap.hpp"
#include "lib/simple_map.hpp"
#include "lib/brodal_queue.hpp"
#include "lib/fibonacci_heap.hpp"
#include "test/queue_test.hpp"
#include "test/map_test.hpp"
#include "test/brodal_test.hpp"
#include "utils/stopwatch.hpp"
#include "compare/boost_heap.hpp"
#include "compare/dijkstra.hpp"

#include <map>
#include <iomanip>
#include <queue>

auto test_pairing_heap()
{
    using namespace mix::ds;
    auto constexpr seed = 54654564321; 
    auto constexpr n    = 5'000'000;
    queue_test_random_all <pairing_heap, merge_modes::two_pass> (n, seed);
    // queue_test_random_all <boost_pairing_heap> (n, seed);
    // queue_test_random_all <pairing_heap, merge_modes::fifo_queue> (n, seed);
    // queue_test_other <pairing_heap, merge_modes::two_pass>   (n, seed);    
    // queue_test_other <pairing_heap, merge_modes::fifo_queue> (n, seed);
}

auto test_simple_map()
{
    using namespace mix::ds;
    auto constexpr seed = 935135; 
    auto constexpr n    = 1'000;

    map_test_find_erase<simple_map>(n, seed);
    map_test_insert<simple_map>(n, seed);
}

auto test_brodal_queue()
{
    // using namespace mix::ds;
    // test_guide();
    // test_queue();
    // real_test_brodal_queue();

    // auto constexpr seed = 1212121;
    // test_dijkstra_to_point<pairing_heap>(2000, seed);
    // test_dijkstra_to_point<boost_pairing_heap>(2000, seed);
    // test_dijkstra_to_point<brodal_queue>(2000, seed);
}

auto test_fibonacci_heap()
{
    using namespace mix::ds;
    auto constexpr seed = 54686454;
    auto constexpr n    = 3'000'000;
    queue_test_random_all <fibonacci_heap> (n, seed);
    queue_test_other <fibonacci_heap> (n, seed);;
}

auto test_dijkstra()
{
    using namespace mix::ds;
    auto constexpr seed = 1321545132;
    auto constexpr n    = 3000;
    test_dijkstra_to_point <pairing_heap>       (n, seed);
    test_dijkstra_to_point <boost_pairing_heap> (n, seed);
    // test_dijkstra_to_point <brodal_queue>       (n, seed);
    // test_dijkstra_to_point <fibonacci_heap>     (n, seed);
}

auto example_priority_queue()
{
    auto heap = mix::ds::pairing_heap<int>();
    auto handles = std::vector { heap.insert(20)
                               , heap.insert(10)
                               , heap.insert(30) };
    //             ^^^^^^^^^^^
    // Notice the deduction of the template parameter. Alternative:
    // using handle_t = mix::ds::pairing_heap<int>::handle_t;
    // std::vector<handle_t>

    auto handle30to5 = handles.back();
    // Dereference it just like iterator:
    *handle30to5 = 5;
    heap.decrease_key(handle30to5);

    std::cout << heap.find_min() << std::endl; // 5
    heap.delete_min();
    std::cout << heap.find_min() << std::endl; // 10
    heap.delete_min();

    for (auto i : heap)
    {
        std::cout << i << std::endl; // 20
    }
}

auto compare_queues_ops()
{
    using namespace mix::ds;
    auto constexpr seed = 78945651; 
    auto constexpr n    = 5'000'000;

    // queue_test_random_all <pairing_heap> (n, seed);
    // queue_test_random_all <boost_pairing_heap> (n, seed);

    queue_test_random_all <fibonacci_heap> (n, seed);
    // queue_test_random_all <boost_fibonacci_heap> (n, seed);

    // queue_test_delete <fibonacci_heap> (n, seed);
    // queue_test_delete <boost_fibonacci_heap> (n, seed);
}

auto compare_queues_dijkstra()
{
    using namespace mix::ds;
    auto constexpr seed = 54654564321; 
    auto constexpr n    = 3'000;

    // test_dijkstra_to_point <pairing_heap>       (n, seed);
    // test_dijkstra_to_point <boost_pairing_heap> (n, seed);

    // test_dijkstra_to_point <fibonacci_heap>       (n, seed);
    test_dijkstra_to_point <boost_fibonacci_heap> (n, seed);
}

int main()
{
    using namespace mix::ds;
    auto watch = mix::utils::stopwatch();

    // test_fibonacci_heap();
    test_pairing_heap();
    // test_brodal_queue();
    // test_baseline_map();
    // test_dijkstra();

    // compare_queues_ops();
    // compare_queues_dijkstra();

    auto const elapsed = watch.elapsed_time().count();
    std::cout << "Time taken " << elapsed << " ms" << std::endl;
}
