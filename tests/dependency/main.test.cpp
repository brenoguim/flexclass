#include "flexclass.hpp"

struct Foo
{
    Foo() : a(1), b(1.0) {}
    int a;
    float b;
};

struct Message
{
    auto fc_handles() { return fc::v2::make_tuple(&c, &d); }
    int a;
    float b;
    fc::Array<long> c;
    fc::Array<Foo> d;
};

int main()
{
    auto m = fc::v2::make<Message>(100, 2)(1, 1.0);
}
