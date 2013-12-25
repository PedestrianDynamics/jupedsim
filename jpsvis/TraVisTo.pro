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
    INCLUDEPATH += /usr/local/include/vtk-6.0
    
    LIBS += -L/usr/local/lib/ \
            -lvtkalglib-6.0  \
            -lvtkChartsCore-6.0  \
            -lvtkCommonColor-6.0  \
            -lvtkCommonComputationalGeometry-6.0  \
            -lvtkCommonCore-6.0  \
            -lvtkCommonDataModel-6.0  \
            -lvtkCommonExecutionModel-6.0  \
            -lvtkCommonMath-6.0  \
            -lvtkCommonMisc-6.0  \
            -lvtkCommonSystem-6.0  \
            -lvtkCommonTransforms-6.0  \
            -lvtkDICOMParser-6.0  \
            -lvtkDomainsChemistry-6.0  \
            -lvtkexoIIc-6.0  \
            -lvtkexpat-6.0  \
            -lvtkFiltersAMR-6.0  \
            -lvtkFiltersCore-6.0  \
            -lvtkFiltersExtraction-6.0  \
            -lvtkFiltersFlowPaths-6.0  \
            -lvtkFiltersGeneral-6.0  \
            -lvtkFiltersGeneric-6.0  \
            -lvtkFiltersGeometry-6.0  \
            -lvtkFiltersHybrid-6.0  \
            -lvtkFiltersHyperTree-6.0  \
            -lvtkFiltersImaging-6.0  \
            -lvtkFiltersModeling-6.0  \
            -lvtkFiltersParallel-6.0  \
            -lvtkFiltersParallelImaging-6.0  \
            -lvtkFiltersProgrammable-6.0  \
            -lvtkFiltersSelection-6.0  \
            -lvtkFiltersSources-6.0  \
            -lvtkFiltersStatistics-6.0  \
            -lvtkFiltersTexture-6.0  \
            -lvtkFiltersVerdict-6.0  \
            -lvtkfreetype-6.0  \
            -lvtkftgl-6.0  \
            -lvtkGeovisCore-6.0  \
            -lvtkgl2ps-6.0  \
            -lvtkGUISupportQt-6.0  \
            -lvtkGUISupportQtOpenGL-6.0  \
            -lvtkGUISupportQtSQL-6.0  \
            -lvtkGUISupportQtWebkit-6.0  \
            -lvtkhdf5-6.0  \
            -lvtkhdf5_hl-6.0  \
            -lvtkImagingColor-6.0  \
            -lvtkImagingCore-6.0  \
            -lvtkImagingFourier-6.0  \
            -lvtkImagingGeneral-6.0  \
            -lvtkImagingHybrid-6.0  \
            -lvtkImagingMath-6.0  \
            -lvtkImagingMorphological-6.0  \
            -lvtkImagingSources-6.0  \
            -lvtkImagingStatistics-6.0  \
            -lvtkImagingStencil-6.0  \
            -lvtkInfovisCore-6.0  \
            -lvtkInfovisLayout-6.0  \
            -lvtkInteractionImage-6.0  \
            -lvtkInteractionStyle-6.0  \
            -lvtkInteractionWidgets-6.0  \
            -lvtkIOAMR-6.0  \
            -lvtkIOCore-6.0  \
            -lvtkIOEnSight-6.0  \
            -lvtkIOExodus-6.0  \
            -lvtkIOExport-6.0  \
            -lvtkIOGeometry-6.0  \
            -lvtkIOImage-6.0  \
            -lvtkIOImport-6.0  \
            -lvtkIOInfovis-6.0  \
            -lvtkIOLegacy-6.0  \
            -lvtkIOLSDyna-6.0  \
            -lvtkIOMINC-6.0  \
            -lvtkIOMovie-6.0  \
            -lvtkIONetCDF-6.0  \
            -lvtkIOParallel-6.0  \
            -lvtkIOPLY-6.0  \
            -lvtkIOSQL-6.0  \
            -lvtkIOVideo-6.0  \
            -lvtkIOXML-6.0  \
            -lvtkIOXMLParser-6.0  \
            -lvtkjpeg-6.0  \
            -lvtkjsoncpp-6.0  \
            -lvtklibxml2-6.0  \
            -lvtkmetaio-6.0  \
            -lvtkNetCDF-6.0  \
            -lvtkNetCDF_cxx-6.0  \
            -lvtkoggtheora-6.0  \
            -lvtkParallelCore-6.0  \
            -lvtkpng-6.0  \
            -lvtkproj4-6.0  \
            -lvtkRenderingAnnotation-6.0  \
            -lvtkRenderingContext2D-6.0  \
            -lvtkRenderingCore-6.0  \
            -lvtkRenderingFreeType-6.0  \
            -lvtkRenderingFreeTypeOpenGL-6.0  \
            -lvtkRenderingGL2PS-6.0  \
            -lvtkRenderingHybridOpenGL-6.0  \
            -lvtkRenderingImage-6.0  \
            -lvtkRenderingLabel-6.0  \
            -lvtkRenderingLOD-6.0  \
            -lvtkRenderingOpenGL-6.0  \
            -lvtkRenderingQt-6.0  \
            -lvtkRenderingVolume-6.0  \
            -lvtkRenderingVolumeAMR-6.0  \
            -lvtkRenderingVolumeOpenGL-6.0  \
            -lvtksqlite-6.0  \
            -lvtksys-6.0  \
            -lvtktiff-6.0  \
            -lvtkverdict-6.0  \
            -lvtkViewsContext2D-6.0  \
            -lvtkViewsCore-6.0  \
            -lvtkViewsGeovis-6.0  \
            -lvtkViewsInfovis-6.0  \
            -lvtkViewsQt-6.0  \
            -lvtkzlib-6.0  \
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
