	//DXPLUS
	float *dxplus;
	dxplus=(float *)malloc(imageW*imageH*sizeof(*dxplus));

	float *SR;
	SR=(float *)malloc(imageW*imageH*sizeof(*SR));
	shiftR(SR, phi);
	for(int i=0;i<imageW*imageH;i++){
		dxplus[i]=SR[i]-phi[i];
	}
	free(SR);

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
		free(phi);