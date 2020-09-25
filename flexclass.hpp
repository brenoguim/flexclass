// MIT License
// 
// Copyright (c) 2020 Breno Rodrigues Guimarães
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef FC_FLEXCLASS_FLEXCLASS_HPP
#define FC_FLEXCLASS_FLEXCLASS_HPP
#ifndef FC_FLEXCLASS_ALGORITHM_HPP
#define FC_FLEXCLASS_ALGORITHM_HPP
#include <initializer_list>
namespace fc
{
/*! Calls the destructor on all elements in the range
 *     from "begin" up to "end" in reverse order
 */
template <class T>
void reverseDestroy(T* begin, T* end)
{
    while (begin != end)
    {
        end--;
        end->~T();
    }
}
/*! Roll our own "std::max_element" with the "naive"
 * in the name to be clear that this is a custom implementation
 *
 * This was done to avoid including the <algorithm> header which
 * is very heavy
 */
template <class It>
constexpr It naiveMaxElement(It begin, It end)
{
    It max = end;
    It it = end;
    while (it != begin)
    {
        --it;
        if (max == end || *max < *it)
            max = it;
    }
    return max;
}
/*! Roll our own "std::mad" with the "naive"
 * in the name to be clear that this is a custom implementation
 *
 * This was done to avoid including the <algorithm> header which
 * is very heavy
 */
template <class T>
constexpr auto naiveMax(std::initializer_list<T> list)
{
    return *naiveMaxElement(list.begin(), list.end());
}
} // namespace fc
#endif
#ifndef FC_FLEXCLASS_ARRAYS_HPP
#define FC_FLEXCLASS_ARRAYS_HPP
#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP
#ifndef FLEXCLASS_MEMORY_HPP
#define FLEXCLASS_MEMORY_HPP
#include <cstdint>
#include <utility>
namespace fc
{
/* Default allocator that calls operator new/delete
 */
struct NewDeleteAllocator
{
    void* allocate(std::size_t sz) { return ::operator new(sz); }
    void deallocate(void* ptr) { ::operator delete(ptr); }
};
template <class T>
struct ArrayDeleter
{
    ArrayDeleter(T* begin) : m_begin(begin), m_end(begin) {}
    ~ArrayDeleter() { reverseDestroy(m_begin, m_end); }
    void setEnd(T* end) { m_end = end; }
    void release() { m_begin = m_end = nullptr; }
    T *m_begin{nullptr}, *m_end{nullptr};
};
constexpr std::uintptr_t findNextAlignedPosition(std::size_t pos, std::size_t desiredAlignment,
                                                 std::size_t currentAlignment = 1)
{
    return (pos - 1u + desiredAlignment) & -desiredAlignment;
}
template <class T>
constexpr std::uintptr_t findNextAlignedPosition(T* ptr, std::size_t desiredAlignment,
                                                 std::size_t currentAlignment = alignof(T))
{
    return findNextAlignedPosition(reinterpret_cast<std::uintptr_t>(ptr), desiredAlignment,
                                   currentAlignment);
}
template <class T, class U>
inline auto align(U* u)
{
    return reinterpret_cast<T*>(findNextAlignedPosition(u, alignof(T)));
}
template <class T>
struct aligner_impl
{
    aligner_impl& advance(std::size_t len)
    {
        ptr += len;
        return *this;
    };
    template <class U>
    auto cast()
    {
        return aligner_impl<U>{align<U>(ptr)};
    }
    template <class U>
    auto get()
    {
        return cast<U>().ptr;
    }
    T* ptr;
};
template <class T>
auto aligner(T* t)
{
    return aligner_impl<T>{t};
}
template <class T>
auto aligner(const T* t)
{
    return aligner_impl<T>{const_cast<T*>(t)};
}
template <class T>
auto aligner(T* t, std::size_t len)
{
    return aligner_impl<T>{t}.advance(len);
}
template <class T>
auto aligner(const T* t, std::size_t len)
{
    return aligner_impl<T>{const_cast<T*>(t)}.advance(len);
}
template <class T, class Deleter>
struct unique_ptr : private Deleter
{
    explicit unique_ptr(T* t) : m_t(t) {}
    template <class DeleterArg>
    unique_ptr(T* t, DeleterArg&& darg) : Deleter(std::forward<DeleterArg>(darg)), m_t(t)
    {
    }
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& other)
        : Deleter(std::move(other.get_deleter())), m_t(std::exchange(other.m_t, nullptr))
    {
    }
    unique_ptr& operator=(const unique_ptr&) = delete;
    unique_ptr& operator=(unique_ptr&& other)
    {
        using std::swap;
        swap(get_deleter(), other.get_deleter());
        swap(m_t, other.m_t);
        return *this;
    }
    ~unique_ptr()
    {
        if (m_t)
            get_deleter()(m_t);
    }
    void release() { m_t = nullptr; }
    Deleter& get_deleter() { return *this; }
    T* operator->() { return m_t; }
    T* operator->() const { return m_t; }
    T* get() { return m_t; }
    T* get() const { return m_t; }
    T* m_t;
};
/*! Two step deleter
 *  It manages a void* that is deallocated with the allocator.
 *  The user can then set "m_objectCreated" to true, and it will
 *  also call the destructor of such type.
 */
