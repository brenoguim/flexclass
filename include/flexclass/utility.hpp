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

} // namespace fc

#endif
