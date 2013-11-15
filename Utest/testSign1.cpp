#include <cstdlib>
#include "../math/Mathematics.h"

// test negative number
int testSign1(){
    return (sign(-1.0)==-1)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main()
{
    return testSign1();
}
