#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <math.h>
#include <GL/glut.h>

#define IMAGE			"squares.bmp"
#define IMAGE_FILE_NAME "Output.pgm"


#define ITERATIONS   500
#define THRESHOLD	 10
#define EPSILON		 100



#define RITS		 50

float *phi, *D;
uchar4 *h_Src, *h_Mask;
int imageW, imageH, N;

float *d_phi, *d_D;


void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
void sedt2d(int *_d,unsigned char *_bimg,int _h,int _w);


int its=0;

int r;
int c;
int i;

__global__ void updatephi( float *d_phi, float *d_phi1, float *d_D, int imageW, int imageH, 
						 float *dx, float *dxplus, float *dxminus, float *dxplusy, float *dxminusy,
						 float *maxdxplus, float *maxminusdxminus, float *mindxplus, float *minminusdxminus,
						 float *dy, float *dyplus, float *dyminus, float *dyplusx, float *dyminusx,
						 float *maxdyplus, float *maxminusdyminus, float *mindyplus, float *minminusdyminus,
						 float *nplusx,float *nplusy,float *nminusx,float *nminusy,float *curvature,
						 float *gradphimax,float *gradphimin, float *F, float *gradphi);

void init(){

	int *initsdf;
	unsigned char *mask;
	const char *mask_path = "mask.bmp";
	const char *image_path = IMAGE;
	if((initsdf=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INITSDF\n");
	if((phi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_PHI\n");
	mask = (unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));
	D = (float *)malloc(imageW*imageH*sizeof(float));

	//printf("Init Mask\n");
	LoadBMPFile(&h_Mask, &imageW, &imageH, mask_path);
	LoadBMPFile(&h_Src, &imageW, &imageH, image_path);

	N = imageW*imageH;

	for(r=0;r<imageH;r++){
		for(c=0;c<imageW;c++){
			mask[r*imageW+c] = (h_Mask[r*imageW+c].x)/255;
			//printf("%3d ", mask[r*imageW+c]);
		}
		//printf("\n");
	}

	sedt2d(initsdf,mask,imageH,imageW);
	
	
	//printf("sdf of init mask\n");
	for(i=0;i<N;i++){
		phi[i]=(float)initsdf[i];
		if(phi[i]>0){
			phi[i]=0.5*sqrt(abs(phi[i]));
		} else {
			phi[i]=-0.5*sqrt(abs(phi[i]));
		}
	}

		for(i=0;i<N;i++){
			D[i] =(float)h_Src[i].x;
			D[i] = EPSILON - abs(D[i] - THRESHOLD);
	}

	free(init);
	free(mask);


}

//void reinit_phi(){
//
//	int *intphi;
//	unsigned char *reinit;
//	if((intphi=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INIT\n");
//	reinit=(unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));//TODO check
//
//	for(i=0;i<N;i++){
//		if(phi[i]<0){
//			phi[i]=1;
//		} else {
//			phi[i]=0;
//		}
//		reinit[i]=(int)phi[i];
//	}
//
//
//	sedt2d(intphi,reinit,imageH,imageW);
//
//	printf("ReInit @ %4d its\n",its);
//	for(r=0;r<imageH;r++){
//		for(c=0;c<imageW;c++){
//			phi[r*imageW+c]=(float)intphi[r*imageW+c];
//			if(phi[r*imageW+c]>0){
//				phi[r*imageW+c]=0.5*sqrt(abs(phi[r*imageW+c]));
//			} else {
//				phi[r*imageW+c]=-0.5*sqrt(abs(phi[r*imageW+c]));
//			}
//			//printf("%6.3f ", phi[r*imageW+c]);
//		}
//		//printf("\n");
//	}
//
//	free(reinit);
//	free(intphi);
//}


void cuda_update(dim3 dimGrid, dim3 dimBlock){

	float *dx,*dxplus,*dxminus,*dxplusy,*dxminusy;
	float *maxdxplus, *maxminusdxminus, *mindxplus, *minminusdxminus;

	float *dy, *dyplus,*dyminus,*dyplusx,*dyminusx;
	float *maxdyplus, *maxminusdyminus, *mindyplus, *minminusdyminus;

	float *gradphimax, *gradphimin, *nplusx, *nplusy, *nminusx, *nminusy, *curvature;
    float *F, *gradphi, *d_phi1;
	
	cudaMalloc((void**)&dx,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dxplus,     sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dxminus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dxplusy,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dxminusy,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&maxdxplus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&maxminusdxminus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&mindxplus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&minminusdxminus,    sizeof(float)*imageW*imageH);

	cudaMalloc((void**)&dy,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dyplus,     sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dyminus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dyplusx,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&dyminusx,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&maxdyplus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&maxminusdyminus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&mindyplus,    sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&minminusdyminus,    sizeof(float)*imageW*imageH);

	cudaMalloc((void**)&gradphimax,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&gradphimin,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&nplusx,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&nplusy,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&nminusx,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&nminusy,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&curvature,         sizeof(float)*imageW*imageH);

	cudaMalloc((void**)&F,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&gradphi,         sizeof(float)*imageW*imageH);
	cudaMalloc((void**)&d_phi1,         sizeof(float)*imageW*imageH);

	cudaMemcpy(d_phi1, phi, sizeof(float)*imageW*imageH, cudaMemcpyHostToDevice);




	updatephi<<< dimGrid, dimBlock>>>( d_phi, d_phi1, d_D,  imageW,  imageH, 
						 dx, dxplus, dxminus, dxplusy, dxminusy,
						 maxdxplus, maxminusdxminus, mindxplus, minminusdxminus,
						 dy, dyplus, dyminus, dyplusx, dyminusx,
						 maxdyplus, maxminusdyminus, mindyplus, minminusdyminus,
						 nplusx,nplusy,nminusx,nminusy,curvature,
						 gradphimax,gradphimin, F, gradphi);

	cudaMemcpy(phi, d_phi, sizeof(float)*imageW*imageH, cudaMemcpyDeviceToHost);

	//printf("Speed Function\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%3.0f ", phi[r*imageW+c]);
	//	}
	//	printf("\n");
	//}

	cudaFree(dx);
	cudaFree(dxplus);
	cudaFree(dxminus);
	cudaFree(dxplusy);
	cudaFree(dxminusy);
	cudaFree(maxdxplus);
	cudaFree(maxminusdxminus);
	cudaFree(mindxplus);
	cudaFree(mindxplus);
	cudaFree(dy);
	cudaFree(dyplus);
	cudaFree(dyminus);
	cudaFree(dyplusx);
	cudaFree(dyminusx);
	cudaFree(maxdyplus);
	cudaFree(maxminusdyminus);
	cudaFree(mindyplus);
	cudaFree(minminusdyminus);
	cudaFree(gradphimax);
	cudaFree(gradphimin);
	cudaFree(nplusx);
	cudaFree(nplusy);
	cudaFree(nminusx);
	cudaFree(nminusy);
	cudaFree(curvature);
	cudaFree(F);
	cudaFree(gradphi);
	cudaFree(d_phi1);

}



int main(int argc, char** argv){

	init();

	dim3 dimGrid( ((imageW-1)/4) + 1, ((imageH-1)/32) +1 );
	dim3 dimBlock(4, 32);

	cudaMalloc((void**)&d_D,         sizeof(float)*imageW*imageH);
    cudaMemcpy(d_D, D, sizeof(float)*imageW*imageH, cudaMemcpyHostToDevice);

	cudaMalloc((void**)&d_phi, sizeof(float)*imageW*imageH);
	

	for(its=0;its<ITERATIONS;its++){
		cuda_update(dimGrid, dimBlock);
	}
	
}




//TODO Memory Malloc Free

//TODO Timer

//TODO Comment Code
