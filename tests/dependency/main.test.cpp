#include "flexclass.hpp"

struct Foo
{
    Foo() : a(1), b(1.0) {}
    int a;
    float b;
};

struct Message
{
    auto fc_handles() { return fc::make_tuple(&c, &d); }
    int a;
    double b;
    fc::Array<long> c;
    fc::Array<Foo> d;
};

int main()
{
    auto m = fc::make<Message>(100, 2)(1, 1.0);
}
