/**
 * @file MainWindow.cpp
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * \brief main program class
 *
 *
 *  Created on: 11.05.2009
 *
 */


#include "MainWindow.h"
#include "SaxParser.h"
#include "SystemSettings.h"
#include "./forms/Settings.h"
#include <sstream>


#include "extern_var.h"
#include "TrajectoryPoint.h"
#include "Frame.h"
#include "Debug.h"
#include "ThreadVisualisation.h"
#include "ThreadDataTransfert.h"
#include "SimpleVisualisationWindow.h"

#include "geometry/FacilityGeometry.h"

#include <QMessageBox>
#include <QtXml/QDomDocument>
#include <QTime>
#include <QApplication>
#include <QCloseEvent>
#include <QThread>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QString>
#include <QColorDialog>
#include <QtXml>
#include <QTemporaryFile>
#include <QListView>
#include <QFileSystemModel>
#include <QTableView>
#include <QSplitter>
#include <QStandardItemModel>


#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

//----------- @todo: this part is copy/paste from main.cpp.
std::string ver_string1(int a, int b, int c) {
      std::ostringstream ss;
      ss << a << '.' << b << '.' << c;
      return ss.str();
}

std::string true_cxx1 =
#ifdef __clang__
      "clang++";
#elif defined(__GNUC__)
"g++";
#elif defined(__MINGW32__)
   "MinGW";
#elif defined(_MSC_VER)
  "Visual Studio";
#else
"Compiler not identified";
#endif

std::string true_cxx_ver1 =
#ifdef __clang__
    ver_string1(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    ver_string1(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__MINGW32__)
ver_string1(__MINGW32__, __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
#elif defined( _MSC_VER)
    ver_string1(_MSC_VER, _MSC_FULL_VER,_MSC_BUILD);
#else
"";
#endif
//----------

#ifdef __APPLE__
#include <thread>
#include <dispatch/dispatch.h>
#include "fix/osx_thread_fix.h"

std::thread::id main_thread_id = std::this_thread::get_id();
dispatch_queue_t main_q = dispatch_get_main_queue();

void is_main_thread() {

    if ( main_thread_id == std::this_thread::get_id() )
    {
         std::cout << "This is the main thread.\n";
    }
    else
        std::cout << "This is not the main thread.\n";
}
#endif


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowTitle("JPSvis");

    //disable the maximize Button
    setWindowFlags( Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    //used for saving the settings in a persistant way
    QCoreApplication::setOrganizationName("Forschungszentrum_Juelich_GmbH");
    QCoreApplication::setOrganizationDomain("jupedsim.org");
    QCoreApplication::setApplicationName("jupedsim");

    //create the 2 threads and connect them
    dataTransferThread = new ThreadDataTransfer(this);
    _visualisationThread = new ThreadVisualisation(this);


    travistoOptions = new Settings(this);
    travistoOptions->setWindowTitle("Settings");
    travistoOptions->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);


    if(!QObject::connect(dataTransferThread,
                         SIGNAL(signal_controlSequence(const char*)), this,
                         SLOT(slotControlSequence(const char *)))) {
        Debug::Error("dataTransferThread Thread: control sequence could not be connected");
    }

    if(!QObject::connect(dataTransferThread,
                         SIGNAL(signal_startVisualisationThread(QString,int,float )), this,
                         SLOT(slotStartVisualisationThread(QString,int,float )))) {
        Debug::Error(" signal_startVisualisationThread not connected");
    }

    if(!QObject::connect(dataTransferThread,
                         SIGNAL(signal_stopVisualisationThread(bool )), this,
                         SLOT(slotShutdownVisualisationThread(bool )))) {
        Debug::Error(" signal_stopVisualisationThread not connected ");
    }

    if(!QObject::connect(dataTransferThread,
                         SIGNAL(signal_errorMessage(QString)), this,
                         SLOT(slotErrorOutput(QString)))) {
        Debug::Error("signal_errorMessage  not connected ");
    }

    QObject::connect(_visualisationThread,
                     SIGNAL(signal_controlSequences(const char*)), this,
                     SLOT(slotControlSequence(const char *)));

    QObject::connect(&extern_trajectories_firstSet,
                     SIGNAL(signal_controlSequences(const char*)), this,
                     SLOT(slotControlSequence(const char *)));

    QObject::connect(dataTransferThread, SIGNAL(signal_CurrentAction(QString)),
                     this, SLOT(slotCurrentAction(QString)));

    QObject::connect(this, SIGNAL(signal_controlSequence(QString)),
                     travistoOptions, SLOT(slotControlSequence(QString)));


    QObject::connect(&_visualisationThread->getGeometry().GetModel(),
                     SIGNAL(itemChanged(QStandardItem*)),this,SLOT(slotOnGeometryItemChanged(QStandardItem*)));

    isPlaying = false;
    isPaused = false;
    numberOfDatasetLoaded=0;
    frameSliderHold=false;

    //some hand made stuffs
    ui.BtFullscreen->setVisible(false);

    labelCurrentAction = new QLabel();
    labelCurrentAction->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelCurrentAction->setText("   Idle   ");
    statusBar()->addPermanentWidget(labelCurrentAction);

    labelFrameNumber = new QLabel();
    labelFrameNumber->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelFrameNumber->setText("fps:");
    statusBar()->addPermanentWidget(labelFrameNumber);

    labelRecording = new QLabel();
    labelRecording->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelRecording->setText(" rec: off ");
    statusBar()->addPermanentWidget(labelRecording);

    labelMode = new QLabel();
    labelMode->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelMode->setText(" Offline ");
    statusBar()->addPermanentWidget(labelMode);


    ui.BtNextFrame->setVisible(false);
    ui.BtPreviousFrame->setVisible(false);
    //TODO: Maybe this is not longer needed
    ui.framePerSecondSliderLabel->setVisible(false);
    ui.contrastSettingSlider->setVisible(false);
    //restore the settings
    loadAllSettings();

    QStringList arguments =QApplication::arguments();
    int group=1; // there are max 3 groups of pedestrians
    bool mayPlay=false;

    //arguments.append("-online");
    //arguments.append("-caption");
    arguments.append("-2D");
    // parse arguments list
    if(arguments.size()>1)
        for(int argCount=1; argCount<arguments.size(); argCount++) {

            QString argument=arguments[argCount];

            if(argument.compare("help")==0) {
                Debug::Messages("Usage: jpsvis [file1] [-2D] [-caption] [-online [port]]");
                exit(0);
            } else if(argument.compare("-2D")==0) {
                ui.action2_D->setChecked(true);
                slotToogle2D();
            } else if(argument.compare("-caption")==0) {
                ui.actionShow_Captions->setChecked(true);
                slotShowPedestrianCaption();

            } else if(argument.compare("-online")==0) {
                slotSetOnlineMode(true);
                Debug::Messages("Online option parsed");
                // checking for other possible options [ port...]
                if(argCount!=arguments.size()-1) {
                    bool ok=false;
                    int port = arguments[++argCount].toInt(&ok);
                    Debug::Messages(" listening port: %d",port);

                    if (ok) {
                        SystemSettings::setListeningPort(port);
                    } else { // maybe that wasnt the port
                        argCount--; // to ensure that the "option" will be checked
                    }
                }
                //mayPlay=true;

            } else if(argument.startsWith("-")) {
                const char* std=argument.toStdString().c_str();
                Debug::Error("unknown options: %s",std);
                Debug::Error("Usage: jpsvis [file1] [-2D] [-caption] [-online [port] ]");
            } else if(addPedestrianGroup(group,argument)) {
                //slotHelpAbout();
                 Debug::Messages("group: %d, arg: %s", group, argument.toStdString().c_str());
                group++;
                mayPlay=true;
            }

        }
    Debug::Messages("MayPlay: %s", mayPlay?"True":"False");
    // was call from the command line with a file.
    // disable the online mode if it was enabled
    if(mayPlay)
    {
        slotSetOfflineMode(true);
        slotStartPlaying();
    }
}

MainWindow::~MainWindow()
{
    extern_shutdown_visual_thread=true;
    extern_recording_enable=false;

    //save all settings for the next session
    if(ui.actionRemember_Settings->isChecked())
    {
        saveAllSettings();
        Debug::Messages("saving all settings");
    }
    else
    {
        //first clear everyting
        QSettings settings;
        settings.clear();
        //then remember that we do not want any settings saved
        settings.setValue("options/rememberSettings", false);
        Debug::Messages("clearing all settings");
    }


    if (_visualisationThread->isRunning()) {
        //std::Debug::Messages("Thread  visualisation is still running" << std::endl;
        waitForVisioThread();
    }

    if (dataTransferThread->isRunning()) {
        //std::Debug::Messages("Thread  visualisation is still running" << std::endl;
        //it is your own fault if you were still recording something
        waitForDataThread();
    }

    delete dataTransferThread;
    delete _visualisationThread;
    delete travistoOptions;
    delete labelCurrentAction;
    delete labelFrameNumber;
    delete labelRecording;
    delete labelMode;
}

void MainWindow::slotHelpAbout()
{

     Debug::Messages("About JPSvis");
     QString gittext = QMessageBox::tr(
          "<p style=\"line-height:1.4\" style=\"color:Gray;\"><small><i>Version %1</i></small></p>"
          "<p style=\"line-height:0.4\" style=\"color:Gray;\"><i>CommHash</i> %2</p>"
          "<p  style=\"line-height:0.4\" style=\"color:Gray;\"><i>CommDate</i> %3</p>"
          "<p  style=\"line-height:0.4\" style=\"color:Gray;\"><i>Branch</i> %4</p><hr>"
          ).arg(JPSVIS_VERSION).arg(GIT_COMMIT_HASH).arg(GIT_COMMIT_DATE).arg(GIT_BRANCH);

     QString text = QMessageBox::tr(
          "<p style=\"color:Gray;\"><small><i> &copy; 2009-2019  Ulrich Kemloh <br><a href=\"http://jupedsim.org\">jupedsim.org</a></i></small></p>"

        );





QMessageBox msg(QMessageBox::Information,"About JPSvis", gittext+text, QMessageBox::Ok);
QString logo = ":/new/iconsS/icons/JPSvis.icns" ;
msg.setIconPixmap(QPixmap(logo));

    //Change font
QFont font( "Tokyo" );
font.setPointSize( 10 );
// font.setWeight( QFont::Bold );
//font.setItalic( TRUE );
msg.setFont(font);
//msg.setStandardButtons(0);
msg.exec();
}


/**
 *setup the network port to bind
 *
 */
void MainWindow::slotNetworkSettings()
{
    bool ok;
    int port = QInputDialog::getInt(this, tr("input a port "), tr(
                                        "port(default to 8989):"), 8989, 5000, 65355, 1, &ok);
    if (ok) {
        SystemSettings::setListeningPort(port);
    }
}

void MainWindow::slotStartPlaying()
{

    //first reset this variable. just for the case
    // the thread was shutdown from the reset option
    extern_shutdown_visual_thread = false;

    if (!isPlaying) {
        statusBar()->showMessage(QString::fromUtf8("waiting for data"));
        //only start the visualisation thread if in offline modus
        // otherwise both threads should be started
        if (extern_offline_mode) {
            if(anyDatasetLoaded()) { //at least one dataset was loaded, restarting the playback

                // starting the playback from the beginning
                extern_trajectories_firstSet.resetFrameCursor();

            } else if (!slotAddDataSet()) { //else load a dataset
                return;//could not read any data
            }
#ifdef __APPLE__
            //std::thread::id main_thread_id = std::this_thread::get_id();
            dispatch_queue_t main_q = dispatch_get_main_queue();
            dispatch_async(main_q, ^(void){
                               //visualisationThread->moveToThread(QApplication::instance()->thread());
                               _visualisationThread->run();
                           });
#else
            _visualisationThread->start();

#endif

        } else { /*if (extern_online_mode)*/  //live visualisation

#ifdef __APPLE__
            dispatch_async(main_q, ^(void){
                               dataTransferThread->run();
                           });
#else
            dataTransferThread->start();

#endif
            //dataTransferThread->start();
            //visualisationThread->start();
        }

        //enable recording
        ui.BtRecord->setEnabled(true);
        labelCurrentAction->setText("   playing   ");

        //change Icon to Pause
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(
                                    ":/new/iconsS/icons/PauseHot.png")), QIcon::Normal, QIcon::Off);
        ui.BtStart->setToolTip("pause the visualisation");
        ui.BtStart->setIcon(icon1);

        //anyDataLoaded=true;
        isPlaying = true;
        extern_is_pause = false;

    } else if (isPaused) { //resume playing
        isPaused = false;
        extern_is_pause = false;
        //restore the play Icon
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(
                                    ":/new/iconsS/icons/PauseHot.png")), QIcon::Normal, QIcon::Off);
        ui.BtStart->setIcon(icon1);
        labelCurrentAction->setText("   playing   ");
        ui.BtStart->setToolTip("pause the visualisation");

    } else { //call pause
        isPaused = true;
        extern_is_pause = true;
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(
                                    ":/new/iconsS/icons/Play1Hot.png")), QIcon::Normal, QIcon::Off);
        ui.BtStart->setIcon(icon1);
        ui.BtStart->setToolTip("resume the visualisation");
        labelCurrentAction->setText("   paused   ");
    }

    //no matter what, the stop button should be enabled
    ui.BtStop->setEnabled(true);
}

