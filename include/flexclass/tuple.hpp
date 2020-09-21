#pragma once

#include <utility>
#include <type_traits>

namespace fc
{

template<class T> struct printer;

// type list
template<class... T> struct list { using type = void; };

// push_back_t creates a list with another element in the end
template<class A, class B> struct push_back;
template<class... A, class B> struct push_back<list<A...>, B>
{
    using type = list<A..., B>;
};
template<class A, class B> using push_back_t = typename push_back<A, B>::type;

template<class A, class B> struct push_front;
template<class... A, class B> struct push_front<list<A...>, B>
{
    using type = list<B, A...>;
};
template<class A, class B> using push_front_t = typename push_front<A, B>::type;


// concat
template<class A, class B> struct concat;
template<class... T1, class... T2> struct concat<list<T1...>, list<T2...>>
{
    using type = list<T1..., T2...>;
};

// reverse_t creates a list in the reverse order of the input
template<class L> struct reverse;
template<> struct reverse<list<>> { using type = list<>; };

template<class Head, class... Tail>
struct reverse<list<Head, Tail...>>
{
    using type = push_back_t<  typename reverse<list<Tail...>>::type   , Head>;
};

template<class T> using reverse_t = typename reverse<T>::type;

// add_ids: Transforms a list<T...> into list<list<id<0>,T1>, list<id<1>, T2>...>
template<int Id, class T> struct item
{
    static constexpr auto value = Id;
    using type = T;
};

template<int i, class List> struct reverse_add_ids;

template<int i, class Head, class... Tail>
struct reverse_add_ids<i, list<Head, Tail...>>
{
    using type = push_back_t<typename reverse_add_ids<i+1, list<Tail...>>::type, item<i, Head>>;
};

template<int i, class Head>
struct reverse_add_ids<i, list<Head>>
{
    using type = list<item<i, Head>>;
};

template<int i>
struct reverse_add_ids<i, list<>>
{
    using type = list<>;
};

template<class... T>
using reverse_add_ids_t = typename reverse_add_ids<0, T...>::type;

// Bring empty to end t

template<class T>
struct can_be_base {
    static constexpr auto value = std::is_empty<T>::value &&
                                  std::is_trivially_destructible<T>::value;
};

template<class T> struct bring_empty_to_end;

template<> struct bring_empty_to_end<list<>>
{
    using empties = list<>;
    using nonempties = list<>;
};

template<class Head, class... Tail>
struct bring_empty_to_end<list<Head, Tail...>>
{
    using empties = std::conditional_t<
        can_be_base<typename Head::type>::value,
            push_back_t<typename bring_empty_to_end<list<Tail...>>::empties, Head>,
            typename bring_empty_to_end<list<Tail...>>::empties
    >;
    using nonempties = std::conditional_t<
        !can_be_base<typename Head::type>::value,
            push_front_t<typename bring_empty_to_end<list<Tail...>>::nonempties, Head>,
            typename bring_empty_to_end<list<Tail...>>::nonempties
    >;
};

template<class Head>
struct bring_empty_to_end<list<Head>>
{
    using empties = std::conditional_t<
        can_be_base<typename Head::type>::value,
            list<Head>,
            list<>
    >;
    using nonempties = std::conditional_t<
        !can_be_base<typename Head::type>::value,
            list<Head>,
            list<>
    >;
};

template<class T> using bring_empty_to_end_t =
    typename concat<typename bring_empty_to_end<T>::nonempties,
                    typename bring_empty_to_end<T>::empties
             >::type;

// Utility that returns a parameter with idx "i" from a parameter list
template<int i, int N, class Arg1, class... Args>
decltype(auto) selectParamI(Arg1&& arg1, Args&&... args)
{
    if constexpr (i == N) return std::forward<Arg1>(arg1);
    else return selectParamI<i+1, N>(std::forward<Args>(args)...);
}

template<int i, class... Args>
decltype(auto) selectParam(Args&&... args)
{
    return selectParamI<0, i>(std::forward<Args>(args)...);
}

// Tuple implementation


template<class T, bool> struct tuple_impl;

template<class T> struct tuple_choice_i
{
    using type = tuple_impl<T, false>;
};

template<class Head, class... T>
struct tuple_choice_i<list<Head, T...>>
{
    using type = tuple_impl<list<Head, T...>, can_be_base<typename Head::type>::value>;
};

template<class T>
using tuple_choice = typename tuple_choice_i<T>::type;

template<class T, bool> struct tuple_impl;

template<class Head, class... Tail>
struct tuple_impl<list<Head, Tail...>, false> : public tuple_choice<list<Tail...>>
{
    using Base = tuple_choice<list<Tail...>>;

    template<class... Args>
    tuple_impl(Args&&... args)
        : Base(std::forward<Args>(args)...)
        , m_data(selectParam<Head::value>(std::forward<Args>(args)...))
    {}

    template<int i> auto& get() const
    {
        if constexpr (i == Head::value)
            return m_data;
        else
            return Base::template get<i>();
    };

    template<int i> auto& get()
    {
        if constexpr (i == Head::value)
            return m_data;
        else
            return Base::template get<i>();
    };

    typename Head::type m_data;
};

template<int idx, class T>
struct Derivable : public T
{
    T& get() { return *this; }
    const T& get() const { return *this; }
};

template<class Head, class... Tail>
struct tuple_impl<list<Head, Tail...>, true>
    : public tuple_choice<list<Tail...>>
    , public Derivable<Head::value, typename Head::type>
{
    using Base = tuple_choice<list<Tail...>>;
    using Data = Derivable<Head::value, typename Head::type>;

    template<class... Args>
    tuple_impl(Args&&... args)
        : Base(std::forward<Args>(args)...)
        , Data{selectParam<Head::value>(std::forward<Args>(args)...)}
    {}

    template<int i> auto& get() const
    {
        if constexpr (i == Head::value)
            return Data::get();
        else
            return Base::template get<i>();
    };

    template<int i> auto& get()
    {
        if constexpr (i == Head::value)
            return Data::get();
        else
            return Base::template get<i>();
    };
};

template<> struct tuple_impl<list<>, false>
{
    template<class... Args> tuple_impl(Args&&... args) {}
};

template<class... Members>
struct tuple : public tuple_choice<bring_empty_to_end_t<reverse_add_ids_t<list<Members...>>>>
{
    using Base = tuple_choice<bring_empty_to_end_t<reverse_add_ids_t<list<Members...>>>>;
    static constexpr auto Size = sizeof...(Members);

    template<class... Args>
    tuple(Args&&... args)
        : Base(std::forward<Args>(args)...)
    {}

    template<int i> auto& get() const
    {
        static_assert(i < Size, "Index exceeds tuple size");
        return Base::template get<i>();
    };

    template<int i> auto& get()
    {
        static_assert(i < Size, "Index exceeds tuple size");
        return Base::template get<i>();
    };
};

template<int i, class...T>
auto& get_element(tuple<T...>& t) { return t.template get<i>(); }

template<int i, class...T>
auto& get_element(const tuple<T...>& t) { return t.template get<i>(); }

}
