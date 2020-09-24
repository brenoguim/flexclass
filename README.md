# Flexclass
A library for structures with flexible layout. See the [User Guide](../master/UserGuide.md) for a complete walkthrough.

## Problem statement

`C++` offers the `struct` or `class` as a way to group objects and abstract complexity. However, sometimes that leads to suboptimal designs:

```
struct Message
{
    std::string header;
    std::unique_ptr<char[]> m_data;
};

Message* msgFactory(std::string header, int dataSize)
{
    auto* m = new Message{std::move(header), std::make_unique<char[]>(dataSize)};
    std::strcpy(m->m_data.get(), "Default message");
    return m;
}
```

In this scenario, everytime `msgFactory` is called two allocations will occur:
- One for the `m_data` field containing the data
- One for the `Message` object itself

It is a rather common technique to group these allocations together. Somes examples are:
- `std::make_shared<T>` will use a single allocation to create the control block and the T object
- `new Object[10]` where `Object` is non-trivially-destructible will force the compiler to allocate extra space for the size of the array
    - This array size is necessary iterate on all elements calling the destructor
- Many high-performance codebases (such as `LLVM`) will apply this technique to avoid allocations

The goal of merging these allocations is to improve performance through less `malloc` calls, improving locality and reducing fragmentation.

## Enter Flexclass

The same `Message` can be implemented with `Flexclass`:

```
namespace Message {
    enum Members {Header, Data};
    using Type = fc::FlexibleClass<std::string, char[]>;
}

Message::Type* msgFactory(std::string header, int dataSize)
{
    auto* m = Message::Type::make(std::move(header), dataSize);
    std::strcpy(m->begin<Message::Data>(), "Default message");
    return m;
}
```

[See this example on Compiler Explorer](https://godbolt.org/z/EGTvhP)

In this new version, members are declared as arguments of `fc::FlexibleClass`. It's is convenient to create an enumeration with the names of each member: `m->get<Header>()` , `m->begin<Data>()`

To build the layout, `Flexclass` inspects the argument list, finds `char[]` and understands that a `char` array must be attached to the allocation.

The layout of the `Message` is:
```
                   _____
                  |     |
                  |     V
|[std::string] [char*]| [char] [char] [char] ... [char]
|                     |
|                     |
|      Message        |
```

`Flexclass` is not limited to one array, so the following declaration is perfectly valid:
```
struct Message : public fc::FlexibleBase<Message, int[], std::string, std::string[], bool>
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
|                       Message                           |
```

Notice the layout contains an `end` pointer for the `std::string` array. Since `std::string` is non-trivially-destructible, `Flexclass` needs to iterate on the array to call destructors when destroying the `Message`.

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::Range` helper:

```
struct Message : public fc::FlexibleBase<Message, std::string, fc::Range<char>>
```

In this case, there will be an `end` pointer to the `char` array too, giving the user method `end()`:
```
auto b = m->begin<Message::Data>();
auto e = m->end<Message::Data>();

std::cout << "Size: " << e - b << std::endl;

for (auto it = b; it != e; ++it) std::cout << (int) c << std::endl;
```

On the other hand, one can reduce the size even more by using the fact that the first array will always be at the end of the `Message`:
```
struct Message : public fc::FlexibleBase<Message, std::string, fc::AdjacentArray<char>>
```

Which would generate the following compact layout:
```
|[std::string]| [char] [char] [char] ... [char]
|             |
|             |
|   Message   |
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


### Variant Array

TODO: Add a description for this example

[See the full example here](../master/tests/unit/variant_array_example.test.cpp)


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