void MainWindow::slotStopPlaying()
{

    if (extern_recording_enable) {
        int res = QMessageBox::question(this, "action",
                                        "do you wish to stop the recording?", QMessageBox::Discard
                                        | QMessageBox::Yes, QMessageBox::Yes);
        if (res == QMessageBox::Yes) {
            extern_recording_enable = false;
            //currentAction->setText("   Idle   ");
            labelRecording->setText("rec: OFF");

        } else {
            return;
        }
    }

    //shutdown the threads
    //fixme: shut the network thread as well
    // maybe this is a feature !
    extern_shutdown_visual_thread=true;
    waitForVisioThread();
    //waitForDataThread();

    //reset all frames cursors
    resetAllFrameCursor();

    //disable/reset all graphical elements
    isPlaying = false;
    isPaused = false;
    resetGraphicalElements();
    labelCurrentAction->setText(" Idle ");
}

/**
 * open a file containing visualisation data, parse them
 * and initialize the visualization
 *
 */
bool MainWindow::slotLoadFile()
{
    return slotAddDataSet();
}

// This function is only used in online Mode
void MainWindow::parseGeometry(const QDomNode &geoNode)
{

     Debug::Messages("parsing the geo");
    if(geoNode.isNull()) return ;

    //check if there is a tag 'file' there in
    QString fileName = geoNode.toElement().elementsByTagName("file").item(0).toElement().attribute("location");
    auto&& geometry = _visualisationThread->getGeometry();

    if(!fileName.isEmpty()) {
        if (fileName.endsWith(".xml",Qt::CaseInsensitive)) {
            //parsing the file
            SaxParser::parseGeometryJPS(fileName,geometry);
        } else if (fileName.endsWith(".trav",Qt::CaseInsensitive)) {
            //must not be a file name
            SaxParser::parseGeometryTRAV(fileName,geometry);
        }
    }
    // I assume it is a trav format node,
    //which is the only one which can directly be inserted into a file
    else {
        //cout<<"online geo: "<<geoNode.toDocument().toString().toStdString()<<endl; exit(0);
        //geoNode.toText().toComment().toDocument().toString()
        QDomDocument doc("");
        QDomNode geoNode;
        if(!geoNode.isNull()) {
             std::cout << "online geo: "<<geoNode.toElement().toDocument().toString().toStdString();
            exit(0);
        }

        //must not be a file name
        SaxParser::parseGeometryTRAV(fileName,geometry,geoNode);
    }
    //return geometry;
}

