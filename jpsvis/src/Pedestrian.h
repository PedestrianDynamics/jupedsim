/**
 * @headerfile  Pedestrian.h
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * @brief The Pedestrian class represents pedestrian with all their parameters.
 *
 * Created on: 05.05.2009
 *
 */


#ifndef PEDESTRIAN_H_
#define PEDESTRIAN_H_

#include <QStack>
#include <QColor>


//extern variables
//TODO: pipe my through the setting class
extern bool   extern_tracking_enable;

//forwarded classes
class vtkDiskSource;
class vtkAssembly;
class vtkActor;
class vtkDataArray;
class vtkTextActor3D ;
class PointPlotter;
class LinePlotter;
class JPoint;
class vtkPoints;
class TrajectoryPoint;
class vtkCamera;
//class QRgb;

class Pedestrian {
public:
    /**
     * constructor
     * @param ID
     * @return
     */
    Pedestrian(int ID);
    /**
     *  constructor
     *
     * @param ID
     * @param x
     * @param y
     * @param z
     * @return
     */
    Pedestrian(int ID, double x ,double y, double z);

    /**
     * Class destructor
     *
    */
    virtual ~Pedestrian();

    /**
    * \brief  to provide a virtual reality-like impression
    * @param cam a vtkcamera object
    */
    static void setCamera(vtkCamera* cam);

    /**
     * set the agent id, whose eyes you want to see the scene through, -1
     * will reset to world/scene camera
     *
     * @param agent, the ID of the agent you want to see the world through
     */
    //static void setVirtualRealityAgent(int agent);

    ///returns the pedestrian ID
    int getID();

    ///returns the pedestrians  Y coordinate
    double getY();

    ///returns the pedestrians  X coordinate
    double getX();

    ///returns the pedestrians  Z coordinate
    double getZ();


    /// return the actor to the pedestrians trail/trace
    vtkAssembly* getTrailActor();

    /// call after the pedestrian has been created
    vtkAssembly* getActor();

    /// move the pedestrian to  the next position
    /// @param [x y z] coordinates and the velocities
    void moveTo(TrajectoryPoint *Point );

    /// change the size of the caption
    /// @todo: also change size and orientation
    void  setCaptionSize(int size);

    /// enable/disable the pedestrians captions
    void enableCaption(bool status);


    /// set the pedestrian individual visibility status
    void setVisibility(bool isVisible);

    /// get the individual visibility status
    bool isVisible();

    /// set the group visibility status
    void setGroupVisibility(bool status);

    /// get the group visibility
    bool getGroupVisibility();

    /// set the initial visibility status
    /// this may be useful because, all pedestrians are created at the beginning.
    /// and placed at the coordinates [ 0 0 0 ]
    void initVisibility(bool status);


    /// switch to 2 Dimensional
    /// FIXME not working when visualisation is already started
    void setVisualModeTo2D(bool mode);

    /// true if the pedestrian is out of the system
    /// it might be deleted
    ///void setOutOfTheSystem(bool isOut);

    /// set the pedestrian size, height in cm
    void setSize(double size);

    /// plot the pedestrian trails
    void plotTrail(double x, double y, double z);
    //void plotTrail(double* points,int size);

    /// set the trail types
    /// 0 points, 1 polygon
    void setTrailGeometry(int type);

    //tODO???? what is todo?
    void setColor(int color[3]);
    void setColor(int color);


    /// set the pedestrian resolution.
    /// 3 will results in a triangle
    void setResolution(int pts);


    /**
     * Set the caption colour. If not specified,<br>
     * colour will be set to automatic mode.
     *
     * @brief set the caption colour
     * @param a value between 0 and 255 specifying the desired colour
     */
    void setCaptionsColor(QColor& col);

    /// enable or disable automatic caption color
    void setCaptionsColorModeToAuto(bool status);

private:
    void createActor();
    void CreateActor2D();
    void CreateActor3D();
    void createSnowMan();
    void createTrailActor();
    void triggerPlotTrail();

private :
    // pedestrian properties
    int ID;
    int type;
    double posX;
    double posY;
    double posZ;

    bool groupVisibilityStatus;
    double pedSize;
    double pedsColors[3];
    bool autoCaptionColorMode;

    /// the virtual reality camera
    static vtkCamera * virtualCam;

    /// throug its eyes you will see the world
    //static int virtualRealityAgent;

    /// define the pedestrian body;
    vtkActor* bodyActor;

    ///define the pedestrian structure
    vtkAssembly* pedestrianAssembly;

    /// define the pedestrian structure 3D
    vtkAssembly* assembly3D;

    /// define the pedestrian structure 2D

    vtkAssembly* assembly2D;

    /// to increase the radius, the actor could be scale up or down.
    vtkDiskSource* spaceNeeded ; //private sphere needed by agents for moving

    /// private sphere actor needed by the pedestrian. It is a function of the velocity
    vtkActor* ellipseActor;

    /// trail actor left by the pedestrian
    vtkAssembly* trailActor;

    /// caption actor
    vtkTextActor3D* caption;

    /// trails plotter
    QStack<JPoint*> trailPoint;
    PointPlotter* trailPlotterPoint;
    LinePlotter* trailPlotterLine;
};

#endif /* PEDESTRIAN_H_ */
