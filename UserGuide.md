# Basic Concepts

`Flexclass` empowers the user to declare structures in the following form:

```
template<class... Types> struct fc::FlexibleClass<Types...>;
```

Some of those types are special, we call them `Handles`.
Handles may request extra memory to be allocated upon construction, so they can refer to it.

For example:
```
using Message = fc::FlexibleClass<int, fc::Array<long>, std::string>;
```

`fc::Array<long>` is identified as a `Handle`, meaning it wants to point to a group of `long` objects.
So we have the following layout:

```
            ____________________
           |                    V
| [int] [long*] [std::string] | [long] [long] [long] ...
|                             |
|           Base              | 
```

We use the term `Base` to refer to the `fc::FlexibleClass<...>`.

## Instantiating a Flexclass

A Flexclass has dynamic size, so it cannot be allocated on the stack. To allocate one on the heap, use the `::make_unique` method:

```
auto m = Message::make_unique(...);
```

This returns a RAII enabled object, which will cleanup after itself when it's destroyed.

If you prefer to manually manage this object, use `::make` and `fc::destroy`:
```
auto* m = Message::make(...);
fc::destroy(m);
```

## Arguments

`make` and `make_unique` methods require one argument for each type declared in  `fc::FlexibleClass`.
Each of those arguments will be passed to initalize an object of each type.

```
fc::FlexibleClass<std::string, float, fc::Array<long>, float>::make(
    "passed to the string",
    1.0,   // this goes to the first float
    3000,  // This serves the handle
    3.7);  // this goes to the second float
```

Handles are special in this case. The library will pick the argument destinated to handles (`3000` in this case) and create an array of such size.
In this example, an array of `3000` `long` objects will be allocated after the `Base`.
Finally, the `begin` and `end` pointers will be passed to the handle, so it can track this array.

## Accessing the data

Every object in the structure can be accessed via their indices in the type list:

```
auto m = fc::FlexibleClass<int, fc::Array<long>, std::string>::make_unique(...);

m->get<0>()    -> returns an int&
m->begin<1>()  -> returns the pointer to the begin of the long array
m->end<1>()    -> returns a pointer to the end of the long array
m->get<2>()    -> returns a std::string&
```

It is recomended to use a namespace to tie names to each index:
```
namespace Message
{
    enum Members { Name, Age, Data };
    using Type = fc::FlexibleBase<std::string, int, Foo[]>;
}

...

auto name      = m->get<Message::Name>();
auto age       = m->get<Message::Age>();
auto dataBegin = m->->begin<Message::Data>();
```

Another alternative is to use a `FlexibleBase`.


## FlexibleBase

As noticed from the previous example, accessing elements via numbers can be hard to read. A solution is to use a `FlexibleBase` with an enumeration:

```
struct MyStruct : public fc::FlexibleBase<MyStruct, int, fc::Array<long>, std::string>
{
    enum { Age, Data, Name };
    using FLB::FLB;
};
```

This allows the user to:
- Access members using the enumeration as indices: `m->get<Age>()`, `m->begin<Data>()`, `m->get<Name>()`
- Define custom member functions.


# Provided Handles

`Flexclass` provides a handful of handles so the user doesn't have to write them by hand.

There are two types of handles:
- `fc::handle::array`: Able to point just to the begin of the sequence of elements
- `fc::handle::range`: Able to point to both begin and end.

Available handles are:
- `fc::Array<T>`: Contains one `T*` to indicate the begin of the object sequence
- `fc::Range<T>`: Contains two `T*` to indicate both begin and end
- `fc::AdjacentArray<T, int Idx = -1>`: Contains no data as it assumes its array is adjacent to the data from handle in `Idx`
    - If `Idx` is `-1`, it assumes the begin of its array is after the Base
- `fc::AdjacentRange<T, int Idx = -1>`: Like `fc::AdjacentArray<T>` but contains a `T*` to also know the end of the object sequence
- `T[]`: Automatically converted into a `fc::Array<T>` in case `T` is trivially-destructible, and `fc::Range<T>` otherwise

# Custom handles

Handles being provided with the library use the framework to implement custom handles.
Here is a customization example of a handle that assumes that the data is very close to the base and is very small:

```
template <class T>
struct NearAndSmallRange // I know my Range will be very close and very small
{
    template <class U> NearAndSmallRange(U&&) {} // Boilerplate that won't be needed in future versions.
    
    // Definition of some types to make this recognizable as a handle
    using type = T;
    using fc_handle = fc::handle::range;
    
    // This is called by the library to inform where the array for this handle was placed
    void setLocation(T* begin, T* end)
    {
        auto size = end - begin;
        auto offset = (uintptr_t)begin - (uintptr_t)this;
        
        assert(size   <= std::numeric_limits<std::uint8_t>::max());
        assert(offset <= std::numeric_limits<std::uint8_t>::max());
        
        m_beginOffset = offset;
        m_size = size;
    }

    // This is called by the library when the user asks where is the begin of the array
    // The pointer of the base itself is provided, in case this handle needs information from the base
    //     See the implementation of fc::AdjacentArray
    template <class Base>
    auto begin(const Base* ptr) const
    {
        return (T*)((uintptr_t)this + m_beginOffset);
    }
    
    template <class Base>
    auto end(const Base* ptr) const
    {
        return begin(ptr) + m_size;
    }

    std::uint8_t m_beginOffset, m_size;
}
```

# Advanced initialization

## Member initialization
As mentioned before, the user must pass an argument for each type of the class. If the user desires to not initialize the non-handle member, the `fc::Default` object can be used:
```
auto m = fc::FlexibleClass<std::string, int>::make(fc::Default{}, fc::Default{});
```

In this scenario, the default constructor of the `std::string` will be called, and the  `int` **will be left uninitialized** as if it was declared with `int i;`.

## Handle initialization

For each handle, the user is expected to pass a size of the array that will be allocated for it. Arrays are default initialized, but the user may use `fc::arg` to pass an input iterator that will be called to obtain initial values each element:

```
std::vector<int> v {1,2,3,4,5,6,7,8,9,10};
auto m = fc::FlexibleClass<fc::Array<int>>::make(fc::arg(10, v.begin()));
```
In this scenario, the FlexibleClass will contain values `1` to `10` obtained from the iterator.

# Exception Guarantees

`Flexclass` is well behaved with respect to lifetimes and exceptions. That means all objects created by it will be destroyed in the reverse order:

```
    auto m = fc::FlexibleClass<A, B, C[], D, E[]>::make(A{}, B{}, 2, D{}, 3);
```
In this cenario, constructors will be called in the following order:    `C0 C1 E0 E1 E2 A B D`

When `fc::destroy` is called, destructors will be invoked in the order: `D B A E2 E1 E0 C1 C0`

If at any point during the construction of the FlexibleClass an exception is thrown, all objects that were fully created will be destroyed in the reverse order of construction.
