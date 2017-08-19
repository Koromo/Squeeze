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

    int getx() { return x; }
    void setx(int x_) { x = x_; }
    int gety() { return y; }
    void sety(int y_) { y = y_; }
};

Vec getVec(int x, int y) { return Vec(x * 2, y); }

TEST(SCRIPT, CLASS)
{
    HVM vm;
    vm.open(1024);

    HClass<Vec> c(vm);
    c.ctor<int, int>();
    c.fun(SQZ_T("sum"), &Vec::sum);
    c.setter(SQZ_T("x"), &Vec::setx);
    c.getter(SQZ_T("x"), &Vec::getx);
    c.prop(SQZ_T("y"), &Vec::gety, &Vec::sety);

    auto table = vm.rootTable();
    table.clazz(SQZ_T("Vec2"), c);
    table.fun(SQZ_T("getVec"), wrapConv(getVec, SQZ_T("Vec2")));

    HScript script(vm);
    HTable env(vm);

    script.compileFile(SQZ_T("test.nut"));
    script.run(env);

    CHECK(env.call<int>(SQZ_T("vecsum"), env, 5, 2) == 7);
    CHECK(env.call<int>(SQZ_T("vecsum_withget"), env, 5, 2) == 12);
    CHECK(env.call<int>(SQZ_T("setget"), env, 5, 2) == 10);

    vm.close();
}

TEST(SCRIPT, CLASS_LAMBDA)
{
    HVM vm;
    vm.open(1024);

    HClass<Vec> c(vm);
    c.ctor<int, int>();
    c.fun(SQZ_T("sum"), [](Vec* c) { return c->sum(); });
    c.setter(SQZ_T("x"), [](Vec* c, int x) { c->x = x; });
    c.getter(SQZ_T("x"), [](Vec* c) { return c->x; });
    c.prop(SQZ_T("y"), [](Vec* c) { return c->y; }, [](Vec* c, int y) { c->y = y; });

    auto table = vm.rootTable();
    table.clazz(SQZ_T("Vec2"), c);
    table.fun(SQZ_T("getVec"), [](int x, int y) { return ClassConv<Vec>{getVec(x, y), SQZ_T("Vec2")}; });

    HScript script(vm);
    HTable env(vm);

    script.compileFile(SQZ_T("test.nut"));
    script.run(env);

    CHECK(env.call<int>(SQZ_T("vecsum"), env, 5, 2) == 7);
    CHECK(env.call<int>(SQZ_T("vecsum_withget"), env, 5, 2) == 12);
    CHECK(env.call<int>(SQZ_T("setget"), env, 5, 2) == 10);

    vm.close();
}