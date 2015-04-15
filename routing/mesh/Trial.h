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


class Trial // father := smaller; child := bigger (in terms of cost); cost/speed := ptr to its cost/speed
{
    public:
        unsigned long int key;
        int* flag;
        Trial* child;
        Trial* father;
        double* cost; //pointer to its cost  (cost  = &costarray[key])
        double* speed;//pointer to its speed (speed = &speedarray[key])

        Trial() {
            key = 0;
            flag = nullptr;
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
        void insert(Trial* &smallest, Trial* &biggest, Trial* add) {
            if(smallest != nullptr) {
                //unsigned long int key_curr = smallest->key;
                if (smallest->cost[0] > add->cost[0]) {
                    add->child = smallest;
                    add->father = smallest->father;
                    smallest->father = add;
                    smallest = add;
                } else {
                    insert(smallest->child, biggest, add);
                }
            } else { //wenn add der groesste wert ist, wird er durchgereich und muss korr am ende eingebunden werden.
                smallest = add; //wird anstelle des nullptr eingefuegt (smallest ist nicht der global smallest sondern das rekursionsargument)
                if (biggest == nullptr) { //indicator, dass keine liste existierte, also setze smallest und biggest auf add
                    add->father = nullptr;
                } else {                  //es gab eine liste und das vorher letzte (biggest) wird nun vorletzter
                    add->father = biggest;
                }

                add->child = nullptr;     //add ist letzter, dahinter nichts
                biggest = add;            //add wird neuer letzter (bzw. einziger im sonderfall "alles leer")
            }
        }

//        void sort(Trial* &smallest, Trial* &biggest) { //only asserts that biggest is at end
//            if (smallest != nullptr) {
//                Trial* aux;
//                aux = smallest->child;
//                if (aux != nullptr) {
//                    if (smallest->cost[0] > aux->cost[0]) {
//                        if (aux->child != nullptr) {
//                            (aux->child)->father = smallest;
//                        } else {
//                            biggest = smallest;
//                        }
//                        if (smallest->father != nullptr) {
//                            (smallest->father)->child = aux;
//                        }
//                        aux->father = smallest->father;
//                        smallest->child = aux->child;
//                        smallest->father = aux;
//                        aux->child = smallest;
//                    }
//                    sort(aux, biggest);
//                }
//            }
//        }

        void lift(Trial* &smallest, Trial* &biggest) { //only asserts that smallest is at start
            if (biggest != nullptr) {
                Trial* aux;
                aux = biggest->father;
                if (aux != nullptr) {
                    if (biggest->cost[0] < aux->cost[0]) {
                        if (aux->father != nullptr) {
                            (aux->father)->child = biggest;
                        } else {
                            smallest = biggest;
                        }
                        if (biggest->child != nullptr) {
                            (biggest->child)->father = aux;
                        }
                        aux->child = biggest->child;
                        biggest->father = aux->father;
                        biggest->child = aux;
                        aux->father = biggest;
                    }
                    lift(smallest, aux);
                }
            }
        }

        void remove(Trial* &smallest, Trial* &biggest, Trial* curr) {
            if (smallest == curr && biggest == curr) {
                smallest = nullptr;
                biggest = nullptr;
            } else {
                if (smallest == curr) {
                    curr->child->father = nullptr;
                    smallest = curr->child;
                } else {
                    if (biggest == curr) {
                        curr->father->child = nullptr;
                        biggest = curr->father;
                    } else {
                        curr->father->child = curr->child;
                        curr->child->father = curr->father;
                    }
                }
            }
        }



    protected:
    private:
};

#endif // TRIAL_H
