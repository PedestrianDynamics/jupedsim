#ifndef TRAILPLOTTER_H
#define TRAILPLOTTER_H

//forwarded classes
class vtkPolyData;
class vtkActor;
class vtkAppendPolyData;
class vtkAppendPolyData;
class vtkCleanPolyData;


class TrailPlotter
{
public:
    /// constructor
    TrailPlotter();

    /// destructor
    virtual ~TrailPlotter();

    /// add a dataset to plot
    void AddDataSet(vtkPolyData * _polydata);

    /// return the actor responsible for the plotting
    vtkActor* getActor();

    /// set the visibility satus of the trails
    void SetVisibility(bool status);

private:
    vtkActor* _trailActor;
    // combine polydatas sets
    vtkAppendPolyData* _appendFilter;
    // Remove any duplicate points.
    vtkCleanPolyData* _cleanFilter;
};

#endif // TRAILPLOTTER_H
