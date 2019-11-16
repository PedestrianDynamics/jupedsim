from numpy import *
import matplotlib
import matplotlib.pyplot as plt
import argparse
from numpy import *
import glob,os
from mpl_toolkits.axes_grid1 import make_axes_locatable
import matplotlib.cm as cm
import datetime
import shutil

def getParserArgs():
    parser = argparse.ArgumentParser(description='Combine French data to one file')
    parser.add_argument("-N", "--pedNum", type=int , default=16, help='The number of pedestrians in the run (default 16)')
    parser.add_argument("-r", "--runName", default="XP01", help='give the name of each run')
    args = parser.parse_args()
    return args

def get_dateString():
    date = str(datetime.datetime.today())                   #
    date = date.replace('-', ' ')
    date = date.replace(':', ' ')
    date = date.replace('.', ' ')
    date = date.split()
    date[0] = str(int(date[0])-2000)
    dateString = str(date[0] + date[1] + date[2] + '_' + date[3] + date[4] + date[5])

    return dateString

def get_archive(dateString):
    os.makedirs('%s_traj_OneD_V'                        %(dateString))
    os.makedirs('%s_traj_OneD_V/INPUT'                  %(dateString))
    if not os.path.exists('%s_traj_OneD_V/OUTPUT/'%(dateString)):
        os.makedirs('%s_traj_OneD_V/OUTPUT'             %(dateString))


def get_backupInput(dateString):

    #  Sichern der INPUT-Dateien
    #==========================================================================
    dst = '%s_traj_OneD_V/INPUT' %(dateString)
    src = '2Dto1D_ID.py'
    shutil.copy(src, dst)
    print ('--> Gesichert: 2Dto1D_ID.py')

def traj_TwoD2OneD(trajectory):
    traj=loadtxt(trajectory)
    traj[:,2]/=100.0
    traj[:,3]/=100.0
    lastpos=0
    lastID=1
    data=array([[0,0,0,0]])
    ID=lastID
    for i in range(shape(traj)[0]):
        if(traj[i,2]>-2.5 and traj[i,2]<2.5 and traj[i,3]<0):
            posx=traj[i,2]+2.5
            posy=0.5*(2.1+2.9)-fabs(traj[i,3])
        elif(traj[i,2]>-2.5 and traj[i,2]<2.5 and traj[i,3]>0):
            posx=12.85+2.5-traj[i,2]
            posy=0.5*(2.1+2.9)-fabs(traj[i,3])
        elif(traj[i,2]>=2.5):
            #print(traj[i,2])
            dis=square(traj[i,3]+2.5)+square(traj[i,2]-2.5)
            #print(dis)
            #print((6.25+square(traj[i,3])+square(traj[i,2])-dis)/12.5)
            theta=arccos((6.25+square(traj[i,3])+square(traj[i,2]-2.5)-dis)/(5*sqrt(square(traj[i,3])+square(traj[i,2]-2.5))))
            posx=5+theta*2.5
            disy=sqrt(pow(traj[i,2]-2.5,2)+pow(traj[i,3],2))
            posy=0.5*(2.1+2.9)-disy
            
        elif(traj[i,2]<=-2.5):
            #print(traj[i,2])
            dis=square(traj[i,3]-2.5)+square(traj[i,2]+2.5)
            theta=arccos((6.25+square(traj[i,3])+square(traj[i,2]+2.5)-dis)/(5*sqrt(square(traj[i,3])+square(traj[i,2]+2.5))))
            posx=17.85+theta*2.5
            disy=sqrt(pow(traj[i,2]+2.5,2)+pow(traj[i,3],2))
            posy=0.5*(2.1+2.9)-disy
#        if(fabs(lastpos-posx)>25):
#            ID+=1
#        elif (traj[i,0]!=lastID):
        if (traj[i,0]!=lastID):
            ID+=1
        lastpos=posx
        lastID=traj[i,0]
        data=append(data,array([[ID,traj[i,1],posx,posy]]),axis=0)
    data=data[1:]
    return data
    #savetxt("traj_ring_adults_06_06.txt",data, fmt='%d %d %.3f %.3f', delimiter='\t')