// This function is only used in online Mode
void MainWindow::parseGeometry(const QString& geometryString)
{
    QFile file("_geometry_tmp_file.xml");
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << geometryString << endl;
    }

    QString tmpFileName = file.fileName();

    //check if there is a tag 'file' there in
    QString geofileName = SaxParser::extractGeometryFilename(tmpFileName);

    auto&& geometry = _visualisationThread->getGeometry();

    if(!geofileName.isEmpty()) {
        SystemSettings::CreateLogfile();
        if (geofileName.endsWith(".xml",Qt::CaseInsensitive)) {
            //parsing the file
            if(!SaxParser::parseGeometryJPS(geofileName,geometry)) {
                SaxParser::parseGeometryXMLV04(geofileName,geometry);
            }
        } else if (geofileName.endsWith(".trav",Qt::CaseInsensitive)) {
            //must not be a file name
            SaxParser::parseGeometryTRAV(geofileName,geometry);
        }
        SystemSettings::DeleteLogfile();
    }
    // I assume it is a trav format node,
    //which is the only one which can directly be inserted into a file
    else {

        QDomNode geoNode;
        //must not be a file name
        SaxParser::parseGeometryTRAV(geometryString,geometry,geoNode);
    }

    //workaround. In some cases there are still hlines in the input files which are not parsed.
    if(geofileName.endsWith(".xml",Qt::CaseInsensitive))
    {
        QXmlInputSource source;
        source.setData(geometryString);
        QXmlSimpleReader reader;

        SyncData dummy;
        double fps;
        SaxParser handler(geometry,dummy,&fps);
        reader.setContentHandler(&handler);
        reader.parse(source);
    }

    //delete the file
    file.remove();
}

// TODO: still used?
// bool MainWindow::parsePedestrianShapes(QDomNode shapeNode, int groupID)
// {

//     if(shapeNode.isNull()) return false;
//     QStringList heights;
//     QStringList colors;

//     QDomNodeList agents = shapeNode.toElement().elementsByTagName("agentInfo");

//     for (int i = 0; i < agents.length(); i++) {

//         bool ok=false;
//         int id=agents.item(i).toElement().attribute("ID").toInt(&ok);
//         if(!ok) continue; // invalid ID
//         double height=agents.item(i).toElement().attribute("height").toDouble(&ok);
//         if(!ok)height=std::numeric_limits<double>::quiet_NaN();

//         int color=agents.item(i).toElement().attribute("color").toDouble(&ok);
//         if(!ok)color=std::numeric_limits<int>::quiet_NaN();
//         Debug::Messages("id= %d height= %lf color =%d",id,height,color);

//         if(height!=height) {
//             heights.append(QString::number(id));
//             heights.append(QString::number(height));
//         }
//         if(color!=color) {
//             colors.append(QString::number(id));
//             colors.append(QString::number(color));
//         }

//     }
//     switch (groupID) {

//     case 1:
//         extern_trajectories_firstSet.setInitialHeights(heights);
//         extern_trajectories_firstSet.setInitialColors(colors);
//         break;
//     }
//     return true;
// }

/// add a new dataset
bool MainWindow::slotAddDataSet()
{

    // just continue
    numberOfDatasetLoaded=1;
    if(addPedestrianGroup(numberOfDatasetLoaded)==false) {
        numberOfDatasetLoaded--;
        return false;
    }

    QString stre(numberOfDatasetLoaded);
    stre.setNum(numberOfDatasetLoaded);
    stre.append(" dataset loaded");
    statusBar()->showMessage(stre);

    slotStartPlaying();

    return true;
}

///clear all datasets previously entered.
void MainWindow::slotClearAllDataset()
{
    clearDataSet(1);
    numberOfDatasetLoaded=0;
}

