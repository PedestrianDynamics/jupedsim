TEMPLATE = app
TARGET = JPSvis
CONFIG += qt
CONFIG += static
CONFIG += c++11
QT += xml network

#avoid some annoying dependencies
#QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
#QMAKE_LFLAGS += -static-libstdc++ -lstdc++
#QMAKE_CXXFLAGS += -static -static -static-libstdc++
#QMAKE_CXXFLAGS += -static
#QMAKE_LFLAGS_RELEASE += -static-libgcc

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wno-deprecated -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wno-unknown-pragmas

greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

#Dynamic linking
#Windows  VTK 6.1
win32_old {
        INCLUDEPATH += C:/VTK/VTK6.1/include
        LIBS += -LC:/VTK/VTK6.1/bin_shared \
        -lwsock32 \
        -lvtkCommonCore-6.1    \
        -lvtkRenderingOpenGL-6.1  \
        -lvtkCommonDataModel-6.1    \
        -lvtkCommonMath-6.1   \
        -lvtkCommonMisc-6.1    \
        -lvtkCommonSystem-6.1    \
        -lvtkCommonTransforms-6.1   \
        -lvtkCommonExecutionModel-6.1   \
        -lvtkImagingHybrid-6.1   \
        -lvtkIOImage-6.1   \
        -lvtkDICOMParser-6.1   \
        -lvtkjpeg-6.1   \
        -lvtkmetaio-6.1   \
        -lvtkzlib-6.1   \
        -lvtkpng-6.1   \
        -lvtktiff-6.1   \
        -lvtkRenderingCore-6.1   \
        -lvtkCommonComputationalGeometry-6.1   \
        -lvtkFiltersCore-6.1   \
        -lvtkFiltersExtraction-6.1   \
        -lvtkFiltersGeneral-6.1   \
        -lvtkFiltersStatistics-6.1   \
        -lvtkalglib-6.1   \
        -lvtkImagingFourier-6.1   \
        -lvtkImagingCore-6.1   \
        -lvtkFiltersGeometry-6.1   \
        -lvtkFiltersSources-6.1   \
        -lvtkRenderingLabel-6.1   \
        -lvtkRenderingFreeType-6.1   \
        -lvtkfreetype-6.1   \
        -lvtkftgl-6.1   \
        -lvtkRenderingFreeTypeOpenGL-6.1   \
        -lvtkRenderingAnnotation-6.1   \
        -lvtkIOMovie-6.1   \
        -lvtkoggtheora-6.1   \
        -lvtkInteractionStyle-6.1   \
        -lvtkCommonCore-6.1    \
        -lvtksys-6.1     \
        }

