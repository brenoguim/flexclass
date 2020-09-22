// MIT License
// 
// Copyright (c) 2020 Breno Rodrigues Guimar;es
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
#ifndef FC_FLEXCLASS_TUPLE_HPP
#define FC_FLEXCLASS_TUPLE_HPP
#include <cstdint>
#include <algorithm>
#include <memory>
namespace fc
{
constexpr std::size_t findNextAlignedPosition(std::size_t pos, std::size_t desiredAlignment)
{
    return (pos - 1u + desiredAlignment) & -desiredAlignment;
}
template<class... T> struct List;
template<class A, class B> struct concat_i;
template<class... A, class... B> struct concat_i<List<A...>, List<B...>>
{
    using type = List<A..., B...>;
};
template<class A, class B> using concat = typename concat_i<A, B>::type;
template<class... T>
static constexpr std::size_t maxAlign = std::max({std::size_t(1), alignof(T)...});
template<class T>
static constexpr auto CSizeOf = std::is_empty_v<T> ? 0 : sizeof(T);
template<std::size_t Pos, class List> struct InsertPadding;
template<std::size_t Pos, class T> struct Item
{
    static constexpr auto pos = Pos;
    using type = T;
};
template<std::size_t Pos, class Head, class... Tail>
struct InsertPadding<Pos, List<Head, Tail...>>
{
    static constexpr auto RoundedPos = findNextAlignedPosition(Pos, alignof(Head));
    using SubType = InsertPadding<RoundedPos + CSizeOf<Head>
                                 , List<Tail...>
                                 >;
    using type = concat<List<Item<RoundedPos, Head>>,
                        typename SubType::type
                        >;
    static constexpr auto NumBytes = SubType::NumBytes;
    static constexpr auto Alignment = maxAlign<Head, Tail...>;
    static constexpr auto NumElements = sizeof...(Tail) + 1;
};
template<std::size_t Pos, class Head>
struct InsertPadding<Pos, List<Head>>
{
    static constexpr auto RoundedPos = findNextAlignedPosition(Pos, alignof(Head));
    using type = List<Item<RoundedPos, Head>>;
    static constexpr auto NumBytes = RoundedPos + CSizeOf<Head>;
    static constexpr auto Alignment = alignof(Head);
    static constexpr std::size_t NumElements = 1;
};
template<std::size_t Pos>
struct InsertPadding<Pos, List<>>
{
    using type = List<>;
    static constexpr auto NumBytes = 0;
    static constexpr auto Alignment = 1;
    static constexpr std::size_t NumElements = 0;
};
template<class... T>
using WithPadding = InsertPadding<0, List<T...>>;
template<class L> struct TupleBuilder;
template<class Head, class... Tail>
struct TupleBuilder<List<Head, Tail...>> : public TupleBuilder<List<Tail...>>
{
    using Base = TupleBuilder<List<Tail...>>;
    using TheType = typename Head::type;
    template<std::size_t id, class Arg1, class... Args>
    static void build(void* buf, std::size_t& count, Arg1&& arg1, Args&&... args)
    {
        ::new (obj(buf)) TheType(std::forward<Arg1>(arg1));
        count = id + 1;
        if constexpr (sizeof...(Tail) > 0)
            Base::template build<id+1>(buf, count, std::forward<Args>(args)...);
    }
    template<std::size_t id>
    static void destroy(void* buf, std::size_t count) noexcept
    {
        if constexpr (sizeof...(Tail) > 0)
            Base::template destroy<id+1>(buf, count);
        if (count > id)
            obj(buf)->~TheType();
    }
    template<std::size_t id>
    static auto& get(void* buf) noexcept
    {
        if constexpr (id == 0)
            return *obj(buf);
        else if constexpr (sizeof...(Tail) > 0)
            return Base::template get<id-1>(buf);
    }
    static auto obj(void* buf) { return static_cast<TheType*>(static_cast<void*>(static_cast<char*>(buf) + Head::pos)); }
};
template<> struct TupleBuilder<List<>> {};
template<class... T>
struct tuple
{
    using P = WithPadding<T...>;
    using TP = TupleBuilder<typename P::type>;
    static constexpr auto Size = sizeof...(T);
    template<class... Args>
    tuple(Args&&... args)
    {
        if constexpr (Size > 0)
        {
            std::size_t count = 0;
            try
            {
                TP::template build<0>(&m_data, count,
                    std::forward<Args>(args)...);
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
    template<std::size_t i> auto& get() { return TP::template get<i>(&m_data); }
    template<std::size_t i> const auto& get() const { return const_cast<tuple*>(this)->get<i>(); }
    std::aligned_storage_t<P::NumBytes, P::Alignment> m_data;
};
template<int i, class...T>
auto& get_element(tuple<T...>& t) { return t.template get<i>(); }
template<int i, class...T>
auto& get_element(const tuple<T...>& t) { return t.template get<i>(); }
}
#endif // FC_FLEXCLASS_TUPLE_HPP
#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP
#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
namespace fc
{
namespace handle
{
struct range {};
struct array {};
}
template<class T>
void reverse_destroy(T* b, T* e)
{
    if (b != e)
    {
        while (1)
        {
            e--;
            e->~T();
            if (e == b) break;
        }
    }
}
template<class T>
struct ArrayDeleter
{
    ArrayDeleter(T* begin) : m_begin(begin), m_end(begin) {}
    ~ArrayDeleter()
    {
        reverse_destroy(m_begin, m_end);
    }
    void setEnd(T* end) { m_end = end; }
    void release() { m_begin = m_end = nullptr; }
    T *m_begin {nullptr}, *m_end {nullptr};
};
inline void* incr(void* in, std::size_t len) { return static_cast<char*>(in) + len; }
template<class T>
inline auto align(void* ptr)
{
    std::size_t space = std::numeric_limits<std::size_t>::max();
    return static_cast<T*>(std::align(alignof(T), 0, ptr, space));
}
template<class T>
struct aligner_impl
{
    aligner_impl& advance(std::size_t len) { ptr += len; return *this; };
    template<class U> auto cast() { return aligner_impl<U>{align<U>(ptr)}; }
    template<class U> auto get() { return cast<U>().ptr; }
    T* ptr;
};
template<class T> auto aligner(T* t) { return aligner_impl<T>{t}; }
template<class T> auto aligner(const T* t) { return aligner_impl<T>{const_cast<T*>(t)}; }
template<class T> auto aligner(T* t, std::size_t len) { return aligner_impl<T>{t}.advance(len); }
template<class T> auto aligner(const T* t, std::size_t len) { return aligner_impl<T>{const_cast<T*>(t)}.advance(len); }
template<class T>
struct ArrayBuilder
{
    ArrayBuilder(std::size_t size) : m_size(size) {}
    ~ArrayBuilder()
    {
        if (m_ptr) reverse_destroy(begin(), end());
    }
    void consume(void*& buf, std::size_t& space)
    {
        auto ptr = std::align(alignof(T), numBytes(), buf, space);
        assert(ptr);
        space -= numBytes();
        buf = incr(buf, numBytes());
        auto b = static_cast<T*>(ptr);
        auto e = b + m_size;
        ArrayDeleter<T> deleter(b);
        while (b != e)
        {
            new (b) T;
            deleter.setEnd(++b);
        }
        deleter.release();
        m_ptr = ptr;
    }
    std::size_t numRequiredBytes(std::size_t offset)
    {
        std::size_t space = std::numeric_limits<std::size_t>::max();
        auto originalSpace = space;
        void* ptr = static_cast<char*>(nullptr) + offset;
        auto r = std::align(alignof(T), numBytes(), ptr, space);
        assert(r);
        return (originalSpace - space) + numBytes();
    }
    auto numBytes() const { return m_size*sizeof(T); }
    T* begin() const { return static_cast<T*>(m_ptr); }
    T* end() const { return begin() + m_size; }
    void release() { m_ptr = nullptr; }
    std::size_t m_size;
    void* m_ptr {nullptr};
};
template<class T> struct ArraySelector { using type = T; };
template<class T> struct is_array_placeholder : std::false_type {};
template<class T> struct is_array_placeholder<ArrayBuilder<T>> : std::true_type {};
template<class T> struct void_ { using type = void; };
template<class T, class = void> struct is_fc_array : std::false_type {};
template<class T> struct is_fc_array<T, typename void_<typename T::fc_handle>::type> : std::true_type
{
    using enable = T;
};
struct Ignore { Ignore() = default; template<class T> Ignore(T&&) {} };
template<class T, class = void> struct PreImplConverter { using type = Ignore; };
template<class T> struct PreImplConverter<T[], void> { using type = ArrayBuilder<T>; };
template<class T> struct PreImplConverter<T, typename void_<typename is_fc_array<T>::enable>::type>
{ using type = ArrayBuilder<typename T::type>; };
namespace detail
{
    template<typename T, typename F, int... Is>
    void for_each(T&& t, F f, std::integer_sequence<int, Is...>)
    {
        if constexpr (std::remove_reference_t<T>::Size > 0)
            auto l = { (f(get_element<Is>(t)), 0)... };
    }
    template<int I, int N, typename T1, typename T2, typename F>
    void for_each_zipped(T1&& t1, T2&& t2, F f)
    {
        if constexpr (I != N)
        {
            f(get_element<I>(t1), get_element<I>(t2));
            for_each_zipped<I+1, N, T1, T2, F>(t1, t2, f);
        }
    }
}
template<typename... Ts, typename F>
void for_each_in_tuple(fc::tuple<Ts...>& t, F f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template <int ... Is>
constexpr auto reverseIntegerSequence(std::integer_sequence<int, Is...> const &)
{ return std::integer_sequence<int, sizeof...(Is)-1-Is...>{}; }
template<typename... Ts, typename F>
void reverse_for_each_in_tuple(const fc::tuple<Ts...>& t, F f)
{
    detail::for_each(t, f, reverseIntegerSequence(std::make_integer_sequence<int, sizeof...(Ts)>()));
}
template<typename... Ts, typename F>
void for_each_in_tuple(const fc::tuple<Ts...>& t, F f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}
template<int I, typename T1, typename T2, typename F>
void for_each_zipped(T1& t1, T2& t2, F f)
{
    detail::for_each_zipped<0, I>(t1, t2, f);
}
template<class T, class = void> struct GetAlignmentRequirement { static constexpr auto value = alignof(T); };
template<class T> struct GetAlignmentRequirement<T[], void> { static constexpr auto value = alignof(T); };
template<class T> struct GetAlignmentRequirement<T, typename void_<typename is_fc_array<T>::enable>::type>
{ static constexpr std::size_t value = T::array_alignment; };
template<class... Types>
struct CollectAlignment
{
    static constexpr auto value = std::max({std::size_t(1), GetAlignmentRequirement<Types>::value...});
};
struct DeleteFn { void operator()(void* ptr) const { ::operator delete(ptr); } };
template<class Derived, class... T>
class alignas(CollectAlignment<T...>::value) FlexibleBase : public fc::tuple<typename ArraySelector<T>::type...>
{
  private:
    using Base = fc::tuple<typename ArraySelector<T>::type...>;
    using Base::Base;
  protected:
    using FLB = FlexibleBase;
    ~FlexibleBase() = default;
  public:
    static constexpr auto numMembers() { return Base::Size; }
    template<auto e> decltype(auto) get() { return get_element<e>(*this); }
    template<auto e> decltype(auto) get() const { return get_element<e>(*this); }
    template<auto e> decltype(auto) begin() const { return get_element<e>(*this).begin(this); }
    template<auto e> decltype(auto) end() const { return get_element<e>(*this).end(this); }
    template<auto e> decltype(auto) begin() { return get_element<e>(*this).begin(this); }
    template<auto e> decltype(auto) end() { return get_element<e>(*this).end(this); }
    struct DestroyFn { void operator()(Derived* ptr) const { Derived::destroy(ptr); } };
    using UniquePtr = std::unique_ptr<Derived, DestroyFn>;
    template<class... Args>
    static auto make_unique(Args&&... args)
    {
        return UniquePtr(make(std::forward<Args>(args)...));
    }
    template<class... Args>
    static auto make(Args&&... args)
    {
        static_assert(sizeof(Derived) == sizeof(FlexibleBase));
        using PreImpl = fc::tuple<typename PreImplConverter<T>::type...>;
        std::size_t numBytesForArrays = 0;
        {
            PreImpl pi(args...);
            for_each_in_tuple(pi,
                [&numBytesForArrays]<class U>(U& u) mutable {
                    if constexpr (is_array_placeholder<U>::value)
                        numBytesForArrays += u.numRequiredBytes(sizeof(FlexibleBase) + numBytesForArrays);
                });
        }
        auto implBuffer = std::unique_ptr<void, DeleteFn>(::operator new(sizeof(FlexibleBase) + numBytesForArrays));
        void* arrayBuffer = static_cast<char*>(implBuffer.get()) + sizeof(FlexibleBase);
        PreImpl pi(args...);
        for_each_in_tuple(pi,
            [&]<class U>(U& u) mutable {
                if constexpr (is_array_placeholder<U>::value)
                    u.consume(arrayBuffer, numBytesForArrays);
            });
        auto ret = new (implBuffer.get()) Derived(std::forward<Args>(args)...);
        for_each_zipped<sizeof...(T)>(*ret, pi,
            []<class U, class K>(U& u, K& k) {
                if constexpr (is_array_placeholder<K>::value)
                {
                    u.setLocation(k.begin(), k.end());
                    k.release();
                }
            });
        implBuffer.release();
        return ret;
    }
    static void destroy(const Derived* p)
    {
        if (!p) return;
        reverse_for_each_in_tuple(*p,
            [p]<class U>(U& u) {
                if constexpr (is_fc_array<std::remove_cv_t<U>>::value)
                if constexpr (!std::is_trivially_destructible<typename U::type>::value)
                {
                    reverse_destroy(u.begin(p), u.end(p));
                }
            });
        p->~Derived();
        ::operator delete(const_cast<Derived*>(p));
    }
};
template<class T> void destroy(const T* p) { T::destroy(p); }
template<class... Args>
struct FlexibleClass : public FlexibleBase<FlexibleClass<Args...>, Args...>
{
    using FlexibleBase<FlexibleClass<Args...>, Args...>::FlexibleBase;
};
}
#endif // FC_FLEXCLASS_CORE_HPP
#ifndef FC_FLEXCLASS_ARRAYS_HPP
#define FC_FLEXCLASS_ARRAYS_HPP
namespace fc
{
template<class T> struct Array
{
    template<class U> Array(U&&) {}
    void setLocation(T* begin, T* end) { m_begin = begin; }
    using type = T;
    using fc_handle = handle::array;
    static constexpr auto array_alignment = alignof(T);
    template<class Derived>
    auto begin(const Derived* ptr) const { return m_begin; }
    auto begin() const { return m_begin; }
    T* m_begin;
};
template<class T> struct Range
{
    template<class U> Range(U&&) {}
    void setLocation(T* begin, T* end) { m_begin = begin; m_end = end; }
    using type = T;
    using fc_handle = handle::range;
    static constexpr auto array_alignment = alignof(T);
    template<class Derived>
    auto begin(const Derived* ptr) const { return m_begin; }
    template<class Derived>
    auto end(const Derived* ptr) const { return m_end; }
    auto begin() const { return m_begin; }
    auto end() const { return m_end; }
    T* m_begin;
    T* m_end;
};
template<class T, int El = -1> struct AdjacentArray
{
    template<class U> AdjacentArray(U&&) {}
    void setLocation(T* begin, T* end) {}
    using type = T;
    using fc_handle = handle::array;
    static constexpr auto array_alignment = alignof(T);
    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
};
template<class T, int El = -1> struct AdjacentRange
{
    template<class U> AdjacentRange(U&&) {}
    void setLocation(T* begin, T* end) { m_end = end; }
    using type = T;
    using fc_handle = handle::range;
    static constexpr auto array_alignment = alignof(T);
    template<class Derived>
    auto begin(const Derived* ptr) const
    {
        if constexpr (El == -1)
            return aligner(ptr, 1).template get<T>();
        else
            return aligner(ptr->template end<El>()).template get<T>();
    }
    template<class Derived>
    auto end(const Derived* ptr) const { return m_end; }
    T* m_end;
};
template<class T>
struct ArraySelector<T[]>
{
    using type =
        std::conditional_t<std::is_trivially_destructible<T>::value,
                           Array<T>, Range<T>>;
};
}
#endif // FC_FLEXCLASS_ARRAYS_HPP
#endif // FC_FLEXCLASS_FLEXCLASS_HPP
