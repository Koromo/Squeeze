#include <CppUTest/CommandLineTestRunner.h>
#include <cstdlib>

int main(int ac, char** av)
{
    const auto code = CommandLineTestRunner::RunAllTests(ac, av);
    std::system("pause");
    return code;
}