#Static linking
#Windows  VTK 6.1
win32_old {
        INCLUDEPATH += C:/VTK/VTK6.1/include
        LIBS += -LC:/VTK/VTK6.1/bin_static \
            -lvtkCommonCore-6.1  \
            -lvtkalglib-6.1  \
            -lvtkChartsCore-6.1  \
            -lvtkCommonColor-6.1  \
            -lvtkCommonComputationalGeometry-6.1  \
            -lvtkCommonDataModel-6.1  \
            -lvtkCommonExecutionModel-6.1  \
            -lvtkCommonMath-6.1  \
            -lvtkCommonMisc-6.1  \
            -lvtkCommonSystem-6.1  \
            -lvtkCommonTransforms-6.1  \
            -lvtkDICOMParser-6.1  \
            -lvtkDomainsChemistry-6.1  \
            -lvtkexoIIc-6.1  \
            -lvtkexpat-6.1  \
            -lvtkFiltersAMR-6.1  \
            -lvtkFiltersCore-6.1  \
            -lvtkFiltersExtraction-6.1  \
            -lvtkFiltersFlowPaths-6.1  \
            -lvtkFiltersGeneral-6.1  \
            -lvtkFiltersGeneric-6.1  \
            -lvtkFiltersGeometry-6.1  \
            -lvtkFiltersHybrid-6.1  \
            -lvtkFiltersHyperTree-6.1  \
            -lvtkFiltersImaging-6.1  \
            -lvtkFiltersModeling-6.1  \
            -lvtkFiltersParallel-6.1  \
            -lvtkFiltersParallelImaging-6.1  \
            -lvtkFiltersProgrammable-6.1  \
            -lvtkFiltersSelection-6.1  \
            -lvtkFiltersSources-6.1  \
            -lvtkFiltersStatistics-6.1  \
            -lvtkFiltersTexture-6.1  \
            -lvtkFiltersVerdict-6.1  \
            -lvtkfreetype-6.1  \
            -lvtkftgl-6.1  \
            -lvtkGeovisCore-6.1  \
            -lvtkgl2ps-6.1  \
            -lvtkGUISupportQt-6.1  \
            -lvtkGUISupportQtOpenGL-6.1  \
            -lvtkGUISupportQtSQL-6.1  \
            -lvtkGUISupportQtWebkit-6.1  \
            -lvtkhdf5-6.1  \
            -lvtkhdf5_hl-6.1  \
            -lvtkImagingColor-6.1  \
            -lvtkImagingCore-6.1  \
            -lvtkImagingFourier-6.1  \
            -lvtkImagingGeneral-6.1  \
            -lvtkImagingHybrid-6.1  \
            -lvtkImagingMath-6.1  \
            -lvtkImagingMorphological-6.1  \
            -lvtkImagingSources-6.1  \
            -lvtkImagingStatistics-6.1  \
            -lvtkImagingStencil-6.1  \
            -lvtkInfovisCore-6.1  \
            -lvtkInfovisLayout-6.1  \
            -lvtkInteractionImage-6.1  \
            -lvtkInteractionStyle-6.1  \
            -lvtkInteractionWidgets-6.1  \
            -lvtkIOAMR-6.1  \
            -lvtkIOCore-6.1  \
            -lvtkIOEnSight-6.1  \
            -lvtkIOExodus-6.1  \
            -lvtkIOExport-6.1  \
            -lvtkIOGeometry-6.1  \
            -lvtkIOImage-6.1  \
            -lvtkIOImport-6.1  \
            -lvtkIOInfovis-6.1  \
            -lvtkIOLegacy-6.1  \
            -lvtkIOLSDyna-6.1  \
            -lvtkIOMINC-6.1  \
            -lvtkIOMovie-6.1  \
            -lvtkIONetCDF-6.1  \
            -lvtkIOParallel-6.1  \
            -lvtkIOPLY-6.1  \
            -lvtkIOSQL-6.1  \
            -lvtkIOVideo-6.1  \
            -lvtkIOXML-6.1  \
            -lvtkIOXMLParser-6.1  \
            -lvtkjpeg-6.1  \
            -lvtkjsoncpp-6.1  \
            -lvtklibxml2-6.1  \
            -lvtkmetaio-6.1  \
            -lvtkNetCDF-6.1  \
            -lvtkNetCDF_cxx-6.1  \
            -lvtkoggtheora-6.1  \
            -lvtkParallelCore-6.1  \
            -lvtkpng-6.1  \
            -lvtkproj4-6.1  \
            -lvtkRenderingAnnotation-6.1  \
            -lvtkRenderingContext2D-6.1  \
            -lvtkRenderingCore-6.1  \
            -lvtkRenderingFreeType-6.1  \
            -lvtkRenderingFreeTypeOpenGL-6.1  \
            -lvtkRenderingGL2PS-6.1  \
#            -lvtkRenderingHybridOpenGL-6.1  \
            -lvtkRenderingImage-6.1  \
            -lvtkRenderingLabel-6.1  \
            -lvtkRenderingLOD-6.1  \
            -lvtkRenderingOpenGL-6.1  \
            -lvtkRenderingQt-6.1  \
            -lvtkRenderingVolume-6.1  \
            -lvtkRenderingVolumeAMR-6.1  \
            -lvtkRenderingVolumeOpenGL-6.1  \
 #           -lvtksqlite-6.1  \
            -lvtksys-6.1  \
            -lvtktiff-6.1  \
            -lvtkverdict-6.1  \
            -lvtkViewsContext2D-6.1  \
            -lvtkViewsCore-6.1  \
            -lvtkViewsGeovis-6.1  \
            -lvtkViewsInfovis-6.1  \
            -lvtkViewsQt-6.1  \
            -lvtkzlib-6.1  \
            -lwsock32 -lglu32 -lvfw32 -lgdi32  -lopengl32  -lws2_32 -lgdi32 \
            -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 \
            -luuid -lcomdlg32 -ladvapi32\
        }
	
