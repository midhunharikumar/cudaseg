#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <math.h>
#include <GL/glut.h>

#define IMAGE			"bigbrain.bmp"
#define IMAGE_FILE_NAME "Output.pgm"


#define ITERATIONS   500
#define THRESHOLD	 10
#define EPSILON		 100

#define ALPHA		 0.009
#define DT			 0.2

#define RITS		 50

float *phi, *D, *contour;
uchar4 *h_Src, *h_Mask;
int imageW, imageH, N;



void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
void sedt2d(int *_d,unsigned char *_bimg,int _h,int _w);


int its=0;

int r;
int c;
int i;

void init_phi(){

	int *init;
	unsigned char *mask;
	const char *mask_path = "mask.bmp";
	if((init=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INIT\n");
	if((phi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_PHI\n");
	mask = (unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));

	//printf("Init Mask\n");
	LoadBMPFile(&h_Mask, &imageW, &imageH, mask_path);
	

	for(r=0;r<imageH;r++){
		for(c=0;c<imageW;c++){
			mask[r*imageW+c] = (h_Mask[r*imageW+c].x)/255;
			//printf("%3d ", mask[r*imageW+c]);
		}
		//printf("\n");
	}

	

	sedt2d(init,mask,imageH,imageW);



	//printf("sdf of init mask\n");
	for(r=0;r<imageH;r++){
		for(c=0;c<imageW;c++){
			phi[r*imageW+c]=(float)init[r*imageW+c];
			if(phi[r*imageW+c]>0){
				phi[r*imageW+c]=0.5*sqrt(abs(phi[r*imageW+c]));
			} else {
				phi[r*imageW+c]=-0.5*sqrt(abs(phi[r*imageW+c]));
			}
			//printf("%6.3f ", phi[r*imageW+c]);
		}
		//printf("\n");
	}

	free(init);
	free(mask);
}

void reinit_phi(){

	int *intphi;
	unsigned char *reinit;
	if((intphi=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INIT\n");
	reinit=(unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));//TODO check

	for(i=0;i<N;i++){
		if(phi[i]<0){
			phi[i]=1;
		} else {
			phi[i]=0;
		}
		reinit[i]=(int)phi[i];
	}


	sedt2d(intphi,reinit,imageH,imageW);

	printf("ReInit @ %4d its\n",its);
	for(r=0;r<imageH;r++){
		for(c=0;c<imageW;c++){
			phi[r*imageW+c]=(float)intphi[r*imageW+c];
			if(phi[r*imageW+c]>0){
				phi[r*imageW+c]=0.5*sqrt(abs(phi[r*imageW+c]));
			} else {
				phi[r*imageW+c]=-0.5*sqrt(abs(phi[r*imageW+c]));
			}
			//printf("%6.3f ", phi[r*imageW+c]);
		}
		//printf("\n");
	}

	free(reinit);
	free(intphi);
}


void update_phi(){

float *dx;
float *dxplus;
float *dxminus;
float *dxplusy;
float *dxminusy;
float *maxdxplus;
float *maxminusdxminus;
float *mindxplus;
float *minminusdxminus;

float *dy;
float *dyplus;
float *dyminus;
float *dyplusx;
float *dyminusx;
float *maxdyplus;
float *maxminusdyminus;
float *mindyplus;
float *minminusdyminus;

float *gradphimax;
float *gradphimin;

float *nplusx;
float *nplusy;
float *nminusx;
float *nminusy;
float *curvature;

float *F;
float *gradphi;
float *phi1;

if((dx=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_DX\n");
if((dxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_DX+\n");
if((dxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_DX-\n");
if((dxplusy=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dxplusy\n");
if((dxminusy=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dxminusy\n");
if((maxdxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_MDX+\n");
if((maxminusdxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-MDX-\n");
if((mindxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_MDX+\n");
if((minminusdxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-MDX-\n");

if((dy=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dy\n");
if((dyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dy+\n");
if((dyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dy+\n");
if((dyplusx=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dyplusx\n");
if((dyminusx=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dyminusx\n");
if((maxdyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_Mdy+\n");
if((maxminusdyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-Mdy-\n");
if((mindyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_Mdy+\n");
if((minminusdyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-Mdy-\n");

if((gradphimax=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHIMAX\n");
if((gradphimin=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHImin\n");
if((nplusx=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("nplusx\n");
if((nplusy=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("nplusy\n");	
if((nminusx=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("nminusx\n");
if((nminusy=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("nminusy\n");
if((curvature=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("curvature\n");

if((F=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("F\n");
if((gradphi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHI\n");
if((phi1=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHI\n");

for(i=0;i<N;i++){
	phi1[i]=phi[i];
}

for(r=0;r<imageH;r++){
	for(c=0;c<imageW;c++){

		i=r*imageW+c;

		if(c==0||c>imageW-2){dx[i]=0;} else {dx[i]=(phi1[i+1]-phi1[i-1])/2;}
		if(c==imageW-1){dxplus[i]=0;} else {dxplus[i]=(phi1[i+1]-phi1[i]);}
		if(c==0){dxminus[i]=0;} else {dxminus[i]=(phi1[i]-phi1[i-1]);}
		if(r==0||c==0||c==imageW-1){dxplusy[i]=0;} else {dxplusy[i]=(phi1[i-imageW+1]-phi1[i-imageW-1])/2;}
		if(r==imageH-1||c==0||c==imageW-1){dxminusy[i]=0;} else {dxminusy[i]=(phi1[i+imageW+1]-phi1[i+imageW-1])/2;}
		if(dxplus[i]<0){maxdxplus[i]=0;} else { maxdxplus[i]= dxplus[i]*dxplus[i]; }
		if(-dxminus[i]<0){maxminusdxminus[i]=0;} else { maxminusdxminus[i]= dxminus[i]*dxminus[i]; }
		if(dxplus[i]>0){mindxplus[i]=0;} else { mindxplus[i]= dxplus[i]*dxplus[i]; }
		if(-dxminus[i]>0){minminusdxminus[i]=0;} else { minminusdxminus[i]= dxminus[i]*dxminus[i]; }

		if(r==0||r==imageH-1){dy[i]=0;} else {dy[i]=(phi1[i-imageW]-phi1[i+imageW])/2;}
		if(r==0){dyplus[i]=0;} else {dyplus[i]=(phi1[i-imageW]-phi1[i]);}
		if(r==imageH-1){dyminus[i]=0;} else {dyminus[i]=(phi1[i]-phi1[i+imageW]);}
		if(r==0||c==imageW-1||r==imageH-1){dyplusx[i]=0;} else {dyplusx[i]=(phi1[i-imageW+1]-phi1[i+imageW+1])/2;}
		if(r==0||c==0||r==imageH-1){dyminusx[i]=0;} else {dyminusx[i]=(phi1[i-imageW-1]-phi1[i+imageW-1])/2;}
		if(dyplus[i]<0){maxdyplus[i]=0;} else { maxdyplus[i]= dyplus[i]*dyplus[i]; }
		if(-dyminus[i]<0){maxminusdyminus[i]=0;} else { maxminusdyminus[i]= dyminus[i]*dyminus[i]; }
		if(dyplus[i]>0){mindyplus[i]=0;} else { mindyplus[i]= dyplus[i]*dyplus[i]; }
		if(-dyminus[i]>0){minminusdyminus[i]=0;} else { minminusdyminus[i]= dyminus[i]*dyminus[i]; }

		gradphimax[i]=sqrt((sqrt(maxdxplus[i]+maxminusdxminus[i]))*(sqrt(maxdxplus[i]+maxminusdxminus[i]))+(sqrt(maxdyplus[i]+maxminusdyminus[i]))*(sqrt(maxdyplus[i]+maxminusdyminus[i])));
		gradphimin[i]=sqrt((sqrt(mindxplus[i]+minminusdxminus[i]))*(sqrt(mindxplus[i]+minminusdxminus[i]))+(sqrt(mindyplus[i]+minminusdyminus[i]))*(sqrt(mindyplus[i]+minminusdyminus[i])));
		nplusx[i]= dxplus[i] / sqrt(FLT_EPSILON + (dxplus[i]*dxplus[i]) + ((dyplusx[i] + dy[i])*(dyplusx[i] + dy[i])*0.25) );
		nplusy[i]= dyplus[i] / sqrt(FLT_EPSILON + (dyplus[i]*dyplus[i]) + ((dxplusy[i] + dx[i])*(dxplusy[i] + dx[i])*0.25) );
		nminusx[i]= dxminus[i] / sqrt(FLT_EPSILON + (dxminus[i]*dxminus[i]) + ((dyminusx[i] + dy[i])*(dyminusx[i] + dy[i])*0.25) );
		nminusy[i]= dyminus[i] / sqrt(FLT_EPSILON + (dyminus[i]*dyminus[i]) + ((dxminusy[i] + dx[i])*(dxminusy[i] + dx[i])*0.25) );
		curvature[i]= ((nplusx[i]-nminusx[i])+(nplusy[i]-nminusy[i]))/2;

		F[i] = (ALPHA * D[i]) + ((1-ALPHA) * curvature[i]);
		if(F[i]>0) {gradphi[i]=gradphimax[i];} else {gradphi[i]=gradphimin[i];}
		phi[i]=phi1[i] + (DT * F[i] * gradphi[i]);
	}
}

free(dx);
free(dxplus);
free(dxminus);
free(dxminusy);
free(maxdxplus);
free(maxminusdxminus);
free(mindxplus);
free(minminusdxminus);
//
free(dy);
free(dyplus);
free(dyminus);
free(dyminusx);
free(maxdyplus);
free(maxminusdyminus);
free(mindyplus);
free(minminusdyminus);
//
free(gradphi);
free(gradphimax);
free(gradphimin);
//
free(nplusx);
free(nplusy);
free(nminusx);
free(nminusy);
free(curvature);

free(F);
free(phi1);

}

void disp(void){
	
	glClear(GL_COLOR_BUFFER_BIT);

	update_phi();

	//if(its%RITS==0){
	//		reinit_phi();
	//		glutPostRedisplay();
	//	}

	//for(i=0;i<N;i++){
	//	contour[i]=phi[i];
	//	if((contour[i]<-3) | (contour[i]>3)){
	//		contour[i]=0;
	//	} else {
	//		contour[i]=contour[i]/3;
	//	}
	//}

	glDrawPixels(imageW, imageH, GL_GREEN, GL_FLOAT, phi);

	glutSwapBuffers();
	
	its++;

	if(its<ITERATIONS){
		glutPostRedisplay();
	} else {

		}
}



int main(int argc, char** argv){

	const char *image_path = IMAGE;
	
	//TODO : declare ALL variables here

	LoadBMPFile(&h_Src, &imageW, &imageH, image_path);
	D = (float *)malloc(imageW*imageH*sizeof(float));

	//printf("Input Image\n");
	for(r=0;r<imageH;r++){
		for(c=0;c<imageW;c++){
			D[r*imageW+c] = h_Src[r*imageW+c].x;
			/*printf("%3.0f ", D[r*imageW+c]);*/
		}
		//printf("\n");
	}

	N = imageW*imageH;

	for(i=0;i<N;i++){
		D[i] = EPSILON - abs(D[i] - THRESHOLD);
	}

	//printf("Speed Function\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%3.0f ", D[r*imageW+c]);
	//	}
	//	printf("\n");
	//}


	init_phi();
	if((contour=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("Contour\n");

//update_phi();

		  // GL initialisation
		  glutInit(&argc, argv);
		  glutInitDisplayMode(GLUT_ALPHA | GLUT_DOUBLE);
		  glutInitWindowSize(imageW,imageH);
		  glutInitWindowPosition(100,100);
		  glutCreateWindow("GL Level Set Evolution");
		  glClearColor(0.0,0.0,0.0,0.0);


		  glutDisplayFunc(disp);
		  glutMainLoop();

	//printf("phi+1\n");
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%6.3f ", phi[r*imageW+c]);
	//	}
	//	printf("\n");
	//}
}


//TODO Memory Malloc Free

//TODO Timer

//TODO Comment Code
