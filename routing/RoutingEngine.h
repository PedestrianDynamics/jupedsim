/**
 * \file RoutingEngine.h
 *
 * \brief Manage all routers in the simulation. Provide the agent with the appropriate router.
 * \date Created on: Jan 10, 2013
 * \author Ulrich Kemloh
 *
 */

#ifndef ROUTINGENGINE_H_
#define ROUTINGENGINE_H_

#include "Router.h"

class RoutingEngine {
public:
	/**
	 * Constructor
	 */
	RoutingEngine();

	/**
	 * Destructor
	 */
	virtual ~RoutingEngine();

	/**
	 * Add a final destination in the system.
	 * The destinations are segments (@see Transitions @see Crossings)
	 * @param id
	 */
	void AddFinalDestinationID(int id);

	/**
	 * Add a new trip to the system. Individual pedestrian can be assigned a particular trip.
	 * @param trip
	 */
	void AddTrip(vector<string> trip);

	/**
	 * Return a trip/route with the particular id
	 * FIXME referenz?
	 * @param id
	 * @return
	 */
	const vector<string> GetTrip(int id) const;

	/**
	 * Find the next destination using the appropriate router from
	 * the collection for the pedestrian ped.
	 */
	void FindRoute(Pedestrian* ped);


	/**
	 * Add a new router to the routing system
	 *
	 */
	void AddRouter(Router* router);

	/**
	 * Return the router with the specified  strategy
	 */
	Router* GetRouter(RoutingStrategy strategy) const;

	/**
	 * Return the router with the specified  id
	 */
	Router* GetRouter(int id) const;

	/**
	 * Initialize all routers with the current building object
	 * @param building
	 */
	void Init(Building* building);

private:
	/// collections of all routers used
	vector<Router*> _routersCollection;
	/// collection of all trips/routes
	vector<vector<string> >_tripsCollection;
};

#endif /* ROUTINGENGINE_H_ */
