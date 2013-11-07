#include <cstdlib>
#include "../math/Mathematics.h"
// test positive number
int testSign(int argc, char* argv[])
{
    return (sign(1.0)==1)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
    return testSign(argc, argv);
}
