/*
 * MPICore.h
 *
 *  Created on: Feb 14, 2011
 *      Author: piccolo
 */

#ifndef MPICORE_H_
#define MPICORE_H_

class MPICore {
public:
	MPICore();
	virtual ~MPICore();

	void SendMessage();
};

#endif /* MPICORE_H_ */
