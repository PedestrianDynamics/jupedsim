/**
 * \file
 * \date        Mar 26, 2015
 * \version     N/A (v0.6)
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
 * Implementation of classes for ...
 *
 *
 **/

#ifndef TRIAL_H
#define TRIAL_H


class Trial
{
    public:
        unsigned long int key;
        int* flag;
        Trial* child;
        Trial* father;
        double* cost;
        double* speed;

        Trial() {
            key = 0;
            child = nullptr;
            father = nullptr;
            cost = nullptr;
            speed = nullptr;
        }

        Trial(unsigned long int keyArg, Trial* fatherArg, Trial* childArg, double* t, double* f, int* flagArg) {
            key = keyArg;
            father = fatherArg;
            child = childArg;
            cost = t;
            speed = f;
            flag = flagArg;
        }

        virtual ~Trial() {}

        // insertion (in order)
        void insert(Trial* &biggest, Trial* &smallest, Trial* add) {
            if(smallest != nullptr) {
                unsigned long int key_curr = smallest->key;
                if ( smallest->cost[key_curr] > add->cost[add->key] ) {
                    add->child = smallest;
                    add->father = smallest->father;
                    smallest->father = add;
                    smallest = add;
                } else {
                    insert(biggest, smallest->child, add);
                }
            } else { //list empty !!!!!!! @todo ar.graf continue !!!!!!! wenn add der groesste wert ist, wird er durchgereich und muss korr am ende eingebunden werden.
                smallest = add;
                biggest = add;
                add->father = nullptr;
                add->child = nullptr;
            }
        }
    protected:
    private:
};

#endif // TRIAL_H
