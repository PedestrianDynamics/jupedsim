#ifndef TRAIN_VAR_H_
#define TRAIN_VAR_H_ 1

#include "../geometry/Point.h"
#include "../geometry/Transition.h"

#include <vtkPolyDataMapper.h>
#include <vtkTextActor3D.h>

struct TrainTimeTable {
    int id;
    std::string type;
    int rid;      // room id
    int sid;      // subroom id
    double tin;   // arrival time
    double tout;  // leaving time
    Point pstart; // track start
    Point pend;   // track end
    Point tstart; // train start
    Point tend;   // train end
    int pid;      // Platform id
    bool arrival;
    bool departure;
    bool reversed;
    double train_offset;
    double elevation;
    vtkSmartPointer<vtkPolyDataMapper> mapper;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkTextActor3D> textActor;
};


/**
 * Information of train doors, as position, width, and allowed flow.
 */
struct TrainDoor {
    double _distance; /** Distance to start of train. */
    double _width;    /** Width of train door. */
    double _flow;     /** Max. allowed flow at train door. */
};

struct TrainType {
    std::string _type; /** Name of the train type, used for stating which train arrives/deptarts. */
    int _maxAgents;    /** Max. number of agents allowed in the train. */
    double _length;    /** Length of the train. */
    std::vector<TrainDoor> _doors; /** Doors of the train. */
};

struct TrainEventInfo {
    int trainID;
    int trackID;
    TrainType trainType;
    double trainStartOffset;
    bool reversed;
};


#endif /* TRAIN_H_ */
