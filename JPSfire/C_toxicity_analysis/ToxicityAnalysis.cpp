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

#include "general/Filesystem.h"
#include "JPSfire/generic/FDSMesh.h"
#include "JPSfire/generic/FDSMeshStorage.h"
#include "pedestrian/Pedestrian.h"

#include <tinyxml.h>


ToxicityAnalysis::ToxicityAnalysis(const std::string & projectFileName, double fps):  _projectFileName(projectFileName), _fps(fps)
{
    _FMStorage = nullptr;
    _dt = 1/20.;     // @todo time fraction for which doses are cumulated. Parse in inifile?
    _t_prev = -1;
    LoadJPSfireInfo(projectFileName);
}

ToxicityAnalysis::~ToxicityAnalysis()
{

}

bool ToxicityAnalysis::LoadJPSfireInfo(const std::string projectFilename )
{
     fs::path p(projectFilename);
     std::string projectRootDir = p.parent_path().string();

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
           std::string toxicity_grids = xmltoa(JPSfireCompElem->Attribute("toxicity_grids"), "");
           std::string filepath = projectRootDir + "/" +  toxicity_grids;
           if (projectRootDir.size() == 0 ) // directory is "."
                filepath = toxicity_grids;
           double updateIntervall = xmltof(JPSfireCompElem->Attribute("update_time"), 0.);
           double finalTime = xmltof(JPSfireCompElem->Attribute("final_time"), 0.);
           std::string study = "";
           std::string irritant = "";
           Log->Write("INFO:\tJPSfire Module C_toxicity_analysis: \n \tdata: %s \n\tupdate time: %.1f s | final time: %.1f s",
                      filepath.c_str(), updateIntervall, finalTime);
           //@todo Is there a posibility to pass a variable number of arguments to a function?
           _FMStorage = std::make_shared<FDSMeshStorage>(filepath, finalTime, updateIntervall, study, irritant);

           InitializeWriteOut();

           return true;
       }
   }
   return false;
}


