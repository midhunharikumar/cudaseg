#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <GL/glew.h>
#include <cuda_runtime.h>
#include <math.h>
#include <GL/glut.h>

#define IMAGE			"bigbrain.bmp"

#define ITERATIONS   5000
#define THRESHOLD	 100
#define EPSILON		 40

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

__global__ void updatephi( float *d_phi, float *d_phi1, float *d_D, int imageW, int imageH);

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



void cuda_update(){

    float *d_phi1;
	
	cudaMalloc((void**)&d_phi1,         sizeof(float)*imageW*imageH);

	cudaMemcpy(d_phi1, phi, sizeof(float)*imageW*imageH, cudaMemcpyHostToDevice);


	dim3 dimGrid( ((imageW-1)/4) + 1, ((imageH-1)/32) +1 );
	dim3 dimBlock(4, 32);


	updatephi<<< dimGrid, dimBlock>>>(d_phi, d_phi1, d_D,  imageW, imageH);

	cudaMemcpy(phi, d_phi, sizeof(float)*imageW*imageH, cudaMemcpyDeviceToHost);


	printf("%3d\n", its);
	//printf("Speed Function\n");	
	//for(int r=0;r<imageH;r++){
	//	for(int c=0;c<imageW;c++){
	//		printf("%4.1f ", phi[r*imageW+c]);
	//	}
	//	printf("\n");
	//}


	cudaFree(d_phi1);

}

void disp(void){
	
	glClear(GL_COLOR_BUFFER_BIT);

	cuda_update();

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

	init_phi();



	cudaMalloc((void**)&d_D,         sizeof(float)*imageW*imageH);
    cudaMemcpy(d_D, D, sizeof(float)*imageW*imageH, cudaMemcpyHostToDevice);

	cudaMalloc((void**)&d_phi, sizeof(float)*imageW*imageH);
	

		  // GL initialisation
		  glutInit(&argc, argv);
		  glutInitDisplayMode(GLUT_ALPHA | GLUT_DOUBLE);
		  glutInitWindowSize(imageW,imageH);
		  glutInitWindowPosition(100,100);
		  glutCreateWindow("GL Level Set Evolution");
		  glClearColor(0.0,0.0,0.0,0.0);


		  glutDisplayFunc(disp);
		  glutMainLoop();
	
}




//TODO Memory Malloc Free

//TODO Timer

//TODO Comment Code