bool MainWindow::addPedestrianGroup(int groupID,QString fileName)
{
     Debug::Messages("Enter MainWindow::addPedestrianGroup with filename <%s>", fileName.toStdString().c_str());

    statusBar()->showMessage(tr("Select a file"));
     if(fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,
                                                "Select the file containing the data to visualize",
                                                QDir::currentPath(),
                                                "JuPedSim Files (*.xml *.txt);;All Files (*.*)");

    //the action was cancelled
    if (fileName.isNull()) {
        return false;
    }

    //get and set the working dir
    QFileInfo fileInfo(fileName);
    QString wd=fileInfo.absoluteDir().absolutePath();
    Debug::Messages("MainWindow::addPedestrianGroup: wd:  <%s>", wd.toStdString().c_str());
    SystemSettings::setWorkingDirectory(wd);
    SystemSettings::setFilenamePrefix(QFileInfo ( fileName ).baseName()+"_");

    //the geometry actor
    auto&& geometry = _visualisationThread->getGeometry();
    QString geometry_file;
    //try to get a geometry filename
    if(fileName.endsWith(".xml",Qt::CaseInsensitive))
    {
         Debug::Messages("1. Extract geometry file from <%s>", fileName.toStdString().c_str());
         geometry_file=SaxParser::extractGeometryFilename(fileName);
    }
    else
    {
         Debug::Messages("Extract geometry file from <%s>", fileName.toStdString().c_str());
         geometry_file=SaxParser::extractGeometryFilenameTXT(fileName);
    }

    Debug::Messages("MainWindow::addPedestrianGroup: geometry name: <%s>", geometry_file.toStdString().c_str());
    if(geometry_file.isEmpty())
    {
         auto fileDir = fileInfo.path();
         if(fileName.endsWith(".txt",Qt::CaseInsensitive))
         {
              int res = QMessageBox::warning(this, "Did not find geometry name in TXT file",
                                             "Warning: Did not find geometry name in TXT file\nOpen geometry file?"
                                             , QMessageBox::Yes
                                             | QMessageBox::No, QMessageBox::Yes);
              if (res == QMessageBox::No) {
                   exit(EXIT_FAILURE);
                   //return false;
              }
              geometry_file = QFileDialog::getOpenFileName(this,
                                                           "Select a geometry file",
                                                           fileDir,
                                                           "Geometry (*.xml)");
              Debug::Messages("Got geometry file: <%s>", geometry_file.toStdString().c_str());
              QFileInfo check_file(geometry_file);
              if( !(check_file.exists() && check_file.isFile()) )
              {
                   Debug::Error("Geomery file does not exist.");
                   //exit(EXIT_FAILURE);
                   return(false);

              }
              //geometry_file =  check_file.fileName();
         }
         // @todo: check xml file too, although probably xml files
         // always have a geometry tag

    }
    std::cout << "---> geometry " << geometry_file.toStdString().c_str() << "\n" ;

    // if xml is detected, just load and show the geometry then exit
    if(geometry_file.endsWith(".xml",Qt::CaseInsensitive)) {

        //try to parse the correct way
        // fall back to this if it fails
        SystemSettings::CreateLogfile();
        Debug::Messages("Calling parseGeometryJPS with <%s>", geometry_file.toStdString().c_str());
        if(! SaxParser::parseGeometryJPS(geometry_file,geometry)) {
            int res = QMessageBox::warning(this, "Errors in Geometry. Continue Parsing?",
                                           "JuPedSim has detected an error in the supplied geometry.\n"
                                           "<"
                                           +geometry_file+
                                           ">"
                                           "The simulation will likely fail using this geometry.\n"
                                           "More information are provided in the log file:\n"
                                           +SystemSettings::getLogfile()+
                                           "\n\nShould I try to parse and display what I can?"
                                           , QMessageBox::Yes
                                           | QMessageBox::No, QMessageBox::No);
            if (res == QMessageBox::No) {
                return false;
            }
            SaxParser::parseGeometryXMLV04(wd+"/"+geometry_file,geometry);//@todo:
                                                                          //use
                                                                          //qt sep
        } else {
            //everything was fine. Delete the log file
             //std::cout << "won't delete logfile\n";
            SystemSettings::DeleteLogfile();
        }

        //SaxParser::parseGeometryXMLV04(fileName,geometry);
        //slotLoadParseShowGeometry(fileName);
        //return false;
    }

    //check if it is vtrk file containinf gradient
    if(fileName.endsWith(".vtk",Qt::CaseInsensitive))
    {
        if (false==SaxParser::ParseGradientFieldVTK(fileName,geometry))
            return false;

    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        Debug::Error("parseGeometryJPS:  could not open the File: ",fileName.toStdString().c_str());
        return false;

    }

    SyncData* dataset=NULL;

    extern_trajectories_firstSet.clearFrames();


    vtkSmartPointer<vtkSphereSource> org = vtkSphereSource::New();
    org->SetRadius(10);
    // extern_mysphere = org;


    switch(groupID) {
    case 1:
        Debug::Messages("handling first set");
        dataset=&extern_trajectories_firstSet;
        extern_first_dataset_loaded=true;
        extern_first_dataset_visible=true;
        ui.actionFirst_Group->setEnabled(true);
        ui.actionFirst_Group->setChecked(true);
        slotToggleFirstPedestrianGroup();
        break;

    default:
        Debug::Error("Only one dataset can be loaded at a time");
        //return false;
        break;
    }

    //no other geometry format was detected
    double frameRate=16; //default frame rate
    statusBar()->showMessage(tr("parsing the file"));



    //parsing the xml file
    if(fileName.endsWith(".xml",Qt::CaseInsensitive))
    {
        QXmlInputSource source(&file);
        QXmlSimpleReader reader;

        SaxParser handler(geometry,*dataset,&frameRate);
        reader.setContentHandler(&handler);
        reader.parse(source);
        file.close();
    }
    //parsing the vtk file
//    else if(fileName.endsWith(".vtk",Qt::CaseInsensitive))
//    {
//        if (false==SaxParser::ParseGradientFieldVTK(fileName,geometry))
//            return false;

//    }
    // try to parse the txt file
    else if(fileName.endsWith(".txt",Qt::CaseInsensitive))
    {
         QString source_file= wd + QDir::separator() + SaxParser::extractSourceFileTXT(fileName);
         QString ttt_file= wd + QDir::separator() + SaxParser::extractTrainTimeTableFileTXT(fileName);
         QString tt_file= wd + QDir::separator() + SaxParser::extractTrainTypeFileTXT(fileName);
         QString goal_file=wd + QDir::separator() + SaxParser::extractGoalFileTXT(fileName);
         QFileInfo check_file(source_file);
         if( !(check_file.exists() && check_file.isFile()) )
        {
             Debug::Messages("WARNING: MainWindow::addPedestrianGroup: source name: <%s> not found!", source_file.toStdString().c_str());
        }
         else
              Debug::Messages("INFO: MainWindow::addPedestrianGroup: source name: <%s>", source_file.toStdString().c_str());

         check_file = goal_file;
         if( !(check_file.exists() && check_file.isFile()) )
        {
             Debug::Messages("WARNING: MainWindow::addPedestrianGroup: goal name: <%s> not found!", goal_file.toStdString().c_str());
        }
         else
              Debug::Messages("INFO: MainWindow::addPedestrianGroup: goal name: <%s>", goal_file.toStdString().c_str());


         check_file = ttt_file;
         if( !(check_file.exists() && check_file.isFile()) )
        {
             Debug::Messages("WARNING: MainWindow::addPedestrianGroup: ttt name: <%s> not found!", ttt_file.toStdString().c_str());
        }
         else
              Debug::Messages("INFO: MainWindow::addPedestrianGroup: ttt name: <%s>", ttt_file.toStdString().c_str());

         check_file = tt_file;
         if( !(check_file.exists() && check_file.isFile()) )
        {
             Debug::Messages("WARNING: MainWindow::addPedestrianGroup: tt name: <%s> not found!", tt_file.toStdString().c_str());
        }
         else
              Debug::Messages("INFO: MainWindow::addPedestrianGroup: tt name: <%s>", tt_file.toStdString().c_str());




        // ------ parsing sources
        QFile file(source_file);
        QXmlInputSource source(&file);
        QXmlSimpleReader reader;
        SaxParser handler(geometry,*dataset,&frameRate);
        reader.setContentHandler(&handler);
        reader.parse(source);
        file.close();
        // -----
        // // ---- parsing goals
        // -----
        QFile file2(goal_file);
        QXmlInputSource source2(&file2);
        reader.parse(source2);
        file2.close();
        // parsing trains
        // train type
        std::map<int, std::shared_ptr<TrainTimeTable> > trainTimeTable;
        std::map<std::string, std::shared_ptr<TrainType> > trainTypes;
        SaxParser::LoadTrainType(tt_file.toStdString(), trainTypes);
        extern_trainTypes = trainTypes;

        bool ret = SaxParser::LoadTrainTimetable(ttt_file.toStdString(), trainTimeTable);

        extern_trainTimeTables = trainTimeTable;
        QString geofileName = SaxParser::extractGeometryFilenameTXT(fileName);

        std::tuple<Point, Point>  trackStartEnd;
        double elevation;
        for(auto tab: trainTimeTable)
        {
              int trackId = tab.second->pid;
              trackStartEnd = SaxParser::GetTrackStartEnd(geofileName, trackId);
              // todo: 
              // int roomId = SaxParser::GetRoomId(tab.second->pid)
              // int subroomId = SaxParser::GetSubroomId(tab.second->pid)
              // elevation = SaxParser::GetElevation(geofileName, roomId, subroomId);
              //--------
              elevation = 0;

              Point trackStart = std::get<0>(trackStartEnd);
              Point trackEnd = std::get<1>(trackStartEnd);

              tab.second->pstart = trackStart;
              tab.second->pend = trackEnd;
              tab.second->elevation = elevation;

              std::cout << "=======\n";
              std::cout << "tab: " << tab.first << "\n";
              std::cout << "Track start: " <<  trackStart._x << ", " << trackStart._y << "\n";
              std::cout << "Track end: " <<  trackEnd._x << ", " << trackEnd._y << "\n";
              std::cout << " room " << tab.second->rid << "\n";
              std::cout << " subroom " << tab.second->sid << "\n";
              std::cout << " elevation " << tab.second->elevation << "\n";
              std::cout << "=======\n";
        }
        for(auto tab: trainTypes)
             std::cout << "type: " << tab.first << "\n";

        if(false==SaxParser::ParseTxtFormat(fileName, dataset,&frameRate))
            return false;

    }

    QString frameRateStr=QString::number(frameRate);
    // set the visualisation window title
    _visualisationThread->setWindowTitle(fileName);
    _visualisationThread->slotSetFrameRate(frameRate);
    //visualisationThread->setGeometry(geometry);
    //visualisationThread->setWindowTitle(caption);
    labelFrameNumber->setText("fps: " + frameRateStr+"/"+frameRateStr);

    //shutdown the visio thread
    extern_shutdown_visual_thread=true;
    waitForVisioThread();

    statusBar()->showMessage(tr("file loaded and parsed"));

    return true;
}

void MainWindow::slotRecord()
{

    if (extern_recording_enable) {
        int res = QMessageBox::warning(this, "action",
                                       "JuPedSim is already recording a video\n"
                                       "do you wish to stop the recording?", QMessageBox::Yes
                                       | QMessageBox::No, QMessageBox::No);
        if (res == QMessageBox::Yes) {
            extern_recording_enable = false;
            ui.BtRecord->setToolTip("Start Recording");
            labelCurrentAction->setText("   Playing   ");
            labelRecording->setText(" rec: off ");
            return;
        }
    }
    extern_launch_recording = true;
    ui.BtRecord->setToolTip("Stop Recording");
    labelRecording->setText(" rec: on ");
    //labelCurrentAction->setText("   recording   ");

}

QString MainWindow::getTagValueFromElement(QDomNode node,
                                           const char * tagName)
{
    if (node.isNull())
        return "";
    return node.toElement().namedItem(tagName).firstChild().nodeValue();

}

void MainWindow::slotFullScreen(bool status)
{

    Debug::Messages("changing full screen status %d",status);
    extern_fullscreen_enable = true;
    //dont forget this.
    extern_force_system_update=true;
}

void MainWindow::slotSetOfflineMode(bool status)
{
    if (status) {
        ui.actionOnline->setChecked(false);
        extern_offline_mode = true;
        labelMode->setText(" Offline ");
    } else {
        ui.actionOnline->setChecked(true);
        extern_offline_mode = false;
        labelMode->setText(" Online ");
    }
}