def add_instantneousV(file, fps=25, timeinterval=10):
    data=traj_TwoD2OneD(file)
    deltaT=int(timeinterval/2)
    V = array([[0,0]])
    V=delete(V,0,0)
    for j in unique(data[:,0]):
        trajectory=data[data[:,0]==j]
        Frames=trajectory[:,1]
        T=trajectory[:,1]/fps
        Sx=trajectory[:,2]
        Sy=trajectory[:,3]
        for t in range(len(Frames)):
            if(len(Frames)>timeinterval):
                if (t < deltaT):
                    if (Sx[t+deltaT] - Sx[t])<-20: 
                        vx = (Sx[t+deltaT] - Sx[t]+25.7)/(T[t+deltaT]-T[t])
                    elif (Sx[t+deltaT] - Sx[t])>20: 
                        vx = (Sx[t+deltaT] - Sx[t]-25.7)/(T[t+deltaT]-T[t])
                    else:
                        vx = (Sx[t+deltaT] - Sx[t])/(T[t+deltaT]-T[t])
                    vy = (Sy[t+deltaT] - Sy[t])/(T[t+deltaT]-T[t])
                elif (t > len(Frames)-deltaT-1):
                    if (Sx[t] - Sx[t-deltaT])<-20: 
                        vx = (Sx[t] - Sx[t-deltaT]+25.7)/(T[t]-T[t-deltaT])
                    elif (Sx[t] - Sx[t-deltaT])>20: 
                        vx = (Sx[t] - Sx[t-deltaT]-25.7)/(T[t]-T[t-deltaT])
                    else:
                        vx = (Sx[t] - Sx[t-deltaT])/(T[t]-T[t-deltaT])
                    vy = (Sy[t] - Sy[t-deltaT])/(T[t]-T[t-deltaT])
                else:
                    if (Sx[t+deltaT] - Sx[t-deltaT])<-20: 
                        vx = (Sx[t+deltaT] - Sx[t-deltaT]+25.7)/(T[t+deltaT]-T[t-deltaT])
                    elif (Sx[t+deltaT] - Sx[t-deltaT])>20: 
                        vx = (Sx[t+deltaT] - Sx[t-deltaT]-25.7)/(T[t+deltaT]-T[t-deltaT])
                    else:
                        vx = (Sx[t+deltaT] - Sx[t-deltaT])/(T[t+deltaT]-T[t-deltaT])
                    vy = (Sy[t+deltaT] - Sy[t-deltaT])/(T[t+deltaT]-T[t-deltaT])
            else:
                vx=0
                vy=0
            V=append(V,array([[vx,vy]]),axis=0)
    data=append(data,V,axis=1)
    return data


def traj_TwoD2OneD_V(directory, dateString):
    files = glob.glob(directory+"/*students*.txt")
    for file in files:
        print(file)
        oneD_traj_file=dateString+"_traj_OneD_V/OUTPUT/OneD_traj_"+file.split("ring_")[1].split("_shifted")[0]+".txt"
        header="#description: %s\n#framerate: %d\n#ID: the agent ID\n#FR: the current frame\n#X,Y,v: the agents coordinates (in metres) and instantneous velocity (m/s)\n\n#ID\tFR\tX\tY\tVx\tVy"%(file,fps)
        data=add_instantneousV(file)
        savetxt(oneD_traj_file, data,fmt='%d %d %.3f %.3f %.3f %.3f', header=header, comments="", delimiter='\t',newline='\r\n')


if __name__ == '__main__':
    args = getParserArgs()
    Npeds=args.pedNum         
    runName=args.runName
    fps=25.0
    dateString = get_dateString()
    get_archive(dateString)
    get_backupInput(dateString)
    traj_TwoD2OneD_V("./_trajectory",dateString)
#    files = glob.glob("./_traj_OneD/stop/*.txt")
#    for file in files:
#        print(file)
#        data=loadtxt(file)
#        figname=dateString+"_TimeSpace/OUTPUT/TS_"+file.split("OneD_traj_")[1].split(".")[0]
#        print(figname)
#        plot_VTimeSpaceDiagram2(data,figname)
    
