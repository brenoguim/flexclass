# Flexclass
A library for structures with flexible layout. See the [User Guide](../master/UserGuide.md) for a complete walkthrough.

## Problem statement

Consider the following implementation of a `Node` in a graph structure:
```
struct Node
{
    std::size_t              id;
    void*                    userData {nullptr};
    std::unique_ptr<Node*[]> links;
};

Node* makeGraphNode(std::size_t id, const std::vector<Node*>& links)
{
    auto n = new Node{id};
    n->links = std::make_unique<Node*[]>(links.size());
    std::copy(links.begin(), links.end(), n->links.get());
    return n;
}
```

In this scenario, everytime `makeGraphNode` is called two allocations will occur:
- One for `links` array
- One for `Node` object itself

Multiple allocations can be costly and incur in memory fragmentation which reduce cache friendlyness.

## Enter Flexclass

The same `Node` can be implemented with `Flexclass`:

```
struct Node : fc::FlexibleBase<Node,
                  /*Members:*/ std::size_t, void*, Node*[]>
{
    enum Members {Id, UserData, Links};
    using FLB::FLB;
};

Node* makeGraphNode(std::size_t id, const std::vector<Node*>& links)
{
    auto n = Node::make(id, /*user data*/nullptr, /*array size*/links.size());
    std::copy(links.begin(), links.end(), n->begin<Node::Links>());
    return n;
}
```

[See this example on Compiler Explorer](https://godbolt.org/z/Pcbroj)

In this new version, members are declared as arguments of `fc::FlexibleBase`. The `Members` enum serves as a convience to allow: `m->get<Node::Id>()` , `m->begin<Node::Links>()`

To build the layout, `Flexclass` inspects the member type list, finds `Node*[]` and understands that a `Node*` array must be attached to the allocation.

The layout of the `Node` is:
```
                                _______
                               |       |
                               |       V
| [std::size_t]   [void*]   [Node**] | [Node*] [Node*] [Node*] ... [Node*]
|       Id       UserData    Links   |  
|                                    |
|                  Node              |
```

Now a single allocation is performed to store both the `Node` and the array of `Node*`.

One can reduce the size even more by using the fact that the `Node*` array will always be at the end of the `Node`:
```
struct Node : public fc::FlexibleBase<Node, std::size_t, void*, fc::AdjacentArray<Node*>>
```

Which would generate the following compact layout:
```
|[std::size_t]   [void*]      []    | [Node*] [Node*] [Node*] ... [Node*]
|      Id       UserData    Links   |  
|                                   |
|                 Node              |
```


`Flexclass` is not limited to one array, so the following declaration is perfectly valid:
```
struct MyType : public fc::FlexibleBase<MyType, int[], std::string, std::string[], bool>
```

Which will generate the following layout:

```
                                           _______________________________________________________________
                                          |                                                               |
                              ____________|_________________________________                              |
                             |            |                                 |                             |
     ________________________|____________|_________________                |                             |
    |                        |            |                 V               V                             V
|[int*] [std::string] [std::string*] [std::string*] [bool]| [int] ... [int] [std::string] ... [std::string]
|                                                         |
|                                                         |
|                       MyType                            |
```

Notice the layout contains an `end` pointer for the `std::string` array. Since `std::string` is non-trivially-destructible, `Flexclass` needs to iterate on the array to call destructors when destroying the `Message`.

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::Range` helper:

```
struct Message : public fc::FlexibleBase<Message, std::size_t, fc::Range<char>>
```

In this case, there will be an `end` pointer to the `char` array too, giving the user method `end()`:
```
auto b = m->begin<Message::Data>();
auto e = m->end<Message::Data>();

std::cout << "Size: " << e - b << std::endl;

for (auto it = b; it != e; ++it) std::cout << (int) c << std::endl;
```


## Feature summary

### `Range` and `Array`
- `Range<T>` requests pointers to both `begin` and `end` of the array (cost: 2 pointers)
- `Array<T>` requests a pointer only to the `begin` of the array (cost: 1 pointer)

### `AdjacentArray` and `AdjacentRange`

Adjacent handles deduce their `begin` from another element:
- By default, they are adjacent to the base
- They can also be adjacent to another array passed as second template argument:

```
enum Members                  {RefCount,  Data1,                   Data2 };
using Impl = fc::FlexibleClass<long,      fc::AdjacentRange<long>, fc::AdjacentArray<char, Data1>;
```

In this case:
- `Data1` array uses the end of the base to reference itself
- `Data2` array uses the `end` of `Data1` array to reference itself.

Cost:
- `AdjacentArray<T>` cost 0 pointers
- `AdjacentRange<T>` cost 1 pointer

### Raw `T[]`
- `T[]` will translate to `Array<T>` if `T` is trivially-destructible and `Range<T>` otherwise

## Cool Applications

### Shared Array

A shared array implementation needs a reference counter and the data (in this case an array). So it can be modeled as:

```
template<class T> class SharedArray;
template<class T>
class SharedArray<T[]>
{
    enum Members                  {RefCount, Data};
    using Impl = fc::FlexibleClass<unsigned, T[]>;

  public:
    /* Interesting public API */
    static SharedArray make(std::size_t len) { return {Impl::make(/*num references*/1, len)}; }

    decltype(auto) operator[](std::size_t i) { return m_data->template begin<Data>()[i]; }
    decltype(auto) operator[](std::size_t i) const { return m_data->template begin<Data>()[i]; }

    /* Boilerplate special member functions */
  private:
    SharedArray(Impl* data) : m_data(data) {}
    void incr() { if (m_data) m_data->template get<RefCount>()++; }
    void decr() { if (m_data && m_data->template get<RefCount>()-- == 1) Impl::destroy(m_data); }
    Impl* m_data {nullptr};
};
```

Notice this implementation can be easily tweaked to use an atomic reference counter, or to store the size of the array:
```
    enum Members                   {RefCount,             Size,     Data};
    using Impl = fc::FlexibleClass<std::atomic<unsigned>, unsigned, T[]>;
    ...
```
[See the full example here](../master/tests/unit/shared_array_example.test.cpp)


## TODO/Known issues
- Provide ways to convert from a Adjacent member to base
    - `convert<int element>(T*) -> fc::FlexibleClass<int, fc::AdjacentArray<T>>*`
- Add performance tests
- Add range-for support for AdjacentRanges.
- Use static asserts to provide better diagnostics on common mistakes:
    - Passing the wrong number of parameters
    - Instantiating a flexclass containing an undefined type
- Check if available features are enough to replace code in LLVM (User/Uses classes)
- Documentation - review
- Implement `Optional` or `Maybe` as a short-cut for an array with 0 or 1 element.
    - Then it would be possible to add an example of creating a mixin system
    - Or maybe just a MultiVariant<A, B, C>, where existing objects could be just "A", "B", "C", "A B", "B C", "A C" or "A B C"
        - what's the application for that?
- Support arrays before the base