double ToxicityAnalysis::GetFDSQuantity(const Pedestrian * pedestrian, std::string quantity)
{
    //std::cout << "\n" << quantity << std::endl;
    //try to get gas components, 0 if gas component is not provided by JPSfire
    double concentration;
    try {
        const FDSMesh& meshref = _FMStorage->GetFDSMesh(pedestrian->GetGlobalTime(), pedestrian->GetElevation(), quantity);
        concentration = (meshref.GetKnotValue(pedestrian->GetPos()._x , pedestrian->GetPos()._y));
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

double ToxicityAnalysis::CalculateFEDIn(Pedestrian* p, double CO2, double CO, double O2, double HCN, double FED_In)
{
    double VE = 50.; //breath rate (L/min)
    double D = 20.; //Exposure dose (percent COHb) for incapacitation

    double FED_In_CO = (3.317/(1E5 * D) * pow(CO, 1.036)) * _dt ;
    double FED_In_HCN = (pow(HCN, 2.36)/(2.43*1E7)) * _dt ;
    double FED_In_O2 = 0.0; // MC: hope this initialisation is ok
    //this if statement is necessary since Pursers correlation is not considering
    //normal O2 concentrations (20.9%). It would thus increase the FED_in slightly
    if(O2==209000){
        FED_In_O2 = 0.;
    }
    else if(O2<209000){
        FED_In_O2 = 1/exp(8.13-0.54*(20.9-O2/10000)) * _dt ; //Vol%
    }
    double VCO2 = exp(CO2/10000/5) ; //Vol%

    // overall FED_In Fractional Effective Dose until incapacitation
    FED_In = p->GetFEDIn();
    FED_In += ( ( FED_In_CO + FED_In_HCN ) * VE * VCO2 + FED_In_O2 ) ;
    p->SetFEDIn(FED_In);

    return FED_In;
}

double ToxicityAnalysis::CalculateFEDHeat(Pedestrian* p, double T, double FED_Heat)
{
    double tolerance_time = 2 * pow(10,31) * pow(T,-16.963) + 4*1E8 * pow(T, -3.7561);

   // overall FED_Heat Fractional Effective Dose
    FED_Heat = p->GetFEDHeat();
    FED_Heat += ( 1/tolerance_time ) * _dt;
    p->SetFEDHeat(FED_Heat);

    return FED_Heat;
}

void ToxicityAnalysis::HazardAnalysis(Pedestrian* p)
{
    double FEC_Smoke, FED_In = 0.0, FED_Heat = 0.0, FIC_Im, FIC_In;
    // Smoke extinction in 1/m
    double E = GetFDSQuantity(p, "EXTINCTION_COEFFICIENT"); //@todo check this string.. SOOT??

    // gas species in ppm
    double CO2 = 0., CO = 0., HCN = 0., HCL = 0., O2 = 0.;
    CO2 = GetFDSQuantity(p, "CARBON_DIOXIDE_VOLUME_FRACTION")*1E6;
    CO = GetFDSQuantity(p, "CARBON_MONOXIDE_VOLUME_FRACTION")*1E6;
    HCN = GetFDSQuantity(p, "HYDROGEN_CYANIDE_VOLUME_FRACTION")*1E6;
    HCL = GetFDSQuantity(p, "HYDROGEN_CHLORIDE_VOLUME_FRACTION")*1E6;
    //derive O2 concentration from balance calculation
    O2 = 209000 - CO2 - CO - HCN - HCL;

    // gas temperature in C
    double T = 20.;
    T = GetFDSQuantity(p, "TEMPERATURE");

    // FEC Smoke
    FEC_Smoke = E/0.23;

    // FED Incapacitation dose calculation according to SFPE2016 Chap. 63
    FED_In = CalculateFEDIn(p, CO2, CO, O2, HCN, FED_In); // FED_In as
                                                          // argument
                                                          // is not used!

    // FIC Fractional Irritant Concentration for impairment and incapacitation
    // according to SFPE/BS7899-2
    FIC_Im = HCL/200;
    FIC_In = HCL/900;

    //FED Heat dose calculation according to SFPE2016 Chap. 63
    FED_Heat = CalculateFEDHeat(p, T, FED_Heat);

    WriteOutHazardAnalysis(p, E, FEC_Smoke, O2, CO2, CO, HCN, HCL, FED_In, FIC_Im, FIC_In, T, FED_Heat);
}

void ToxicityAnalysis::InitializeWriteOut()
{
     fs::path p(_projectFileName);
     std::string ToxAnalysisXML =  "toxicity_output_" + p.stem().string() + p.extension().string();
     fs::path t(ToxAnalysisXML);
     t = p.parent_path() / t;
     _outputhandler = std::make_shared<ToxicityOutputHandler>(t);
     _outputhandler->WriteToFileHeader();
}

void ToxicityAnalysis::WriteOutHazardAnalysis(const Pedestrian* p, double E, double FEC_Smoke, double O2, double CO2, double CO, double HCN, double HCL, double FED_In, double FIC_Im, double FIC_In, double T, double FED_Heat)
{
    std::string data;
    char tmp[CLENGTH] = "";
    //_fps = 1; // fixme: why 1?
    int frameNr = int(p->GetGlobalTime()/_fps);


    if(_t_prev == -1){
         sprintf(tmp, "\t<frame ID=\"%i\">\n", frameNr );
         data.append(tmp);
         _t_prev = p->GetGlobalTime();
    }

    else if(p->GetGlobalTime() > _t_prev){
         sprintf(tmp, "\t</frame>\n \t<frame ID=\"%i\">\n", frameNr );
         data.append(tmp);
         _t_prev = p->GetGlobalTime();
    }

    sprintf(tmp, "\t<agent ID=\"%i\"\tt=\"%.0f\"\tE=\"%.4f\"\tFEC_Smoke=\"%.4f\"\tc_O2=\"%.0f\"\tc_CO2=\"%.0f\"\tc_CO=\"%.0f\"\tc_HCN=\"%.0f\"\tc_HCl=\"%.0f\"\tFED_In=\"%.4f\"\tFIC_Im=\"%.4f\"\tFIC_In=\"%.4f\"\tT=\"%.1f\"\tFED_Heat=\"%.4f\"/>",
         p->GetID(), p->GetGlobalTime(), E, FEC_Smoke, O2, CO2, CO, HCN, HCL, FED_In, FIC_Im, FIC_In, T, FED_Heat);
        data.append(tmp);


   _outputhandler->WriteToFile(data);

}

bool ToxicityAnalysis::ConductToxicityAnalysis()
{
   return _FMStorage!=nullptr;
}

const std::string ToxicityAnalysis::getProjectFileName(void)
{
     return _projectFileName;
}
