		



		//DXPLUS
		float *dxplus;
		dxplus=(float *)malloc(imageW*imageH*sizeof(*phi));

		float *SR;
		SR=(float *)malloc(imageW*imageH*sizeof(*SR));
		shiftR(SR, phi);
		for(int i=0;i<imageW*imageH;i++){
			dxplus[i]=SR[i]-phi[i];
		}
		free(SR);
		//minusDXMINUS
		float *minusdxminus;
		minusdxminus=(float *)malloc(imageW*imageH*sizeof(*phi));

		float *SL;
		SL=(float *)malloc(imageW*imageH*sizeof(*SL));
		shiftL(SL, phi);
		for(int i=0;i<imageW*imageH;i++){
			minusdxminus[i]=SL[i]-phi[i];
		}
		free(SL);
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

	float *maxdxplus;
	maxdxplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxdxplus[i]=max(&dxplus[i]);
		maxdxplus[i]=maxdxplus[i]*maxdxplus[i];
	}
	float *maxminusdxminus;
	maxminusdxminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxminusdxminus[i]=max(&minusdxminus[i]);
		maxminusdxminus[i]=maxminusdxminus[i]*maxminusdxminus[i];
	}
	free(minusdxminus);

		//GRADPHIMAX_X
		float *gradphimax_x;
		gradphimax_x=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimax_x[i]=sqrt(maxdxplus[i]+maxminusdxminus[i]);
		}

		free(maxdxplus);
		free(maxminusdxminus);

			printf("sdf of init mask\n");
		for(int r=0;r<imageH;r++){
			for(int c=0;c<imageW;c++){
				printf("%3d ", gradphimax_x[r*imageW+c]);
			}
			printf("\n");
		}

	float *mindxplus;
	mindxplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		mindxplus[i]=min(&dxplus[i]);
	}
	free(dxplus);

	float *minminusdxminus;
	minminusdxminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		minminusdxminus[i]=min(&minusdxminus[i]);
	}
	free(minusdxminus);

		//GRADPHIMIN_X
		float *gradphimin_x;
		gradphimin_x=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimin_x[i]=sqrt(mindxplus[i]+minminusdxminus[i]);
		}

		free(mindxplus);
		free(minminusdxminus);

	float *maxdyplus;
	maxdyplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxdyplus[i]=max(&dyplus[i]);
	}

	float *maxminusdyminus;
	maxminusdyminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		maxminusdyminus[i]=max(&minusdyminus[i]);
	}

		//GRADPHIMAX_Y
		float *gradphimax_y;
		gradphimax_y=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimax_y[i]=sqrt(maxdyplus[i]+maxminusdyminus[i]);
		}

		free(maxdyplus);
		free(maxminusdyminus);


	float *mindyplus;
	mindyplus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		mindyplus[i]=min(&dyplus[i]);
	}
	free(dyplus);

	float *minminusdyminus;
	minminusdyminus=(float *)malloc(imageW*imageH*sizeof(*phi));
	for(int i=0;i<imageW*imageH;i++){
		minminusdyminus[i]=min(&minusdyminus[i]);
	}
	free(minusdyminus);

		//GRADPHIMIN_Y
		float *gradphimin_y;
		gradphimin_y=(float *)malloc(imageW*imageH*sizeof(*phi));
		for(int i=0;i<imageW*imageH;i++){
			gradphimin_y[i]=sqrt(mindyplus[i]+minminusdyminus[i]);
		}

		free(mindyplus);
		free(minminusdyminus);