void MainWindow::slotSetOnlineMode(bool status)
{
    if (status) {
        ui.actionOffline->setChecked(false);
        extern_offline_mode = false;
        labelMode->setText(" Online  ");
    } else {
        ui.actionOffline->setChecked(true);
        extern_offline_mode = true;
        labelMode->setText(" Offline ");

    }
}

void MainWindow::slotReset()
{

    //stop any recording
    if (extern_recording_enable) {
        int res = QMessageBox::question(this, "action",
                                        "do you wish to stop the recording?", QMessageBox::Discard
                                        | QMessageBox::Yes, QMessageBox::Yes);
        if (res == QMessageBox::Yes) {
            extern_recording_enable = false;
            labelCurrentAction->setText("   Playing   ");
        } else {
            return;
        }
    }

    if(anyDatasetLoaded()) {
        int res = QMessageBox::question(this, "action",
                                        "This will also clear any dataset if loaded.\n"
                                        "Do you wish to continue?", QMessageBox::Discard
                                        | QMessageBox::Yes, QMessageBox::Yes);
        if (res == QMessageBox::No) {
            return;
        }
    }


    //shutdown the visual thread
    extern_shutdown_visual_thread = true;
    waitForVisioThread();
    waitForDataThread();

    //reset all buttons
    //anyDataLoaded = false;
    slotClearAllDataset();
    isPlaying = false;
    isPaused  = false;
    labelCurrentAction->setText("   Idle   ");
    resetGraphicalElements();

}

void MainWindow::slotCurrentAction(QString msg)
{
    msg = " " + msg + " ";
    //labelCurrentAction->setText(msg);
    statusBar()->showMessage(msg);
    //	labelMode->setText(msg);
}

void MainWindow::slotFrameNumber(unsigned long actualFrameCount, unsigned long minFrame)
{

    //compute the  mamixum framenumber
    int maxFrameCount=1;
    if(extern_first_dataset_loaded) {
        maxFrameCount=extern_trajectories_firstSet.getFramesNumber();
    }
    maxFrameCount += minFrame;

    if(actualFrameCount>maxFrameCount) actualFrameCount=maxFrameCount;
    QString msg;
    msg.setNum(actualFrameCount);
    ui.frameSliderLabel->setText("Frame: " + msg);

    if(!frameSliderHold)if(maxFrameCount!=0)//TODO WTF, otherwise an arrymtic exeption arises
        ui.framesIndicatorSlider->setValue((ui.framesIndicatorSlider->maximum()*actualFrameCount)/maxFrameCount);
}

void MainWindow::slotRunningTime(unsigned long timems)
{
    //	QString msg;
    //	msg.setNum(timems);
    //	msg.append(" milsec");
    //labelRunningTime->setText(msg);
    ui.lcdRunningTime->display((int)timems);
// TODO HHH this slows down the thread
    // std::cout << timems  << "\n";

    // QColor col="green";
    // if(fmod(timems,1000) <=100 )
    // {
    //      col=Qt::red;
    //      _visualisationThread->setExitsColor(col);
    // }
    // else if(fmod(timems,5000) <=100 )
    // {
    //      col=Qt::white;
    //      _visualisationThread->setExitsColor(col);
    // }


}

void MainWindow::slotRenderingTime(int fps)
{
    QString msg =labelFrameNumber->text().replace(QRegExp("[0-9]+/"),QString::number(fps)+"/");
    labelFrameNumber->setText(msg);
}
void MainWindow::slotExit()
{
     Debug::Messages("Exit jpsvis");
    cleanUp();
    qApp->exit();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    hide();
    cleanUp();
    event->accept();
}

void MainWindow::cleanUp()
{
    //stop the recording process
    extern_recording_enable = false;
    extern_shutdown_visual_thread = true;

    if(SystemSettings::getRecordPNGsequence())
        slotRecordPNGsequence();

    waitForVisioThread();
    waitForDataThread();

}

void MainWindow::slotControlSequence(const char * sex)
{
    QString str(sex);

    //todo: check wich stack is empty and disable
    // the correspinding datasets.
    // are all stacks empty? then stop
    // visualisation adn recording if any
    if (str.compare("CONTROL_STACK_EMPTY") == 0) {

        statusBar()->showMessage(tr(" stack empty "));
        extern_recording_enable = false;
        ui.BtRecord->setToolTip("Start Recording");
        labelRecording->setText(" rec: off ");
    } else if (str.compare("CONTROL_RESET") == 0) {
        Debug::Messages("resetting all");
        isPlaying = false;
        isPaused = false;
        labelCurrentAction->setText("   Idle   ");
        extern_update_step=1;
        //extern_screen_contrast=1;

        //reset the frames cursor
        extern_trajectories_firstSet.resetFrameCursor();

        resetGraphicalElements();

    } else if (str.compare("STACK_REACHS_BEGINNING")==0) {
        //return to normal speed
        if(extern_update_step<0) {
            Debug::Messages("stack reaches beginning, resuming the playback with vel 1");
            extern_update_step=1;
            ui.speedSettingSlider->setValue(1);
        }

    }
}

void MainWindow::resetGraphicalElements()
{
    //restore play button
    QIcon icon1;
    icon1.addPixmap(QPixmap(
                        QString::fromUtf8(":/new/iconsS/icons/Play1Hot.png")),
                    QIcon::Normal, QIcon::Off);
    ui.BtStart->setIcon(icon1);

    //disable record button
    ui.BtRecord->setEnabled(false);

    //disable fullscreen
    //ui.BtFullscreen->setEnabled(false);

    //disable stop button

    ui.BtStop->setEnabled(false);
    //reset the slider
    ui.framesIndicatorSlider->setValue(ui.framesIndicatorSlider->minimum());

    //reset the speed cursor
    ui.speedSettingSlider->setValue(1);
    //reset the contrast

    // bla bla
    ui.actionShow_Legend->setEnabled(true);

    //restore the play Icon

    //
    ui.actionShow_Trajectories->setEnabled(true);

    ui.action3_D->setEnabled(true);
    ui.action2_D->setEnabled(true);

    labelRecording->setText("rec: off");
    statusBar()->showMessage(tr("select a File"));

    //resetting the start/stop recording action
    //check whether the a png recording sequence was playing, stop if the case
    if(SystemSettings::getRecordPNGsequence())
        slotRecordPNGsequence();

}

void MainWindow::slotToggleFirstPedestrianGroup()
{
    if(ui.actionFirst_Group->isChecked()) {
        extern_first_dataset_visible=true;
    } else {
        extern_first_dataset_visible=false;
    }
    extern_force_system_update=true;

}

bool MainWindow::anyDatasetLoaded()
{
    return extern_first_dataset_loaded;
}

void MainWindow::slotShowTrajectoryOnly()
{
    SystemSettings::setShowTrajectories(ui.actionShow_Trajectories->isChecked());
    extern_force_system_update=true;
}

void MainWindow::slotShowPedestrianOnly()
{

    if(ui.actionShow_Agents->isChecked()) {
        SystemSettings::setShowAgents(true);
    } else {
        SystemSettings::setShowAgents(false);
    }
    extern_force_system_update=true;
}

void MainWindow::slotShowGeometry()
{
    if (ui.actionShow_Geometry->isChecked()) {
        _visualisationThread->setGeometryVisibility(true);
        ui.actionShow_Exits->setEnabled(true);
        ui.actionShow_Walls->setEnabled(true);
        ui.actionShow_Geometry_Captions->setEnabled(true);
        ui.actionShow_Navigation_Lines->setEnabled(true);
        ui.actionShow_Floor->setEnabled(true);
        SystemSettings::setShowGeometry(true);
    } else {
        _visualisationThread->setGeometryVisibility(false);
        ui.actionShow_Exits->setEnabled(false);
        ui.actionShow_Walls->setEnabled(false);
        ui.actionShow_Geometry_Captions->setEnabled(false);
        ui.actionShow_Navigation_Lines->setEnabled(false);
        ui.actionShow_Floor->setEnabled(false);
        SystemSettings::setShowGeometry(false);
    }
    extern_force_system_update=true;
}

