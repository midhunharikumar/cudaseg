#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <cutil.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/glew.h>

#define ITERATIONS   400
#define THRESHOLD	 130
#define EPSILON		 30


float *phi, *D, *displayphi;
uchar4 *h_Src, *h_Mask;
int imageW, imageH, N;
unsigned char *mask;


void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
void sedt2d(int *_d,unsigned char *_bimg,int _h,int _w);


int its=0;



//printf("\n");	
//for(int r=0;r<imageH;r++){
//	for(int c=0;c<imageW;c++){
//		printf("%3.0f ", dxminus[r*imageW+c]);
//	}
//	printf("\n");
//}

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

	printf("reinit\n");
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

	float *dxplus, *ptr2dxplus;
	if((dxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_DX+\n");
	for(int i=0;i<N;i++)dxplus[i]=phi[i];
	ptr2dxplus=dxplus;
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW-1;c++){
			*ptr2dxplus = *(ptr2dxplus+1) - *ptr2dxplus;
			ptr2dxplus++;
		}
		*ptr2dxplus=0;
		ptr2dxplus++;
	}

	float *dxminus, *ptr2dxminus;
	if((dxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_DX-\n");
	for(int i=0;i<N;i++)dxminus[i]=phi[i];
	ptr2dxminus=dxminus+N-1;
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW-1;c++){
			*ptr2dxminus = *ptr2dxminus - *(ptr2dxminus-1);
			ptr2dxminus--;
		}
		*ptr2dxminus=0;
		ptr2dxminus--;
	
	}

	float *maxdxplus, *ptr2maxdxplus;
	if((maxdxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_MDX+\n");
	for(int i=0;i<N;i++)maxdxplus[i]=dxplus[i];
	ptr2maxdxplus = maxdxplus;
	for(int i=0;i<N;i++){
		if (*ptr2maxdxplus < 0) {
			*ptr2maxdxplus = 0;
		} else {
			*ptr2maxdxplus *= *ptr2maxdxplus;
		}
		ptr2maxdxplus++;
	}

	float *maxminusdxminus, *ptr2maxminusdxminus;
	if((maxminusdxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-MDX-\n");
	for(int i=0;i<N;i++)maxminusdxminus[i]=-dxminus[i];
	ptr2maxminusdxminus = maxminusdxminus;
	for(int i=0;i<N;i++){
		if (*ptr2maxminusdxminus < 0) {
			*ptr2maxminusdxminus = 0;
		} else {
			*ptr2maxminusdxminus *= *ptr2maxminusdxminus;
		}
		ptr2maxminusdxminus++;
	}

	float *mindxplus, *ptr2mindxplus;
	if((mindxplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_MDX+\n");
	for(int i=0;i<N;i++)mindxplus[i]=dxplus[i];
	ptr2mindxplus = mindxplus;
	for(int i=0;i<N;i++){
		if (*ptr2mindxplus > 0) {
			*ptr2mindxplus = 0;
		} else {
			*ptr2mindxplus *= *ptr2mindxplus;
		}
		ptr2mindxplus++;
	}

	float *minminusdxminus, *ptr2minminusdxminus;
	if((minminusdxminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-MDX-\n");
	for(int i=0;i<N;i++)minminusdxminus[i]=-dxminus[i];
	ptr2minminusdxminus = minminusdxminus;
	for(int i=0;i<N;i++){
		if (*ptr2minminusdxminus > 0) {
			*ptr2minminusdxminus = 0;
		} else {
			*ptr2minminusdxminus *= *ptr2minminusdxminus;
		}
		ptr2minminusdxminus++;
	}

	float *dyplus, *ptr2dyplus;
	if((dyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dy+\n");
	for(int i=0;i<N;i++)dyplus[i]=phi[i];
	ptr2dyplus=dyplus+N-1;
	for(int r=0;r<imageH-1;r++){
		for(int c=0;c<imageW;c++){
			*ptr2dyplus = *(ptr2dyplus-imageW) - *ptr2dyplus;
			ptr2dyplus--;
		}
	}
	for(int c=0;c<imageW;c++){
		*ptr2dyplus = 0;
		ptr2dyplus--;
	}

	float *dyminus, *ptr2dyminus;
	if((dyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_dy+\n");
	for(int i=0;i<N;i++)dyminus[i]=phi[i];
	ptr2dyminus=dyminus;
	for(int r=0;r<imageH-1;r++){
		for(int c=0;c<imageW;c++){
			*ptr2dyminus = *ptr2dyminus - *(ptr2dyminus+imageW);
			ptr2dyminus++;
		}
	}
	for(int c=0;c<imageW;c++){
		*ptr2dyminus = 0;
		ptr2dyminus++;
	}

	float *maxdyplus, *ptr2maxdyplus;
	if((maxdyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_Mdy+\n");
	for(int i=0;i<N;i++)maxdyplus[i]=dyplus[i];
	ptr2maxdyplus = maxdyplus;
	for(int i=0;i<N;i++){
		if (*ptr2maxdyplus < 0) {
			*ptr2maxdyplus = 0;
		} else {
			*ptr2maxdyplus *= *ptr2maxdyplus;
		}
		ptr2maxdyplus++;
	}

	float *maxminusdyminus, *ptr2maxminusdyminus;
	if((maxminusdyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-Mdy-\n");
	for(int i=0;i<N;i++)maxminusdyminus[i]=-dyminus[i];
	ptr2maxminusdyminus = maxminusdyminus;
	for(int i=0;i<N;i++){
		if (*ptr2maxminusdyminus < 0) {
			*ptr2maxminusdyminus = 0;
		} else {
			*ptr2maxminusdyminus *= *ptr2maxminusdyminus;
		}
		ptr2maxminusdyminus++;
	}

	float *mindyplus, *ptr2mindyplus;
	if((mindyplus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_Mdy+\n");
	for(int i=0;i<N;i++)mindyplus[i]=dyplus[i];
	ptr2mindyplus = mindyplus;
	for(int i=0;i<N;i++){
		if (*ptr2mindyplus > 0) {
			*ptr2mindyplus = 0;
		} else {
			*ptr2mindyplus *= *ptr2mindyplus;
		}
		ptr2mindyplus++;
	}

	float *minminusdyminus, *ptr2minminusdyminus;
	if((minminusdyminus=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_-Mdy-\n");
	for(int i=0;i<N;i++)minminusdyminus[i]=-dyminus[i];
	ptr2minminusdyminus = minminusdyminus;
	for(int i=0;i<N;i++){
		if (*ptr2minminusdyminus > 0) {
			*ptr2minminusdyminus = 0;
		} else {
			*ptr2minminusdyminus *= *ptr2minminusdyminus;
		}
		ptr2minminusdyminus++;
	}

		float *gradphimax;
		if((gradphimax=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHIMAX\n");
		for(int i=0;i<N;i++){
			gradphimax[i]=sqrt((sqrt(maxdxplus[i]+maxminusdxminus[i]))*(sqrt(maxdxplus[i]+maxminusdxminus[i]))+(sqrt(maxdyplus[i]+maxminusdyminus[i]))*(sqrt(maxdyplus[i]+maxminusdyminus[i])));
		}

		float *gradphimin;
		if((gradphimin=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHImin\n");
		for(int i=0;i<N;i++){
			gradphimin[i]=sqrt((sqrt(mindxplus[i]+minminusdxminus[i]))*(sqrt(mindxplus[i]+minminusdxminus[i]))+(sqrt(mindyplus[i]+minminusdyminus[i]))*(sqrt(mindyplus[i]+minminusdyminus[i])));
		}



		float *gradphi, *ptr2gradphi, *ptr2gradphimax, *ptr2gradphimin, *ptr2F;
		if((gradphi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("GRADPHI\n");
		ptr2gradphi=gradphi;
		ptr2gradphimax=gradphimax;
		ptr2gradphimin=gradphimin;
		ptr2F=D;
		for(int i=0; i<N; i++){
			if(*ptr2F>0){
				*ptr2gradphi = *ptr2gradphimax;
			} else {
				*ptr2gradphi = *ptr2gradphimin;
			}
			ptr2gradphi++;
			ptr2F++;
			ptr2gradphimax++;
			ptr2gradphimin++;
		}

	//printf("\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%6.3f ", dyplus[r*imageW+c]);
	//	}
	//	printf("\n");
	//}
	//printf("\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%6.3f ", dyminus[r*imageW+c]);
	//	}
	//	printf("\n");
	//}

		//printf("gradphimax\n");	
		//for(int r=0;r<imageH;r++){
		//	for(int c=0;c<imageW;c++){
		//		printf("%6.3f ", gradphimax[r*imageW+c]);
		//	}
		//	printf("\n");
		//}

		//printf("gradphimin\n");	
		//for(int r=0;r<imageH;r++){
		//	for(int c=0;c<imageW;c++){
		//		printf("%6.3f ", gradphimin[r*imageW+c]);
		//	}
		//	printf("\n");
		//}

			//printf("gradphi\n");	
			//for(int r=0;r<imageH;r++){
			//	for(int c=0;c<imageW;c++){
			//		printf("%6.3f ", gradphi[r*imageW+c]);
			//	}
			//	printf("\n");
			//}

		float dt;
		dt=0.01f;

		float *ptr2phi;
		ptr2phi=phi;
		ptr2gradphi=gradphi;
		ptr2F=D;
		for(int i=0; i<N; i++){
			*ptr2phi = *ptr2phi + dt * (*ptr2F) * (*ptr2gradphi);
			ptr2phi++;
			ptr2gradphi++;
			ptr2F++;
		}
	//printf("phi+1\n");
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%6.3f ", phi[r*imageW+c]);
	//	}
	//	printf("\n");
	//}

//printf("Freeing Memory\n");
if(gradphi!=NULL)free(gradphi);
if(gradphimax!=NULL)free(gradphimax);
if(gradphimin!=NULL)free(gradphimin);
if(maxdxplus!=NULL)free(maxdxplus);
if(maxminusdxminus!=NULL)free(maxminusdxminus);
if(mindyplus!=NULL)free(mindyplus);
if(minminusdyminus!=NULL)free(minminusdyminus);

}

void disp(void){
	
	glClear(GL_COLOR_BUFFER_BIT);

	update_phi();

	if(its%25==0){
			reinit_phi();
			glutPostRedisplay();
		}

	for(int i=0;i<N;i++){
		displayphi[i]=phi[i];
		if((displayphi[i]<-5) | (displayphi[i]>5)){
			displayphi[i]=0;
		} else {
			displayphi[i]=displayphi[i];
		}
	}

	glDrawPixels(imageH, imageW, GL_GREEN, GL_FLOAT, displayphi);

	glutSwapBuffers();
	
	its++;

	if(its<ITERATIONS){
		glutPostRedisplay();
	} else {

		//printf("phi+1\n");
		//for(int r=0;r<imageH;r++){
		//	for(int c=0;c<imageW;c++){
		//		printf("%3.1f ", phi[r*imageW+c]);
		//	}
		//	printf("\n");
		//}
	}
}



int main(int argc, char** argv){

	const char *image_path = "brain.bmp";
	
	//TODO : declare ALL variables here

	LoadBMPFile(&h_Src, &imageW, &imageH, image_path);
	D = (float *)malloc(imageW*imageH*sizeof(float));

	//printf("Input Image\n");
	for(int r=0;r<imageH;r++){
		for(int c=0;c<imageW;c++){
			D[r*imageW+c] = h_Src[r*imageW+c].x;
			/*printf("%3.0f ", D[r*imageW+c]);*/
		}
		//printf("\n");
	}

	N = imageW*imageH;

	float *ptr2D;
	ptr2D=D;
	for(int i=0;i<N;i++){
		*ptr2D = EPSILON - abs(*ptr2D - THRESHOLD);
		ptr2D++;
	}

	//printf("Speed Function\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%3.0f ", D[r*imageW+c]);
	//	}
	//	printf("\n");
	//}


	init_phi();
	if((displayphi=(float *)malloc(imageW*imageH*sizeof(float)))==NULL)printf("ME_PHI\n");

		  // GL initialisation
		  glutInit(&argc, argv);
		  glutInitDisplayMode(GLUT_ALPHA | GLUT_DOUBLE);
		  glutInitWindowSize(imageH,imageW);
		  glutInitWindowPosition(100,100);
		  glutCreateWindow("Muz's Badass OpenGL Skills Yo");
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