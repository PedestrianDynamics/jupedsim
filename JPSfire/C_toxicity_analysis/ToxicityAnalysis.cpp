/**
 * \file        WalkingSpeed.cpp
 * \date        Jan 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * detects smoke in the next rooms
 *
 *
 **/

#include "ToxicityAnalysis.h"
#include "../../geometry/Building.h"
#include "../../pedestrian/Pedestrian.h"
#include "../generic/FDSMesh.h"
#include "../generic/FDSMeshStorage.h"
#include "../../pedestrian/PedDistributor.h"
#include "../../tinyxml/tinyxml.h"
#include <set>
#include <algorithm>
#include <math.h>
#include "../../IO/OutputHandler.h"

ToxicityAnalysis::ToxicityAnalysis(const Building * b)
{
    _building = b;
    _FMStorage = nullptr;
    LoadJPSfireInfo(_building->GetProjectFilename());
    string fileNameWithoutExtension = _building->GetProjectFilename().substr(0, _building->GetProjectFilename().find_last_of(".")); 
    std::string ToxAnalysisXML = "tox_" + fileNameWithoutExtension + ".xml";
    _outputhandler = std::make_shared<ToxicityOutputHandler>(ToxAnalysisXML.c_str());
    _outputhandler->WriteToFileHeader();
    _frame=0;

}

ToxicityAnalysis::~ToxicityAnalysis()
{

}

bool ToxicityAnalysis::LoadJPSfireInfo(const std::string projectFilename )
{
   TiXmlDocument doc(projectFilename);
   if (!doc.LoadFile()) {
        Log->Write("ERROR: \t%s", doc.ErrorDesc());
        Log->Write("ERROR: \t could not parse the project file");
        return false;
   }

   TiXmlNode* JPSfireNode = doc.RootElement()->FirstChild("JPSfire");
   if( ! JPSfireNode ) {
        Log->Write("INFO:\tcould not find any JPSfire information");
        return true;
   }

   TiXmlElement* JPSfireCompElem = JPSfireNode->FirstChildElement("C_toxicity_analysis");
   if(JPSfireCompElem) {
       if(JPSfireCompElem->FirstAttribute()){
           //std::string filepath = xmltoa(JPSfireCompElem->Attribute("toxicity_grids"), "");
           std::string filepath = _building->GetProjectRootDir() + xmltoa(JPSfireCompElem->Attribute("toxicity_grids"), "");
           double updateIntervall = xmltof(JPSfireCompElem->Attribute("update_time"), 0.);
           double finalTime = xmltof(JPSfireCompElem->Attribute("final_time"), 0.);
           std::string study = "";
           std::string irritant = "";
           Log->Write("INFO:\tJPSfire Module C_toxicity_analysis: \n \tdata: %s \n\tupdate time: %.1f s | final time: %.1f s",
                      filepath.c_str(), updateIntervall, finalTime);
           //TODO Is there a posibility to pass a variable number of arguments to a function?
           _FMStorage = std::make_shared<FDSMeshStorage>(filepath, finalTime, updateIntervall, study, irritant);
           return true;
       }
   }
   return false;
}


std::string ToxicityAnalysis::GetName() const
{
    return "ToxicityAnalysis";
}


double ToxicityAnalysis::GetGasConcentration(const Pedestrian * pedestrian, std::string quantity)
{
    //std::cout << "\n" << quantity << std::endl;
    //try to get gas components, 0 if gas component is not provided by JPSfire
    double concentration;
    try {
        const FDSMesh& meshref = _FMStorage->GetFDSMesh(pedestrian->GetGlobalTime(), pedestrian->GetElevation(), quantity);
        concentration = (meshref.GetKnotValue(pedestrian->GetPos()._x , pedestrian->GetPos()._y))*10E6;
        if(concentration != concentration){
            concentration = 0.0;
        }
        return concentration ;
    } catch (int e) {
        //std::cout <<  pedestrian->GetPos()._x << pedestrian->GetPos()._y << pedestrian->GetElevation() << quantity  << std::endl;
        return 0.0;
    }
}

