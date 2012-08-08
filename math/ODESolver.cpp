/**
 * File:   ODESolver.cpp
 *
 * Created on 17. August 2010, 15:31
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */


#include "ODESolver.h"

ODESolver::ODESolver(ForceModel *model) : model(model) {

}

EulerSolver::EulerSolver(ForceModel *model) : ODESolver(model) {

}

/* Löst die Differentialgleichung mit Hilfe des Euler Verfahrens
 * Parameter:
 *   - ti: aktueller Zeitpunkt
 *   - tip1: nächster Zeitpunkt, Zeitschritt h = tip1 - ti
 *   - building: Geometrie, mit Wänden und allen Fußgängern
 * Algorithmus:
 *   - model->CalculateForce() berechnet die Kräfe und daraus Beschleunigung und Geschwindigkeit
 *   - entsprechend werden dann die neuen Gescwindigkeiten und Positionen der Fußgänger gesetzt
 * */

void EulerSolver::solveODE(double ti, double tip1, Building* building) const {
    double h = tip1 - ti;
    vector< vector < vector < Point > > > result_acc_room = vector<vector < vector < Point > > >();

    // Schleife übr alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        vector < vector < Point > > result_acc_rooms = vector < vector < Point > > ();
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            vector< Point > result_acc = vector< Point > ();
            if (anzpeds != 0) {
                model->CalculateForce(ti, result_acc, building, i, s->GetSubRoomID());
            }
            result_acc_rooms.push_back(result_acc);
            result_acc.clear();
        }
        result_acc_room.push_back(result_acc_rooms);
        result_acc_rooms.clear();
    }

    /* PRARALLELES UPDATE */

    // Schleife über alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            // Schleife über alle Fußgänger im SubRoom
            for (int k = 0; k < anzpeds; ++k) {
                Point result_acc = result_acc_room[i][j][k];
                Pedestrian* ped = s->GetPedestrian(k);
                Point pos_neu, v_neu; // neue Positionen und Geschwindigkeiten setzen
                v_neu = (result_acc * h) + ped->GetV();
                if (v_neu.Norm() < EPS_V)
                    pos_neu = ped->GetPos();
                else
                    pos_neu = (v_neu * h) + ped->GetPos();
                // neue Werte setzten
                ped->SetPos(pos_neu);
                ped->SetV(v_neu);
                ped->SetPhiPed();
                ped->UpdateJamData();
            }
        }
    }
    result_acc_room.clear();
}

VelocityVerletSolver::VelocityVerletSolver(ForceModel *model) : ODESolver(model) {
};

void VelocityVerletSolver::solveODE(double ti, double tip1, Building* building) const {
    double h = tip1 - ti;
    double h2 = h*h;
    double lambda = 0.5;

    vector< vector < vector < Point > > > result1_acc_room = vector<vector < vector < Point > > >();
    vector< vector < vector < Point > > > result2_acc_room = vector<vector < vector < Point > > >();
    // Schleife über alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        vector < vector < Point > > result1_acc_subroom = vector < vector < Point > > ();
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            vector< Point > result1_acc = vector< Point > ();
            if (anzpeds != 0) {
                model->CalculateForce(ti, result1_acc, building, i, s->GetSubRoomID());
            }
            result1_acc_subroom.push_back(result1_acc);
            result1_acc.clear();
        }
        result1_acc_room.push_back(result1_acc_subroom);
        result1_acc_subroom.clear();
    }

    /* ERSTES PRARALLELES UPDATE */

    // Schleife über alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            // Schleife über alle Fußgänger im SubRoom
            for (int k = 0; k < anzpeds; ++k) {
                Pedestrian* ped = s->GetPedestrian(k);
                Point pos_neu, v_neu; // neue Positionen und Geschwindigkeiten setzen
                if ((ped->GetV() + result1_acc_room[i][j][k]*0.5 * h).Norm() < EPS_V)
                    pos_neu = ped->GetPos();
                else
                    pos_neu = ped->GetPos() + ped->GetV() * h + result1_acc_room[i][j][k]*0.5 * h2;
                v_neu = ped->GetV() + result1_acc_room[i][j][k] * h * lambda;
                ped->SetPos(pos_neu);
                ped->SetV(v_neu);
            }
        }
    }

    // Schleife über alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        vector < vector < Point > > result2_acc_subroom = vector < vector < Point > > ();
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            vector< Point > result2_acc = vector< Point > ();
            if (anzpeds != 0) {
                model->CalculateForce(ti, result2_acc, building, i, s->GetSubRoomID());
            }
            result2_acc_subroom.push_back(result2_acc);
            result2_acc.clear();
        }
        result2_acc_room.push_back(result2_acc_subroom);
        result2_acc_subroom.clear();
    }


    /* ZWEITES PRARALLELES UPDATE */

    // Schleife übr alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            for (int k = 0; k < anzpeds; ++k) {
                Pedestrian* ped = s->GetPedestrian(k);
                Point v_neu; // neue Positionen und Geschwindigkeiten setzen
                v_neu = ped->GetV() + (result1_acc_room[i][j][k] + result2_acc_room[i][j][k])*0.5 * h;
                ped->SetV(v_neu);
                ped->SetPhiPed();
            }
        }
    }

    result1_acc_room.clear();
    result2_acc_room.clear();

};

