#ifndef FLEXCLASS_UTILITY_HPP
#define FLEXCLASS_UTILITY_HPP

namespace fc
{

//! Utility type to ease SFINAE
template <class T>
struct void_
{
    using type = void;
};

//! Removes all references, const and volatile from T
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

} // namespace fc

#endif
