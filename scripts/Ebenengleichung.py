# -*- coding: utf-8 -*-
"""
Created on Mon Mar 30 20:47:09 2015

@author: Max
"""


"""
Gleichung einer Ebene durch 3 Punkte: P_1, P_2, P_3
"""


# Punkt P_1:
P_1_x = 6.39707
P_1_y = 2.3033 
P_1_z = 0

# Punkt P_2:    
P_2_x = 4.94007
P_2_y = 2.62914
P_2_z = -1

# Punkt P_3:
P_3_x = 6.39707
P_3_y = 2.62914
P_3_z = -1

# Angabe von d erforderlich, Wert egal, außer 0
d = 1


# Berechnung der Parameter
c = (((1-P_3_x/P_1_x*1)-((P_3_y-P_3_x/P_1_x*P_1_y)/(P_2_y-P_2_x/P_1_x*P_1_y))*(1-P_2_x/P_1_x*1))/((P_3_z-P_3_x/P_1_x*P_1_z)-((P_3_y-P_3_x/P_1_x*P_1_y)/(P_2_y-P_2_x/P_1_x*P_1_y))*(P_2_z-P_2_x/P_1_x*P_1_z)))*d
b = ((1-P_3_x/P_1_x*1)*d-(P_3_z-P_3_x/P_1_x*P_1_z)*c)/(P_3_y-P_3_x/P_1_x*P_1_y)
a = (d-P_1_z*c-P_1_y*b)/P_1_x

a_x = -a/c
b_y = -b/c
c_z = d/c

# ausgabe der Parameter
print("A_x= %.2f B_y= %.2f C_z = %.2f" %(a_x, b_y, c_z))

