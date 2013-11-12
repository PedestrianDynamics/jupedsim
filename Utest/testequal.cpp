// Teste die folgenden Funktion der Klasse Equal:
// GetPair, GetRand
#include <cstdlib>
#include "../math/Distribution.h"
#include <iostream>



int test_equal(){

	double mu = 1.24;
	double sigma = 0.26;

	Equal distequal(mu, sigma);

	return (distequal.GetRand()< mu-sigma || mu+sigma<distequal.GetRand())?EXIT_SUCCESS:EXIT_FAILURE;


}



int main(int argc, char* argv[])
{
    return test_equal();
}
