#include <catch.hpp>
#include <flexclass.hpp>

template<class... T>
class VariantArray
{
    enum Members                  {Size,        Active};
    using Impl = fc::FlexibleClass<std::size_t, std::size_t, fc::AdjacentArray<T>...>;

  public:
    /* Interesting public API */
    template<class U>
    static VariantArray make(std::size_t len)
    {
        std::size_t idxOfActive, idx = 2;
        auto dummy = { (std::is_same_v<U, T> ? (idxOfActive = idx) : ++idx)... };

        return { Impl::make(len, idxOfActive, (std::is_same_v<U, T> ? len : 0)...) };
    }

    auto size() const { return m_data->template get<Size>(); }
    auto activeIdx() const { return m_data->template get<Active>(); }

    template<class Fn> void visit(Fn&& fn)
    {
        visitImpl<2, 2 + sizeof...(T)>(activeIdx(), fn);
    }

    VariantArray(VariantArray&& other) : m_data(std::exchange(other.m_data, nullptr)) {}
    VariantArray(const VariantArray& other) = delete;
    VariantArray& operator=(VariantArray&& other) { Impl::destroy(m_data); std::exchange(other.m_data, nullptr); }
    VariantArray& operator=(const VariantArray& other) = delete;
    ~VariantArray() { Impl::destroy(m_data); }

  private:
    template<std::size_t I, std::size_t N, class Fn>
    void visitImpl(std::size_t active, Fn&& fn)
    {
        if constexpr (I != N)
        {
            if (active == I) { fn(m_data->template begin<I>()); }
            else visitImpl<I+1, N>(active, fn);
        }
    }

    VariantArray(Impl* data) : m_data(data) {}
    Impl* m_data {nullptr};
};

TEST_CASE( "Exercise the VariantArray", "[variant_array_example]" )
{
    using VA = VariantArray<char, float, int, double>;

    {
        auto va = VA::make<int>(10);
        CHECK(va.size() == 10);

        bool isInt = false;
        int numCalls = 0;
        va.visit(
            [&] <class T>(T* begin) {
                numCalls++;
                if constexpr (std::is_same_v<T, int>)
                {
                    isInt = true;
                    *begin = 16;
                }
            });

        CHECK(isInt);
        CHECK(numCalls == 1);

        va.visit(
            [] <class T>(T* begin) {
                if constexpr (std::is_same_v<T, int>)
                    CHECK(*begin == 16);
            });
    }


    {
        auto va = VA::make<float>(1);
        CHECK(va.size() == 1);

        bool isFloat = false;
        int numCalls = 0;
        va.visit(
            [&] <class T>(T* begin) {
                numCalls++;
                if constexpr (std::is_same_v<T, float>)
                {
                    isFloat = true;
                    *begin = 1.0;
                }
            });

        CHECK(isFloat);
        CHECK(numCalls == 1);

        va.visit(
            [] <class T>(T* begin) {
                if constexpr (std::is_same_v<T, float>)
                    CHECK(*begin == 1.0);
            });
    }
}
