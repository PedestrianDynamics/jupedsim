# -*- coding: utf-8 -*-
"""
Created on Fri Jul 29 09:40:56 2016

@author: Erik
"""

import matplotlib.pyplot as plt


#x=[2081.023,2080.851,2082.237,2082.408,2081.023]
#y=[-14346.094,-14345.927,-14344.506,-14344.673,-14346.094]
#
#
#plt.plot(x,y,'o--')
#
#x2=[2080.975,2080.975,2090.675,2090.675,2080.975]
#y2=[-14346.220,-14345.98,-14345.98,-14346.220,-14346.220]
#
#plt.plot(x2,y2,'ro--')

###############################Geometrie########################
import numpy as np

data=np.loadtxt("./isovists/walls.txt")

#data_landmarks=np.loadtxt("./isovists/landmarks.txt")

#print (data)

ped=1

import matplotlib.pylab as plt
from matplotlib.patches import Ellipse
n=0
j=1
while n<=6000:

    plt.figure()
    ax=plt.gca()
    plt.axis('off')
    lowerx=min([min(data[:,0]),min(data[:,2])])
    upperx=max([min(data[:,0]),max(data[:,2])])
    lowery=min([min(data[:,1]),min(data[:,3])])
    uppery=max([min(data[:,1]),max(data[:,3])])
    plt.xlim([lowerx-1,upperx+5])
    plt.ylim([lowery-5,uppery+1])
    
    for i in range(len(data)):
        ax.plot([data[i,0],data[i,2]],[data[i,1],data[i,3]],"k",lw=1)

#    string='./isovists/landmarks_%d_%d.txt'%(ped,n)
#    data_landmarks=np.loadtxt(string)    
#    
#    
#
#    #ellipse = Ellipse(xy=(157.18, 68.4705), width=0.036, height=0.012, 
#    #                    edgecolor='r', fc='None', lw=2)
#    #ax.add_patch(ellipse)
#    #print(np.shape(data_landmarks))
#    if (np.shape(data_landmarks)==(7,)):
#        ax.plot(data_landmarks[0],data_landmarks[1],'ys')
#        
#        ellipse = Ellipse(xy=(data_landmarks[2], data_landmarks[3]), width=data_landmarks[4], 
#                          height=data_landmarks[5], edgecolor='r', fc='None', lw=2)
#        ax.add_patch(ellipse)
#        
#        if (data_landmarks[6]==0):
#            stringl="Landmark"
#        elif (data_landmarks[6]==1):
#            stringl="Next Target"
#        else:
#            stringl="Visited Landmark"
#            
#        ax.text(data_landmarks[2]-1.25, data_landmarks[3]-0.2,stringl,color="k",fontsize=9)
#                
#    else:
#        ax.plot(data_landmarks[:,0],data_landmarks[:,1],'ys')
##############################Visibility Graph#########################

    string='./isovists/current_env_%d_%d.txt'%(ped,n)
    data2=np.loadtxt(string)
    x=np.zeros(len(data2)+1)
    y=np.zeros(len(data2)+1)
    x[:-1]=data2[:,0]
    y[:-1]=data2[:,1]
    x[-1]=data2[0,0]
    y[-1]=data2[0,1]
    
    ax.fill(x,y,'r',alpha=0.8)
    
    string='./isovists/seen_env_%d_%d.txt'%(ped,n)
    data2=np.loadtxt(string)
    
    #plt.fill(x,y)#,'r',lw=2)
    
    x=np.zeros(len(data2)+1)
    y=np.zeros(len(data2)+1)
    x[:-1]=data2[:,0]
    y[:-1]=data2[:,1]
    x[-1]=data2[0,0]
    y[-1]=data2[0,1]
    
    ax.fill(x,y,'b',alpha=0.4)
    
    string='./isovists/real_pos%d_%d.txt'%(ped,n)
    data3=np.loadtxt(string)
    ax.plot(data3[0],data3[1],'ro')
    
    
    string='./isovists/navLines_pos%d_%d.txt'%(ped,n)
    data4=np.loadtxt(string)
    ax.plot(data4[0],data4[1],'m*')
    
    string='./isovists/navLinesBefore_pos%d_%d.txt'%(ped,n)
    data5=np.loadtxt(string)
    ax.plot(data5[0],data5[1],'b*')
    
    
    string='./isovists/pictures/img%d.png'%j
    
    plt.savefig(string)
    plt.close()
    
    j+=1
    n+=20


