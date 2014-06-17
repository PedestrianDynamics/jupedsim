#include <cstdlib>
#include "../math/Mathematics.h"
// test positive number
int testSign()
{
     return (sign(1.0)==1)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main()
{
     return testSign();
}
