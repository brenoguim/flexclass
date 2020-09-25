#ifndef FC_FLEXCLASS_TUPLE_HPP
#define FC_FLEXCLASS_TUPLE_HPP

#include <cstdint>

namespace fc
{

/*! Placeholder type to indicate to the library that
 *  a type should just get default initialization
 */
struct Default
{
};

constexpr std::size_t findNextAlignedPosition(std::size_t pos, std::size_t desiredAlignment)
{
    return (pos - 1u + desiredAlignment) & -desiredAlignment;
}

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
static constexpr std::size_t maxAlign = std::max({std::size_t(1), alignof(T)...});

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
