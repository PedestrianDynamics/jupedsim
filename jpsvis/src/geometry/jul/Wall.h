/* 
 * File:   Wall.h
 * Author: andrea
 *
 * Created on 30. September 2010, 11:47
 */

#ifndef _WALL_H
#define	_WALL_H

#include "Line.h"

class Wall {
private:
    Line pLine;
public:
    Wall();
    Wall(const Wall& orig);
    virtual ~Wall();

    // Setter -Funktionen
    void SetLine(Line l);

    // Getter - Funktionen
    const Line& GetLine() const;

    //Ausgabe
    void WriteToErrorLog() const;

};

#endif	/* _WALL_H */

