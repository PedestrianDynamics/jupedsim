/*
 * File:   Wall.h
 * Author: andrea
 *
 * Created on 16. November 2010, 12:55
 */

#ifndef _WALL_H
#define	_WALL_H

#include "Line.h"

/************************************************************
 Wall
 ************************************************************/
class Wall : public Line {
private:
    // keine neuen Elemente zu Line
public:
    Wall();
    Wall(const Point& p1, const Point& p2);
    Wall(const Wall& orig);
    //virtual ~Wall();

    // Ausgabe
    void WriteToErrorLog() const; // zur Kontrolle nach Log
    virtual string Write() const;
};


#endif	/* _WALL_H */

