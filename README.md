# flexclass
A library for structures with flexible layout.

# Problem statement

`C++` offers the `struct` or `class` as a way to group objects and abstract complexity. However, sometimes that leads to suboptimal designs:

```
struct Message
{
    std::string header;
    std::unique<char[]> m_data;
};

Message* makeMessage(std::string header, int dataSize)
{
    return new Message{std::move(header), std::make_unique<char[]>(dataSize)};
}
```

In this scenario, everytime `makeMessage` is called two allocations will occur:
- One for the `m_data` field containing the data
- One for the `Message` object itself

It is a rather common technique to group these allocations together. Somes examples are:
- `std::make_shared<T>` will use a single allocation to create the control block and the T object
- `new Object[10]` where `Object` is non-trivially-destructible will force the compiler to allocate extra space for the size of the array
    - This array size is necessary to delete with `delete[]`
- Many high-performance codebases (such as `LLVM`) will apply this technique to avoid allocations

The goal of merging these allocations is to improve performance through less `malloc` calls, improving locallity and reducing fragmentation.

# Enter Flexclass

The same `Message` can be implemented with `Flexclass`:

```
struct Message : public fc::FlexibleLayoutClass<Message, std::string, char[]>
{
    enum Members {Header, Data};
    using FLC::FLC;
};

Message* makeMessage(std::string header, int dataSize) { return Message::niw(std::move(header), dataSize); }
```

In this new version, members are declared as arguments of `fc::FlexibleLayoutClass`. To make member access convenient, it's also recommended to create an enumeration with the names of each member: `m->get<Header>()` , `m->get<Data>()`

With that out of the way, this `Flexclass` will now inspect the argument list, find `char[]` and understand that you want an array together with your datastructure.

The layout of the `Message` is:
```
    [std::string] [char* const] [char] [char] [char] ... [char]
                       |           ^   
                       |-----------|
```

Note: The library can avoid the trailing `char* const` in future versions, since it can be inferred that the array of `char` starts right after the `Message` object.

# Customizations

`Flexclass` not limited to one array, so the following declaration is perfectly valid:
```
struct Message : public fc::FlexibleLayoutClass<Message, int[], std::string, std::string[], bool>
```

Which will generate the following layout:

```
                                                            _________________________________________________________________________
                                                           |                                                                         |
                                      _____________________|_________________________________________                                |
                                     |                     |                                         |                               | 
                                     |                     |                                         v                               v
[int* const] [std::string] [std::string* const] [std::string* const] [bool] [int] [int] ... [int] [std::string] [std::string] ... [std::string]
     |                                                                       ^
     |                                                                       |
     |_______________________________________________________________________|
```

Notice that for `std::string[]` the layout will contain pointers to the first and last elements in the array. That's necessary so the destructors of all non-trivially-destructible objects are called when the `Message` is destroyed.

Storing the size is sometimes useful, so the user can force the type to hold the begin/end with the `fc::SizedArray` helper:

```
struct Message : public fc::FlexibleLayoutClass<Message, fc::SizedArray<int>, std::string, std::string[], bool>
```

# TODO/Known issues
- Avoid the pointer for trailing arrays
- Sometimes the begin of an array can be inferred from the class state. Implement a customization infrastructure to query the class in such case.
