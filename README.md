# Contents
* [Intro](#intro)
* [Data structures](#data-structures)
    - [Pairing heap](#pairing-heap)
* [Documentation](#documentation)
    - [Priority queue](#priority-queue)
* [Examples](#examples)
    - [Priority queue](#priority-queue-1)
* [Comparison](#comparison)

# Intro
This is a small data structure library for C++. Some structures are useful in practice and some of them that are interesting in theory can be used in some experiments and comparisons. Each structure is implemented in a single independent [header file](./src/lib). Using them is therefore very simple. You just need to include particular header file in your project. You will also need a compiler that supports C++17. It was tested with ```gcc 10.1.0```, ```clang++ 10.0.0``` and ```Visual Studio 2019```.

# Data structures
## Pairing heap
Pairing heap is simple and efficient implementation of the [priority queue](https://en.wikipedia.org/wiki/Priority_queue). It performs very well in [Discrete-event simulation](https://en.wikipedia.org/wiki/Discrete-event_simulation). You can read a formal description on the [Wikipedia](https://en.wikipedia.org/wiki/Pairing_heap) , in the [original paper](https://www.cs.cmu.edu/~sleator/papers/pairing-heaps.pdf) and on many other places on the [internet](https://www.google.com/search?q=pairing+heap&oq=pairing+heap). We are not gonna repeat that here.  
Pairing heap is implemented in [Boost](https://www.boost.org/doc/libs/1_73_0/doc/html/boost/heap/pairing_heap.html) and you will probably find many other implementations but some of them are naive or not generic. Our implementation is a generic recursion-free container. We use a binary tree to represent the heap and we also support two *merge modes* => two pass merge *(default)* and fifo queue. It can be specified by a template parameter.

# Documentation
Naming conventions and interfaces are almost identical to STL. Each structure also satisfies [Container](https://en.cppreference.com/w/cpp/named_req/Container) named requirements. If you are familiar with [STL containers](https://en.cppreference.com/w/cpp/container) using these structures should be easy.  
Currently there are two types of data structure: [priority queue](https://en.wikipedia.org/wiki/Priority_queue) and [table (map, dictionary, ...)](https://en.wikipedia.org/wiki/Associative_array).

## Priority queue
### Element insertion
```C++
template<class... Args>
auto emplace (Args&&... args)          -> handle_t; // 1.
auto insert  (value_type const& value) -> handle_t; // 2.
auto insert  (value_type&& value)      -> handle_t; // 3.
```
1. New element is constructed in place using args.
2. New element is copy constructed.
3. New element is move constructed.  

Each method returns a handle that is associated with an inserted element. It can be used later to erase the element or to increase its priority. Handle is merely a pointer wrap so it can be passed by value with no overhead.

### Element access
```C++
auto find_min ()       -> reference;        // 1.
auto find_min () const -> const_reference;  // 2.
auto begin    ()       -> iterator;         // 3.
auto end      ()       -> iterator;         // 4.
auto begin    () const -> const_iterator;   // 5.
auto end      () const -> const_iterator;   // 6.
auto cbegin   () const -> const_iterator;   // 7.
auto cend     () const -> const_iterator;   // 8.
```
1. Returns reference to the element with the highest priority. 
2. Returns const reference to the element with the highest priority.
3. ... 4. Whole structure is iterable. However be careful not to change a priority of some element. It would cause undefined behaviour.

### Element removal
```C++
auto delete_min ()                      -> void; // 1.
auto erase      (handle_t const handle) -> void; // 2.
auto erase      (iterator pos)          -> void; // 3.
auto erase      (const_iterator pos)    -> void; // 4.
```
1. Removes an element with the highest priority from the queue.
2. ... 4. Removes an element associated with given handle/iterator.

### Element modification
```C++
auto decrease_key (handle_t const handle) -> void; // 1.
auto decrease_key (iterator pos)          -> void; // 2.
auto decrease_key (const_iterator pos)    -> void; // 3.
```
1. ... 3. Corrects a position of an element that is associated with given handle/iterator after its priority have been **increased**. See the **example** below.  

*Similar operation ```increase_key``` is not supported in general but might be supported by some specific implementations.*

### Other
Each priority queue takes [Compare](https://en.cppreference.com/w/cpp/named_req/Compare) type as the second template parameter. [std::less](https://en.cppreference.com/w/cpp/utility/functional/less) is used by default so as long as ```operator<``` is defined for given type of elements you don't need to provide your own.

## Table
*Comming soon...*

# Examples
## Priority queue
```C++
auto heap = mix::ds::pairing_heap<int>();
auto handles = std::vector { heap.insert(20)
                            , heap.insert(10)
                            , heap.insert(30) };
//             ^^^^^^^^^^^
// Notice the deduction of the template parameter. Alternative:
// using handle_t = mix::ds::pairing_heap<int>::handle_t;
// std::vector<handle_t>

auto handle30to5 = handles.back();
// Dereference it just like an iterator:
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
```

# Comparison
Some structures in this library are also implemented in other libraries. In this comparison we want to show that our structures are comparable with high quality libraries as Boost. In the test we have performed series of random operations (insertions, deletions, ...) on the particular structure that contained millions of elements. This of course doesn't necessarily simulate practical usage. Numbers in the table show the time it took finish the test. 
| Structure    | mix-ds [ms] | Boost [ms] |
|--------------|-------------|------------|
| pairing_heap |    13347    |    21050   |