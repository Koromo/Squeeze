#include <squeeze/squeeze.h>
#include <CppUTest/CommandLineTestRunner.h>

using namespace squeeze;

TEST_GROUP(SCRIPT)
{
};

TEST(SCRIPT, COMPILE)
{
    HVM vm;
    vm.open(1024);

    HScript script(vm);
    HTable env(vm);

    script.compileFile(SQZ_T("test.nut"));
    script.run(env);

    CHECK(env.is(ObjectType::Function, SQZ_T("foo")));
    CHECK(env.is(ObjectType::Integer, SQZ_T("integer")));
    CHECK(!env.is(ObjectType::Integer, SQZ_T("localinteger")));
    CHECK(env.call<int>(SQZ_T("foo"), env, 6) == 30);

    vm.close();
}

struct Vec
{
    int x, y;
    Vec(int x_, int y_) : x(x_), y(y_) {}
    int sum() { return x + y; }
};

Vec getVec(int x, int y) { return Vec(x * 2, y); }

TEST(SCRIPT, CLASS)
{
    HVM vm;
    vm.open(1024);

    HClass<Vec> c(vm);
    c.ctor<int, int>();
    c.fun(SQZ_T("sum"), &Vec::sum);

    auto table = vm.rootTable();
    table.clazz(SQZ_T("Vec2"), c);
    table.fun(SQZ_T("getVec"), getVec, SQZ_T("Vec2"));

    HScript script(vm);
    HTable env(vm);

    script.compileFile(SQZ_T("test.nut"));
    script.run(env);

    CHECK(env.call<int>(SQZ_T("vecsum"), env, 5, 2) == 7);
    CHECK(env.call<int>(SQZ_T("vecsum_withget"), env, 5, 2) == 12);

    vm.close();
}