template <class T, class Alloc>
struct DeleteFn
{
    DeleteFn(Alloc& alloc) : m_alloc(&alloc) {}
    void operator()(void* ptr) const
    {
        if (m_objectCreated)
            static_cast<T*>(ptr)->~T();
        m_alloc->deallocate(ptr);
    }
    Alloc* m_alloc;
    bool m_objectCreated{false};
};
} // namespace fc
#endif
#ifndef FC_FLEXCLASS_TUPLE_HPP
#define FC_FLEXCLASS_TUPLE_HPP
#include <cstddef>
#include <cstdint>
namespace fc
{
/*! Placeholder type to indicate to the library that
 *  a type should just get default initialization
 */
struct Default
{
};
template <class... T>
struct List;
template <class A, class B>
struct concat_i;
template <class... A, class... B>
struct concat_i<List<A...>, List<B...>>
{
    using type = List<A..., B...>;
};
template <class A, class B>
using concat = typename concat_i<A, B>::type;
template <class... T>
static constexpr std::size_t maxAlign = naiveMax({alignof(T)...});
template <class T>
static constexpr auto CSizeOf = std::is_empty_v<T> ? 0 : sizeof(T);
template <std::size_t Pos, class List>
struct InsertPadding;
template <std::size_t Pos, class T>
struct Item
{
    static constexpr auto pos = Pos;
    using type = T;
};
template <std::size_t Pos, class Head, class... Tail>
struct InsertPadding<Pos, List<Head, Tail...>>
{
    static constexpr auto RoundedPos = findNextAlignedPosition(Pos, alignof(Head));
    using SubType = InsertPadding<RoundedPos + CSizeOf<Head>, List<Tail...>>;
    using type = concat<List<Item<RoundedPos, Head>>, typename SubType::type>;
    static constexpr auto NumBytes = SubType::NumBytes;
    static constexpr auto Alignment = maxAlign<Head, Tail...>;
    static constexpr auto NumElements = sizeof...(Tail) + 1;
};
template <std::size_t Pos, class Head>
struct InsertPadding<Pos, List<Head>>
{
    static constexpr auto RoundedPos = findNextAlignedPosition(Pos, alignof(Head));
    using type = List<Item<RoundedPos, Head>>;
    static constexpr auto NumBytes = RoundedPos + CSizeOf<Head>;
    static constexpr auto Alignment = alignof(Head);
    static constexpr std::size_t NumElements = 1;
};
template <std::size_t Pos>
struct InsertPadding<Pos, List<>>
{
    using type = List<>;
    static constexpr auto NumBytes = 0;
    static constexpr auto Alignment = 1;
    static constexpr std::size_t NumElements = 0;
};
template <class... T>
using WithPadding = InsertPadding<0, List<T...>>;
template <class L>
struct TupleBuilder;
template <class Head, class... Tail>
struct TupleBuilder<List<Head, Tail...>> : public TupleBuilder<List<Tail...>>
{
    using Base = TupleBuilder<List<Tail...>>;
    using TheType = typename Head::type;
    template <std::size_t id, class Arg1, class... Args>
    static void build(void* buf, std::size_t& count, Arg1&& arg1, Args&&... args)
    {
        if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<Arg1>>, Default>)
            ::new (obj(buf)) TheType;
        else
            ::new (obj(buf)) TheType(std::forward<Arg1>(arg1));
        count = id + 1;
        if constexpr (sizeof...(Tail) > 0)
            Base::template build<id + 1>(buf, count, std::forward<Args>(args)...);
    }
    template <std::size_t id>
    static void build(void* buf, std::size_t& count)
    {
        ::new (obj(buf)) TheType;
        count = id + 1;
        if constexpr (sizeof...(Tail) > 0)
            Base::template build<id + 1>(buf, count);
    }
    template <std::size_t id>
    static void destroy(void* buf, std::size_t count) noexcept
    {
        if constexpr (sizeof...(Tail) > 0)
            Base::template destroy<id + 1>(buf, count);
        if (count > id)
            obj(buf)->~TheType();
    }
    template <std::size_t id>
    static auto& get(void* buf) noexcept
    {
        if constexpr (id == 0)
            return *obj(buf);
        else if constexpr (sizeof...(Tail) > 0)
            return Base::template get<id - 1>(buf);
    }
    static auto obj(void* buf)
    {
        return static_cast<TheType*>(static_cast<void*>(static_cast<std::byte*>(buf) + Head::pos));
    }
};
template <>
struct TupleBuilder<List<>>
{
};
template <class... T>
struct tuple
{
    using P = WithPadding<T...>;
    using TP = TupleBuilder<typename P::type>;
    static constexpr auto Size = sizeof...(T);
    template <class... Args>
    tuple(Args&&... args)
    {
        if constexpr (Size > 0)
        {
            std::size_t count = 0;
            try
            {
                TP::template build<0>(&m_data, count, std::forward<Args>(args)...);
            }
            catch (...)
            {
                TP::template destroy<0>(&m_data, count);
                throw;
            }
        }
    }
    tuple()
    {
        if constexpr (Size > 0)
        {
            std::size_t count = 0;
            try
            {
                TP::template build<0>(&m_data, count);
            }
            catch (...)
            {
                TP::template destroy<0>(&m_data, count);
                throw;
            }
        }
    }
    ~tuple()
    {
        if constexpr (Size > 0)
            TP::template destroy<0>(&m_data, P::NumElements);
    }
    template <std::size_t i>
    auto& get()
    {
        return TP::template get<i>(&m_data);
    }
    template <std::size_t i>
    const auto& get() const
    {
        return const_cast<tuple*>(this)->get<i>();
    }
    std::aligned_storage_t<P::NumBytes, P::Alignment> m_data;
};
template <int i, class... T>
auto& get_element(tuple<T...>& t)
{
    return t.template get<i>();
}
template <int i, class... T>
auto& get_element(const tuple<T...>& t)
{
    return t.template get<i>();
}
namespace detail
{
template <int Idx, class T, class Fn>
void callWithIdx(const T& t, Fn&& f)
{
    f(get_element<Idx>(t), std::integral_constant<int, Idx>());
}
template <int Idx, class T, class Fn>
void callWithIdx(T& t, Fn&& f)
{
    f(get_element<Idx>(t), std::integral_constant<int, Idx>());
}
template <typename T, typename F, int... Is>
void for_each(T&& t, F&& f, std::integer_sequence<int, Is...>)
{
    if constexpr (std::remove_reference_t<T>::Size > 0)
        auto l = {(callWithIdx<Is>(t, f), 0)...};
}
template <int I, int N, typename T1, typename T2, typename F>
void for_each_zipped(T1&& t1, T2&& t2, F&& f)
{
    if constexpr (I != N)
    {
        f(get_element<I>(t1), get_element<I>(t2));
        for_each_zipped<I + 1, N, T1, T2, F>(t1, t2, f);
    }
}
} // namespace detail
template <typename... Ts, typename F>
void for_each_in_tuple(fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template <int... Is>
constexpr auto reverseIntegerSequence(std::integer_sequence<int, Is...> const&)
{
    return std::integer_sequence<int, sizeof...(Is) - 1 - Is...>{};
}
template <typename... Ts, typename F>
void reverse_for_each_in_tuple(const fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f,
                     reverseIntegerSequence(std::make_integer_sequence<int, sizeof...(Ts)>()));
}
template <typename... Ts, typename F>
void for_each_in_tuple(const fc::tuple<Ts...>& t, F&& f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template <int I, typename T1, typename T2, typename F>
void for_each_zipped(T1& t1, T2& t2, F f)
{
    detail::for_each_zipped<0, I>(t1, t2, f);
}
} // namespace fc
#endif // FC_FLEXCLASS_TUPLE_HPP
#ifndef FLEXCLASS_UTILITY_HPP
#define FLEXCLASS_UTILITY_HPP
namespace fc
{
template <std::size_t Idx, class Arg1, class... Args>
decltype(auto) pickFromPack(Arg1&& arg1, Args&&... args)
{
    if constexpr (Idx == 0)
        return std::forward<Arg1>(arg1);
    else
        return pickFromPack<Idx - 1>(std::forward<Args>(args)...);
}
//! Utility type to ease SFINAE
template <class T>
struct void_
{
    using type = void;
};
// Helper type that will be constructed from any input, but will
// ignore it
struct Ignore
{
    Ignore() = default;
    template <class T>
    Ignore(T&&)
    {
    }
};
//! Removes all references, const and volatile from T
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
} // namespace fc
#endif
#include <cassert>
#include <new>
#include <type_traits>
namespace fc
{
/*! Base class for a Handle
 *  Defines useful types the library uses
 *  to recognize and operate with the handle
 */
template <class T>
struct Handle
{
    /*! Workaround to allow the handle to be created
     *  from the argument that was passed from the user
     *  The argument (likely the array size) will be
     *  interpreted by the library, which will create
     *  an array and later call "setLocation" on
     *  the handle.
     *  This is why this input is discarded
     */
    template <class U>
    Handle(U&&)
    {
    }
    //! If fc_handle_type is defined, then this is handle
    using fc_handle_type = T;
};
namespace detail
{
//! Placeholder to indicate no iterator was passed by the user
struct NoIterator
{
};
} // namespace detail
/*! internal
 * Class used to store the arguments for an array creation
 * It stores both the array size and an inputiterator that
 * will provide the initial values for the elements
 * If the iterator is the "fc::detail::NoIterator" class, then elements
 * are default initialized
 */
template <class InputIt>
struct Arg
{
    Arg(std::size_t size) : m_size(size) {}
    Arg(std::size_t size, InputIt it) : m_size(size), m_it(it) {}
    std::size_t m_size;
    InputIt m_it;
};
//! Use this as argument for creating an array
inline auto arg(std::size_t size) { return Arg<detail::NoIterator>{size}; }
//! Use this as argument for creating an array with an initial value
//  The input iterator will be called for each element
template <class InputIt>
auto arg(std::size_t size, InputIt it)
{
    return Arg<InputIt>{size, it};
}
/*! Placeholder type and values to call ::make to indicate the first
 * argument is an allocator
 */
struct WithAllocator
{
};
static constexpr WithAllocator withAllocator;
/*! internal
 *
 * An ArrayBuilder is responsible for two steps of the process
 * 1 - It is first queried for how many bytes it will need to the
 * array it is supposed to build.
 *
 * 2 - It is provided with a buffer and it should write initialize
 * the array in there. The buffer is guaranteed to fit all data.
 *
 * Exceptions are treated with care. If at any point, an exception
 * is thrown, then all objects already created must be destroyed.
 */
template <class T>
struct ArrayBuilder
{
    //! Destroys all elements in case they are still being tracked
    ~ArrayBuilder()
    {
        if (m_begin)
            reverseDestroy(m_begin, m_end);
    }
    //! Creates the array of size "sz" in the given buffer.
    auto buildArray(std::byte* buf, std::size_t sz)
    {
        return buildArray(buf, Arg<detail::NoIterator>{sz});
    }
    //! Creates the array with inputs specified by Arg in the given buffer.
    template <class InputIt>
    auto buildArray(std::byte* buf, Arg<InputIt>&& arg)
    {
        return buildArray(buf, arg);
    }
    //! Creates the array with inputs specified by Arg in the given buffer.
    template <class InputIt>
    auto buildArray(std::byte* buf, Arg<InputIt>& arg)
    {
        // Find the fist aligned byte suitable for creating T objects
        auto b = aligner(buf).get<T>();
        auto e = b + arg.m_size;
        // In case of an exception, ArrayDeleter will make sure
        //  all objects created up to the point are destroyed
        //  in reverse order
        ArrayDeleter<T> deleter(b);
        for (auto it = b; it != e;)
        {
            // Special handling for default initialization
            if constexpr (std::is_same_v<InputIt, detail::NoIterator>)
                new (it) T;
            else
                new (it) T(*arg.m_it++);
            // Tell the ArrayDeleter another element was createdgc
            deleter.setEnd(++it);
        }
        // No exception was thrown, let the array deleter stop tracking
        // and the ArrayBuilder will track objects from now on
        deleter.release();
        m_begin = b;
        m_end = e;
        return reinterpret_cast<std::byte*>(e);
    }
    //! Query for the number of bytes necessary to create an T array of size
    //! "sz"
    // "offset" is the offset in an imaginary array starting from 0
    static std::size_t numRequiredBytes(std::size_t offset, std::size_t sz)
    {
        return numRequiredBytes(offset, Arg<detail::NoIterator>{sz});
    }
    //! Query for the number of bytes necessary to create an T array of size
    //! "sz"
    // "offset" is the offset in an imaginary array starting from 0
    template <class InputIt>
    static std::size_t numRequiredBytes(std::size_t offset, const Arg<InputIt>& arg)
    {
        auto numBytes = arg.m_size * sizeof(T);
        auto newOffset = findNextAlignedPosition(offset, alignof(T));
        newOffset += numBytes;
        return newOffset - offset;
    }
    //! Let the array builder stop tracking the array
    void release() { m_begin = m_end = nullptr; }
    T* m_begin{nullptr};
    T* m_end{nullptr};
};
//! Transforms type T into a suitable handle. Used mainly to convert
// T[] to Array<T> or Range<T>
template <class T>
struct ArraySelector
{
    using type = T;
};
//! Trait to check if a given type is an ArrayBuilder
template <class T>
struct isArrayBuilder : std::false_type
{
};
template <class T>
struct isArrayBuilder<ArrayBuilder<T>> : std::true_type
{
};
template <class T, class = void>
struct isHandle : std::false_type
{
};
template <class T>
struct isHandle<T, typename void_<typename T::fc_handle_type>::type> : std::true_type
{
    using enable = T;
};
/*! Finds handle types in the types passed by the user
 * and convert them either to "Ignore" or "ArrayBuilder"
 * TODO: It should use ArraySelector instead of hard coded
 * T[]
 */
template <class T, class = void>
struct ArrayBuildersConverter
{
    using type = Ignore;
};
template <class T>
struct ArrayBuildersConverter<T[], void>
{
    using type = ArrayBuilder<T>;
};
template <class T>
struct ArrayBuildersConverter<T, typename void_<typename isHandle<T>::enable>::type>
{
    using type = ArrayBuilder<typename T::fc_handle_type>;
};
/*! Finds handle types in the types passed by the user
 * and query them for the alignment of T
 * TODO: It should use ArraySelector instead of hard coded
 * T[]
 */
template <class T, class = void>
struct GetAlignmentRequirement
{
    static constexpr auto value = alignof(T);
};
template <class T>
struct GetAlignmentRequirement<T[], void>
{
    static constexpr auto value = alignof(T);
};
template <class T>
struct GetAlignmentRequirement<T, typename void_<typename isHandle<T>::enable>::type>
{
    static constexpr std::size_t value = alignof(typename T::fc_handle_type);
};
template <class... Types>
struct CollectAlignment
{
    static constexpr auto value =
        naiveMax({std::size_t(1), GetAlignmentRequirement<Types>::value...});
};
template <class Derived, class... T>
class alignas(CollectAlignment<T...>::value) FlexibleBase
    : public fc::tuple<typename ArraySelector<T>::type...>
{
  private:
    using Base = fc::tuple<typename ArraySelector<T>::type...>;
    using Base::Base;
  protected:
    using FLB = FlexibleBase;
    ~FlexibleBase() = default;
  public:
    static constexpr auto numMembers() { return Base::Size; }
    template <auto e>
    decltype(auto) get()
    {
        return get_element<e>(*this);
    }
    template <auto e>
    decltype(auto) get() const
    {
        return get_element<e>(*this);
    }
    template <auto e>
    decltype(auto) begin() const
    {
        return get_element<e>(*this).begin(this);
    }
    template <auto e>
    decltype(auto) end() const
    {
        return get_element<e>(*this).end(this);
    }
    template <auto e>
    decltype(auto) begin()
    {
        return get_element<e>(*this).begin(this);
    }
    template <auto e>
    decltype(auto) end()
    {
        return get_element<e>(*this).end(this);
    }
    struct DestroyFn
    {
        void operator()(Derived* ptr) const { Derived::destroy(ptr); }
    };
    using UniquePtr = unique_ptr<Derived, DestroyFn>;
    template <class... Args>
    static auto make_unique(Args&&... args)
    {
        return UniquePtr(make(std::forward<Args>(args)...));
    }
    template <class... Args>
    static auto make(Args&&... args)
    {
        NewDeleteAllocator alloc;
        return makeWithAllocator(alloc, std::forward<Args>(args)...);
    }
    template <class Alloc, class... Args>
    static auto make(WithAllocator, Alloc& alloc, Args&&... args)
    {
        return makeWithAllocator(alloc, std::forward<Args>(args)...);
    }
    template <class Alloc, class... Args>
    static auto makeWithAllocator(Alloc& alloc, Args&&... args)
    {
        using ArrayBuilders = fc::tuple<typename ArrayBuildersConverter<T>::type...>;
        std::size_t numBytesForArrays = 0;
        {
            for_each_in_tuple(ArrayBuilders(), [&](const auto& element, auto idx) mutable {
                using Element = remove_cvref_t<decltype(element)>;
                using Idx = decltype(idx);
                if constexpr (isArrayBuilder<Element>::value)
                {
                    numBytesForArrays += element.numRequiredBytes(
                        sizeof(Derived) + numBytesForArrays, pickFromPack<Idx::value>(args...));
                }
            });
        }
        auto memBuffer = unique_ptr<void, DeleteFn<Derived, Alloc>>(
            alloc.allocate(sizeof(Derived) + numBytesForArrays), alloc);
        auto ret = new (memBuffer.get()) Derived(std::forward<Args>(args)...);
        memBuffer.get_deleter().m_objectCreated = true;
        // Start creating arrays right after the Derived object
        std::byte* arrayBuffer = reinterpret_cast<std::byte*>(ret + 1);
        ArrayBuilders arrayBuilders;
        for_each_in_tuple(arrayBuilders, [&](auto& element, auto idx) mutable {
            using Element = remove_cvref_t<decltype(element)>;
            using Idx = decltype(idx);
            if constexpr (isArrayBuilder<Element>::value)
            {
                arrayBuffer = element.buildArray(
                    arrayBuffer, pickFromPack<Idx::value>(std::forward<Args>(args)...));
            }
        });
        for_each_zipped<sizeof...(T)>(*ret, arrayBuilders, [](auto& handle, auto& arrayBuilder) {
            using ArrBuildersElement = remove_cvref_t<decltype(arrayBuilder)>;
            if constexpr (isArrayBuilder<ArrBuildersElement>::value)
            {
                handle.setLocation(arrayBuilder.m_begin, arrayBuilder.m_end);
                arrayBuilder.release();
            }
        });
        memBuffer.release();
        return ret;
    }
    static void destroy(const Derived* p)
    {
        NewDeleteAllocator alloc;
        destroyWithAllocator(p, alloc);
    }
    template <class Alloc>
    static void destroy(const Derived* p, Alloc&& alloc)
    {
        destroyWithAllocator(p, alloc);
    }
    template <class Alloc>
    static void destroyWithAllocator(const Derived* p, Alloc&& alloc)
    {
        if (!p)
            return;
        reverse_for_each_in_tuple(*p, [p](auto& u, auto idx) {
            using U = remove_cvref_t<decltype(u)>;
            if constexpr (isHandle<U>::value)
                if constexpr (!std::is_trivially_destructible<typename U::fc_handle_type>::value)
                {
                    reverseDestroy(u.begin(p), u.end(p));
                }
        });
        p->~Derived();
        alloc.deallocate(const_cast<Derived*>(p));
    }
};
template <class T>
void destroy(const T* p)
{
    T::destroy(p);
}
template <class T, class Alloc>
void destroy(const T* p, Alloc&& alloc)
{
    T::destroy(p, alloc);
}
template <class... Args>
struct FlexibleClass : public FlexibleBase<FlexibleClass<Args...>, Args...>
{
    using FlexibleBase<FlexibleClass<Args...>, Args...>::FlexibleBase;
};
} // namespace fc
#endif // FC_FLEXCLASS_CORE_HPP
/*! Contains builtin handle implementations for common applications
 *
 * A handle must derive from fc::Handle<T> and define the following
 * methods:
 *
 * // Called by the library to let the handle know where T objects
 * // were created
 *
 * void setLocation(T* begin, T* end);
 *
 * // Called by the library when the user requests the begin of the
 * // object sequence
 * // The base is passed as a parameter for handles that need to
 * // query it
 *
 * template <class Base>
 * auto begin(const Base*) const -> T*;
 *
 * [optional]
 * // Called by the library when the user requests the end of the
 * // object sequence
 * // The base is passed as a parameter for handles that need to
 * // query it
 *
 * template <class Base>
 * auto end(const Base*) const -> T*;
 *
 */
