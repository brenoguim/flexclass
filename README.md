# Flexclass
A library for structures with flexible layout.

# Problem statement

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

# Enter Flexclass

The same `Message` can be implemented with `Flexclass`:

```
struct Message : public fc::FlexibleBase<Message, std::string, char[]>
{
    enum Members {Header, Data};
    using FLB::FLB;
};

Message* msgFactory(std::string header, int dataSize)
{
    auto* m = Message::make(std::move(header), dataSize);
    std::strcpy(m->get<Message::Data>(), "Default message");
    return m;
}
```

In this new version, members are declared as arguments of `fc::FlexibleBase`. It's is convenient to create an enumeration with the names of each member: `m->get<Header>()` , `m->get<Data>()`

To build the layout `Flexclass` inspects the argument list, finds `char[]` and understands that a `char` array must be attached to the allocation.

The layout of the `Message` is:
```
                     _________
                    |         |
                    |         V
|[std::string] [char* const]| [char] [char] [char] ... [char]
|                           |
|                           |
|         Message           |
```

`Flexclass` is not limited to one array, so the following declaration is perfectly valid:
```
struct Message : public fc::FlexibleBase<Message, int[], std::string, std::string[], bool>
```

Which will generate the following layout:

```
                                                             _______________________________________________________________
                                                            |                                                               |
                                       _____________________|_________________________________                              |
                                      |                     |                                 |                             |
       _______________________________|_____________________|_________________                |                             |
      |                               |                     |                 V               V                             V
|[int* const] [std::string] [std::string* const] [std::string* const] [bool]| [int] ... [int] [std::string] ... [std::string]
|                                                                           |
|                                                                           |
|                               Message                                     |
```

Notice the layout contains an `end` pointer for the `std::string` array. Since `std::string` is non-trivially-destructible, `Flexclass` needs to iterate on the array to call destructors when destroying the `Message`.

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::Range` helper:

```
struct Message : public fc::FlexibleBase<Message, std::string, fc::Range<char>>
```

In this case, there will be an `end` pointer to the `char` array too, giving the user methods `end()` and `size()`:
```
std::cout << "Size of the array is: " << m->get<Message::Data>().size() << '\n';

// Print all chars as ints:
for (char c : m->get<Message::Data>()) std::cout << static_cast<int>(c) << ' ';
```

On the other hand, one can reduce the size even more by using the fact that the first array will always aligned at the end of the `Message`:
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

# Feature summary

### `Range` and `Array`
- `Range<T>` requests pointers to both `begin` and `end` of the array (cost: 2 pointers)
- `Array<T>` requests a pointer only to the `begin` of the array (cost: 1 pointer)

### `AdjacentArray` and `AdjacentRange`

Adjacent handles deduce their `begin` from another element:
- By default, they are adjacent to the base
- They can also be adjacent to another array passed as second template argument:

```
enum Members { RefCount, Data1, Data2 };
using Impl = fc::FlexibleClass<long, fc::AdjacentRange<long>, fc::AdjacentArray<char, Data1>;
```

In this case:
- `Data1` array uses the end of the base to reference itself
- `Data2` array uses the `end` of `Data1` array to reference itself.

Cost:
- `AdjacentArray<T>` cost 0 pointers
- `AdjacentRange<T>` cost 1 pointer

### Raw `T[]`
- `T[]` will translate to `Array<T>` if `T` is trivially-destructible and `Range<T>` otherwise

# Cool Applications

## Shared array

A shared array implementation needs a reference counter and the data (in this case an array). So it can be modeled as:

```
template<class T> class SharedArray;
template<class T>
class SharedArray<T[]>
{
    enum Members {RefCount, Data};
    using Impl = fc::FlexibleClass<unsigned, T[]>;

  public:
    /* Interesting public API */
    static SharedArray make(std::size_t len) { return {Impl::make(/*num references*/1, len)}; }

    decltype(auto) operator[](std::size_t i) { return m_data->template begin<Data>()[i]; }
    decltype(auto) operator[](std::size_t i) const { return m_data->template begin<Data>()[i]; }

    /* Boilerplate */
    SharedArray(SharedArray&& other) : m_data(std::exchange(other.m_data, nullptr)) {}
    SharedArray(const SharedArray& other) { m_data = other.m_data; incr(); }
    SharedArray& operator=(SharedArray&& other) { decr(); m_data = std::exchange(other.m_data, nullptr); return *this; }
    SharedArray& operator=(const SharedArray& other) { decr(); m_data = other.m_data; incr(); return *this; }
    ~SharedArray() { decr(); }
  private:
    SharedArray(Impl* data) : m_data(data) {}
    void incr() { if (m_data) m_data->template get<RefCount>()++; }
    void decr() { if (m_data && m_data->template get<RefCount>()-- == 1) Impl::destroy(m_data); }
    Impl* m_data {nullptr};
};
```

Notice this implementation can be easily tweaked to use an atomic reference counter, or to store the size of the array:
```
    enum Members {RefCount, Size, Data};
    using Impl = fc::FlexibleClass<std::atomic<unsigned>, unsigned, T[]>;
    ...
```

# How `Flexclass` works

This is a simplified implementation/pseudo-algorithm:
```
FlexibleBase::make(Args... args)
{
    // What we call "Base" is a tuple with all types the user declared, but with T[] converted to Array handles.
    using Base = std::tuple</* Args... with T[] -> Array<T[]> */>;
    
    // We cannot initialize this Base with the user arguments, because each array gets the "size" as a parameter
    // and Array does not have such constructor.
    
    // To workaround that, we create an intermediate representation, with each Array<T> converted to ArrayBuilder<T>
    //    other T are transformed into an "Ignore" class which is just created from any parameter, and ignores it
    using Intermediate = std::tuple< /* Use Base but convert Array types to ArrayBuilder */>;

    // Now we can instantiate the intermediate type:
    Intermediate ir(args...);
    
    // Each ArrayBuilder knows the requested size of the array and it's type, 
    //    so we iterate on these builders and ask for the number of bytes they need
    
    //foreach (ar : ArrayBuilders) numBytesForArrays += ar.neededBytes();
    
    // Now we can create the storage
    void* storage = ::operator new(sizeof(Base) + numBytesForArrays);
    
    // Each ArrayBuilder is now provided with the storage for it to create the array it wants to manage
    // foreach (ar : ArrayBuilders) ar.neededBytes(storage);
    
    // Finally we create the "Base" tuple from the user arguments. Array types will ignore the "int" type being passed
    auto r = new (storage) Base(std::move(ir));

    // Before returning, we iterate on the intermediate representation and the Base assigning array locations
    foreach ([b, i] : [*r, ir]) { b.setLocation(i.begin(), i.end()); }

    return r;
}
```

# TODO/Known issues
- Improve tests
    - Rethink how tests can be done in a more extensive and automatic way
- Add CI
- clang-format
- Add range-for support for sized arrays
- Support arrays before the base
- Check if available features are enough to replace code in LLVM (User/Uses classes)
- Add RAII wrapper
- Document customization infrastructure
- Make aligner constexpr? I suspect it can be much more efficient than it is now
- Add support passing arguments to the array constructor
