#define J_EPS 0.001
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void calculateForce(__global double* V_x, __global double* V_y, __global double* V0Norm, __global double* pos_x,
 __global double* pos_y, __global int* roomID, __global double* f_x, __global double* f_y, __global double* _sin, __global double* _cos,
 __global double* EA, __global double* EB, __global double* Xp, __global double* Mass,
 const double cellSize, const double delta, const double zeit, const int nSize,
 const double _distEffMaxPed, const double _intp_widthPed, const double _nuPed, const double _maxfPed,
 __global double* partialSum_x, __global double* partialSum_y, __local double* localSum_x, __local double* localSum_y){

    int pid=get_global_id(1);
    int pitems=get_global_size(1);
    int nid=get_global_id(0);
    int nitems=get_global_size(0);
    double tmpf_x, tmpf_y;

    for(int p=pid;p<nSize;p+=1024){
        double pedV_x=V_x[p];
        double pedV_y=V_y[p];
        double pedV0Norm=V0Norm[p];
        double xPed=pos_x[p];
        double yPed=pos_y[p];
        double pEA=EA[p];
        double pEB=EB[p];
        double pXp=Xp[p];
        double normVi = pedV_x*pedV_x + pedV_y*pedV_y;
        double tmp = (pedV0Norm+delta) * (pedV0Norm+delta);
        if(normVi>tmp && pedV0Norm>0){
            //fprintf(stderr, "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
            //"is bigger than desired velocity (%f) at time: %fs\n",
            //sqrt(normVi),ID[p], pedV0Norm, zeit);
            //exit(EXIT_FAILURE);
        }
	//Kraefte der Fussgaenger untereinander
	//Nachbarn finden
        for(int n=nid;n<nSize;n+=nitems){
            if(n!=p){
                if(roomID[p]==roomID[n]){
                    double nPed_x=pos_x[n];
                    double nPed_y=pos_y[n];
                    double pCosPhi=_cos[p];
                    double pSinPhi=_sin[p];
                    double nCosPhi=_cos[n];
                    double nSinPhi=_sin[n];
                    double nEA=EA[n];
                    double nEB=EB[n];
                    double nXp=Xp[n];
                    double dist=(nPed_x-xPed)*(nPed_x-xPed) + (nPed_y-yPed)*(nPed_y-yPed);
                    if(dist<cellSize*cellSize){//n liegt in der Nachbarschaft, Kraft berechnen
                        // x- and y-coordinate of the distance between p1 and p2
                        double distp12_x = nPed_x - xPed;
                        double distp12_y = nPed_y - yPed;
                        //Distanz zwischen den Ellipsen
                        double eff_dist;
                        //E2inE1
                        double tmp_x=nPed_x-xPed;
                        double tmp_y=nPed_y-yPed;
                        double E2inE1_x=tmp_x*pCosPhi-tmp_y*(-pSinPhi);
                        double E2inE1_y=tmp_x*(-pSinPhi)+tmp_y*pCosPhi;
                        //E1inE2
                        tmp_x=xPed-nPed_x;
                        tmp_y=yPed-nPed_y;
                        double E1inE2_x=tmp_x*nCosPhi-tmp_y*(-nSinPhi);
			double E1inE2_y=tmp_x*(-nSinPhi)+tmp_y*nCosPhi;
                        // distance between centers of E1 and E2
                        double elDist=sqrt(tmp_x*tmp_x+tmp_y*tmp_y);
                        //PointOnEllipse() R1 und R2
                        double R1_x, R1_y, R2_x, R2_y;
                        //R1
                        double r=E2inE1_x*E2inE1_x+E2inE1_y*E2inE1_y;
                        if(r<0.001*0.001){
                            double cp_x=pEA;
                            double cp_y=0.0;
                            //cp.CoordTransToCart
                            R1_x=cp_x*pCosPhi-cp_y*pSinPhi+xPed;
                            R1_y=cp_x*pSinPhi+cp_y*pCosPhi+yPed;
                        }
                        else{
                            r=sqrt(r);
                            double cosTheta=E2inE1_x/r;
                            double sinTheta=E2inE1_y/r;
                            double a=pEA;
                            double b=pEB;
                            double s_x=a*cosTheta;
                            double s_y=b*sinTheta;
                            //s.CoordTransToCart
                            R1_x=s_x*pCosPhi-s_y*pSinPhi+xPed;
                            R1_y=s_x*pSinPhi+s_y*pCosPhi+yPed;
                        }
                        //R2
                        r=E1inE2_x*E1inE2_x+E1inE2_y*E1inE2_y;
			if(r<0.001*0.001){
                            double cp_x=nEA;
                            double cp_y=0.0;
                            //cp.CoordTransToCart
                            R2_x=cp_x*nCosPhi-cp_y*nSinPhi+nPed_x;
                            R2_y=cp_x*nSinPhi+cp_y*nCosPhi+nPed_y;
                        }
                        else{
                            r=sqrt(r);
                            double cosTheta=E1inE2_x/r;
                            double sinTheta=E1inE2_y/r;
                            double a=nEA;
                            double b=nEB;
                            double s_x=a*cosTheta;
                            double s_y=b*sinTheta;
                            //s.CoordTransToCart
                            R2_x=s_x*nCosPhi-s_y*nSinPhi+nPed_x;
                            R2_y=s_x*nSinPhi+s_y*nCosPhi+nPed_y;
                        }
                        //effective distance
                        double norm1=sqrt((xPed-R1_x) * (xPed-R1_x) + (yPed-R1_y) * (yPed-R1_y));
                        double norm2=sqrt((nPed_x-R2_x) * (nPed_x-R2_x) + (nPed_y-R2_y) * (nPed_y-R2_y));
                        eff_dist=elDist-norm1-norm2;
                        // If the pedestrian is outside the cutoff distance, the force is zero.
                        if(eff_dist >= _distEffMaxPed) {
				f_x[p*nSize+n]=0.0;
				f_y[p*nSize+n]=0.0;
                        }
                        else{//sonst Kraft berechnen
                            double p1_x=pXp*pCosPhi+xPed;
                            double p1_y=pXp*pSinPhi+yPed;
                            double p2_x=nXp*nCosPhi+nPed_x;
                            double p2_y=nXp*nSinPhi+nPed_y;
                            double ep12_x, ep12_y;
                            distp12_x = p2_x - p1_x;
                            distp12_y = p2_y - p1_y;

                            //mindist = E1.MinimumDistanceToEllipse(E2); //ONE
                            double mindist = 0.5; //for performance reasons, it is assumed that this distance is about 50 cm
                            double dist_intpol_left = mindist + _intp_widthPed; // lower cut-off for Frep (modCFM)
                            double dist_intpol_right = _distEffMaxPed - _intp_widthPed; //upper cut-off for Frep (modCFM)
                            double smax = mindist - _intp_widthPed; //max overlapping
                            double norm=sqrt(distp12_x*distp12_x + distp12_y*distp12_y);
                            double f = 0.0f, f1 = 0.0f; //function value and its derivative at the interpolation point'
                            if ( norm >= 0.001) {
                                ep12_x = distp12_x/norm;
                                ep12_y = distp12_y/norm;
                                // calculate the parameter (whatever dist is)
                                double tmp = (pedV_x-V_x[n])*ep12_x + (pedV_y-V_y[n])*ep12_y; // < v_ij , e_ij >
                                double v_ij = 0.5 * (tmp + fabs(tmp));
                                double tmp2 = pedV_x*ep12_x + pedV_y*ep12_y; // < v_i , e_ij >
                                double K_ij;
                                //todo: runtime normsquare?
                                if (sqrt(pedV_x*pedV_x + pedV_y*pedV_y) < J_EPS) { // if(norm(v_i)==0)
                                    K_ij = 0;
                                } else {
                                    double bla = tmp2 + fabs(tmp2);
                                    K_ij = 0.25 * bla * bla / (pedV_x*pedV_x + pedV_y*pedV_y); //squared
                                    if (K_ij < 0.001 * 0.001) {
                                        f_x[p*nSize+n]=0.0;
                                        f_y[p*nSize+n]=0.0;
					continue;
                                    }
                                }
                                double nom = _nuPed * pedV0Norm + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
                                nom = nom * nom;

                                K_ij = sqrt(K_ij);
                                if (eff_dist <= smax) { //5
                                    f = -Mass[p] * K_ij * nom / dist_intpol_left;
                                    f_x[p*nSize+n]=(ep12_x * _maxfPed * f);
                                    f_y[p*nSize+n]=(ep12_y * _maxfPed * f);
                                }
                                else{
                                    if (eff_dist >= dist_intpol_right) { //2
                                        f = -Mass[p] * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
                                        f1 = -f / dist_intpol_right;
                                        //hermite_interp
                                        double t = eff_dist;
                                        double x1 = dist_intpol_right;
                                        double x2 = _distEffMaxPed;
                                        double y1 = f;
                                        double y2 =0.0;
                                        double dy1 = f1;
                                        double dy2 = 0.0;
                                        double scale = x2 - x1;
                                        t = ( t - x1 ) / scale;
                                        double t2 = t * t;
                                        double t3 = t2 * t;
                                        double h1 = 2 * t3 - 3 * t2 + 1;
                                        double h2 = -2 * t3 + 3 * t2;
                                        double h3 = t3 - 2 * t2 + t;
                                        double h4 = t3 - t2;
                                        double left = y1 * h1 + dy1 * h3 * scale;
                                        double right = y2 * h2 + dy2 * h4 * scale;
                                        double px = left + right; //hermite interpolation end
                                        f_x[p*nSize+n] =(ep12_x * px);
                                        f_y[p*nSize+n] =(ep12_y * px);
                                    }
                                    else if (eff_dist >= dist_intpol_left) { //3
                                        f = -Mass[p] * K_ij * nom / fabs(eff_dist); // abs(NR-Dv(i)+Sa)
                                        f_x[p*nSize+n] =(ep12_x * f);
                                        f_y[p*nSize+n] =(ep12_y * f);
                                    }
                                    else {//4
                                        f = -Mass[p]* K_ij * nom / dist_intpol_left;
                                        f1 = -f / dist_intpol_left;
                                        //hermite_interp
                                        double t = eff_dist;
                                        double x1 = smax;
                                        double x2 = dist_intpol_left;
                                        double y1 = _maxfPed*f;
                                        double y2 =f;
                                        double dy1 = 0;
                                        double dy2 = f1;
                                        double scale = x2 - x1;
                                        t = ( t - x1 ) / scale;
                                        double t2 = t * t;
                                        double t3 = t2 * t;
                                        double h1 = 2 * t3 - 3 * t2 + 1;
                                        double h2 = -2 * t3 + 3 * t2;
                                        double h3 = t3 - 2 * t2 + t;
                                        double h4 = t3 - t2;
                                        double left = y1 * h1 + dy1 * h3 * scale;
                                        double right = y2 * h2 + dy2 * h4 * scale;
                                        double px = left + right; //hermite interpolation end
                                        f_x[p*nSize+n] =(ep12_x * px);
                                        f_y[p*nSize+n] =(ep12_y * px);
                                    }//end if (eff_dist >= dist_intpol_right)
                                }//end if (eff_dist <= smax)
                            }//end if ( norm >= 0.001)
                        }//end if(eff_dist >= _distEffMaxPed)
                    }//end if(dist<cellSize*cellSize)
                }//end if(roomID[p]==roomID[n])
            }//end if(n!=p)
        }//end for n
    }

    barrier(CLK_GLOBAL_MEM_FENCE);
    uint local_id=get_local_id(0);
    uint group_size=get_local_size(0);
    uint row=get_global_id(1);
    uint column=get_global_id(0);
    uint row_group=get_group_id(1);
    uint column_group=get_group_id(0);

    //ueber alle Zeilen
    for(uint p=row;p<nSize; p+=pitems){
        //Jedes workitem holt sich seine Werte aus dem globalen Speicher 
        //und addiert sie auf seine Speicherstelle im lokalen Speicher
	barrier(CLK_LOCAL_MEM_FENCE);
        localSum_x[local_id]=0.0;
        localSum_y[local_id]=0.0;
        for(uint i=local_id;i<nSize;i+=group_size){
            localSum_x[local_id]+=f_x[p*nSize+i];
            localSum_y[local_id]+=f_y[p*nSize+i];
        }

        //Reduktion zwischen den workitems
        for(uint stride=group_size/2; stride>0; stride/=2){
            barrier(CLK_LOCAL_MEM_FENCE);
            if(local_id<stride){
                localSum_x[local_id]+=localSum_x[local_id+stride];
                localSum_y[local_id]+=localSum_y[local_id+stride];
            }
        }

        if(local_id==0){
            partialSum_x[p]=localSum_x[0];
            partialSum_y[p]=localSum_y[0];
        }
    }
}
