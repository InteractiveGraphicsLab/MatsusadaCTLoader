//#include <stdio.h>
//#include "TifIO.h"
//#include <iostream>
//
//using namespace std;
//g++ -O6 -c TifIO.cpp
//g++ -O6 TifIO.o TifIOtest.cpp -lm
//./a.out inputfilename outputfilename

/*
int main(int argc, char* argv[])
{
	if(argc<3){
		cout<<"usage: "<<argv[0]<<" in.tif out.tif"<<endl;
		return -1;
	}

	TifIO *mytifio = new TifIO(argv[1]);

	Image *I2d = new Image();
	Image3D *I3d = new Image3D();
	Image *R2d = new Image();
	Image *G2d = new Image();
	Image *B2d = new Image();
	Image3D *R3d = new Image3D();
	Image3D *G3d = new Image3D();
	Image3D *B3d = new Image3D();

	if(mytifio->isApplicable()==false){
		printf("File \"%s\" is not supported.\n",mytifio->filename.c_str());
		return -1;
	}
	int PN = mytifio->getPageNumber();
	bool color = mytifio->isColor();
	printf("PN = %d\n",PN);
	if(color){
	  printf("Color\n");
	}else{
	  printf("Gray\n");
	}
	double dmax,dmin;
	if(PN==1){
		if(color){
			mytifio->getColorImage(R2d,G2d,B2d);
			R2d->px=1.0;
			R2d->py=1.0;
			R2d->setMaxMin(&dmax,&dmin);
		}else{
			mytifio->getImage(I2d);
			I2d->px=1.0;
			I2d->py=1.0;
			I2d->setMaxMin(&dmax,&dmin);
		}
	}else{
		if(color){
			mytifio->getColorImage(R3d,G3d,B3d);
			R3d->px=1.0;
			R3d->py=1.0;
			R3d->pz=1.0;
			R3d->setMaxMin(&dmax,&dmin);
		}else{
			mytifio->getImage(I3d);
			I3d->px=1.0;
			I3d->py=1.0;
			I3d->pz=1.0;
			I3d->setMaxMin(&dmax,&dmin);
		}
	}

	mytifio->setFilename(argv[2]);
	if(PN==1){
		if(color){
			mytifio->saveColorImage(R2d,G2d,B2d,dmax,dmin,16);
		}else{
			mytifio->saveImage(I2d,dmax,dmin,16);
		}
	}else{
		if(color){
			mytifio->saveColorImage(R3d,G3d,B3d,dmax,dmin,16);
		}else{
			mytifio->saveImage(I3d,dmax,dmin,16);
		}
	}

	delete mytifio;

	if(I2d!=NULL)delete I2d;
	if(I3d!=NULL)delete I3d;
	if(R2d!=NULL)delete R2d;
	if(G2d!=NULL)delete G2d;
	if(B2d!=NULL)delete B2d;

	if(R3d!=NULL)delete R3d;
	if(G3d!=NULL)delete G3d;
	if(B3d!=NULL)delete B3d;

	return 0;
}
*/