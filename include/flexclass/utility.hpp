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
