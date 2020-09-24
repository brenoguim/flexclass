#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP

#include "tuple.hpp"

#include <cassert>
#include <limits>
#include <type_traits>

namespace fc
{
struct WithAllocator
{
};
static constexpr WithAllocator withAllocator;

template <class T>
struct Handle
{
    template <class U>
    Handle(U&&)
    {
    }
    using fc_handle = T;
    using type = T;
};

template <class T>
void reverse_destroy(T* b, T* e)
{
    if (b != e)
    {
        while (1)
        {
            e--;
            e->~T();
            if (e == b)
                break;
        }
    }
}

template <class T>
struct ArrayDeleter
{
    ArrayDeleter(T* begin) : m_begin(begin), m_end(begin) {}

    ~ArrayDeleter() { reverse_destroy(m_begin, m_end); }

    void setEnd(T* end) { m_end = end; }
    void release() { m_begin = m_end = nullptr; }

    T *m_begin{nullptr}, *m_end{nullptr};
};

inline void* incr(void* in, std::size_t len)
{
    return static_cast<char*>(in) + len;
}

template <class T, class U>
inline auto align(U* u)
{
    void* ptr = u;
    if (alignof(U) >= alignof(T))
        return static_cast<T*>(ptr);

    std::size_t space = std::numeric_limits<std::size_t>::max();
    return static_cast<T*>(std::align(alignof(T), 0, ptr, space));
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

struct Default
{
};

template <class InputIt>
struct Arg
{
    Arg(std::size_t size) : m_size(size) {}
    Arg(std::size_t size, InputIt it) : m_size(size), m_it(it) {}
    std::size_t m_size;
    InputIt m_it;
};

auto arg(std::size_t size) { return Arg<Default>{size}; }

template <class InputIt>
auto arg(std::size_t size, InputIt it)
{
    return Arg<InputIt>{size, it};
}

template <class T>
struct ArrayBuilder
{
    ~ArrayBuilder()
    {
        if (m_begin)
            reverse_destroy(m_begin, m_end);
    }

    void consume(void*& buf, std::size_t& space, std::size_t sz)
    {
        consume(buf, space, Arg<Default>{sz});
    }

    template <class InputIt>
    void consume(void*& buf, std::size_t& space, Arg<InputIt>&& arg)
    {
        consume(buf, space, arg);
    }

    template <class InputIt>
    void consume(void*& buf, std::size_t& space, Arg<InputIt>& arg)
    {
        auto numBytes = arg.m_size * sizeof(T);
        auto ptr = std::align(alignof(T), numBytes, buf, space);
        assert(ptr);
        space -= numBytes;
        buf = incr(buf, numBytes);

        auto b = static_cast<T*>(ptr);
        auto e = b + arg.m_size;

        ArrayDeleter<T> deleter(b);
        for (auto it = b; it != e;)
        {
            if constexpr (std::is_same_v<InputIt, Default>)
                new (it) T;
            else
                new (it) T(*arg.m_it++);
            deleter.setEnd(++it);
        }
        deleter.release();

        m_begin = b;
        m_end = e;
    }

    static std::size_t numRequiredBytes(std::size_t offset, std::size_t sz)
    {
        return numRequiredBytes(offset, Arg<Default>{sz});
    }

    template <class InputIt>
    static std::size_t numRequiredBytes(std::size_t offset,
                                        const Arg<InputIt>& arg)
    {
        auto numBytes = arg.m_size * sizeof(T);
        std::size_t space = std::numeric_limits<std::size_t>::max();
        auto originalSpace = space;
        void* ptr = static_cast<char*>(nullptr) + offset;
        auto r = std::align(alignof(T), numBytes, ptr, space);
        assert(r);
        return (originalSpace - space) + numBytes;
    }

    void release() { m_begin = m_end = nullptr; }

    T* m_begin{nullptr};
    T* m_end{nullptr};
};

template <class T>
struct ArraySelector
{
    using type = T;
};

template <class T>
struct is_array_placeholder : std::false_type
{
};
template <class T>
struct is_array_placeholder<ArrayBuilder<T>> : std::true_type
{
};

template <class T>
struct void_
{
    using type = void;
};

template <class T, class = void>
struct is_handle : std::false_type
{
};
template <class T>
struct is_handle<T, typename void_<typename T::fc_handle>::type>
    : std::true_type
{
    using enable = T;
};

struct Ignore
{
    Ignore() = default;
    template <class T>
    Ignore(T&&)
    {
    }
};
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
struct ArrayBuildersConverter<
    T, typename void_<typename is_handle<T>::enable>::type>
{
    using type = ArrayBuilder<typename T::type>;
};

namespace detail
{
template <std::size_t Idx, class Arg1, class... Args>
decltype(auto) pickFromPack(Arg1&& arg1, Args&&... args)
{
    if constexpr (Idx == 0)
        return std::forward<Arg1>(arg1);
    else
        return pickFromPack<Idx - 1>(std::forward<Args>(args)...);
}

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
                     reverseIntegerSequence(
                         std::make_integer_sequence<int, sizeof...(Ts)>()));
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
struct GetAlignmentRequirement<
    T, typename void_<typename is_handle<T>::enable>::type>
{
    static constexpr std::size_t value = alignof(typename T::type);
};

template <class... Types>
struct CollectAlignment
{
    static constexpr auto value =
        std::max({std::size_t(1), GetAlignmentRequirement<Types>::value...});
};

template <class T, class Alloc>
struct DeleteFn
{
    DeleteFn(Alloc& alloc) : m_alloc(&alloc) {}
    void operator()(void* ptr) const
    {
        if (typeCreated)
            static_cast<T*>(ptr)->~T();
        m_alloc->deallocate(ptr);
    }
    Alloc* m_alloc;
    bool typeCreated{false};
};

template <class T, class Deleter>
struct unique_ptr : private Deleter
{
    explicit unique_ptr(T* t) : m_t(t) {}
    template <class DeleterArg>
    unique_ptr(T* t, DeleterArg&& darg)
        : Deleter(std::forward<DeleterArg>(darg)), m_t(t)
    {
    }
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& other)
        : Deleter(std::move(other.get_deleter())),
          m_t(std::exchange(other.m_t, nullptr))
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

struct NewDeleteAllocator
{
    void* allocate(std::size_t sz) { return ::operator new(sz); }
    void deallocate(void* ptr) { ::operator delete(const_cast<void*>(ptr)); }
};

template <class T>
using base_type = std::remove_cv_t<std::remove_reference_t<T>>;

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
        using ArrayBuilders =
            fc::tuple<typename ArrayBuildersConverter<T>::type...>;

        std::size_t numBytesForArrays = 0;
        {
            for_each_in_tuple(
                ArrayBuilders(), [&](const auto& u, auto idx) mutable {
                    using U = base_type<decltype(u)>;
                    using Idx = decltype(idx);
                    if constexpr (is_array_placeholder<U>::value)
                        numBytesForArrays += u.numRequiredBytes(
                            sizeof(Derived) + numBytesForArrays,
                            detail::pickFromPack<Idx::value>(args...));
                });
        }

        auto implBuffer = unique_ptr<void, DeleteFn<Derived, Alloc>>(
            alloc.allocate(sizeof(Derived) + numBytesForArrays), alloc);

        ArrayBuilders pi;

        auto ret = new (implBuffer.get()) Derived(std::forward<Args>(args)...);
        implBuffer.get_deleter().typeCreated = true;

        void* arrayBuffer = ret + 1;
        for_each_in_tuple(pi, [&](auto& u, auto idx) mutable {
            using U = base_type<decltype(u)>;
            using Idx = decltype(idx);
            if constexpr (is_array_placeholder<U>::value)
                u.consume(arrayBuffer, numBytesForArrays,
                          detail::pickFromPack<Idx::value>(
                              std::forward<Args>(args)...));
        });

        for_each_zipped<sizeof...(T)>(*ret, pi, [](auto& u, auto& k) {
            using K = base_type<decltype(k)>;
            if constexpr (is_array_placeholder<K>::value)
            {
                u.setLocation(k.m_begin, k.m_end);
                k.release();
            }
        });

        implBuffer.release();
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
            using U = base_type<decltype(u)>;
            if constexpr (is_handle<U>::value)
                if constexpr (!std::is_trivially_destructible<
                                  typename U::type>::value)
                {
                    reverse_destroy(u.begin(p), u.end(p));
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
