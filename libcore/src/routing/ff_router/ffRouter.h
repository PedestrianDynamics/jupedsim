/**
 * \file        ffRouter.h
 * \date        Feb 19, 2016
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * This router is an update of the former Router.{cpp, h} - Global-, Quickest
 * Router System. In the __former__ version, a graph was created with doors and
 * hlines as nodes and the distances of (doors, hlines), connected with a line-
 * of-sight, was used as edge-costs. If there was no line-of-sight, there was no
 * connecting edge. On the resulting graph, the Floyd-Warshall algorithm was
 * used to find any paths. In the "quickest-___" variants, the edge cost was not
 * determined by the distance, but by the distance multiplied by a speed-
 * estimate, to find the path with minimum travel times. This whole construct
 * worked pretty well, but dependend on hlines to create paths with line-of-
 * sights to the next target (hline/door).
 *
 * In the ffRouter, we want to overcome hlines by using floor fields to
 * determine the distances. A line of sight is not required any more. We hope to
 * reduce the graph complexity and the preparation-needs for new geometries.
 *
 * To find a counterpart for the "quickest-____" router, we can either use
 * __special__ floor fields, that respect the travel time in the input-speed map,
 * or take the distance-floor field and multiply it by a speed-estimate (analog
 * to the former construct.
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * Questions to solve: how to deal with goalID == doorID problem in matrix
 *
 * Restrictions/Requirements: Floorfields are not really 3D supporting:
 *
 * A room may not consist of subrooms which overlap in their projection (onto
 * x-y-plane). So subrooms, that are positioned on top of others (in stairways
 * for example), must be separated into different rooms.
 *
 * floorfields do not consider z-coordinates. Distances of two grid points are
 * functions of (x, y) and not (x, y, z). Any slope will be neglected.
 *
 **/
#pragma once

#include "UnivFFviaFM.h"
#include "general/Macros.h"
#include "geometry/Building.h"
#include "routing/Router.h"

class Building;
class Pedestrian;
class OutputHandler;

/*!
 * \class FFRouter
 *
 * \brief router using floor fields to measure distances of doors, no hlines
 *
 *\ingroup Router
 * This router is an update of the former Router.{cpp, h} - Global-, Quickest
 * Router System. In the __former__ version, a graph was created with doors and
 * hlines as nodes and the distances of (doors, hlines), connected with a line-
 * of-sight, was used as edge-costs. If there was no line-of-sight, there was no
 * connecting edge. On the resulting graph, the Floyd-Warshall algorithm was
 * used to find any paths. In the "quickest-___" variants, the edge cost was not
 * determined by the distance, but by the distance multiplied by a speed-
 * estimate, to find the path with minimum travel times. This whole construct
 * worked pretty well, but dependend on hlines to create paths with line-of-
 * sights to the next target (hline/door).
 *
 * In the ffRouter, we want to overcome hlines by using floor fields to
 * determine the distances. A line of sight is not required any more. We hope to
 * reduce the graph complexity and the preparation-needs for new geometries.
 *
 * To find a counterpart for the "quickest-____" router, we can either use
 * __special__ floor fields, that respect the travel time in the input-speed map,
 * or take the distance-floor field and multiply it by a speed-estimate (analog
 * to the former construct.
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * \author Arne Graf
 * \date Feb, 2016
 */

class FFRouter : public Router
{
public:
    /**
     * Constructor for FFRouter.
     * @param id id of router in simulation.
     * @param s routing strategy which should be used to find best way.
     * @param hasSpecificGoals specifies if the peds have specific goals (true) or head to the outside (false).
     * @param config configuration of simulation.
     */
    FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals, Configuration * config);

    /**
      * Destructor for FFRouter.
      */
    ~FFRouter() override;

    /**
      * \brief Init the router (must be called before use)
      *
      * Init() will construct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It needs the floor fields.
      *  Will call CalculateFloorFields.
      * @param building used geometry.
      */
    bool Init(Building * building) override;

    int FindExit(Pedestrian * p) override;

    void Update() override;

    /**
      * \brief ReInit the router if quickest router is used. Current position of agents is
      * considered.
      *
      * ReInit() will reconstruct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It will reconstruct the floorfield to
      * evaluate the best doors to certain goals as they could change.
      * Will call CalculateFloorFields.
      */
    bool ReInit();


    /**
     * Returns if the router needs an update if \a _mode == "quickest".
     *
     * @return if the router needs an update.
     */
    bool MustReInit();

    /**
     * Sets the time the floor field needs to be updated the next time, needed if \a _mode == "quickest".
     * @param t current time in simulation.
     */
    void SetRecalc(double t);

private:
    /**
     * \brief Performs the Floyd-Warshall algorithm.
     *
     * Computes the distances depending on the costs and the corresponding paths with the Floyd-Warshall algoritm.
     * @post \a _distMatrix contains the actual distance (cost dependent), _pathMatrix contains the corresponding paths.
     */
    void FloydWarshall();

    /**
     * \brief Computes the needed floor fields and distances.
     *
     * Sets up the needed maps and computes distances via FloydWarshall().
     */
    void CalculateFloorFields();

protected:
    /**
     * Configuration used during simulation.
     */
    Configuration * _config{};

    /**
     * Distance matrix: _distMatrix[<door1, door2>] returns the distance from door1 to door2.
     */
    std::map<std::pair<int, int>, double> _distMatrix;

    /**
     * Path matrix: _pathsMatrix[<door1, door2>] returns the next target on the way from door1 to door2.
     */
    std::map<std::pair<int, int>, int> _pathsMatrix;

    /**
     * Vector containing the UIDs of all doors in \a _building.
     */
    std::vector<int> _allDoorUIDs;

    /**
     * Vector containing the UIDs of all rooms which are directional escalators.
     */
    std::vector<int> _directionalEscalatorsUID;

    /**
     * Geometry used during the simulation.
     */
    Building * _building{};

    /**
     * Map of the underlying floorfields. _locffviafm[id] gives the floorfield in room with ID==id.
     */
    std::map<int, UnivFFviaFM *> _locffviafm;

    /**
     * Map containing all the exits from the geometry.
     */
    std::map<int, Transition *> _ExitsByUID;

    /**
     * Map containing all doors in geometry.
     */
    std::map<int, Crossing *> _CroTrByUID;

    /**
     * Time when the next recalculation is needed when using \a _strategy=ROUTING_FF_QUICKEST.
     */
    double _timeToRecalc = 0.;

    /**
     * Time span between calculations when using \a _strategy=ROUTING_FF_QUICKEST.
     */
    double _recalc_interval{};

    /**
     * Router needs a recalculation when using \a _strategy=ROUTING_FF_QUICKEST.
     */
    bool _plzReInit = false;

    /**
     * The pedestrian head for specific goals (true) or just for the outside (false)
     */
    bool _hasSpecificGoals{};

    /**
     * Defines if the router is used room or subroom wise.
     */
    bool _targetWithinSubroom{};

    /**
     * Map from goalID to the closest exit. It maps goals to door UID.
     */
    std::map<int, int> _goalToLineUIDmap;
};
