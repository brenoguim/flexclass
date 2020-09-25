#include "flexclass.hpp"

struct Foo
{
    Foo() : a(1), b(1.0) {}
    int a;
    float b;
};

int main()
{
    auto m = fc::FlexibleClass<int, float, long[], Foo[]>::make_unique(1, 1.0, 100, 2);
}
