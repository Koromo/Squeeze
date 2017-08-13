#include <squeeze/squeeze.h>
#include <CppUTest/CommandLineTestRunner.h>

using namespace squeeze;

TEST_GROUP(CLASS)
{
};

class Foo
{
public:
    Foo(int, float) {}
    int add(int, int) { return 1; }
};

TEST(CLASS, SET_FUN)
{
    HVM vm;
    vm.open(1024);

    HClass<Foo> c(vm);
    c.ctor<int, float>();
    c.fun(SQZ_T("add"), &Foo::add);

    CHECK(c.is(ObjectType::HostFunction, SQZ_T("constructor")));
    CHECK(c.is(ObjectType::HostFunction, SQZ_T("add")));

    vm.close();
}