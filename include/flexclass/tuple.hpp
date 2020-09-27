#ifndef FC_FLEXCLASS_TUPLE_HPP
#define FC_FLEXCLASS_TUPLE_HPP

#include "memory.hpp"

#include <cstddef>
#include <cstdint>

namespace fc
{

template <class T1, class... Ts, class Buf, class Arg1, class... Args>
void callConstructors(int& constructed, Buf* buf, Arg1&& arg1, Args&&... args)
{
    if constexpr (std::is_aggregate_v<T1>)
        new (buf) T1{std::forward<Arg1>(arg1)};
    else
        new (buf) T1(std::forward<Arg1>(arg1));

    constructed++;

    if constexpr (sizeof...(Ts) > 0)
        callConstructors<Ts...>(constructed, buf + 1, std::forward<Args>(args)...);
}

template <class T1, class... Ts, class Buf>
void callDefaultConstructors(int& constructed, Buf* buf)
{
    new (buf) T1;

    constructed++;

    if constexpr (sizeof...(Ts) > 0)
        callDefaultConstructors<Ts...>(constructed, buf + 1);
}

template <int Id, class T1, class... Ts, class Buf>
void callDestructors(int constructed, Buf* buf)
{
    if constexpr (sizeof...(Ts) > 0)
        callDestructors<Id + 1, Ts...>(constructed, buf + 1);

    if (Id < constructed)
        reinterpret_cast<T1*>(buf)->~T1();
}

template <class... T>
struct MemInfo;
template <class F, class... T>
struct MemInfo<F, T...> : public MemInfo<T...>
{
    using Base = MemInfo<T...>;
    static constexpr auto Size = sizeof(F) > Base::Size ? sizeof(F) : Base::Size;
    static constexpr auto Align = alignof(F) > Base::Align ? alignof(F) : Base::Align;
};

template <>
struct MemInfo<>
{
    static constexpr auto Size = 1;
    static constexpr auto Align = 1;
};

template <int I, class T, class... Ts>
constexpr auto typePtrAtIdx()
{
    if constexpr (I == 0)
        return static_cast<T*>(nullptr);
    else
        return typePtrAtIdx<I - 1, Ts...>();
}

template <int I, class... Ts>
using TypeAtIdx = std::remove_pointer_t<decltype(typePtrAtIdx<I, Ts...>())>;

template <class... T>
struct tuple
{
    static constexpr auto Size = sizeof...(T);

    tuple()
    {
        if constexpr (Size > 0)
        {
            int constructed = 0;
            try
            {
                callDefaultConstructors<T...>(constructed, elements);
            }
            catch (...)
            {
                callDestructors<0, T...>(constructed, elements);
                throw;
            }
        }
    }

    template <class... Args>
    tuple(Args&&... args)
    {
        if constexpr (Size > 0)
        {
            int constructed = 0;
            try
            {
                callConstructors<T...>(constructed, elements, std::forward<Args>(args)...);
            }
            catch (...)
            {
                callDestructors<0, T...>(constructed, elements);
                throw;
            }
        }
    }

    ~tuple()
    {
        if constexpr (Size > 0)
            callDestructors<0, T...>(Size, elements);
    }

    tuple(const tuple&) = delete;
    tuple(tuple&&) = delete;
    tuple& operator=(const tuple&) = delete;
    tuple& operator=(tuple&&) = delete;

    template <int I>
    auto& get()
    {
        return reinterpret_cast<TypeAtIdx<I, T...>&>(elements[I]);
    }
    template <int I>
    auto& get() const
    {
        return reinterpret_cast<const TypeAtIdx<I, T...>&>(elements[I]);
    }

    using MI = MemInfo<T...>;

    std::aligned_storage_t<MI::Size, MI::Align> elements[Size];
};

namespace detail
{

template <int Idx, class T, class Fn>
void callWithIdx(const T& t, Fn&& f)
{
    f(t.template get<Idx>(), std::integral_constant<int, Idx>());
}

template <int Idx, class T, class Fn>
void callWithIdx(T& t, Fn&& f)
{
    f(t.template get<Idx>(), std::integral_constant<int, Idx>());
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
        f(t1.template get<I>(), t2.template get<I>(t2));
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

template <int I, class Fn, class First, class... T>
void for_each_constexpr_impl2(Fn&& fn)
{
    fn(static_cast<First*>(nullptr), std::integral_constant<int, I>());
    if constexpr (sizeof...(T) > 0)
        for_each_constexpr_impl2<I + 1, Fn, T...>(fn);
}

template <class Fn, class... T>
void for_each_constexpr_impl(Fn&& fn, fc::tuple<T...>*)
{
    if constexpr (sizeof...(T) > 0)
        for_each_constexpr_impl2<0, Fn, T...>(fn);
}

template <class Tuple, class Fn>
void for_each_constexpr(Fn&& fn)
{
    for_each_constexpr_impl(fn, static_cast<Tuple*>(nullptr));
}

template <class... Args>
auto make_tuple(Args&&... args)
{
    return fc::tuple<std::remove_reference_t<Args>...>(std::forward<Args>(args)...);
}

} // namespace fc

#endif // FC_FLEXCLASS_TUPLE_HPP
