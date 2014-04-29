/*
 * NAvLineState.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: David Haensel
 */


#include "NavLineState.h"



NavLineState::NavLineState() 
{
    open = true;
    timeOfInformation = 0;
    timeFirstSeen = 0;
}

NavLineState::~NavLineState()
{
    
}

void NavLineState::close(double time){
    open = false;
    timeOfInformation = time;
    timeFirstSeen = time;
}

bool NavLineState::closed() 
{
    return !open;
   
}

bool NavLineState::isShareable(double time) 
{
    if(!timeOfInformation) return true;
    if(timeOfInformation+INFO_OFFSET < time) {
        timeOfInformation = 0;
        return true;
    }
    return false;
    
    
}

bool NavLineState::mergeDoor(NavLineState & orig, double time)
{
    if(timeFirstSeen == 0 || orig.timeFirstSeen > timeFirstSeen) {
        open = orig.open;
        timeFirstSeen = orig.timeFirstSeen;
        timeOfInformation = time;
        return true;
    }
    return false;
}

void NavLineState::print() {
    std::cout << open << " - "<< timeFirstSeen << " - " << timeOfInformation << std::endl;
}
