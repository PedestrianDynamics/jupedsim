#ifndef TRAIN_VAR_H_
#define TRAIN_VAR_H_ 1

#include "geometry/Point.h"
#include "geometry/Transition.h"

struct TrainTimeTable
{
     int id;
     std::string type;
     int rid; // room id
     int sid; // subroom id
     double tin; // arrival time
     double tout; //leaving time
     Point pstart; // track start
     Point pend; // track end
     Point tstart; // train start
     Point tend; // train end
     int pid; // Platform id
     bool arrival;
     bool departure;
};
struct TrainType
{
     std::string type;
     int nmax; // agents_max
     float len; //length
     std::vector<Transition> doors;
};

#endif /* TRAIN_H_ */
