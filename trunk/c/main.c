#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <math.h>
#include <GL/glut.h>

#define IMAGE			"liver.bmp"
#define IMAGE_FILE_NAME "Output.pgm"


#define ITERATIONS   500
#define THRESHOLD	 10
#define EPSILON		 100

#define ALPHA		 0.004
#define DT			 0.3

#define RITS		 200

float *phi, *D, *contour;
uchar4 *h_Src, *h_Mask;
int imageW, imageH, N;
unsigned char *mask;


void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
void sedt2d(int *_d,unsigned char *_bimg,int _h,int _w);


int its=0;

void init_phi(){

	const char *mask_path = "mask.bmp";

	//printf("Init Mask\n");
	LoadBMPFile(&h_Mask, &imageW, &imageH, mask_path);
	mask = (unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));

	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			mask[r*imageW+c] = (h_Mask[r*imageW+c].x)/255;
			//printf("%3d ", mask[r*imageW+c]);
		}
		//printf("\n");
	}

	int *init;
	if((init=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INIT\n");
	if((phi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_PHI\n");
	sedt2d(init,mask,imageH,imageW);



	//printf("sdf of init mask\n");
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
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

	unsigned char *reinit;
	reinit=(unsigned char *)malloc(imageW*imageH*sizeof(unsigned char));
	int *intphi;
	if((intphi=(int *)malloc(imageW*imageH*sizeof(int)))==NULL)printf("ME_INIT\n");
	
	for(int i=0;i<N;i++){
		if(phi[i]<0){
			phi[i]=1;
		} else {
			phi[i]=0;
		}
		reinit[i]=(int)phi[i];
	}


	sedt2d(intphi,reinit,imageH,imageW);

	printf("ReInit @ %4d its\n",its);
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
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

for(int r=0;r<imageH;r++){
	for(int c=0;c<imageW;c++){

		if(c==0||c>imageW-2){dx[r*imageW+c]=0;} else {dx[r*imageW+c]=(phi[r*imageW+c+1]-phi[r*imageW+c-1])/2;}
		if(c==imageW-1){dxplus[r*imageW+c]=0;} else {dxplus[r*imageW+c]=(phi[r*imageW+c+1]-phi[r*imageW+c]);}
		if(c==0){dxminus[r*imageW+c]=0;} else {dxminus[r*imageW+c]=(phi[r*imageW+c]-phi[r*imageW+c-1]);}
		if(r==0||c==0||c==imageW-1){dxplusy[r*imageW+c]=0;} else {dxplusy[r*imageW+c]=(phi[r*imageW+c-imageW+1]-phi[r*imageW+c-imageW-1])/2;}
		if(r==imageH-1||c==0||c==imageW-1){dxminusy[r*imageW+c]=0;} else {dxminusy[r*imageW+c]=(phi[r*imageW+c+imageW+1]-phi[r*imageW+c+imageW-1])/2;}
		if(dxplus[r*imageW+c]<0){maxdxplus[r*imageW+c]=0;} else { maxdxplus[r*imageW+c]= dxplus[r*imageW+c]*dxplus[r*imageW+c]; }
		if(-dxminus[r*imageW+c]<0){maxminusdxminus[r*imageW+c]=0;} else { maxminusdxminus[r*imageW+c]= dxminus[r*imageW+c]*dxminus[r*imageW+c]; }
		if(dxplus[r*imageW+c]>0){mindxplus[r*imageW+c]=0;} else { mindxplus[r*imageW+c]= dxplus[r*imageW+c]*dxplus[r*imageW+c]; }
		if(-dxminus[r*imageW+c]>0){minminusdxminus[r*imageW+c]=0;} else { minminusdxminus[r*imageW+c]= dxminus[r*imageW+c]*dxminus[r*imageW+c]; }

		if(r==0||r==imageH-1){dy[r*imageW+c]=0;} else {dy[r*imageW+c]=(phi[r*imageW+c-imageW]-phi[r*imageW+c+imageW])/2;}
		if(r==0){dyplus[r*imageW+c]=0;} else {dyplus[r*imageW+c]=(phi[r*imageW+c-imageW]-phi[r*imageW+c]);}
		if(r==imageH-1){dyminus[r*imageW+c]=0;} else {dyminus[r*imageW+c]=(phi[r*imageW+c]-phi[r*imageW+c+imageW]);}
		if(r==0||c==imageW-1||r==imageH-1){dyplusx[r*imageW+c]=0;} else {dyplusx[r*imageW+c]=(phi[r*imageW+c-imageW+1]-phi[r*imageW+c+imageW+1])/2;}
		if(r==0||c==0||r==imageH-1){dyminusx[r*imageW+c]=0;} else {dyminusx[r*imageW+c]=(phi[r*imageW+c-imageW-1]-phi[r*imageW+c+imageW-1])/2;}
		if(dyplus[r*imageW+c]<0){maxdyplus[r*imageW+c]=0;} else { maxdyplus[r*imageW+c]= dyplus[r*imageW+c]*dyplus[r*imageW+c]; }
		if(-dyminus[r*imageW+c]<0){maxminusdyminus[r*imageW+c]=0;} else { maxminusdyminus[r*imageW+c]= dyminus[r*imageW+c]*dyminus[r*imageW+c]; }
		if(dyplus[r*imageW+c]>0){mindyplus[r*imageW+c]=0;} else { mindyplus[r*imageW+c]= dyplus[r*imageW+c]*dyplus[r*imageW+c]; }
		if(-dyminus[r*imageW+c]>0){minminusdyminus[r*imageW+c]=0;} else { minminusdyminus[r*imageW+c]= dyminus[r*imageW+c]*dyminus[r*imageW+c]; }

		gradphimax[r*imageW+c]=sqrt((sqrt(maxdxplus[r*imageW+c]+maxminusdxminus[r*imageW+c]))*(sqrt(maxdxplus[r*imageW+c]+maxminusdxminus[r*imageW+c]))+(sqrt(maxdyplus[r*imageW+c]+maxminusdyminus[r*imageW+c]))*(sqrt(maxdyplus[r*imageW+c]+maxminusdyminus[r*imageW+c])));
		gradphimin[r*imageW+c]=sqrt((sqrt(mindxplus[r*imageW+c]+minminusdxminus[r*imageW+c]))*(sqrt(mindxplus[r*imageW+c]+minminusdxminus[r*imageW+c]))+(sqrt(mindyplus[r*imageW+c]+minminusdyminus[r*imageW+c]))*(sqrt(mindyplus[r*imageW+c]+minminusdyminus[r*imageW+c])));
		nplusx[r*imageW+c]= dxplus[r*imageW+c] / sqrt(FLT_EPSILON + (dxplus[r*imageW+c]*dxplus[r*imageW+c]) + ((dyplusx[r*imageW+c] + dy[r*imageW+c])*(dyplusx[r*imageW+c] + dy[r*imageW+c])*0.25) );
		nplusy[r*imageW+c]= dyplus[r*imageW+c] / sqrt(FLT_EPSILON + (dyplus[r*imageW+c]*dyplus[r*imageW+c]) + ((dxplusy[r*imageW+c] + dx[r*imageW+c])*(dxplusy[r*imageW+c] + dx[r*imageW+c])*0.25) );
		nminusx[r*imageW+c]= dxminus[r*imageW+c] / sqrt(FLT_EPSILON + (dxminus[r*imageW+c]*dxminus[r*imageW+c]) + ((dyminusx[r*imageW+c] + dy[r*imageW+c])*(dyminusx[r*imageW+c] + dy[r*imageW+c])*0.25) );
		nminusy[r*imageW+c]= dyminus[r*imageW+c] / sqrt(FLT_EPSILON + (dyminus[r*imageW+c]*dyminus[r*imageW+c]) + ((dxminusy[r*imageW+c] + dx[r*imageW+c])*(dxminusy[r*imageW+c] + dx[r*imageW+c])*0.25) );
		curvature[r*imageW+c]= ((nplusx[r*imageW+c]-nminusx[r*imageW+c])+(nplusy[r*imageW+c]-nminusy[r*imageW+c])/2);

		F[r*imageW+c] = (ALPHA * D[r*imageW+c]) + ((1-ALPHA) * curvature[r*imageW+c]);
		if(F[r*imageW+c]>0) {gradphi[r*imageW+c]=gradphimax[r*imageW+c];} else {gradphi[r*imageW+c]=gradphimin[r*imageW+c];}
		phi[r*imageW+c]=phi[r*imageW+c] + (DT * F[r*imageW+c] * gradphi[r*imageW+c]);
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

}

void disp(void){
	
	glClear(GL_COLOR_BUFFER_BIT);

	update_phi();

	if(its%RITS==0){
			reinit_phi();
			glutPostRedisplay();
		}

	for(int i=0;i<N;i++){
		contour[i]=phi[i];
		if((contour[i]<-3) | (contour[i]>3)){
			contour[i]=0;
		} else {
			contour[i]=contour[i]/3;
		}
	}

	glDrawPixels(imageW, imageH, GL_GREEN, GL_FLOAT, contour);

	glutSwapBuffers();
	
	its++;

	if(its<ITERATIONS){
		glutPostRedisplay();
	} else {

		printf("Writing Output\n");
		for(int i=0;i<N;i++){
			if(phi[i]>0){
				D[i]=0;
			} else {
				D[i]= D[i];
			}
			FILE* imagefile;
			imagefile=fopen(IMAGE_FILE_NAME, "w");

			if( imagefile == NULL)
			{
				perror( "Can't create '" IMAGE_FILE_NAME "'");
				exit( 1);
			}

			fprintf(imagefile,"P5\n%u %u 255\n", imageW,imageH );
			fwrite(D, 4, N, imagefile);
			fclose(imagefile);
			exit(0);

		}
	}
}



int main(int argc, char** argv){

	const char *image_path = IMAGE;
	
	//TODO : declare ALL variables here

	LoadBMPFile(&h_Src, &imageW, &imageH, image_path);
	D = (float *)malloc(imageW*imageH*sizeof(float));

	//printf("Input Image\n");
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			D[(r)*imageW+c] = h_Src[r*imageW+c].x;
			/*printf("%3.0f ", D[r*imageW+c]);*/
		}
		//printf("\n");
	}

	N = imageW*imageH;

	for(int i=0;i<N;i++){
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
