/*
 * Analysis.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: ZhangJun
 */

#include "Analysis.h"
#include <vector>
#include <math.h>
#include <stdio.h>
using namespace std;


FILE *IndividualFD;
Analysis::Analysis() {
	// TODO Auto-generated constructor stub

}

Analysis::~Analysis() {
	// TODO Auto-generated destructor stub

	fclose(IndividualFD);
}

//---------------------------------------------------------------------------------------------
void Analysis::initial(string datafile)
{
#ifdef WIN32
	string Individualfundment="Fundamental_Diagram\\Individual_FD\\IndividualFD"+datafile+".dat";
#else
	  string Individualfundment="Fundamental_Diagram/Individual_FD/IndividualFD"+datafile+".dat";
#endif

	if((IndividualFD=fopen(Individualfundment.c_str(),"w"))==NULL)
	{
		printf("cannot open the file individual\n");
		exit(0);
	}
	fprintf(IndividualFD,"#Individual density(m^(-2))\t	Individual velocity(m/s)\n");
}

/*
 * calculate the voronoi density according to the area of the voronoi cell and the measurement area.
 * input: the list of the voronoi polygons and the measurement area
 * output: the voronoi density in the measurement area
 * note the unit of the polygons
 */
float Analysis::getVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea)
{
	vector<polygon_2d>::iterator polygon_iterator;
	float density=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
		//intersection_inserter<polygon_2d>(measureArea, *polygon_iterator, std::back_inserter(v));
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			density+=area(v[0])/area(*polygon_iterator);
			if((area(v[0])/area(*polygon_iterator))>1.00001)
			{
				std::cout<<"this is a wrong result "<<area(v[0])<<'\t'<<area(*polygon_iterator)<<"\n";
			}
		}
	}
	density=10000*density/area(measureArea);
	return density;
}

//---------------------------------------------------------------------------------------------

/*
 * calculate the classical density according to the coordinate of pedestrians (xs,ys) and the measurement area.
 * input: xs,ys, the number of pedestrians in the geometry and the measurement area
 * output: the classical density in the measurement area
 * note that the number of pedestrians should be the pedestrians in the the geometry used to cut the voronoi diagram.this
 * is very important. because sometimes the selected
 * geometry is smaller than the actual one. in this case, some pedestrian can not included in the geometry.
 */
float Analysis::getclassicalDensity(float *xs, float *ys, int pednum, polygon_2d measureArea)
{
	int pedsinMeasureArea=0;
	for(int i=0;i<pednum;i++)
	{
		if(within(make<point_2d>(xs[i], ys[i]), measureArea))
		{
			pedsinMeasureArea++;
		}
	}

	float density=pedsinMeasureArea*10000.00/area(measureArea);
	return density;
}
//---------------------------------------------------------------------------------------------

/*
 * calculate the voronoi velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
 * input: voronoi cell and velocity of each pedestrian and the measurement area
 * output: the voronoi velocity in the measurement area
 */
float Analysis::getVoronoiVelocity(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea)
{
	vector<polygon_2d>::iterator polygon_iterator;
	float meanV=0;
	int temp=0;

	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;

		polygon_list v;
		//intersection_inserter<polygon_2d>(measureArea, *polygon_iterator, std::back_inserter(v));
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			meanV+=Velocity[temp]*area(v[0])/area(measureArea);
			if((area(v[0])/area(*polygon_iterator))>1.00001)
			{
				std::cout<<"this is a wrong result"<<area(v[0])<<'\t'<<area(*polygon_iterator);
			}
		}
		temp++;
	}

	return meanV;
}

///---------------------------------------------------------------------------------------------------------------------
		/*this function is to obtain the frequency distribution of the pedestrian movement through a line from (Line_startX,Line_startY)
		 * to (Line_endX, Line_endY) according to the coordination of a person in two adjacent frames (pt1_X,pt1_Y) and (pt2_X,pt2_Y)
		 * input: a empty array that will be used to save the results. "fraction" is the number of same parts that the line will be divided into.
		 * output: the number of persons in each part of the line "frequency".
		 */
