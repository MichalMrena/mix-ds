#include "lib/pairing_heap.hpp"
#include "lib/simple_map.hpp"
#include "lib/brodal_queue.hpp"
#include "test/queue_test.hpp"
#include "test/map_test.hpp"
#include "test/brodal_test.hpp"
#include "utils/stopwatch.hpp"
#include "compare/boost_pairing_heap.hpp"
#include "compare/dijkstra.hpp"

#include <map>
#include <iomanip>

auto test_pairing_heap()
{
    using namespace mix::ds;
    auto constexpr seed = 65468321; 
    // auto constexpr n    = 5'000'000;
    auto constexpr n    = 10'000;

    // queue_test_insert <pairing_heap, merge_modes::fifo_queue> (n, seed);
    // queue_test_insert <pairing_heap, merge_modes::two_pass>   (n, seed);

    // queue_test_delete <pairing_heap, merge_modes::two_pass>   (n, seed);
    // queue_test_delete <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_decrease <pairing_heap, merge_modes::two_pass>   (n, seed);
    // queue_test_decrease <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_copy <pairing_heap, merge_modes::two_pass>   (n, seed);
    // queue_test_copy <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_erase <pairing_heap, merge_modes::two_pass>   (n, seed);
    // queue_test_erase <pairing_heap, merge_modes::fifo_queue> (n, seed);

    queue_test_random_all <pairing_heap, merge_modes::two_pass> (n, seed);
    queue_test_random_all <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_meld <pairing_heap, merge_modes::two_pass>   (n, seed);    
    // queue_test_meld <pairing_heap, merge_modes::fifo_queue> (n, seed);    

    // queue_test_other <pairing_heap, merge_modes::two_pass>   (n, seed);    
    // queue_test_other <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_random_all <boost_pairing_heap> (n, seed);
}

auto test_baseline_map()
{
    using namespace mix::ds;
    auto constexpr seed = 935135; 
    auto constexpr n    = 100'000;

    map_test_find_erase<simple_map>(n, seed);
    map_test_insert<simple_map>(n, seed);
}

auto test_brodal_queue()
{
    using namespace mix::ds;
    // test_guide();
    // test_queue();
    real_test_brodal_queue();

    // test_dijkstra_to_point<pairing_heap>(2000, 657);
    // test_dijkstra_to_point<boost_pairing_heap>(2000, 657);
    // test_dijkstra_to_point<brodal_queue>(2000, 657);
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

int main()
{
    using namespace mix::ds;

    auto watch = mix::utils::stopwatch();
    
    // test_pairing_heap();
    // test_baseline_map();
    // example_priority_queue();
    test_brodal_queue();

    auto const elapsed = watch.elapsed_time().count();
    std::cout << "Time taken " << elapsed << " ms" << std::endl;
}
