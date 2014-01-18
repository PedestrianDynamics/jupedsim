TEMPLATE = app
TARGET = TraVisTo
CONFIG += qt
QT += xml \
    network
greaterThan(QT_MAJOR_VERSION, 4):QT += widgets
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
        -lvtkInfovis \
        -lvtkViews \
        -lwsock32 \
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
    HEADERS += src/geometry/Building.h \
        src/geometry/Crossing.h \
        src/geometry/Goal.h \
        src/geometry/Hline.h \
        src/geometry/Line.h \
        src/geometry/NavLine.h \
        src/geometry/Obstacle.h \
        src/geometry/Point.h \
        src/geometry/Room.h \
        src/geometry/SubRoom.h \
        src/geometry/Transition.h \
        src/geometry/Wall.h \
        src/geometry/JPoint.h \
        src/tinyxml/tinystr.h \
        src/tinyxml/tinyxml.h \
        src/general/Macros.h \
        src/IO/OutputHandler.h \
        src/IO/TraVisToClient.h \
        forms/Settings.h \
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
        src/geometry/PointPlotter.h \
        src/geometry/LinePlotter2D.h \
        src/geometry/PointPlotter2D.h \
        src/network/TraVisToServer.h
        
    SOURCES += src/geometry/Building.cpp \
        src/geometry/Crossing.cpp \
        src/geometry/Goal.cpp \
        src/geometry/Hline.cpp \
        src/geometry/Line.cpp \
        src/geometry/NavLine.cpp \
        src/geometry/Obstacle.cpp \
        src/geometry/Point.cpp \
        src/geometry/Room.cpp \
        src/geometry/SubRoom.cpp \
        src/geometry/Transition.cpp \
        src/geometry/Wall.cpp \
        src/geometry/JPoint.cpp \
        src/tinyxml/tinystr.cpp \
        src/tinyxml/tinyxml.cpp \
        src/tinyxml/tinyxmlerror.cpp \
        src/tinyxml/tinyxmlparser.cpp \
        src/IO/OutputHandler.cpp \
        src/IO/TraVisToClient.cpp \
        forms/Settings.cpp \
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
        src/geometry/PointPlotter.cpp \
        src/network/TraVisToServer.cpp
    FORMS += forms/settings.ui \
        forms/travisto.ui
    RESOURCES += forms/icons.qrc \
        forms/icons.qrc \
        forms/icons.qrc
    RC_FILE = forms/travisto.rc
