#include "TrailPlotter.h"

#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkDiskSource.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>


#define VTK_CREATE(type, name) \
        vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


TrailPlotter::TrailPlotter()
{
    // trails sources
    VTK_CREATE (vtkDiskSource, agentShape);
    agentShape->SetCircumferentialResolution(20);
    agentShape->SetInnerRadius(0);
    agentShape->SetOuterRadius(30);

    //speed the rendering using triangles stripers
    vtkTriangleFilter *tris = vtkTriangleFilter::New();
    tris->SetInputConnection(agentShape->GetOutputPort());
    //tris->GetOutput()->ReleaseData();

    vtkStripper *strip = vtkStripper::New();
    strip->SetInputConnection(tris->GetOutputPort());
    //strip->GetOutput()->ReleaseData();


    _appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    _appendFilter->SetInputConnection(strip->GetOutputPort());

    // Remove any duplicate points.
    _cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    //_cleanFilter->SetInputConnection(_appendFilter->GetOutputPort());

    //Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    //mapper->SetInputConnection(_cleanFilter->GetOutputPort());

    _trailActor =  vtkSmartPointer<vtkActor>::New();
    //_trailActor->SetMapper(mapper);
}

TrailPlotter::~TrailPlotter()
{

}

void TrailPlotter::AddDataSet(vtkPolyData *_polydata)
{

#if VTK_MAJOR_VERSION <= 5
    _appendFilter->AddInputConnection(_polydata->GetProducerPort());
#else
    _appendFilter->AddInputData(_polydata);
#endif

    _appendFilter->Update();
    _cleanFilter->Update();
}

vtkActor *TrailPlotter::getActor()
{
  return  _trailActor;
}

void TrailPlotter::SetVisibility(bool status)
{
 _trailActor->SetVisibility(status);
}
