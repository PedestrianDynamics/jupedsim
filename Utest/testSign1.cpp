#include <cstdlib>
#include "../math/Mathematics.h"

// test negative number
int testSign1(int argc, char* argv[]){
    return (sign(-1.0)==-1)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
    return testSign1(argc, argv);
}
