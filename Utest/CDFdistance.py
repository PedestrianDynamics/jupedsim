#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Mar  6 10:36:37 2017

@author: valentina
"""

import numpy as np

def CalcBiVarCDF(x,y,xGrid,yGrid):
    """
    Calculate the bivariate CDF of two given input signals on predefined grids. 
    input: 
      - x: array of size n1
      - y: array of size n2
      - xGrid: array of size m1
      - yGrid: array of size m2
    output: 
      - CDF2D: matrix
    """
    nPoints = np.size(x);
    xGridLen = np.size(xGrid);
    yGridLen = np.size(yGrid);
    CDF2D = np.zeros([xGridLen,yGridLen]);
    for i in range(xGridLen):
        for j in range(yGridLen):
            for k in range(nPoints):
                if ((x[k] <= xGrid[i]) and (y[k] <= yGrid[j])):
                     CDF2D[i,j] += 1;

    CDF2D = CDF2D / nPoints;
    return CDF2D


def CDFDistance(x1, y1, x2, y2):
    """
    For two input 2D signals calculate the distance between their CDFs. 
    input: 
      - x1: array of size n
      - y2: array of size n
      - x2: array of size m
      - y2: array of size m
    output: 
      - KSD: not negative number
    """
    xPoints = 100;
    yPoints = 100;
    x = np.hstack((x1, x2))
    xCommonGrid = np.linspace(np.min(x), np.max(x), xPoints);
    y = np.hstack((y1, y2))
    yCommonGrid = np.linspace(np.min(y), np.max(y), yPoints);
    CDF1 = CalcBiVarCDF(x1,y1,xCommonGrid,yCommonGrid);
    CDF2 = CalcBiVarCDF(x2,y2,xCommonGrid,yCommonGrid);
#    KSD = np.linalg.norm(CDF1-CDF2); # Frobenius norm (p=2)
    KSD = np.max(np.abs(CDF1-CDF2)); # Kolmogorov-Smirnov distance (p=inf)
    return KSD
        

if __name__ == "__main__":
    N = 5
    M = 10
    x1 = np.random.sample(N);
    y1 = np.random.random(N);
    x2 = np.random.random(M);
    y2 = np.random.random(M);
    print(CDFDistance(x1, y1, x2, y2));
#    print(x, test(x))