LeapfrogSolver::LeapfrogSolver(ForceModel *model) : ODESolver(model) {

}

/* Löst die Differentialgleichung mit Hilfe des LeapFrog Verfahrens
 * Parameter:
 *   - ti: aktueller Zeitpunkt
 *   - tip1: nächster Zeitpunkt, Zeitschritt h = tip1 - ti
 *   - building: Geometrie, mit Wänden und allen Fußgängern
 * Algorithmus:
 *   - model->CalculateForce() berechnet die Kräfe und daraus Beschleunigung und Geschwindigkeit
 *   - entsprechend werden dann die neuen Gescwindigkeiten und Positionen der Fußgänger gesetzt
 * */

void LeapfrogSolver::solveODE(double ti, double tip1, Building* building) const {
    double h = tip1 - ti;
    vector< vector < vector < Point > > > result_acc_room = vector<vector < vector < Point > > >();

    // Schleife übr alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        vector < vector < Point > > result_acc_rooms = vector < vector < Point > > ();
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            vector< Point > result_acc = vector< Point > ();
            if (anzpeds != 0) {
                model->CalculateForce(ti, result_acc, building, i, s->GetSubRoomID());
            }
            result_acc_rooms.push_back(result_acc);
            result_acc.clear();
        }
        result_acc_room.push_back(result_acc_rooms);
        result_acc_rooms.clear();
    }

    /* PRARALLELES UPDATE */

    // Schleife übr alle Räume
    for (int i = 0; i < building->GetAnzRooms(); i++) {
        Room* r = building->GetRoom(i);
        // Schleife über alle SubRooms im Raum
        for (int j = 0; j < r->GetAnzSubRooms(); j++) {
            SubRoom* s = r->GetSubRoom(j);
            int anzpeds = s->GetAnzPedestrians();
            // Schleife über alle Fußgänger im SubRoom
            for (int k = 0; k < anzpeds; ++k) {
                Point result_acc = result_acc_room[i][j][k];
                Pedestrian* ped = s->GetPedestrian(k);
                Point pos_neu, v_neu; // neue Positionen und Geschwindigkeiten setzen
                if (ti == 0.0) {
                    v_neu = (result_acc * 0.5 * h) + ped->GetV(); // v(0.5) wird aus v(0) berechnet
                } else {
                    v_neu = (result_acc * h) + ped->GetV();
                }
                if (v_neu.Norm() < EPS_V)
                    pos_neu = ped->GetPos();
                else
                    pos_neu = (v_neu * h) + ped->GetPos();
                // neue Werte setzten
                ped->SetPos(pos_neu);
                ped->SetV(v_neu);
                ped->SetPhiPed();
            }
        }
    }
    result_acc_room.clear();
}

EulerSolverLC::EulerSolverLC(ForceModel *model) : ODESolver(model) {
}

void EulerSolverLC::solveODE(double ti, double tip1, Building* building) const {

    model->CalculateForceLC(ti, tip1, building);

}
