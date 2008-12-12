#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <cutil.h>
#include <math.h>

float *phi;
uchar4 *h_Src, *h_Mask;
int imageW, imageH;
unsigned char *img, *mask;

void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
float init_phi();
void sedt2d(int *_d,unsigned char *_bimg,int _h,int _w);
void shiftR(float *SROUT, float *SRIN);
void shiftL(float *SLOUT, float *SLIN);
void shiftU(float *SUOUT, float *SUIN);
void shiftD(float *SDOUT, float *SDIN);
float max(float *MAXIN);
float min(float *MININ);

int main(){

	const char *image_path = "squares.bmp";

	LoadBMPFile(&h_Src, &imageW, &imageH, image_path);
	img = (unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));

	//printf("Input Image\n");
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			img[r*imageW+c] = h_Src[r*imageW+c].x;
			//printf("%3d ", img[r*imageW+c]);
		}
		//printf("\n");
	}
	free(h_Src);
	free(img);
	
	init_phi();

	//DXPLUS
	float *dxplus;
	dxplus=(float *)malloc(imageW*imageH*sizeof(*dxplus));

	float *SR;
	SR=(float *)malloc(imageW*imageH*sizeof(*SR));
	shiftR(SR, phi);
	for(int i=0;i<imageW*imageH;i++){
		dxplus[i]=SR[i]-phi[i];
	}

	//minusDXMINUS
	float *minusdxminus;
	minusdxminus=(float *)malloc(imageW*imageH*sizeof(*minusdxminus));

	float *SL;
	SL=(float *)malloc(imageW*imageH*sizeof(*SL));
	shiftL(SL, phi);
	for(int i=0;i<imageW*imageH;i++){
		minusdxminus[i]=SL[i]-phi[i];
	}
	free(SL);

	float *maxdxplus;
	maxdxplus=(float *)malloc(imageW*imageH*sizeof(*maxdxplus));
	for(int i=0;i<imageW*imageH;i++){
		maxdxplus[i]=max(&dxplus[i]);
		maxdxplus[i]=maxdxplus[i]*maxdxplus[i];
	}

	float *mindxplus;
	mindxplus=(float *)malloc(imageW*imageH*sizeof(*mindxplus));
	for(int i=0;i<imageW*imageH;i++){
		mindxplus[i]=min(&dxplus[i]);
		mindxplus[i]=mindxplus[i]*mindxplus[i];
	}
	free(dxplus);

	float *maxminusdxminus;
	maxminusdxminus=(float *)malloc(imageW*imageH*sizeof(*maxminusdxminus));
	for(int i=0;i<imageW*imageH;i++){
		maxminusdxminus[i]=max(&minusdxminus[i]);
		maxminusdxminus[i]=maxminusdxminus[i]*maxminusdxminus[i];
	}

	float *minminusdxminus;
	minminusdxminus=(float *)malloc(imageW*imageH*sizeof(*minminusdxminus));
	for(int i=0;i<imageW*imageH;i++){
		minminusdxminus[i]=min(&minusdxminus[i]);
		minminusdxminus[i]=minminusdxminus[i]*minminusdxminus[i];
	}
	free(minusdxminus);

		//GRADPHIMAX_X
		float *gradphimax_x;
		gradphimax_x=(float *)malloc(imageW*imageH*sizeof(*gradphimax_x));
		for(int i=0;i<imageW*imageH;i++){
			gradphimax_x[i]=sqrt(maxdxplus[i]+maxminusdxminus[i]);
		}

		free(maxdxplus);
		free(maxminusdxminus);

		//GRADPHIMIN_X
		float *gradphimin_x;
		gradphimin_x=(float *)malloc(imageW*imageH*sizeof(*gradphimin_x));
		for(int i=0;i<imageW*imageH;i++){
			gradphimin_x[i]=sqrt(mindxplus[i]+minminusdxminus[i]);
		}

		free(mindxplus);
		free(minminusdxminus);

		free(gradphimax_x);
		free(gradphimin_x);

	//DYPLUS
	float *dyplus;
	dyplus=(float *)malloc(imageW*imageH*sizeof(*phi));

	float *SU;
	SU=(float *)malloc(imageW*imageH*sizeof(*SU));
	shiftU(SU, phi);
	for(int i=0;i<imageW*imageH;i++){
		dyplus[i]=SU[i]-phi[i];
	}
	free(SU);

	//minusDYMINUS
	float *minusdyminus;
	minusdyminus=(float *)malloc(imageW*imageH*sizeof(*phi));

	float *SD;
	SD=(float *)malloc(imageW*imageH*sizeof(*SD));
	shiftD(SD, phi);
	for(int i=0;i<imageW*imageH;i++){
		minusdyminus[i]=SD[i]-phi[i];
	}
	free(SD);

	float *maxdyplus;
	maxdyplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxdyplus[i]=max(&dyplus[i]);
	}
	float *mindyplus;
	mindyplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		mindyplus[i]=min(&dyplus[i]);
	}
	free(dyplus);

	float *maxminusdyminus;
	maxminusdyminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxminusdyminus[i]=max(&minusdyminus[i]);
	}
	float *minminusdyminus;
	minminusdyminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		minminusdyminus[i]=min(&minusdyminus[i]);
	}
	free(minusdyminus);

		//GRADPHIMAX_Y
		float *gradphimax_y;
		gradphimax_y=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimax_y[i]=sqrt(maxdyplus[i]+maxminusdyminus[i]);
		}

		free(maxdyplus);
		free(maxminusdyminus);

		//GRADPHIMIN_Y
		float *gradphimin_y;
		gradphimin_y=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimin_y[i]=sqrt(mindyplus[i]+minminusdyminus[i]);
		}

		free(mindyplus);
		free(minminusdyminus);

		free(phi);
}


