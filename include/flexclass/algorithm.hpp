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

} // namespace fc

#endif
