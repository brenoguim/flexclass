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