void Analysis::DistributionOnLine(int *frequency,int fraction, float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y)
{
	/* in this function, we firstly calculate the intersection point between the trajectory (the line from point (pt1_X,pt1_Y)
	 *  to (pt2_X,pt2_Y)) of a pedestrian and the Line. then, we determine which part of the line he or she belongs to
	 */
	float sumdistance=distance(Line_startX,Line_startY,Line_endX, Line_endY);
	float A=Line_endY-Line_startY;
	float B=Line_startX-Line_endX;
	float C=Line_endX*Line_startY-Line_startX*Line_endY;
	float d1=A*pt1_X+B*pt1_Y+C;
	float d2=A*pt2_X+B*pt2_Y+C;
	if(d1*d2<0)
	{
		float x1=Line_startX;
		float y1=Line_startY;
		float x2=Line_endX;
		float y2=Line_endY;
		float x3=pt1_X;
		float y3=pt1_Y;
		float x4=pt2_X;
		float y4=pt2_Y;
		float x =((x1 - x2) * (x3 * y4 - x4 * y3) - (x3 - x4) * (x1 * y2 - x2 * y1))
			     /((x3 - x4) * (y1 - y2) - (x1 - x2) * (y3 - y4));

		float y =((y1 - y2) * (x3 * y4 - x4 * y3) - (x1 * y2 - x2 * y1) * (y3 - y4))
			     /((y1 - y2) * (x3 - x4) - (x1 - x2) * (y3 - y4));
		int index=(int)floor(distance(x,y,x1,y1)*fraction/sumdistance);
		frequency[index]++;
	}
	else if(d1*d2==0)
	{
		if(d1==0)
		{
			int index=(int)floor(distance(Line_startX,Line_startY,pt1_X,pt1_Y)*fraction/sumdistance);
			frequency[index]++;
		}
	}

}

//-----------------------------------------------------------------------------------------------------------------
/*
 *  calculate individual density and velocity using voronoi method. the individual density is defined as the inverse
 *  of the area of the pedestrian's voronoi cell. The individual velocity is his instantaneous velocity at this time.
 *  note that, Only the pedestrians in the measurement area are considered.
 */
void Analysis::getIndividualfundmentaldiagram(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea)
{

	vector<polygon_2d>::iterator polygon_iterator;
	float uniquedensity=0;
	float uniquevelocity=0;
	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d> polygon_list;
		polygon_list v;
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			uniquedensity=10000/area(*polygon_iterator);
			uniquevelocity=Velocity[temp];
			fprintf(IndividualFD,"%.3f\t%.3f\n",uniquedensity,uniquevelocity);
		}
		temp++;
	}
}
///---------------------------------------------------------------------------------------------------------------------
float Analysis::distance(float x1, float y1, float x2, float y2)
{
	float distance=sqrt(pow((x1-x2),2)+pow((y1-y2),2));
	return distance;
}
//----------------------------------------------------------------------------------------------------------------------

/*
 *  according to the location of a pedestrian in adjacent frame (pt1_X,pt1_Y) and (pr2_X,pt2_Y), we
 *  adjust whether he pass the line from Line_start to Line_end
 */