/// shows/hide geometry
void MainWindow::slotShowHideExits()
{
    bool status = ui.actionShow_Exits->isChecked();
    _visualisationThread->showDoors(status);
    SystemSettings::setShowExits(status);
}

/// shows/hide geometry
void MainWindow::slotShowHideWalls()
{
    bool status = ui.actionShow_Walls->isChecked();
    _visualisationThread->showWalls(status);
    SystemSettings::setShowWalls(status);
}

void MainWindow::slotShowHideNavLines()
{
    bool status = ui.actionShow_Navigation_Lines->isChecked();
    _visualisationThread->showNavLines(status);
    SystemSettings::setShowNavLines(status);
}

//todo: add to the system settings
void MainWindow::slotShowHideFloor()
{
    bool status = ui.actionShow_Floor->isChecked();
    _visualisationThread->showFloor(status);
    SystemSettings::setShowFloor(status);
}


/// update the playing speed
void MainWindow::slotUpdateSpeedSlider(int newValue)
{
    QString msg;
    msg.setNum(newValue);
    extern_update_step=newValue;
    //Debug::Error( " speed updated to: %d", extern_update_step);
    ui.speedSliderLabel->setText("Speed: " + msg + " ");
}

/// update the position slider
void MainWindow::slotUpdateFrameSlider(int newValue)
{
     // std::cout << "update frame slide " << newValue << "\n";

    // first get the correct position
    int maxFrameCount=1;
    if(extern_first_dataset_loaded) {
        int t=extern_trajectories_firstSet.getFramesNumber();
        if(maxFrameCount<t) maxFrameCount=t;
    }

    int update = ((maxFrameCount*newValue)/ui.framesIndicatorSlider->maximum());

    // then set the correct position
    if(extern_first_dataset_loaded) {
        extern_trajectories_firstSet.setFrameCursorTo(update);
        // Debug::Error( " first dataset frames update to [1] : " <<update<<endl;
    }
}

/// update the contrast
/// TODO: now known as framePerScond slider
void MainWindow::slotUpdateContrastSlider(int newValue)
{

    //   extern_screen_contrast=ui.contrastSettingSlider->value();
    //extern_screen_contrast=newValue;
    QString msg;
    msg.setNum(newValue);
    ui.framePerSecondSliderLabel->setText("Frames Per Second: " +msg +" ");
    Debug::Error( " contrast updated to: %d",newValue);

}

///clear the corresponding dataset;
void MainWindow::clearDataSet(int ID)
{

    switch(ID) {
    case 1:
        //extern_trajectories_firstSet.clear();
        extern_trajectories_firstSet.clearFrames();
        extern_trajectories_firstSet.resetFrameCursor();
        extern_first_dataset_loaded=false;
        extern_first_dataset_visible=false;
        ui.actionFirst_Group->setEnabled(false);
        ui.actionFirst_Group->setChecked(false);
        slotToggleFirstPedestrianGroup();
        numberOfDatasetLoaded--;
        break;

    default:
        break;
    }

    if(numberOfDatasetLoaded<0)numberOfDatasetLoaded=0;

    //finally clear the geometry
    //slotClearGeometry();
}

void MainWindow::resetAllFrameCursor()
{
    extern_trajectories_firstSet.resetFrameCursor();
}

/// wait for visualisation thread to shutdown
///@todo why two different threads shutdown procedure.
void MainWindow::waitForVisioThread()
{
    while(_visualisationThread->isRunning()) {
        _visualisationThread->wait(200);
        Debug::Messages("waiting for visualisation engine to terminate ...");
#ifdef __linux__
        _visualisationThread->quit();
#else
        _visualisationThread->terminate();
#endif
    }
    Debug::Messages("Visualisation Engine shutdown successfully");

    //to avoid some sudden shutting down after resuming
    extern_shutdown_visual_thread=false;
}

/// wait for datatransfer thread to be ready
void MainWindow::waitForDataThread()
{

    while(dataTransferThread->isRunning()) {
        dataTransferThread->shutdown();
        Debug::Messages("Waiting for network engine to terminate ...");
        dataTransferThread->wait(500);
    }
    Debug::Messages("Network Engine shutdown successfully.");
}

/// set visualisation mode to 2D
void MainWindow::slotToogle2D()
{
    if(ui.action2_D->isChecked()) {
        extern_is_3D=false;
        ui.action3_D->setChecked(false);
        SystemSettings::set2D(true);

    } else {
        extern_is_3D=true;
        ui.action3_D->setChecked(true);
        SystemSettings::set2D(false);
    }
    bool status=SystemSettings::get2D() && SystemSettings::getShowGeometry();
    _visualisationThread->setGeometryVisibility2D(status);
    extern_force_system_update=true;
}

/// set visualisation mode to 3D
void MainWindow::slotToogle3D()
{
    if(ui.action3_D->isChecked()) {
        extern_is_3D=true;
        ui.action2_D->setChecked(false);
        SystemSettings::set2D(false);

    } else {
        extern_is_3D=false;
        ui.action2_D->setChecked(true);
        SystemSettings::set2D(true);
    }
    bool status= !SystemSettings::get2D() && SystemSettings::getShowGeometry();
    _visualisationThread->setGeometryVisibility3D(status);
    extern_force_system_update=true;
}

void MainWindow::slotFrameSliderPressed()
{
    frameSliderHold=true;
}

void MainWindow::slotFrameSliderReleased()
{
    frameSliderHold=false;
}

void MainWindow::slotToogleShowLegend()
{
    if(ui.actionShow_Legend->isChecked()) {
        SystemSettings::setShowLegend(true);
    } else {
        SystemSettings::setShowLegend(false);
    }
}

/***
 * @param geodata the geometry data received from the data transfer thread
 */
void MainWindow::slotStartVisualisationThread(QString data,int numberOfAgents,float frameRate)
{

    extern_trajectories_firstSet.setNumberOfAgents(numberOfAgents);

    labelFrameNumber->setText("fps: "+QString::number(frameRate)+"/"+QString::number(frameRate));

    extern_first_dataset_loaded=true;
    extern_first_dataset_visible=true;
    ui.actionFirst_Group->setEnabled(true);
    ui.actionFirst_Group->setChecked(true);
    slotToggleFirstPedestrianGroup();

    QDomDocument doc("");

    QString errorMsg="";
    doc.setContent(data,&errorMsg);

    if(!errorMsg.isEmpty()) {
        Debug::Error("%s", (const char *)errorMsg.toStdString().c_str());
        return;
    }

    //FacilityGeometry *geo = parseGeometry(geoNode);
    parseGeometry(data);
    _visualisationThread->slotSetFrameRate(frameRate);
    //visualisationThread->setGeometry(geo);
    _visualisationThread->start();

    //enable some buttons
    ui.BtRecord->setEnabled(true);
    ui.BtStop->setEnabled(true);

}

/// this method is called by the data transfer thread
/// investigate if you should shutdown the thread
void MainWindow::slotShutdownVisualisationThread( bool clearAndStop)
{
    //FIXME: it could have some side effects.
    //you should close this threads
    if(clearAndStop) {
        extern_shutdown_visual_thread=true;
        waitForVisioThread();
    }
    clearDataSet(1);
}

void MainWindow::slotFramesByFramesNavigation()
{
    if(ui.actionFrames_Navigation->isChecked()) {
        ui.BtNextFrame->setVisible(true);
        ui.BtPreviousFrame->setVisible(true);
    } else {
        ui.BtNextFrame->setVisible(false);
        ui.BtPreviousFrame->setVisible(false);
    }
}

void MainWindow::slotNextFrame()
{

    if(extern_first_dataset_loaded) {
        int newValue=extern_trajectories_firstSet.getFrameCursor()+1;
        extern_trajectories_firstSet.setFrameCursorTo(newValue);
        //Debug::Error( " update to : " <<newValue<<endl;
        // HH
    }
}

void MainWindow::slotPreviousFrame()
{
    if(extern_first_dataset_loaded) {
        int newValue=extern_trajectories_firstSet.getFrameCursor()-1;
        extern_trajectories_firstSet.setFrameCursorTo(newValue);
        //Debug::Error( " update to : " <<newValue<<endl;
    }
}

