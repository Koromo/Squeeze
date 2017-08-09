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
    real_t floating = 0;
    bool boolean = false;
    string_t string = L"A";

    t.var(SQZ_T("Int"), integer);
    t.var(SQZ_T("Float"), floating);
    t.var(SQZ_T("Bool"), boolean);
    t.var(SQZ_T("String"), string);

    CHECK(t.is(TypeTag::Integer, SQZ_T("Int")));
    CHECK(t.is(TypeTag::Float, SQZ_T("Float")));
    CHECK(t.is(TypeTag::Bool, SQZ_T("Bool")));
    CHECK(t.is(TypeTag::String, SQZ_T("String")));

    t.var(SQZ_T("Int"), floating);
    CHECK(t.is(TypeTag::Float, SQZ_T("Int")));

    string = nullptr;
    t.var(SQZ_T("String"), string);
    CHECK(t.is(TypeTag::Null, SQZ_T("String")));

    CHECK_FALSE(t.is(TypeTag::Float, SQZ_T("Bool")));
    CHECK_FALSE(t.is(TypeTag::Integer, SQZ_T("NotExist")));

    t.release();
    vm.close();
}

TEST(TABLE, SET_TABLE)
{
    HVM vm;
    vm.open(1024);

    HTable t(vm);

    HTable table(vm);

    t.table(SQZ_T("Table"), table);
    CHECK(t.is(TypeTag::Table, SQZ_T("Table")));

    table.release();
    t.release();
    vm.close();
}

void testfun() {};

TEST(TABLE, SET_FUN)
{
    HVM vm;
    vm.open(1024);

    HTable t(vm);

    t.fun(SQZ_T("Fun"), &testfun);
    CHECK(t.is(TypeTag::Closure, SQZ_T("Fun")));

    t.release();
    vm.close();
}