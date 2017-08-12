#include <squeeze/squeeze.h>
#include <CppUTest/CommandLineTestRunner.h>

using namespace squeeze;

TEST_GROUP(SCRIPT)
{
};

TEST(SCRIPT, RUN)
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