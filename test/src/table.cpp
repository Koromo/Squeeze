#include <squeeze/squeeze.h>
#include <CppUTest/CommandLineTestRunner.h>

using namespace squeeze;

TEST_GROUP(TABLE)
{
};

TEST(TABLE, SET_VAR)
{
    HVM vm;
    vm.open(1024);

    HTable t(vm);

    int integer = 0;
    float floating = 0;
    bool boolean = false;
    string_t string = L"A";

    t.var(SQZ_T("Int"), integer);
    t.var(SQZ_T("Float"), floating);
    t.var(SQZ_T("Bool"), boolean);
    t.var(SQZ_T("String"), string);

    CHECK(t.is(ObjectType::Integer, SQZ_T("Int")));
    CHECK(t.is(ObjectType::Real, SQZ_T("Float")));
    CHECK(t.is(ObjectType::Bool, SQZ_T("Bool")));
    CHECK(t.is(ObjectType::String, SQZ_T("String")));

    t.var(SQZ_T("Int"), floating);
    CHECK(t.is(ObjectType::Real, SQZ_T("Int")));

    CHECK_FALSE(t.is(ObjectType::Real, SQZ_T("Bool")));
    CHECK_FALSE(t.is(ObjectType::Integer, SQZ_T("NotExist")));

    vm.close();
}

TEST(TABLE, SET_TABLE)
{
    HVM vm;
    vm.open(1024);

    HTable t(vm);

    HTable table(vm);

    t.table(SQZ_T("Table"), table);
    CHECK(t.is(ObjectType::Table, SQZ_T("Table")));

    vm.close();
}

void testfun() {};

TEST(TABLE, SET_FUN)
{
    HVM vm;
    vm.open(1024);

    HTable t(vm);

    t.fun(SQZ_T("Fun"), &testfun);
    CHECK(t.is(ObjectType::HostFunction, SQZ_T("Fun")));

    vm.close();
}