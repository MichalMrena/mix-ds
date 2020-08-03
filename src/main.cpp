#include "lib/pairing_heap.hpp"
#include "test/queue_test.hpp"
#include "utils/stopwatch.hpp"
#include "compare/boost_pairing_heap.hpp"

auto test_pairing_heap()
{
    using namespace mix::ds;
    auto constexpr seed = 84465413; 
    auto constexpr n    = 5'000'000;
    // auto constexpr n    = 1'000;

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

    // queue_test_random_all <pairing_heap, merge_modes::two_pass> (n, seed);
    // queue_test_random_all <pairing_heap, merge_modes::fifo_queue> (n, seed);

    // queue_test_meld <pairing_heap, merge_modes::two_pass>   (n, seed);    
    // queue_test_meld <pairing_heap, merge_modes::fifo_queue> (n, seed);    

    // queue_test_other <pairing_heap, merge_modes::two_pass>   (n, seed);    
    // queue_test_other <pairing_heap, merge_modes::fifo_queue> (n, seed);

    queue_test_random_all <boost_pairing_heap> (n, seed);
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
    auto watch = mix::utils::stopwatch();
    
    test_pairing_heap();
    // example_priority_queue();

    auto const elapsed = watch.elapsed_time().count();
    std::cout << "Time taken " << elapsed << " ms" << std::endl;
}