bool Analysis::IsPassLine(float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y)
{
	float x1=Line_startX;
	float y1=Line_startY;
	float x2=Line_endX;
	float y2=Line_endY;
	float x3=pt1_X;
	float y3=pt1_Y;
	float x4=pt2_X;
	float y4=pt2_Y;

	float d=(y2-y1)*(x4-x3)-(y4-y3)*(x2-x1);
	if(d==0)
	{
		return false;
	}
	else
	{
		float x0=((x2-x1)*(x4-x3)*(y3-y1)+(y2-y1)*(x4-x3)*x1-(y4-y3)*(x2-x1)*x3)/d;
		float y0=((y2-y1)*(y4-y3)*(x3-x1)+(x2-x1)*(y4-y3)*y1-(x4-x3)*(y2-y1)*y3)/(-1.0*d);
		float temp1=(x0-x1)*(x0-x2);
		float temp2=(x0-x3)*(x0-x4);
		float temp3=(y0-y1)*(y0-y2);
		float temp4=(y0-y3)*(y0-y4);
		if(temp1<0.00000001)
		{
			temp1=0.0;
		}
		if(temp2<0.00000001)
		{
			temp2=0.0;
		}
		if(temp3<0.00000001)
		{
			temp3=0.0;
		}
		if(temp4<0.00000001)
		{
			temp4=0.0;
		}
		if(temp1<=0.0&&temp2<=0.0&&temp3<=0.0&&temp4<=0.0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	/*
	float A=Line_endY-Line_startY;
	float B=Line_startX-Line_endX;
	float C=Line_endX*Line_startY-Line_startX*Line_endY;
	float d1=A*pt1_X+B*pt1_Y+C;
	float d2=A*pt2_X+B*pt2_Y+C;
	if()
	if(d1*d2<0)  //d1*d2<0 means the two point lie in the two side of the line
	{
		return true;
	}
	else if(d1==0)  // this means the first point lies on the line, otherwise the other is not
	{
		return true;
	}
	else
	{
		return false;
	}
	*/
}

//-----------------------------------------------------------------------------------------------------------------------------
/*
 * output: the fundamental diagram based on the time a pedestrian enter and exit the measurement area
 * MC 15.08.2012
 * input: outputfile is given not anymore "datafile" 
 */
void Analysis::getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int fps, int LengthMeasurementarea,int Nped, string ofile)
{
	   FILE *fFD_TinTout;
	   string fdTinTout=ofile;
	   if((fFD_TinTout=fopen(fdTinTout.c_str(),"w"))==NULL)
		{
			printf("cannot open the file to write the TinTout data\n");
			exit(0);
		}
	   fprintf(fFD_TinTout,"#person Index\t	Individual density(m^(-2))\t	Individual velocity(m/s)\n");
	   for(int i=1;i<=Nped;i++)
	   {
           float velocity_temp=fps*0.01*LengthMeasurementarea/(Tout[i]-Tin[i]); //0.01 cm -->m
		   float density_temp=0;
		   for(int j=Tin[i];j<Tout[i];j++)
		   {
			   density_temp+=DensityPerFrame[j];
		   }
		   density_temp/=(Tout[i]-Tin[i]);
		   fprintf(fFD_TinTout,"%d\t%f\t%f\n",i,density_temp,velocity_temp);
                   //getc(stdin);
	   }
	   fclose(fFD_TinTout);
}

//void Analysis::getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int LengthMeasurementarea, int minPerIndex,int maxPerIndex, string datafile)
/*void Analysis::getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int fps, int LengthMeasurementarea, int minPerIndex,int maxPerIndex, string ofile)
{
	   FILE *fFD_TinTout;
	   string fdTinTout=ofile;
	   //"Fundamental Diagram\\TinTout\\FundamentalTinTout"+datafile+".dat";
       //cout << "fdTinTout "<<fdTinTout <<" minPerIndex "<<minPerIndex<< " maxPerIndex "<< maxPerIndex<< endl;
       //getc(stdin);
	   if((fFD_TinTout=fopen(fdTinTout.c_str(),"w"))==NULL)
			{
				printf("cannot open the file to write the TinTout data\n");
				exit(0);
			}
	   fprintf(fFD_TinTout,"#person Index\t	Individual density(m^(-2))\t	Individual velocity(m/s)\n");
	   for(int i=minPerIndex;i<=maxPerIndex;i++)
	   {
             //float velocity_temp=16*0.01*LengthMeasurementarea/(Tout[i]-Tin[i]);
             //cout << "i= "<<i<<endl;
             float velocity_temp=fps*0.01*LengthMeasurementarea/(Tout[i]-Tin[i]); //0.01 cm -->m
		   float density_temp=0;
		   for(int j=Tin[i];j<Tout[i];j++)
		   {
			   density_temp+=DensityPerFrame[j];
                           // printf("densityPerFrame[%d]=%f\n",j,DensityPerFrame[j]);
		   }
		   density_temp/=(Tout[i]-Tin[i]);
		   fprintf(fFD_TinTout,"%d\t%f\t%f\n",i,density_temp,velocity_temp);
                   //getc(stdin);
	   }
	   fclose(fFD_TinTout);
}*/
//----------------------------------------------------------------------------------------------------------------------------
/*
 * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
 * Note: here the time interval in calculating the flow rate is modified. it is the actual time between the first person and last person
 * passing the line in DeltaT.
 * MC 15.08.2012
 * input: outputfile is given not anymore "datafile" 
 */
void Analysis::FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<float> AccumVelocity, string ofile)
{

	   FILE *fFD_FlowVelocity;
	   string fdFlowVelocity = ofile;  //"Fundamental Diagram\\FlowVelocity\\FundamentalFlowVelocity"+datafile+".dat";
	   if((fFD_FlowVelocity=fopen(fdFlowVelocity.c_str(),"w"))==NULL)
			{
				printf("cannot open the file to write the Flow-Velocity data\n");
				exit(0);
			}
	   fprintf(fFD_FlowVelocity,"#Flow rate(1/s)	Mean velocity(m/s)\n");
	   int TotalTime=AccumPeds.size();  // the total Frame of in the data file
	   int TotalPeds=AccumPeds[TotalTime-1];  //the total pedestrians in the data file
	   int firstPassT=-1;  // the first time that there are pedestrians pass the line
	   int *pedspassT=new int[TotalPeds+1]; // the time for certain pedestrian passing the line
	   for(int i=0; i<=TotalPeds; i++)
	   {
		   pedspassT[i]=-1;
	   }

	    for(int ix=0; ix<TotalTime; ix++)
	    {
	        if(AccumPeds[ix]>0 && firstPassT<0)
	        {
	        	firstPassT=ix;
	        }
	        if(pedspassT[AccumPeds[ix]]<0)
	        {
	        	pedspassT[AccumPeds[ix]]=ix;
	        }
	    }
		for(int i=firstPassT+DeltaT; i<TotalTime; i+=DeltaT)
		{
			float flow_rate=fps*(AccumPeds[i]-AccumPeds[i-DeltaT])*1.00/(pedspassT[AccumPeds[i]]-pedspassT[AccumPeds[i-DeltaT]]);
			float MeanV=(AccumVelocity[i]-AccumVelocity[i-DeltaT])/(AccumPeds[i]-AccumPeds[i-DeltaT]);
			fprintf(fFD_FlowVelocity,"%.3f\t%.3f\n",flow_rate,MeanV);

		}
		fclose(fFD_FlowVelocity);
		delete []pedspassT;
}