#Dynamic linking
#Windows  VTK 5.10
win32_old {
        INCLUDEPATH += D:/VTK/VTK5.1/include
        LIBS += -LD:/VTK/VTK5.1/bin_shared \
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
            -lvtkHybrid \
            -lvtkWidgets \
            -lvtkInfovis\
            -lvtkViews\
            -lwsock32 -lglu32 -lvfw32 -lgdi32  -lopengl32  -lws2_32 -lgdi32 \
            -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 \
            -luuid -lcomdlg32 -ladvapi32\
}

#Static linking
#VTK 5.10 Windows
win32 {
    INCLUDEPATH += C:/VTK/VTK5.1/static/include
    LIBS += -LC:\VTK\VTK5.1\static\lib \
    C:\VTK\VTK5.1\static\bin/libvtkCharts.a C:\VTK\VTK5.1\static\bin/libvtkViews.a \
    C:\VTK\VTK5.1\static\bin/libvtkInfovis.a C:\VTK\VTK5.1\static\bin/libvtkWidgets.a \
    C:\VTK\VTK5.1\static\bin/libvtkHybrid.a C:\VTK\VTK5.1\static\bin/libvtkVolumeRendering.a \
    C:\VTK\VTK5.1\static\bin/libvtkRendering.a \
    C:\VTK\VTK5.1\static\bin/libvtkGraphics.a C:\VTK\VTK5.1\static\bin/libvtkverdict.a \
    C:\VTK\VTK5.1\static\bin/libvtkImaging.a C:\VTK\VTK5.1\static\bin/libvtkIO.a \
    C:\VTK\VTK5.1\static\bin/libvtkFiltering.a C:\VTK\VTK5.1\static\bin/libvtkDICOMParser.a \
    C:\VTK\VTK5.1\static\bin/libvtkNetCDF_cxx.a C:\VTK\VTK5.1\static\bin/libvtkmetaio.a \
    C:\VTK\VTK5.1\static\bin/libvtksqlite.a C:\VTK\VTK5.1\static\bin/libvtkpng.a \
    C:\VTK\VTK5.1\static\bin/libvtktiff.a C:\VTK\VTK5.1\static\bin/libvtkjpeg.a \
    C:\VTK\VTK5.1\static\bin/libvtkexpat.a  \
    C:\VTK\VTK5.1\static\bin/libvtkCommon.a \
    C:\VTK\VTK5.1\static\bin/libLSDyna.a C:\VTK\VTK5.1\static\bin/libvtksys.a \
    C:\VTK\VTK5.1\static\bin/libvtkexoIIc.a C:\VTK\VTK5.1\static\bin/libvtkNetCDF.a \
    C:\VTK\VTK5.1\static\bin/libvtkhdf5_hl.a C:\VTK\VTK5.1\static\bin/libvtkhdf5.a \
    C:\VTK\VTK5.1\static\bin/libvtklibxml2.a C:\VTK\VTK5.1\static\bin/libvtkzlib.a \
    C:\VTK\VTK5.1\static\bin/libvtkalglib.a C:\VTK\VTK5.1\static\bin/libvtkftgl.a \
    C:\VTK\VTK5.1\static\bin/libvtkfreetype.a       \
    -lwsock32 -lglu32 -lvfw32 -lgdi32  -lopengl32  -lws2_32 -lgdi32 \
    -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 \
    -luuid -lcomdlg32 -ladvapi32\
}

