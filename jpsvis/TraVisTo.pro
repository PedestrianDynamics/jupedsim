TEMPLATE = app
TARGET = TraVisTo
CONFIG += qt
QT += xml \
    network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32 { 
	INCLUDEPATH += C:/VTK/include/vtk-5.10 
	
	LIBS += -LC:/VTK/bin \
	    -lvtksys \
	    -lvtkzlib \
	    -lvtkjpeg \
	    -lvtkpng \
	    -lvtktiff \
	    -lvtkexpat \
	    -lvtkfreetype \
	    -lvtklibxml2 \
	    -lvtkDICOMParser \
	    -lvtkverdict \
	    -lvtkNetCDF \
	    -lvtkmetaio \
	    -lvtkexoIIc \
	    -lvtkalglib \
	    -lvtkftgl \
	    -lvtkCommon \
	    -lvtkFiltering \
	    -lvtkImaging \
	    -lvtkGraphics \
	    -lvtkIO \
	    -lvtkRendering \
	    -lvtkParallel \
	    -lvtkHybrid \
	    -lvtkWidgets \
	    -lvtkInfovis\
	    -lvtkViews\
	    -lwsock32\
}
        
unix { 
    INCLUDEPATH += /usr/include/vtk-5.8
    
    LIBS += -L/usr/lib/ \
        -lvtkCommon \
        -lvtkDICOMParser \
        -lvtkFiltering \
        -lvtkGenericFiltering \
        -lvtkGraphics \
        -lvtkHybrid \
        -lvtkIO \
        -lvtkImaging \
        -lvtkRendering \
        -lvtkVolumeRendering \
        -lvtkWidgets \
        -lvtkexoIIc \
        -lvtkftgl \
        -lvtksys \
        -lvtkverdict \
        -lvtkParallel \
        -lvtkmetaio
}
   
    HEADERS += forms/Settings.h \
        src/SaxParser.h \
        src/Debug.h \
        src/travisto.h \
        src/Frame.h \
        src/InteractorStyle.h \
        src/Message.h \
        src/Pedestrian.h \
        src/SimpleVisualisationWindow.h \
        src/SyncData.h \
        src/SystemSettings.h \
        src/ThreadDataTransfert.h \
        src/ThreadVisualisation.h \
        src/TimerCallback.h \
        src/TrajectoryPoint.h \
        src/extern_var.h \
        src/geometry/FacilityGeometry.h \
        src/geometry/LinePlotter.h \
        src/geometry/Point.h \
        src/geometry/PointPlotter.h \
        src/geometry/LinePlotter2D.h \
        src/geometry/PointPlotter2D.h \
        src/geometry/jul/Building.h \
        src/geometry/jul/Room.h \
        src/geometry/jul/Line.h \
        src/geometry/jul/Transition.h \
        src/geometry/jul/Wall.h \
        src/geometry/jul/CPoint.h \
        src/geometry/jul/Macros.h \
        src/geometry/pg3/CLine.h \
        src/geometry/pg3/CBuilding.h \
        src/geometry/pg3/CRoom.h \
        src/geometry/pg3/CCell.h \
        src/geometry/pg3/CCoordsList.h \
        src/geometry/pg3/Common.h \
        src/geometry/pg3/CTextLog.h \
        src/geometry/pg3/CTransition.h \
        src/network/TraVisToServer.h
        
    SOURCES += forms/Settings.cpp \
        src/SaxParser.cpp \
        src/Debug.cpp \
        src/main.cpp \
        src/travisto.cpp \
        src/Frame.cpp \
        src/InteractorStyle.cpp \
        src/Pedestrian.cpp \
        src/SimpleVisualisationWindow.cpp \
        src/SyncData.cpp \
        src/SystemSettings.cpp \
        src/ThreadDataTransfert.cpp \
        src/ThreadVisualisation.cpp \
        src/TimerCallback.cpp \
        src/TrajectoryPoint.cpp \
        src/geometry/LinePlotter2D.cpp \
        src/geometry/PointPlotter2D.cpp \
        src/geometry/FacilityGeometry.cpp \
        src/geometry/LinePlotter.cpp \
        src/geometry/Point.cpp \
        src/geometry/PointPlotter.cpp \
        src/geometry/jul/Building.cpp \
        src/geometry/jul/Room.cpp \
        src/geometry/jul/Line.cpp \
        src/geometry/jul/Transition.cpp \
        src/geometry/jul/Wall.cpp \
        src/geometry/jul/CPoint.cpp \
        src/geometry/pg3/CLine.cpp \
        src/geometry/pg3/CBuilding.cpp \
        src/geometry/pg3/CRoom.cpp \
        src/geometry/pg3/CCell.cpp \
        src/geometry/pg3/CCoordsList.cpp \
        src/geometry/pg3/Common.cpp \
        src/geometry/pg3/CTextLog.cpp \
        src/geometry/pg3/CTransition.cpp \
        src/network/TraVisToServer.cpp
    FORMS += forms/settings.ui \
        forms/travisto.ui
    RESOURCES += forms/icons.qrc \
        forms/icons.qrc \
        forms/icons.qrc
    RC_FILE = forms/travisto.rc
