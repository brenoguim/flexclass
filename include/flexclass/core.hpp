#ifndef FC_FLEXCLASS_CORE_HPP
#define FC_FLEXCLASS_CORE_HPP

#include "algorithm.hpp"
#include "memory.hpp"
#include "tuple.hpp"
#include "utility.hpp"

#include <cassert>
#include <limits>
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
auto arg(std::size_t size) { return Arg<detail::NoIterator>{size}; }

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
    void buildArray(void*& buf, std::size_t sz) { buildArray(buf, Arg<detail::NoIterator>{sz}); }

    //! Creates the array with inputs specified by Arg in the given buffer.
    template <class InputIt>
    void buildArray(void*& buf, Arg<InputIt>&& arg)
    {
        buildArray(buf, arg);
    }

    //! Creates the array with inputs specified by Arg in the given buffer.
    template <class InputIt>
    void buildArray(void*& buf, Arg<InputIt>& arg)
    {
        // Find the fist aligned byte suitable for creating T objects
        auto numBytes = arg.m_size * sizeof(T);
        auto ptr = (void*)findNextAlignedPosition(buf, alignof(T), 1);
        buf = incr(ptr, numBytes);

        auto b = static_cast<T*>(ptr);
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
        std::max({std::size_t(1), GetAlignmentRequirement<Types>::value...});
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
        void* arrayBuffer = ret + 1;

        ArrayBuilders arrayBuilders;
        for_each_in_tuple(arrayBuilders, [&](auto& element, auto idx) mutable {
            using Element = remove_cvref_t<decltype(element)>;
            using Idx = decltype(idx);
            if constexpr (isArrayBuilder<Element>::value)
                element.buildArray(arrayBuffer,
                                   pickFromPack<Idx::value>(std::forward<Args>(args)...));
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
