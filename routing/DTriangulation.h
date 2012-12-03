/*
 * DTriangulation.h
 *
 *  Created on: Nov 30, 2012
 *      Author: piccolo
 */

#ifndef DTRIANGULATION_H_
#define DTRIANGULATION_H_

#include <cstdlib>
#include <time.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>

#include "../geometry/Point.h"
#include "../poly2tri/poly2tri.h"
using std::vector;
using std::endl;
using std::cout;


class DTriangulation {
public:
	DTriangulation();


	virtual ~DTriangulation();

	void Triangulate();

	std::vector<p2t::Triangle*>  GetTriangles(){
		return _cdt->GetTriangles();
	}

	void SetOuterPolygone(std::vector<Point>  outerConstraints);

	void AddHole(std::vector<Point>  hole);

	//templates for freing and clearing a vector of pointers
	template <class C> void FreeClear( C & cntr ) {
		for ( typename C::iterator it = cntr.begin();
				it != cntr.end(); ++it ) {
			delete * it;
		}
		cntr.clear();
	}

private:
	std::vector< std::vector<p2t::Point*> > _holesPolylines;
	std::vector<p2t::Point*> _outerConstraintsPolyline;
	p2t::CDT* _cdt;

};

#endif /* DTRIANGULATION_H_ */
