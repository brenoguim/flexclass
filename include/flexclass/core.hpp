#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP

#include "algorithm.hpp"
#include "memory.hpp"
#include "tuple.hpp"
#include "utility.hpp"

#include <cassert>
#include <iostream>
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
    Handle() = default;
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

template <class InputIt>
auto arg(Arg<InputIt> a)
{
    return a;
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
 */
template <class T, class = void>
struct ArrayBuildersConverter
{
    using type = Ignore;
};

template <class T>
struct ArrayBuildersConverter<T, typename void_<typename isHandle<T>::enable>::type>
{
    using type = ArrayBuilder<typename T::fc_handle_type>;
};

template <class... Args>
auto make_tuple(Args&&... args)
{
    return fc::tuple<std::remove_reference_t<Args>...>(std::forward<Args>(args)...);
}

template <class... Args>
auto args(Args&&... args)
{
    return ::fc::make_tuple(fc::arg(std::forward<Args>(args))...);
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

template <class Handles>
struct Handles2ArrayBuilders;

template <class... T>
struct Handles2ArrayBuilders<fc::tuple<T*...>>
{
    using type = fc::tuple<ArrayBuilder<typename T::fc_handle_type>...>;
};

template <class FC, class Alloc, class AArgs, class... ClassArgs>
auto makeWithAllocator(Alloc& alloc, AArgs&& aArgs, ClassArgs&&... cArgs)
{
    using Handles = decltype(std::declval<FC>().fc_handles());

    std::size_t numBytesForArrays = 0;
    for_each_constexpr<Handles>([&](auto* type, auto idx) {
        using Element = remove_cvref_t<decltype(**type)>;
        using Idx = decltype(idx);
        using T = typename Element::fc_handle_type;

        ArrayBuilder<T> arrBuilder;

        numBytesForArrays += arrBuilder.numRequiredBytes(sizeof(FC) + numBytesForArrays,
                                                         aArgs.template get<Idx::value>());
    });

    auto memBuffer = unique_ptr<void, DeleteFn<FC, Alloc>>(
        alloc.allocate(sizeof(FC) + numBytesForArrays), alloc);

    FC* ret;
    if constexpr (std::is_aggregate_v<FC>)
        ret = new (memBuffer.get()) FC{std::forward<ClassArgs>(cArgs)...};
    else
        ret = new (memBuffer.get()) FC(std::forward<ClassArgs>(cArgs)...);

    memBuffer.get_deleter().m_objectCreated = true;

    // Start creating arrays right after the FC object
    std::byte* arrayBuffer = reinterpret_cast<std::byte*>(ret + 1);

    using ArrayBuilders = typename Handles2ArrayBuilders<Handles>::type;
    ArrayBuilders arrayBuilders;

    for_each_in_tuple(arrayBuilders, [&](auto& arrayBuilder, auto idx) mutable {
        using Idx = decltype(idx);
        arrayBuffer = arrayBuilder.buildArray(arrayBuffer, aArgs.template get<Idx::value>());
    });

    auto&& handles = ret->fc_handles();
    for_each_in_tuple(arrayBuilders, [&](auto& arrayBuilder, auto idx) mutable {
        using Idx = decltype(idx);
        handles.template get<Idx::value>()->setLocation(arrayBuilder.m_begin, arrayBuilder.m_end);
        arrayBuilder.release();
    });

    memBuffer.release();
    return ret;
}

template <class FC, class Alloc>
void destroyWithAllocator(Alloc& alloc, FC* p)
{
    if (!p)
        return;
    auto&& handles = p->fc_handles();
    reverse_for_each_in_tuple(handles, [p](auto* handle, auto idx) {
        using Handle = remove_cvref_t<decltype(*handle)>;
        if constexpr (!std::is_trivially_destructible<typename Handle::fc_handle_type>::value)
        {
            reverseDestroy(handle->begin(p), handle->end(p));
        }
    });
    p->~FC();
    alloc.deallocate(const_cast<FC*>(p));
}

template <class FC, class AArgs, class... ClassArgs>
auto makeInternal(AArgs&& aArgs, ClassArgs&&... cArgs)
{
    NewDeleteAllocator alloc;
    return makeWithAllocator<FC>(alloc, std::forward<AArgs>(aArgs),
                                 std::forward<ClassArgs>(cArgs)...);
}

template <class FC>
auto destroy(FC* ptr)
{
    NewDeleteAllocator alloc;
    return destroyWithAllocator<FC>(alloc, ptr);
}

template <class FC, class Alloc>
auto destroy(FC* ptr, Alloc& alloc)
{
    return destroyWithAllocator<FC>(alloc, ptr);
}

template <class FC, class... AArgs>
auto make(AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...)](auto&&... cArgs) mutable {
        return fc::makeInternal<FC>(a, std::forward<decltype(cArgs)>(cArgs)...);
    };
}

template <class FC, class Alloc, class... AArgs>
auto make(WithAllocator, Alloc& alloc, AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...), &alloc](auto&&... cArgs) mutable {
        return fc::makeWithAllocator<FC>(alloc, a, std::forward<decltype(cArgs)>(cArgs)...);
    };
}

template <class T>
struct DestroyFn
{
    void operator()(T* t) { fc::destroy(t); }
};

template <class T>
using UniquePtr = fc::unique_ptr<T, fc::DestroyFn<T>>;

template <class FC, class... AArgs>
auto make_unique(AArgs&&... aArgs)
{
    return [a = fc::args(aArgs...)](auto&&... cArgs) mutable {
        return fc::UniquePtr<FC>(
            fc::makeInternal<FC>(a, std::forward<decltype(cArgs)>(cArgs)...));
    };
}

} // namespace fc

#endif // FC_FLEXCLASS_CORE_HPP
