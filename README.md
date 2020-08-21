# Contents
* [Intro](#intro)
* [Data structures](#data-structures)
    - [Pairing heap](#pairing-heap)
    - [Simple map](#simple-map)
* [Documentation](#documentation)
    - [Priority queue](#priority-queue)
    - [Table](#table)
* [Examples](#examples)
    - [Priority queue](#priority-queue-1)
    - [Table](#table-1)
* [Comparison](#comparison)

# Intro
This is a small data structure library for C++. Some structures are useful in practice and some of them that are interesting in theory can be used in some experiments and comparisons. Each structure is implemented in a single independent [header file](./src/lib). Using them is therefore very simple. You just need to include particular header file in your project. You will also need a compiler that supports C++17. It was tested with `gcc 10.1.0`, `clang++ 10.0.0` and `Visual Studio 2019`.

# Data structures
## Pairing heap
Pairing heap is simple and efficient implementation of the [priority queue](https://en.wikipedia.org/wiki/Priority_queue). It performs very well in [Discrete-event simulation](https://en.wikipedia.org/wiki/Discrete-event_simulation). You can read the formal description on [Wikipedia](https://en.wikipedia.org/wiki/Pairing_heap) , in the [original paper](https://www.cs.cmu.edu/~sleator/papers/pairing-heaps.pdf) and on many other places on the [internet](https://www.google.com/search?q=pairing+heap&oq=pairing+heap). We are not gonna repeat that here.  
Pairing heap is implemented in [Boost](https://www.boost.org/doc/libs/1_73_0/doc/html/boost/heap/pairing_heap.html) and you will probably find many other implementations but some of them are naive or not generic. Our implementation is a generic, allocator-aware and recursion-free container. We use a binary tree to represent the heap and we also support two *merge modes* => two pass merge *(default)* and fifo queue. It can be specified by a template parameter.

## Simple map
Simple map is a container adapter which takes a container *(`std::vector` by default)* and turns it into a map. It might be useful when you need a map semantics but you are only working with very small amount of data. In that case simple vector might perform better than sophisticated structures like red-black tree or hash table.

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
1. ... 3. Corrects a position of an element that is associated with given handle/iterator after its priority have been **increased**. See the [example](#priority-queue-1) below.  

*Similar operation `increase_key` is not supported in general but might be supported by some specific implementations.*

### Other
Each priority queue takes [Compare](https://en.cppreference.com/w/cpp/named_req/Compare) type as the second template parameter. [std::less](https://en.cppreference.com/w/cpp/utility/functional/less) is used by default so as long as `operator<` is defined for given type of elements you don't need to provide your own.

## Table
Our tables (maps) have almost the same interface and behaviour as STL maps. You can check [std::map](https://en.cppreference.com/w/cpp/container/map) for detailed documentation. In the [examples section](#table-1) you can find a couple of notes on how to use a map correctly. 

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
// Dereference it just like an output iterator:
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

## Table
First thing we need to clarify is that map stores keys and elements inside `std::pair<const Key, Value>`. Notice that the key is always `const` whether you specify it or not.  
For our examples we will use this simple `struct person` as a value type and `int` as a key type. `person` declared like this is [aggregate](https://en.cppreference.com/w/cpp/language/aggregate_initialization). Prior to `C++20` this struct would need to have a constructor that initializes its fields in order to use `emplace` and similar functions metioned below. `C++20` made som changes that are beyond the scope of this example. All we need to know is that in `C++20` we can also use `()` for unifrom initialization *(but there is still a difference between `()` and `{}`)* and our `person` doesn't need a constructor.
```C++
struct person
{
    int age;
    std::string name;
};

std::map<int, person> personMap;
```
There are couple of different ways to insert an element into a map. First is to use the `insert` member function which takes pair as parameter.
```C++
using pair_t = std::map<int, person>::value_type;
// pair_t is std::pair<const int, person>

pair_t p(1, person {27, "Daniel"});

personMap.insert( p ); // 1.
personMap.insert( pair_t(2, person {47, "Jack"}) ); // 2.
personMap.insert( std::make_pair(3, person {80, "Teal'c"}) ); // 3.
```
1. New pair in the map will be copy constructed from p.
2. New pair in the map will be move constructed from the temporary.
3. `std::make_pair` returns a `std::pair<int, person>` wich is a different type than `std::pair<const int, person>`. However, insert has an overload which takes anything from which a `std::pair<const int, person>` can be [constructed from](https://en.cppreference.com/w/cpp/utility/pair/pair) so in this case the new pair will be memberwise move constructed from the temporary pair.

We can avoid creating a temporary pair by using `emplace` which enables us to construct the new pair directly *in* the map. 
```C++
personMap.emplace( 4, person {32, "Samantha"} ); // 1.
personMap.emplace( std::piecewise_construct
                 , std::forward_as_tuple(5) 
                 , std::forward_as_tuple(55, "George") ); // 2.
```
1. New pair will be constructed in place using given arguments. Note that person will still be move constructed and not constructed in place.
2. New pair and its members will be constructed in place using given arguments. This solution is however bit more verbose so it is up to you to consider whether its worth to do it in order to avoid the move which is usually cheap.

One disadvatage of `emplace` is that it always creates a new pair (in order to have access to the key) and in case there already is an element associated with given key it discards that pair. That is why C++17 introduced `try_emplace`.
```C++
personMap.try_emplace(4, 32, "Samantha"); // 1.
personMap.try_emplace(6, 28, "Jonas");    // 2.
```
1. Does nothing since there already is a key `4`. Complexity depends on a particular map implementation since the key uniqueness always needs to be checked.
2. New pair is constructed in place. The key is copy/move constructed from given key and the value is constructed in place from given arguments.

`operator[]` can also be used to insert elements into a map. 
```C++
personMap[7] = person(40, "Cameron"); // 1.
```
1. Since there is no element associated with key `7` first a new pair is created. The key is copy/move constructed from the given key and the value is [value-initialized](https://en.cppreference.com/w/cpp/language/value_initialization) (person will look like this {.age = 0, .name = ""}).  

Behaviour of `operator[]` is useful when you don't know whether there is an element associated with given key and in case there is you want to assign new value to it. Disadvantage of this approach is that if insertion is performed an *empty* element is first constructed and then assigned with actual value. That is why C++17 introduced `insert_or_assign` which does exactly what it says it does and unlike `operator[]` it does not create temporary *empty* value.
```C++
personMap.insert_or_assign(7, person(43, "Cameron")); // 1.
personMap.insert_or_assign(8, person(33, "Vala"));    // 2.
```
1. Copy/move assigns the person with correct age to the element that already is in the map.
2. Copy/move constructs the new pair as if by using `insert`.  

There are three different ways to access elements of a map.
```C++
auto daniel = personMap.at(1);   // 1.
auto jackIt = personMap.find(2); // 2.
if (personMap.end() != jackIt)
{
    auto jack = jackIt->second;
}
auto hank = personMap[9]; // 3.
```
1. Returns reference to the element associated with given key. Throws `std::out_of_range` if there is no such key.
2. Returns iterator to the pair associated with given key. If there is no such element returns end iterator.
3. Returns reference to the element associated with given key. If there is no such element it value constructs the element and returns reference to it. This is not what we wanted in the example above so you can see the disadvantage of `operator[]`.

Removing elements from a map is quite simple.
```C++
auto danielIt = personMap.find(1);
personMap.erase(danielIt); // 1.
personMap.erase(9);        // 2.
```
1. Removes pair associated with given iterator.
2. Removes pair (if there is one) associated with given key.

#### Recomendations
* use `emplace` or `try_emplace` for insertion
* use `at` or `find` for element access
* use `insert_or_assign` over `operator[]` unless you need the exact behaviour of `operator[]`

# Comparison
Some structures in this library are also implemented in other libraries. In this comparison we want to show that our structures are comparable with high quality libraries as Boost. In the test we have performed series of random operations (insertions, deletions, ...) on the particular structure that contained millions of elements. This of course doesn't necessarily simulate practical usage. Numbers in the table show the time it took finish the test. 
| Structure    | mix-ds [ms] | Boost [ms] |
|--------------|-------------|------------|
| pairing_heap |    13347    |    21050   |