void ToxicityAnalysis::set_FMStorage(const std::shared_ptr<FDSMeshStorage> fmStorage)
{
    _FMStorage=fmStorage;
}

const std::shared_ptr<FDSMeshStorage> ToxicityAnalysis::get_FMStorage()
{
    return _FMStorage;
}

void ToxicityAnalysis::CalculateFED(Pedestrian* p)
{
    double FED_In = p->GetFED();
    double FIC_Im, FIC_In;

    double CO2 = 0., CO = 0., HCN = 0., HCL = 0., O2 = 0.;
    // all gas species in ppm
    CO2 = GetGasConcentration(p, "CARBON_DIOXIDE_VOLUME_FRACTION");
    CO = GetGasConcentration(p, "CARBON_MONOXIDE_VOLUME_FRACTION");
    HCN = GetGasConcentration(p, "HYDROGEN_CYANIDE_VOLUME_FRACTION");
    HCL = GetGasConcentration(p, "HYDROGEN_CHLORIDE_VOLUME_FRACTION");
    O2 = 210000 - CO2 - CO - HCN - HCL;

    double VE = 50.; //breath rate (L/min)
    double D = 20.; //Exposure dose (percent COHb) for incapacitation
    double dt = 1/20.; //time fraction for which doses are cumulated

    double FED_In_CO = (3.317/(10E5 * D) * pow(CO, 1.036)) * dt ;
    double FED_In_HCN = (pow(HCN, 2.36)/10E7*2.43) * dt ;
    double FED_In_O2 = 1/exp(8.13-0.54*(20.9-O2/10000 )) * dt ; //Vol%
    double VCO2 = exp(CO2/10000/5) ; //Vol%

    // overall FED_In Fractional Effective Dose until incapacitation
    FED_In += ( FED_In_CO + FED_In_HCN ) * VE * VCO2 + FED_In_O2 ;

    // FIC Fractional Irritant Concentration for impairment and incapacitation
    // according to SFPE/BS7899-2
    FIC_Im = HCL/200;
    FIC_In = HCL/900;

    p->SetFED(FED_In);

    StoreToxicityAnalysis(p, CO2, CO, HCN, HCL, FIC_Im, FED_In);
}

void ToxicityAnalysis::StoreToxicityAnalysis(const Pedestrian* p, double CO2, double CO, double HCN, double HCL, double FIC, double FED_In)
{

    //for testing purposes. Can be tunnelled to file via jpscore jpscore ... 2> tox

//    fprintf(stderr, "t\tPed ID\tc_CO2\tc_CO\tc_HCN\tc_HCL\tPed FED"
//                    "\n%f\t%i\t%f\t%f\t%f\t%f\t%f\n",
//            p->GetGlobalTime(), p->GetID(), CO2, CO, HCN, HCL, FED);

//    fprintf(stderr, "%i\t%f\t%f\t%f\t%f\t%f\n",p->GetID(), p->GetPos()._x, p->GetPos()._y, p->GetElevation(), p->GetGlobalTime(), FED);

    string data;
    char tmp[CLENGTH] = "";

    sprintf(tmp, "\t<agent ID=\"%i\"\tt=\"%.1f\"\tc_CO2=\"%.0f\"\tc_CO=\"%.0f\"\tc_HCN=\"%.0f\"\tc_HCL=\"%.0f\"\tFIC=\"%.4f\"\tFED_In=\"%.4f\"/>",
         p->GetID(), p->GetGlobalTime(), CO2, CO, HCN, HCL, FIC, FED_In);

        data.append(tmp);

    _outputhandler->WriteToFile(data);
}

bool ToxicityAnalysis::ConductToxicityAnalysis()
{
   return _FMStorage!=nullptr;
}
