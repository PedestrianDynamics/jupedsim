/**
* @headerfile Frame.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
*
* @section LICENSE
* This file is part of OpenPedSim.
*
* OpenPedSim is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* OpenPedSim is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
*
* @section DESCRIPTION
*
* @brief contains the collection of all
* pedestrians coordinates (trajectoryPoint) belonging to the same frame(i.e at the same time)
*
*  Created on: 10.07.2009
*
*/

#ifndef Frame_H_
#define Frame_H_

class TrajectoryPoint;
class vtkPolyData;

class Frame {
public:
	Frame();
	virtual ~Frame();

	/// add an element to the Frame
	void addElement(TrajectoryPoint* point);

	///clear all Points in the frame
	void clear();

	///return the next object in the frame
	TrajectoryPoint* getNextElement();

	int getSize();

	unsigned int getElementCursor();

	void resetCursor();

	vtkPolyData* GetPolyData();

	vtkPolyData* GetSclarData();

private:
	std::vector <TrajectoryPoint *> framePoints;

	vtkPolyData * _polydata;

	/// points to the actual element in the frame
	unsigned int elementCursor;
};

#endif /* Frame_H_ */