void MainWindow::slotShowPedestrianCaption()
{

    SystemSettings::setShowAgentsCaptions(ui.actionShow_Captions->isChecked());
    extern_force_system_update=true;
}

void MainWindow::slotToogleShowAxis()
{

    _visualisationThread->setAxisVisible(ui.actionShow_Axis->isChecked());
}

//todo: rename this to slotChangeSettting
void MainWindow::slotChangePedestrianShape()
{

    //	Qt::WindowFlags flags = Qt::Window  | Qt::WindowCloseButtonHint;
    //
    //	Settings* travistoOptions = new Settings(this);
    //	travistoOptions->setWindowFlags(flags);

    travistoOptions->show();

    //shape->deleteLater();

}

void MainWindow::slotCaptionColorAuto()
{
    emit signal_controlSequence("CAPTION_AUTO");
}

void MainWindow::slotCaptionColorCustom()
{
    emit signal_controlSequence("CAPTION_CUSTOM");
}

void MainWindow::slotChangeBackgroundColor()
{

    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the background");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new background color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    //double  bkcolor[3]= {(double)col.red()/255.0 ,(double)col.green()/255.0 ,(double)col.blue()/255.0};
    _visualisationThread->setBackgroundColor(col);

    QSettings settings;
    settings.setValue("options/bgColor", col);

    delete colorDialog;

}

/// change the wall color
void MainWindow::slotChangeWallsColor()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for walls");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new wall color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    _visualisationThread->setWallsColor(col);

    QSettings settings;
    settings.setValue("options/wallsColor", col);

    delete colorDialog;

}

/// change the exits color
void MainWindow::slotChangeExitsColor()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the exits");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new exit color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    _visualisationThread->setExitsColor(col);

    QSettings settings;
    settings.setValue("options/exitsColor", col);
    Debug::Messages("Change Exits Color to %s", col.name().toStdString().c_str());
    delete colorDialog;
}

/// change the navigation lines colors
void MainWindow::slotChangeNavLinesColor()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for walls");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new navigation lines color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    _visualisationThread->setNavLinesColor(col);

    QSettings settings;
    settings.setValue("options/navLinesColor", col);

    delete colorDialog;
}

void MainWindow::slotChangeFloorColor()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the floor");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new floor color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    _visualisationThread->setFloorColor(col);

    QSettings settings;
    settings.setValue("options/floorColor", col);

    delete colorDialog;
}

void MainWindow::slotChangeObstacleColor()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    colorDialog->setToolTip("Choose a new color for the obstacles");
    QColor col=colorDialog->getColor(Qt::white,this,"Select new obstalce color");

    //the user may have cancelled the process
    if(col.isValid()==false) return;

    _visualisationThread->setObstacleColor(col);

    QSettings settings;
    settings.setValue("options/obstacle", col);

    delete colorDialog;
}

void MainWindow::slotSetCameraPerspectiveToTop()
{
    int p= 1; //TOP

    _visualisationThread->setCameraPerspective(p);
    //disable the virtual agent view
    SystemSettings::setVirtualAgent(-1);
    //cerr <<"Setting camera view to top"<<endl;
}

void MainWindow::slotSetCameraPerspectiveToFront()
{
    int p= 2; //FRONT
    _visualisationThread->setCameraPerspective(p);
    //disable the virtual agent view
    SystemSettings::setVirtualAgent(-1);
    //	cerr <<"Setting camera view to FRONT"<<endl;
}

void MainWindow::slotSetCameraPerspectiveToSide()
{
    int p= 3; //SIDE
    _visualisationThread->setCameraPerspective(p);
    //disable the virtual agent view
    SystemSettings::setVirtualAgent(-1);
    //cerr <<"Setting camera view to Side"<<endl;
}

void MainWindow::slotSetCameraPerspectiveToVirtualAgent()
{

    bool ok=false;
    int agent = QInputDialog::getInt(this, tr("choose the agent you want to see the scene through"), tr(
                                         "agent ID(default to 1):"),1,1,500, 1, &ok);

    if (ok) {
        int p= 4; //virtual agent
        _visualisationThread->setCameraPerspective(p);

        //get the virtual agent ID
        SystemSettings::setVirtualAgent(agent);
    }
}

/// @todo does it work? mem check?
void MainWindow::slotClearGeometry()
{
   _visualisationThread->setGeometry(NULL);
}

void MainWindow::slotErrorOutput(QString err)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText(err);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void MainWindow::slotTakeScreenShot()
{
    //extern_take_screenshot=true;
    extern_take_screenshot=!extern_take_screenshot;
}

/// load settings, parsed from the project file
void MainWindow::loadAllSettings()
{
     Debug::Messages("restoring previous settings");
    QSettings settings;

    //visualisation
    if (settings.contains("vis/offline"))
    {
        bool offline = settings.value("vis/offline").toBool();
        slotSetOnlineMode(!offline);
        slotSetOfflineMode(offline);
        if(!offline){
            int port = SystemSettings::getListeningPort();
            if (settings.contains("options/listeningPort"))
            {
                port = settings.value("options/listeningPort").toInt();
                SystemSettings::setListeningPort(port);
            }
            Debug::Messages("online listening on port: %d",port);
        }
        else
        {
             Debug::Messages("offline: %s", offline?"Yes":"No");
        }
    }

    //view
    if (settings.contains("view/2d"))
    {
        bool checked = settings.value("view/2d").toBool();
        ui.action2_D->setChecked(checked);
        ui.action3_D->setChecked(!checked);
        SystemSettings::set2D(checked);
        Debug::Messages("2D: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showAgents"))
    {
        bool checked = settings.value("view/showAgents").toBool();
        ui.actionShow_Agents->setChecked(checked);
        SystemSettings::setShowAgents(checked);
        Debug::Messages("show Agents: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showCaptions"))
    {
        bool checked = settings.value("view/showCaptions").toBool();
        ui.actionShow_Captions->setChecked(checked);
        SystemSettings::setShowAgentsCaptions(checked);
        Debug::Messages("show Captions: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showTrajectories"))
    {
        bool checked = settings.value("view/showTrajectories").toBool();
        ui.actionShow_Trajectories->setChecked(checked);
        SystemSettings::setShowTrajectories(checked);
        Debug::Messages("show Trajectories: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showGeometry"))
    {
        bool checked = settings.value("view/showGeometry").toBool();
        ui.actionShow_Geometry->setChecked(checked);
        slotShowGeometry();//will take care of the others things like enabling options
        Debug::Messages("show Geometry: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showFloor"))
    {
        bool checked = settings.value("view/showFloor").toBool();
        ui.actionShow_Floor->setChecked(checked);
        slotShowHideFloor();
        Debug::Messages("show Floor: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showExits"))
    {
        bool checked = settings.value("view/showExits").toBool();
        ui.actionShow_Exits->setChecked(checked);
        slotShowHideExits();
        Debug::Messages("show Exits: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showWalls"))
    {
        bool checked = settings.value("view/showWalls").toBool();
        ui.actionShow_Walls->setChecked(checked);
        slotShowHideWalls();
        Debug::Messages("show Walls: ", checked?"Yes":"No");
    }
    if (settings.contains("view/showGeoCaptions"))
    {
        bool checked = settings.value("view/showGeoCaptions").toBool();
        ui.actionShow_Geometry_Captions->setChecked(checked);
        slotShowHideGeometryCaptions();
        Debug::Messages("show geometry Captions: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showNavLines"))
    {
        bool checked = settings.value("view/showNavLines").toBool();
        ui.actionShow_Navigation_Lines->setChecked(checked);
        slotShowHideNavLines();
        Debug::Messages("show Navlines: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showObstacles"))
    {
        bool checked = settings.value("view/showObstacles").toBool();
        ui.actionShow_Obstacles->setChecked(checked);
        slotShowHideObstacles();
        Debug::Messages("show showObstacles: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showOnScreensInfos"))
    {
        bool checked = settings.value("view/showOnScreensInfos").toBool();
        ui.actionShow_Onscreen_Infos->setChecked(checked);
        slotShowOnScreenInfos();
        Debug::Messages("show OnScreensInfos: %s", checked?"Yes":"No");
    }
    if (settings.contains("view/showGradientField"))
    {
        bool checked = settings.value("view/showGradientField").toBool();
        ui.actionShow_Gradient_Field->setChecked(checked);
        slotShowHideGradientField();
        Debug::Messages("show GradientField: %s", checked?"Yes":"No");
    }
    //options
    if (settings.contains("options/rememberSettings"))
    {
        bool checked = settings.value("options/rememberSettings").toBool();
        ui.actionRemember_Settings->setChecked(checked);
        Debug::Messages("remember settings: %s", checked?"Yes":"No");
    }

    if (settings.contains("options/bgColor"))
    {
        QColor color = settings.value("options/bgColor").value<QColor>();
        SystemSettings::setBackgroundColor(color);
        Debug::Messages("background color: %s",color.name().toStdString().c_str());
    }

    if (settings.contains("options/exitsColor"))
    {
        QColor color = settings.value("options/exitsColor").value<QColor>();
        SystemSettings::setExitsColor(color);
        Debug::Messages("Exit color: %s",color.name().toStdString().c_str());
    }

    if (settings.contains("options/floorColor"))
    {
        QColor color = settings.value("options/floorColor").value<QColor>();
        SystemSettings::setFloorColor(color);
        Debug::Messages("Floor color: %s",color.name().toStdString().c_str());
    }

    if (settings.contains("options/wallsColor"))
    {
        QColor color = settings.value("options/wallsColor").value<QColor>();
        SystemSettings::setWallsColor(color);
        Debug::Messages("Walls color: %s",color.name().toStdString().c_str());
    }

    if (settings.contains("options/navLinesColor"))
    {
        QColor color = settings.value("options/navLinesColor").value<QColor>();
        SystemSettings::setNavLinesColor(color);
        Debug::Messages("NavLines color: %s",color.name().toStdString().c_str());
    }
    if (settings.contains("options/obstaclesColor"))
    {
        QColor color = settings.value("options/obstaclesColor").value<QColor>();
        SystemSettings::setObstacleColor(color);
        Debug::Messages("Obstacles color: %s",color.name().toStdString().c_str());
    }

    extern_force_system_update=true;
}

