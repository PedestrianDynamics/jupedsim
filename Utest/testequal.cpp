// Teste die folgenden Funktion der Klasse Equal:
// GetPair, GetRand
#include <cstdlib>
#include "../math/Distribution.h"
#include <iostream>


//x should be in [mu-sigma, mu+sigma]
int test_equal()
{
    double mu = 1.24;
    double sigma = 0.26;
    Equal distequal(mu, sigma);
    return (distequal.GetRand()< mu-sigma || mu+sigma<distequal.GetRand())?EXIT_FAILURE:EXIT_SUCCESS;
}



int main(int argc, char* argv[])
{
    return test_equal();
}
