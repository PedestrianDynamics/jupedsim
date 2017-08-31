import numpy as np

class Point:
    """
    Point
    """
    
    def __init__(self, X, Y, Z):
        self.x = X
        self.y = Y
        self.z = Z
    

def plane(P1, P2, P3):
    A = np.array([[P1.x, P1.y, 1], [P2.x, P2.y, 1], [P3.x, P3.y, 1]])
    b = np.array([P1.z, P2.z, P3.z])
    return np.linalg.solve(A, b)
    

if __name__ == '__main__':
    p1 = Point(6.39707, 2.3033, 0)
    p2 = Point(4.94007, 2.62914, -1)
    p3 = Point(6.39707, 2.62914, -1)
    
    (A, B, C) = plane(p1, p2, p3)
    print("A_x=\"%.2f\" B_y=\"%.2f\" C_z=\"%.2f\"" %(A, B, C) )
