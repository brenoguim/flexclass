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
struct Node
{
    auto fc_handles() { return fc::make_tuple(&links); }

    std::size_t      id;
    void*            userData {nullptr};
    fc::Array<Node*> links;
};

Node* makeGraphNode(std::size_t id, const std::vector<Node*>& links)
{
    auto n = fc::make<Node>(links.size()) (id);
    std::copy(links.begin(), links.end(), n->links.begin());
    return n;
}
```

The required changes are:
- Declare `fc_handles` method that returns the required handles
- Declare `links` as `fc::Array<Node*>` which is a special handle for an array
- Construct the object with the syntax `fc::make< T > ( array sizes ) ( T constructor arguments );`

[See this example on Compiler Explorer](https://godbolt.org/z/9K93Pe)

To build the layout, `Flexclass` uses `fc_handles` to collect a list of array types to be built.

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
fc::AdjacentArray<Node*> links;
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
struct MyType
{
    auto fc_handles() { return fc::make_tuple(&a, &c); }

    fc::Array<int> a;
    std::string b;
    fc::Range<string> c;
    bool;
};
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

Notice that `fc::Range` has an `end` pointer for the `std::string` array. Since `std::string` is non-trivially-destructible, `Flexclass` needs to iterate on the array to call destructors when destroying the `MyType`.

## Feature summary

### `Range` and `Array`
- `Range<T>` requests pointers to both `begin` and `end` of the array (cost: 2 pointers)
- `Array<T>` requests a pointer only to the `begin` of the array (cost: 1 pointer)

### `AdjacentArray` and `AdjacentRange`

Adjacent handles deduce their `begin` from another element:
- By default, they are adjacent to the base
- They can also be adjacent to another array. See the [user guide](../master/UserGuide.md) for the syntax.

Cost:
- `AdjacentArray<T>` cost 0 pointers
- `AdjacentRange<T>` cost 1 pointer

## Cool Applications

### Shared Array

A shared array implementation needs a reference counter and the data (in this case an array). So it can be modeled as:

<!-- snippet: shared_array_example -->
<!-- endSnippet -->


## TODO/Known issues
- Provide ways to convert from a Adjacent member to base
- Add range-for support for AdjacentRanges.
- Use static asserts to provide better diagnostics on common mistakes:
    - Passing the wrong number of parameters
    - Instantiating a flexclass containing an undefined type
- Check if available features are enough to replace code in LLVM (User/Uses classes)
    - They are not. Need to support arrays behind the object.
- Documentation - review
- Implement `Optional` or `Maybe` as a short-cut for an array with 0 or 1 element.
    - Then it would be possible to add an example of creating a mixin system
    - Or maybe just a MultiVariant<A, B, C>, where existing objects could be just "A", "B", "C", "A B", "B C", "A C" or "A B C"
        - what's the application for that?
