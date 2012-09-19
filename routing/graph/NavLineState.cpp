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

NavLineState::NavLineState(bool open) 
{
    open = open;
    timeOfInformation = clock();
    timeFirstSeen = clock();
    return;
    
}


NavLineState::NavLineState(const NavLineState & orig)
{
    open = orig.open;
    timeFirstSeen = orig.timeFirstSeen;
    // set the current clocks() to know how old the information is
    timeOfInformation = clock();    
}

NavLineState::~NavLineState()
{
    
}

void NavLineState::close(){
  open = false;
  timeOfInformation = clock();
  timeFirstSeen = clock();
}

bool NavLineState::closed() 
{
    return !open;
   
}

bool NavLineState::isShareable() 
{
    if(!timeOfInformation) return true;
    if(timeOfInformation+INFO_OFFSET < clock()) {
	timeOfInformation = 0;
	return true;
    }
    return false;
    
    
}

bool NavLineState::mergeDoor(NavLineState & orig)
{
    if(timeFirstSeen == 0 || orig.timeFirstSeen > timeFirstSeen) {
	open = orig.open;
	timeFirstSeen = orig.timeFirstSeen;
	timeOfInformation = clock();
	return true;
    }
    return false;
}

void NavLineState::print() {
  std::cout << open << " - "<< timeFirstSeen << " - " << timeOfInformation << std::endl;
}