namespace fc
{
/*! Uses a pointer to store the location of the first T
 *  in the sequence.
 *  Does not know the size of the array.
 */
template <class T>
struct Array : Handle<T>
{
    using Handle<T>::Handle;
    void setLocation(T* begin, T* end) { m_begin = begin; }
    template <class Base>
    auto begin(const Base* ptr) const
    {
        return m_begin;
    }
    auto begin() const { return m_begin; }
    T* m_begin;
};
/*! Uses two pointers to store the location of the first T
 *  and last T of the sequence.
 */
template <class T>
struct Range : Handle<T>
{
    using Handle<T>::Handle;
    void setLocation(T* begin, T* end)
    {
        m_begin = begin;
        m_end = end;
    }
    template <class Base>
    auto begin(const Base* ptr) const
    {
        return m_begin;
    }
    template <class Base>
    auto end(const Base* ptr) const
    {
        return m_end;
    }
    auto begin() const { return m_begin; }
    auto end() const { return m_end; }
    T* m_begin;
    T* m_end;
};
/*! Uses another handle index to derivate the
 *  sequence position.
 *
 *  If El == -1, then it assumes the begin is
 *  adjacent to the Base.
 *
 *  Otherwise it takes the end() of the El handle
 *  and assumes that is where the T array begins
 */
template <class T, int El = -1>
struct AdjacentArray : Handle<T>
{
    using Handle<T>::Handle;
    void setLocation(T* begin, T* end) {}
    template <class Base>
    auto begin(const Base* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
};
/*! Uses another handle index to derivate the
 *  sequence position.
 *
 *  If El == -1, then it assumes the begin is
 *  adjacent to the Base.
 *
 *  Otherwise it takes the end() of the El handle
 *  and assumes that is where the T array begins
 *
 *  Uses an extra T* to store the end of the array
 */
template <class T, int El = -1>
struct AdjacentRange : Handle<T>
{
    using Handle<T>::Handle;
    void setLocation(T* begin, T* end) { m_end = end; }
    template <class Base>
    auto begin(const Base* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
    template <class Base>
    auto end(const Base* ptr) const
    {
        return m_end;
    }
    T* m_end;
};
/*! Customization point to convert a T[] into a proper handle
 */
template <class T>
struct ArraySelector<T[]>
{
    using type = std::conditional_t<std::is_trivially_destructible<T>::value, Array<T>, Range<T>>;
};
} // namespace fc
#endif // FC_FLEXCLASS_ARRAYS_HPP
#endif // FC_FLEXCLASS_FLEXCLASS_HPP