float init_phi(){

	const char *mask_path = "mask.bmp";

	printf("Init Mask\n");
	LoadBMPFile(&h_Mask, &imageW, &imageH, mask_path);
	mask = (unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));
	
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			mask[r*imageW+c] = h_Mask[r*imageW+c].x;
			printf("%3d ", mask[r*imageW+c]);
		}
		printf("\n");
	}
	
	int *init;
	init=(int *)malloc(imageW*imageH*sizeof(*init));
	phi=(float *)malloc(imageW*imageH*sizeof(*phi));
	sedt2d(init,mask,imageH,imageW);

	free(mask);
	free(h_Mask);

	printf("sdf of init mask\n");
		for(int r=0;r<imageH;r++){
			for(int c=0;c<imageW;c++){
				phi[r*imageW+c]=-(float)init[r*imageW+c];
				printf("%3.0f ", phi[r*imageW+c]);
			}
			printf("\n");
		}

	free(init);

	return *phi;
}

void shiftR(float *SROUT, float *SRIN){

	for(int r=0;r<imageH;r++){
		for(int c=1;c<imageW;c++){
			SROUT[r*imageW+c] = SRIN[r*imageW+c-1];
		}
			SROUT[r*imageW]=0;
	}
}


void shiftL(float *SLOUT, float *SLIN){

	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW-1;c++){
			SLOUT[r*imageW+c] = SLIN[r*imageW+c+1];
		}
	}
	for(int r=0;r<imageH;r++){
		SLOUT[r*imageW+imageW-1]=0;
	}
}

void shiftU(float *SUOUT, int *SUIN){

	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			SUOUT[r*imageW+c] = SUIN[r*imageW+c+imageW];
			SUOUT[(imageH-1)*imageW+c]=0;
		}
	}
}

void shiftD(float *SDOUT, int *SDIN){

	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			SDOUT[r*imageW+c] = SDIN[r*imageW+c-imageW];
			SDOUT[c]=0;
		}
	}
}

float max(float *MAXIN){

	for(int i=0; i<imageW*imageH ; i++){
		if(MAXIN[i]<0)MAXIN[i]=0;
	}
	return *MAXIN;
}

float min(float *MININ){

	for(int i=0; i<imageW*imageH ; i++){
		if(MININ[i]>0)MININ[i]=0;
	}
	return *MININ;
}