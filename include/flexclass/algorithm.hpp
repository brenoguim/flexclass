#ifndef FLEXCLASS_ALGORITHM_HPP
#define FLEXCLASS_ALGORITHM_HPP

namespace fc
{
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
} // namespace fc

#endif
