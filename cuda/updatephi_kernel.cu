
#define ALPHA		 0.00
#define DT			 0.2

__global__ void updatephi( float *d_phi, float *d_phi1, float *d_D, int imageW, int imageH, 
						 float *dx, float *dxplus, float *dxminus, float *dxplusy, float *dxminusy,
						 float *maxdxplus, float *maxminusdxminus, float *mindxplus, float *minminusdxminus,
						 float *dy, float *dyplus, float *dyminus, float *dyplusx, float *dyminusx,
						 float *maxdyplus, float *maxminusdyminus, float *mindyplus, float *minminusdyminus,
						 float *nplusx,float *nplusy,float *nminusx,float *nminusy,float *curvature,
						 float *gradphimax,float *gradphimin, float *F, float *gradphi)

{
	int c= blockIdx.x * blockDim.x + threadIdx.x;
	int r= blockIdx.y * blockDim.y + threadIdx.y;
	int ind= r*imageW+c;

	if(ind<imageW*imageH){

		if(c==0||c==imageW-1){dx[ind]=0;} else {dx[ind]=(d_phi1[ind+1]-d_phi1[ind-1])/2;}
		if(c==imageW-1){dxplus[ind]=0;} else {dxplus[ind]=(d_phi1[ind+1]-d_phi1[ind]);}
		if(c==0){dxminus[ind]=0;} else {dxminus[ind]=(d_phi1[ind]-d_phi1[ind-1]);}
		if(r==0||c==0||c==imageW-1){dxplusy[ind]=0;} else {dxplusy[ind]=(d_phi1[ind-imageW+1]-d_phi1[ind-imageW-1])/2;}
		if(r==imageH-1||c==0||c==imageW-1){dxminusy[ind]=0;} else {dxminusy[ind]=(d_phi1[ind+imageW+1]-d_phi1[ind+imageW-1])/2;}
		if(dxplus[ind]<0){maxdxplus[ind]=0;} else { maxdxplus[ind]= dxplus[ind]*dxplus[ind]; }
		if(-dxminus[ind]<0){maxminusdxminus[ind]=0;} else { maxminusdxminus[ind]= dxminus[ind]*dxminus[ind]; }
		if(dxplus[ind]>0){mindxplus[ind]=0;} else { mindxplus[ind]= dxplus[ind]*dxplus[ind]; }
		if(-dxminus[ind]>0){minminusdxminus[ind]=0;} else { minminusdxminus[ind]= dxminus[ind]*dxminus[ind]; }

		if(r==0||r==imageH-1){dy[ind]=0;} else {dy[ind]=(d_phi1[ind-imageW]-d_phi1[ind+imageW])/2;}
		if(r==0){dyplus[ind]=0;} else {dyplus[ind]=(d_phi1[ind-imageW]-d_phi1[ind]);}
		if(r==imageH-1){dyminus[ind]=0;} else {dyminus[ind]=(d_phi1[ind]-d_phi1[ind+imageW]);}
		if(r==0||c==imageW-1||r==imageH-1){dyplusx[ind]=0;} else {dyplusx[ind]=(d_phi1[ind-imageW+1]-d_phi1[ind+imageW+1])/2;}
		if(r==0||c==0||r==imageH-1){dyminusx[ind]=0;} else {dyminusx[ind]=(d_phi1[ind-imageW-1]-d_phi1[ind+imageW-1])/2;}
		if(dyplus[ind]<0){maxdyplus[ind]=0;} else { maxdyplus[ind]= dyplus[ind]*dyplus[ind]; }
		if(-dyminus[ind]<0){maxminusdyminus[ind]=0;} else { maxminusdyminus[ind]= dyminus[ind]*dyminus[ind]; }
		if(dyplus[ind]>0){mindyplus[ind]=0;} else { mindyplus[ind]= dyplus[ind]*dyplus[ind]; }
		if(-dyminus[ind]>0){minminusdyminus[ind]=0;} else { minminusdyminus[ind]= dyminus[ind]*dyminus[ind]; }

		gradphimax[ind]=sqrt((sqrt(maxdxplus[ind]+maxminusdxminus[ind]))*(sqrt(maxdxplus[ind]+maxminusdxminus[ind]))+(sqrt(maxdyplus[ind]+maxminusdyminus[ind]))*(sqrt(maxdyplus[ind]+maxminusdyminus[ind])));
		gradphimin[ind]=sqrt((sqrt(mindxplus[ind]+minminusdxminus[ind]))*(sqrt(mindxplus[ind]+minminusdxminus[ind]))+(sqrt(mindyplus[ind]+minminusdyminus[ind]))*(sqrt(mindyplus[ind]+minminusdyminus[ind])));
		nplusx[ind]= dxplus[ind] / sqrt(1.192092896e-07F + (dxplus[ind]*dxplus[ind]) + ((dyplusx[ind] + dy[ind])*(dyplusx[ind] + dy[ind])*0.25) );
		nplusy[ind]= dyplus[ind] / sqrt(1.192092896e-07F + (dyplus[ind]*dyplus[ind]) + ((dxplusy[ind] + dx[ind])*(dxplusy[ind] + dx[ind])*0.25) );
		nminusx[ind]= dxminus[ind] / sqrt(1.192092896e-07F + (dxminus[ind]*dxminus[ind]) + ((dyminusx[ind] + dy[ind])*(dyminusx[ind] + dy[ind])*0.25) );
		nminusy[ind]= dyminus[ind] / sqrt(1.192092896e-07F + (dyminus[ind]*dyminus[ind]) + ((dxminusy[ind] + dx[ind])*(dxminusy[ind] + dx[ind])*0.25) );
		curvature[ind]= ((nplusx[ind]-nminusx[ind])+(nplusy[ind]-nminusy[ind]))/2;
		
		F[ind] = (ALPHA * d_D[ind]) + ((1-ALPHA) * curvature[ind]);
		if(F[ind]>0) {gradphi[ind]=gradphimax[ind];} else {gradphi[ind]=gradphimin[ind];}
		d_phi[ind]=d_phi1[ind] + (DT * F[ind] * gradphi[ind]);
	
}
}
		



