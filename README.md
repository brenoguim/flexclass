# flexclass
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
    - This array size is necessary to delete with `delete[]`
- Many high-performance codebases (such as `LLVM`) will apply this technique to avoid allocations

The goal of merging these allocations is to improve performance through less `malloc` calls, improving locality and reducing fragmentation.

# Enter Flexclass

The same `Message` can be implemented with `Flexclass`:

```
struct Message : public fc::FlexibleLayoutClass<Message, std::string, char[]>
{
    enum Members {Header, Data};
    using FLC::FLC;
};

Message* msgFactory(std::string header, int dataSize)
{
    auto* m = Message::niw(std::move(header), dataSize);
    std::strcpy(m->get<Message::Data>(), "Default message");
    return m;
}
```

In this new version, members are declared as arguments of `fc::FlexibleLayoutClass`. To make member access convenient, it's also recommended to create an enumeration with the names of each member: `m->get<Header>()` , `m->get<Data>()`

With that out of the way, this `Flexclass` will now inspect the argument list, find `char[]` and understand that you want an array together with your datastructure.

The layout of the `Message` is:
```
    [std::string] [char* const] [char] [char] [char] ... [char]
                       |        ^   
                       |________|
```

`Flexclass` not limited to one array, so the following declaration is perfectly valid:
```
struct Message : public fc::FlexibleLayoutClass<Message, int[], std::string, std::string[], bool>
```

Which will generate the following layout:

```
                                                            ______________________________________________________________
                                                           |                                                              |
                                      _____________________|________________________________                              |
                                     |                     |                                |                             | 
                                     |                     |                                v                             v
[int* const] [std::string] [std::string* const] [std::string* const] [bool] [int] ... [int] [std::string] ... [std::string]
     |                                                                      ^
     |______________________________________________________________________|
```

Notice the layout contains the `end` pointer for the `std::string` array. Since `std::string` is non-trivially-destructible, `Flexclass` needs to iterate on the array to call destructors of such objects when destroying the `Message`.

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::SizedArray` helper:

```
struct Message : public fc::FlexibleLayoutClass<Message, std::string, fc::SizedArray<char>>
```

In this case, there will be an `end` pointer to the `char` array too, giving the user methods `end()` and `size()`:
```
std::cout << "Size of the array is: " << m->get<Message::Data>().size() << '\n';

// Print all chars as ints:
for (char c : m->get<Message::Data>()) std::cout << static_cast<int>(c) << ' ';
```



# TODO/Known issues
- Sometimes the begin of an array can be inferred from the class state. Implement a customization infrastructure to query the class in such case.
    - The obvious example is the first declared array in the class. It can be assumed it will always sit right after the class object itself.
- All inputs to `niw` are moved into an intermediate representation before being moved to the actual result. So we get two moves. Get rid of that.
- Should this class try to interoperate with `operator new` and `operator delete`?
- Add RAII wrapper