unix_6 {
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
   
#dynamic linking with vtk5.8
unix{
INCLUDEPATH += /usr/include/vtk-5.8
LIBS += -L/usr/lib \
    -lvtkRendering  \
    -lvtkCommon   \
    -lvtkHybrid   \
    -lvtkIO   \
    -lvtkGraphics   \
    -lvtkFiltering   \
    -lvtkCommon   \
    -lvtkverdict   \
    -lvtkParallel   \
    -lvtkexoIIc   \
    -lvtkImaging   \
    -lvtkDICOMParser   \
    -lvtkmetaio   \
    -lvtkftgl  \
    -lvtkViews \
    -lvtksys   \
    #-lvtkpng \
    #-lvtktiff \
    #-lvtkjpeg \
    #-lvtklibxml2 \
    #-lvtkzlib \
    #-lvtkexpat \
    #-lvtkfreetype \
 }
 
#Static compilation linux
unix_static {
#INCLUDEPATH += /usr/include/vtk-5.8
#LIBS += -L/usr/lib \
    INCLUDEPATH +=/usr/local/include/vtk-5.10
    LIBS += -L/usr/local/lib/vtk-5.10 \
    -lvtkRendering  \
    -lvtkCommon   \
    -lvtkHybrid   \
    -lvtkIO   \
    -lvtkGraphics   \
    -lvtkFiltering   \
    -lvtkCommon   \
    -lvtkverdict   \
    -lvtkParallel   \
    -lvtkexoIIc   \
    -lvtkImaging   \
    -lvtkDICOMParser   \
    -lvtkmetaio   \
    -lvtkftgl  \
    -lLSDyna \
    -lvtkViews \
    -lvtksys   \
    -lvtkpng \
    -lvtktiff \
    -lvtkjpeg \
    -lvtklibxml2 \
    -lvtkzlib \
    -lvtkexpat \
    -lvtkfreetype \
    -lGL  \
    -lXt \
    -lX11 \
    -lXext \
    -ldl \
 }

macx{
        INCLUDEPATH += /Users/piccolo/VTK/include/vtk-6.1
        LIBS += -L/Users/piccolo/VTK/lib \
            -lvtkalglib-6.1  \
            -lvtkChartsCore-6.1  \
            -lvtkCommonColor-6.1  \
            -lvtkCommonComputationalGeometry-6.1  \
            -lvtkCommonCore-6.1  \
            -lvtkCommonDataModel-6.1  \
            -lvtkCommonExecutionModel-6.1  \
            -lvtkCommonMath-6.1  \
            -lvtkCommonMisc-6.1  \
            -lvtkCommonSystem-6.1  \
            -lvtkCommonTransforms-6.1  \
            -lvtkDICOMParser-6.1  \
            -lvtkDomainsChemistry-6.1  \
            -lvtkexoIIc-6.1  \
            -lvtkexpat-6.1  \
            -lvtkFiltersAMR-6.1  \
            -lvtkFiltersCore-6.1  \
            -lvtkFiltersExtraction-6.1  \
            -lvtkFiltersFlowPaths-6.1  \
            -lvtkFiltersGeneral-6.1  \
            -lvtkFiltersGeneric-6.1  \
            -lvtkFiltersGeometry-6.1  \
            -lvtkFiltersHybrid-6.1  \
            -lvtkFiltersHyperTree-6.1  \
            -lvtkFiltersImaging-6.1  \
            -lvtkFiltersModeling-6.1  \
            -lvtkFiltersParallel-6.1  \
            -lvtkFiltersParallelImaging-6.1  \
            -lvtkFiltersProgrammable-6.1  \
            -lvtkFiltersSelection-6.1  \
            -lvtkFiltersSources-6.1  \
            -lvtkFiltersStatistics-6.1  \
            -lvtkFiltersTexture-6.1  \
            -lvtkFiltersVerdict-6.1  \
            -lvtkfreetype-6.1  \
            -lvtkftgl-6.1  \
            -lvtkGeovisCore-6.1  \
            -lvtkgl2ps-6.1  \
            -lvtkGUISupportQt-6.1  \
            -lvtkGUISupportQtOpenGL-6.1  \
            -lvtkGUISupportQtSQL-6.1  \
            -lvtkGUISupportQtWebkit-6.1  \
            -lvtkhdf5-6.1  \
            -lvtkhdf5_hl-6.1  \
            -lvtkImagingColor-6.1  \
            -lvtkImagingCore-6.1  \
            -lvtkImagingFourier-6.1  \
            -lvtkImagingGeneral-6.1  \
            -lvtkImagingHybrid-6.1  \
            -lvtkImagingMath-6.1  \
            -lvtkImagingMorphological-6.1  \
            -lvtkImagingSources-6.1  \
            -lvtkImagingStatistics-6.1  \
            -lvtkImagingStencil-6.1  \
            -lvtkInfovisCore-6.1  \
            -lvtkInfovisLayout-6.1  \
            -lvtkInteractionImage-6.1  \
            -lvtkInteractionStyle-6.1  \
            -lvtkInteractionWidgets-6.1  \
            -lvtkIOAMR-6.1  \
            -lvtkIOCore-6.1  \
            -lvtkIOEnSight-6.1  \
            -lvtkIOExodus-6.1  \
            -lvtkIOExport-6.1  \
            -lvtkIOGeometry-6.1  \
            -lvtkIOImage-6.1  \
            -lvtkIOImport-6.1  \
            -lvtkIOInfovis-6.1  \
            -lvtkIOLegacy-6.1  \
            -lvtkIOLSDyna-6.1  \
            -lvtkIOMINC-6.1  \
            -lvtkIOMovie-6.1  \
            -lvtkIONetCDF-6.1  \
            -lvtkIOParallel-6.1  \
            -lvtkIOPLY-6.1  \
            -lvtkIOSQL-6.1  \
            -lvtkIOVideo-6.1  \
            -lvtkIOXML-6.1  \
            -lvtkIOXMLParser-6.1  \
            -lvtkjpeg-6.1  \
            -lvtkjsoncpp-6.1  \
            -lvtklibxml2-6.1  \
            -lvtkmetaio-6.1  \
            -lvtkNetCDF-6.1  \
            -lvtkNetCDF_cxx-6.1  \
            -lvtkoggtheora-6.1  \
            -lvtkParallelCore-6.1  \
            -lvtkpng-6.1  \
            -lvtkproj4-6.1  \
            -lvtkRenderingAnnotation-6.1  \
            -lvtkRenderingContext2D-6.1  \
            -lvtkRenderingCore-6.1  \
            -lvtkRenderingFreeType-6.1  \
            -lvtkRenderingFreeTypeOpenGL-6.1  \
            -lvtkRenderingGL2PS-6.1  \
#            -lvtkRenderingHybridOpenGL-6.1  \
            -lvtkRenderingImage-6.1  \
            -lvtkRenderingLabel-6.1  \
            -lvtkRenderingLOD-6.1  \
            -lvtkRenderingOpenGL-6.1  \
            -lvtkRenderingQt-6.1  \
            -lvtkRenderingVolume-6.1  \
            -lvtkRenderingVolumeAMR-6.1  \
            -lvtkRenderingVolumeOpenGL-6.1  \
 #           -lvtksqlite-6.1  \
            -lvtksys-6.1  \
            -lvtktiff-6.1  \
            -lvtkverdict-6.1  \
            -lvtkViewsContext2D-6.1  \
            -lvtkViewsCore-6.1  \
            -lvtkViewsGeovis-6.1  \
            -lvtkViewsInfovis-6.1  \
            -lvtkViewsQt-6.1  \
            -lvtkzlib-6.1  \
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
    src/FrameElement.h \
    src/extern_var.h \
    src/geometry/FacilityGeometry.h \
    src/geometry/LinePlotter.h \
    src/geometry/PointPlotter.h \
    src/geometry/LinePlotter2D.h \
    src/geometry/PointPlotter2D.h \
    src/network/TraVisToServer.h \
    src/MainWindow.h \
    src/TrailPlotter.h

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
    src/Frame.cpp \
    src/InteractorStyle.cpp \
    src/Pedestrian.cpp \
    src/SimpleVisualisationWindow.cpp \
    src/SyncData.cpp \
    src/SystemSettings.cpp \
    src/ThreadDataTransfert.cpp \
    src/ThreadVisualisation.cpp \
    src/TimerCallback.cpp \
    src/FrameElement.cpp \
    src/geometry/LinePlotter2D.cpp \
    src/geometry/PointPlotter2D.cpp \
    src/geometry/FacilityGeometry.cpp \
    src/geometry/LinePlotter.cpp \
    src/geometry/PointPlotter.cpp \
    src/network/TraVisToServer.cpp \
    src/MainWindow.cpp \
    src/TrailPlotter.cpp

FORMS += forms/settings.ui \
    forms/mainwindow.ui
RESOURCES += forms/icons.qrc
RC_FILE = forms/jpsvis.rc
#osx fix
macx:ICON = forms/jpsvis.icns
