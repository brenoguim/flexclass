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
struct Message : public fc::FlexibleLayoutBase<Message, std::string, char[]>
{
    enum Members {Header, Data};
    using FLB::FLB;
};

Message* msgFactory(std::string header, int dataSize)
{
    auto* m = Message::niw(std::move(header), dataSize);
    std::strcpy(m->get<Message::Data>(), "Default message");
    return m;
}
```

In this new version, members are declared as arguments of `fc::FlexibleLayoutBase`. It's is convenient to create an enumeration with the names of each member: `m->get<Header>()` , `m->get<Data>()`

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
struct Message : public fc::FlexibleLayoutBase<Message, int[], std::string, std::string[], bool>
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

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::SizedArray` helper:

```
struct Message : public fc::FlexibleLayoutBase<Message, std::string, fc::SizedArray<char>>
```

In this case, there will be an `end` pointer to the `char` array too, giving the user methods `end()` and `size()`:
```
std::cout << "Size of the array is: " << m->get<Message::Data>().size() << '\n';

// Print all chars as ints:
for (char c : m->get<Message::Data>()) std::cout << static_cast<int>(c) << ' ';
```

# Feature summary

### `SizedArray` and `UnsizedArray`
- `SizedArray<T>` requests pointers to both `begin` and `end` of the array (cost: 2 pointers)
- `UnsizedArray<T>` requests a pointer only to the `begin` of the array (cost: 1 pointer)

### `UnsizedAdjacentArray` and `SizedAdjacentArray`

Adjacent arrays deduce their `begin` from another element:
- By default, they are adjacent to the base
- They can also be adjacent to another array passed as second template argument:

```
enum Members { RefCount, Data1, Data2 };
using Impl = fc::FlexibleLayoutClass<long, fc::SizedAdjacentArray<long>, fc::AdjacentArray<char, Data1>;
```

In this case:
- `Data1` array uses the end of the base to reference itself
- `Data2` array uses the `end` of `Data1` array to reference itself.

The memory costs for each array are:
- `AdjacentArray<T>` (cost: 0 pointers)
- `SizedAdjacentArray<T>` (cost: 1 pointer)

### Raw `T[]`
- `T[]` will translate to `UnsizedArray<T>` if `T` is trivially-destructible and `SizedArray<T>` otherwise

### Aliases
- `SizedArray<T>`           is equivalent to `Array<T, fc::track_size>` 
- `UnsizedArray<T>`         is equivalent to `Array<T>`  or `Array<T, fc::dont_track_size>`
- `SizedAdjacentArray<T>`   is equivalent to `AdjacentArray<T, fc::track_size>` 
- `UnsizedAdjacentArray<T>` is equivalent to `AdjacentArray<T>`  or `AdjacentArray<T, fc::dont_track_size>`

# Cool Applications

## Shared array

A shared array implementation needs a reference counter and the data (in this case an array). So it can be modeled as:

```
template<class T> class SharedArray;
template<class T>
class SharedArray<T[]>
{
    enum Members {RefCount, Data};
    using Impl = fc::FlexibleLayoutClass<unsigned, T[]>;

  public:
    /* Interesting public API */
    static SharedArray make(std::size_t len) { return {Impl::niw(/*num references*/1, len)}; }

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
    void decr() { if (m_data && m_data->template get<RefCount>()-- == 1) Impl::deleet(m_data); }
    Impl* m_data {nullptr};
};
```

Notice this implementation can be easily tweaked to use an atomic reference counter, or to store the size of the array:
```
    enum Members {RefCount, Size, Data};
    using Impl = fc::FlexibleLayoutClass<std::atomic<unsigned>, unsigned, T[]>;
    ...
```

# How `Flexclass` works

`Flexclass` works by inspecting the types being passed to `FlexibleLayout(Class|Base)`.
The first step is to recognize `T[]` types and convert them to either `SizedArray<T>` or `UnsizedArray<T>`:
```
int, std::string, char[], bool, SizedArray<int>  ==>  int, std::string, UnsizedArray<char>, bool, SizedArray<int>
```

Then it replaces all `array` types by an `ArrayBuilder`:
```
int, std::string, char[], bool, SizedArray<int>  ==>  int, std::string, ArrayBuilder<char>, bool, ArrayBuilder<int>
```

After, it initializes a tuple with these types, passing the arguments the user provided:

```
niw(Args... args = [10, "test", 300, true, 400]) {

    std::tuple<int, std::string, ArrayBuilder<char>, bool, ArrayBuilder<int>> intermediateRepresentation(args...);
}
```

So each `ArrayBuilder` will be initialized with the requested number of elements for the array.

The output tuple is was actually obtained in the begining: `tuple<int, std::string, UnsizedArray<T>, bool, SizedArray<int>>`.
But to be created, it needs to know where the arrays exist - for example, the `UnsizedArray<char>` needs to know the `begin` of the `char` array.

So the following steps are taken:
- Ask each `ArrayBuilder` how much memory it will need
- Allocate a buffer with the sum of the requested sizes + size of the output tuple
- Ask each `ArrayBuilder` to build their arrays in the buffer
- Create the output from the intermediate tuple and return it

# TODO/Known issues
- Document customization infrastructure
- move all examples to be compiled
- All inputs to `niw` are moved into an intermediate representation before being moved to the actual result. So we get two moves. Get rid of that.
- Should this class try to interoperate with `operator new` and `operator delete`?
- Add RAII wrapper
