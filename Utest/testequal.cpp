/**
 * \file        testequal.cpp
 * \date        Jul 4, 2014
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
 
 
// Test Equal.GetRand()
#include <cstdlib>
#include "../math/Distribution.h"
#include <stdio.h>

//x should be in [mu-sigma, mu+sigma]
int test_equal()
{
     FILE * f;
     char fname[FILENAME_MAX] = "log_testEqual.txt";
     f = fopen(fname, "w");

     int res = 0;
     double mu = 1.24;
     double sigma = 0.26;
     double r;
     // test different constructors
     // 1
     Equal distequal(mu, sigma);
     r = distequal.GetRand();
     res = ( r < mu-sigma || mu+sigma < r )?0:1;
     fprintf(f, "1: mu=%.2f\t sigma=%.2f\t [%.2f, %.2f]\t r=%.2f\t res=%d\n",mu, sigma, mu-sigma, mu+sigma, r, res);
     // 2
     Equal distequal1(distequal);
     r = distequal.GetRand();
     res += (distequal.GetRand()< mu-sigma || mu+sigma<distequal.GetRand())?0:1;
     fprintf(f, "2: mu=%.2f\t sigma=%.2f\t [%.2f, %.2f]\t r=%.2f\t res=%d\n",mu, sigma, mu-sigma, mu+sigma, r, res);
     // 3
     Equal distequal2();
     r = distequal.GetRand();
     res += (distequal.GetRand()< mu-sigma || mu+sigma<distequal.GetRand())?0:1;
     fprintf(f, "3: mu=%.2f\t sigma=%.2f\t [%.2f, %.2f]\t r=%.2f\t res=%d\n",mu, sigma, mu-sigma, mu+sigma, r, res);


     fclose(f);
     return (res!=3)?EXIT_FAILURE:EXIT_SUCCESS;
}



int main()
{
     return test_equal();
}