void MainWindow::saveAllSettings()
{
    //visualiation
    QSettings settings;
    settings.setValue("vis/offline", ui.actionOffline->isChecked());

    //view
    settings.setValue("view/2d", ui.action2_D->isChecked());
    settings.setValue("view/showAgents", ui.actionShow_Agents->isChecked());
    settings.setValue("view/showCaptions", ui.actionShow_Captions->isChecked());
    settings.setValue("view/showTrajectories", ui.actionShow_Trajectories->isChecked());
    settings.setValue("view/showGeometry", ui.actionShow_Geometry->isChecked());
    settings.setValue("view/showFloor", ui.actionShow_Floor->isChecked());
    settings.setValue("view/showWalls", ui.actionShow_Walls->isChecked());
    settings.setValue("view/showExits", ui.actionShow_Exits->isChecked());
    settings.setValue("view/showGeoCaptions", ui.actionShow_Geometry_Captions->isChecked());
    settings.setValue("view/showNavLines", ui.actionShow_Navigation_Lines->isChecked());
    settings.setValue("view/showOnScreensInfos", ui.actionShow_Onscreen_Infos->isChecked());
    settings.setValue("view/showObstacles", ui.actionShow_Obstacles->isChecked());
    settings.setValue("view/showGradientField", ui.actionShow_Gradient_Field->isChecked());

    //options: the color settings are saved in the methods where they are used.
    settings.setValue("options/listeningPort", SystemSettings::getListeningPort());
    settings.setValue("options/rememberSettings", ui.actionRemember_Settings->isChecked());

    //settings.setValue("options/bgColor", ui.actionBackground_Color->isChecked());
    // NOT USED: settings.setValue("options/captionColor", ui.action3_D->isChecked());
    //settings.setValue("options/floorColor", ui.action3_D->isChecked());
    //settings.setValue("options/wallsColor", ui.action3_D->isChecked());
    //settings.setValue("options/navLinesColor", ui.action3_D->isChecked());
}

/// start/stop the recording process als png images sequences
void MainWindow::slotRecordPNGsequence()
{
    if(!isPlaying) {
        slotErrorOutput("Start a video first");
    }

    // get the status from the system settings and toogle it
    bool status =SystemSettings::getRecordPNGsequence();

    if(status) {
        ui.actionRecord_PNG_sequences->setText("Record PNG sequence");
    } else {
        ui.actionRecord_PNG_sequences->setText("Stop PNG Recording");
    }

    // toggle the status
    SystemSettings::setRecordPNGsequence(!status);

}

/// render a PNG image sequence to an AVI video
void MainWindow::slotRenderPNG2AVI()
{
    slotErrorOutput("Not Implemented yet, sorry !");

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{

    if (!extern_offline_mode) {
        //slotErrorOutput("online mode, ignoring DnD !");
        slotErrorOutput("You want DnD? I am now disabling the online mode");
        slotSetOfflineMode(true);
        //return;
    }

    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    slotStopPlaying();

    bool mayPlay = false;

    for (int i = 0; i < urls.size(); i++) {
        QString fileName = urls[i].toLocalFile();
        if (fileName.isEmpty())
            continue;
        if (addPedestrianGroup(numberOfDatasetLoaded + i + 1, fileName)) {
            mayPlay = true;
        }
    }
    if (mayPlay) {
        slotStartPlaying();
    }
}

/// show/hide onscreen information
/// information include Time and pedestrians left in the facility
void MainWindow::slotShowOnScreenInfos()
{
    bool value=ui.actionShow_Onscreen_Infos->isChecked();
    _visualisationThread->setOnscreenInformationVisibility(value);
    SystemSettings::setOnScreenInfos(value);
    Debug::Messages("Show On Screen Infos: %s", value?"On":"Off");
}

/// show/hide the geometry captions
void MainWindow::slotShowHideGeometryCaptions()
{
    bool value=ui.actionShow_Geometry_Captions->isChecked();
    _visualisationThread->setGeometryLabelsVisibility(value);
    SystemSettings::setShowGeometryCaptions(value);
    //SystemSettings::setShowCaptions(value);
    //SystemSettings::setOnScreenInfos(value);
}
void MainWindow::slotShowHideObstacles()
{
    bool value=ui.actionShow_Obstacles->isChecked();
    _visualisationThread->showObstacle(value);
    SystemSettings::setShowObstacles(value);
}
void MainWindow::slotShowHideGradientField()
{
    bool value=ui.actionShow_Gradient_Field->isChecked();
    _visualisationThread->showGradientField(value);
    SystemSettings::setShowGradientField(value);
}

void MainWindow::slotShowGeometryStructure()
{
    //QListView list;
    //_geoStructure.setVisible(! _geoStructure.isVisible());
    //_geoStructure.showColumn(0);
    //_geoStructure.show();
    _geoStructure.setHidden(!ui.actionShowGeometry_Structure->isChecked());
    if(_visualisationThread->getGeometry().RefreshView())
    {
        _geoStructure.setWindowTitle("Geometry structure");
        _geoStructure.setModel(&_visualisationThread->getGeometry().GetModel());
    }
    Debug::Messages("Show Geometry Structure: %s", ui.actionShowGeometry_Structure->isChecked()?"On":"Off");
}

void MainWindow::slotOnGeometryItemChanged( QStandardItem *item)
{
    QStringList l=item->data().toString().split(":");
    if(l.length()>1)
    {
        int room=l[0].toInt();
        int subr=l[1].toInt();
        bool state=item->checkState();
        _visualisationThread->getGeometry().UpdateVisibility(room,subr,state);
    }
    else
    {
        for(int i=0;i<item->rowCount();i++)
        {
            QStandardItem *child=   item->child(i);
            child->setCheckState(item->checkState());
            slotOnGeometryItemChanged(child);
        }

    }
}
