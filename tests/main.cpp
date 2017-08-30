#include <CppUTest/CommandLineTestRunner.h>

int main(int ac, char** av)
{
    const auto code = CommandLineTestRunner::RunAllTests(ac, av);
    return code;
}