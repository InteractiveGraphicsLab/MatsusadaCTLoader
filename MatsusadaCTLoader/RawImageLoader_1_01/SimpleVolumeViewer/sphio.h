#pragma once

#pragma warning ( disable : 4996 ) 

#define Native_ENDIAN true
#define Not_Native_ENDIAN false
class SPH{
 public:
  
  SPH(){}

  bool SPH_ByteOrderCheck(const char *filename)
  {
    FILE *in = fopen(filename,"rb");
    int bytecheck=8;
    fread(&bytecheck, 4, 1,in);
    //printf("bytecheck = %d\n",bytecheck);
    fclose(in);
    if(bytecheck==8)return Native_ENDIAN;
    return Not_Native_ENDIAN;
  }

  void swap(char *in,char *out,int len)
  {
    int i;
    for(i=0;i<len;i++){
      out[i] = in[len-1-i];
    }
  }

  void change_endian(char *value,int bytelen,int len)
  {
    
    char *bufbytelenIn = new char[bytelen+1];
    char *bufbytelenOut = new char[bytelen+1];
    
    int i,j;
    for(i=0;i<len;i++){
      for(j=0;j<bytelen;j++){
	bufbytelenIn[j] = value[i*bytelen+j];
      }
      
      swap(bufbytelenIn,bufbytelenOut,bytelen); 
      for(j=0;j<bytelen;j++){
	value[i*bytelen+j] = bufbytelenOut[j];
      } 
    }
    
    
    delete [] bufbytelenIn;
    delete [] bufbytelenOut;
  }
  int getInt(FILE *in){
    int val;
    fread(&val, 4, 1,in);
    char *buf = (char *)(&val);
    
    change_endian(buf,4,1);
    return val;
  }
  long int getLongInt(FILE *in){
    long int val;
    fread(&val, 8, 1,in);
    char *buf = (char *)(&val);
    
    change_endian(buf,8,1);
    return val;
  }
  float getFloat(FILE *in){
    float val;
    fread(&val, 4, 1,in);
    char *buf = (char *)(&val);
    
    change_endian(buf,4,1);
    return val;
  }
  double getDouble(FILE *in){
    double val;
    fread(&val,8, 1,in);
    char *buf = (char *)(&val);
    
    change_endian(buf,8,1);
    return val;
  }
  
  

  void readSPH(Image3D *input, const char *filename){
    int len;
    int svType;
    int dType;
    
    svType = 1;
    dType = 2;
    
    
    if(SPH_ByteOrderCheck(filename)){
      FILE *in = fopen(filename,"rb");
      fread(&len, 4, 1,in);
      fread(&svType, 4, 1,in);
      fread(&dType, 4, 1,in);
      fread(&len, 4, 1,in);
      
      if(dType==2){
	long int imax, jmax, kmax;
	double xorg, yorg, zorg;
	double xpitch, ypitch, zpitch;
	long int step;
	double sphtime;
	fread(&len, 4, 1,in);
	fread(&imax, 8, 1,in);
	fread(&jmax, 8, 1,in);
	fread(&kmax, 8, 1,in);
	fread(&len, 4, 1,in);
	input->sx = imax;
	input->sy = jmax;
	input->sz = kmax;
	
	fread(&len, 4, 1,in);
	fread(&xorg, 8, 1,in);
	fread(&yorg, 8, 1,in);
	fread(&zorg, 8, 1,in);
	fread(&len, 4, 1,in);
	
	fread(&len, 4, 1,in);
	fread(&xpitch, 8, 1,in);
	fread(&ypitch, 8, 1,in);
	fread(&zpitch, 8, 1,in);
	fread(&len, 4, 1,in);
	input->px=xpitch;
	input->py=ypitch;
	input->pz=zpitch;
	
	
	fread(&len, 4, 1,in);
	fread(&step, 8, 1,in);
	fread(&sphtime, 8, 1,in);
	fread(&len, 4, 1,in);
	double time = step;
	fread(&len, 4, 1,in);
	int x, y, z;
	
	input->img = new double** [input->sz];
	
	double data=0.0;
	for (z=0; z<kmax; z++)
	  {
	    input->img[z] = new double* [input->sy];
	    for (y=0; y<jmax; y++)
	      {
		input->img[z][y] = new double[input->sx];
		for (x=0; x<imax; x++)
		  {
		    
		    fread(&data, 8, 1,in);
		    input->img[z][y][x]=data;
		  }
	      }
	    
	  }
	fread(&len, 4, 1,in);
	
      }else{
	int imax, jmax, kmax;
	float xorg, yorg, zorg;
	float xpitch, ypitch, zpitch;
	int step;
	float sphtime;
	fread(&len, 4, 1,in);
	fread(&imax, 4, 1,in);
	fread(&jmax, 4, 1,in);
	fread(&kmax, 4, 1,in);
	fread(&len, 4, 1,in);
	input->sx = imax;
	input->sy = jmax;
	input->sz = kmax;
	
	fread(&len, 4, 1,in);
	fread(&xorg, 4, 1,in);
	fread(&yorg, 4, 1,in);
	fread(&zorg, 4, 1,in);
	fread(&len, 4, 1,in);
	
	fread(&len, 4, 1,in);
	fread(&xpitch, 4, 1,in);
	fread(&ypitch, 4, 1,in);
	fread(&zpitch, 4, 1,in);
	fread(&len, 4, 1,in);
	input->px=((double)(xpitch));
	input->py=((double)(ypitch));
	input->pz=((double)(zpitch));
	
	
	fread(&len, 4, 1,in);
	fread(&step, 4, 1,in);
	fread(&sphtime, 4, 1,in);
	fread(&len, 4, 1,in);
	double time = ((double)(step));
	fread(&len, 4, 1,in);
	int x, y, z;
	
	input->img = new double** [input->sz];

	
	float data=0.0f;
	for (z=0; z<kmax; z++)
	  {
	    input->img[z] = new double* [input->sy];
	    for (y=0; y<jmax; y++)
	      {
		 input->img[z][y] = new double[input->sx];
		for (x=0; x<imax; x++)
		  {
		    
		    fread(&data, 4, 1,in);
		    input->img[z][y][x]=((double)(data));
		  }
	      }
	    
	  }
	fread(&len, 4, 1,in);
	
	
      }
      
      fclose(in);
    }else{
      FILE *in = fopen(filename,"rb");
      
      fread(&len, 4, 1,in);
      svType = getInt(in);
      dType = getInt(in);
      fread(&len, 4, 1,in);
      //printf("svType = %d\n",svType);
      //printf("dType = %d\n",dType);
      if(dType==2){
	long int imax, jmax, kmax;
	double xorg, yorg, zorg;
//	double xpitch, ypitch, zpitch;
	long int step;
	double sphtime;
	fread(&len, 4, 1,in);
	imax = getLongInt(in);
	jmax = getLongInt(in);
	kmax = getLongInt(in);
	
	fread(&len, 4, 1,in);
	input->sx = imax;
	input->sy = jmax;
	input->sz = kmax;
	
	fread(&len, 4, 1,in);
	
	xorg = getDouble(in);
	yorg = getDouble(in);
	zorg = getDouble(in);
	
	fread(&len, 4, 1,in);
	
	fread(&len, 4, 1,in);
	
	
	input->px = getDouble(in);
	input->py = getDouble(in);
	input->pz = getDouble(in);
	
	fread(&len, 4, 1,in);
	
	
	
	fread(&len, 4, 1,in);
	
	step = getLongInt(in);
	sphtime = getDouble(in);
	fread(&len, 4, 1,in);
	double time = ((double)(step));
	fread(&len, 4, 1,in);
	int x, y, z;
	
	input->img = new double** [input->sz];
	
	double data=0.0;
	for (z=0; z<kmax; z++)
	  {
	    input->img[z] = new double * [input->sy];
	    for (y=0; y<jmax; y++)
	      {
		 input->img[z][y] = new double[input->sx];
		for (x=0; x<imax; x++)
		  {
		    
		    data = getDouble(in);
		    input->img[z][y][x]=data;
		  }
	      }
	    
	  }
	fread(&len, 4, 1,in);
	
      }else{
	int imax, jmax, kmax;
	float xorg, yorg, zorg;
	//float xpitch, ypitch, zpitch;
	int step;
	float sphtime;
	fread(&len, 4, 1,in);
	imax = getInt(in);
	jmax = getInt(in);
	kmax = getInt(in);
	fread(&len, 4, 1,in);
	input->sx = imax;
	input->sy = jmax;
	input->sz = kmax;
	
	fread(&len, 4, 1,in);
	xorg = getFloat(in);
	yorg = getFloat(in);
	zorg = getFloat(in);
	fread(&len, 4, 1,in);
	
	fread(&len, 4, 1,in);
	input->px = getFloat(in);
	input->py = getFloat(in);
	input->pz = getFloat(in);
	
	fread(&len, 4, 1,in);
		
	fread(&len, 4, 1,in);
	
	step = getInt(in);
	sphtime = getFloat(in);
	fread(&len, 4, 1,in);
	double time = step;
	fread(&len, 4, 1,in);
	int x, y, z;
	
	input->img = new double ** [input->sz];
	
	float data=0.0f;
	for (z=0; z<kmax; z++)
	  {
	    input->img[z] = new double* [input->sy];
	    for (y=0; y<jmax; y++)
	      { input->img[z][y] = new double[input->sx];
		for (x=0; x<imax; x++)
		  {
		    data = getFloat(in);
		    input->img[z][y][x]=((double)(data));
		  }
	      }
	    
	  }
	fread(&len, 4, 1,in);
	
	
      }
      
      fclose(in);
    
    

    }
    
    
    
    
}



void saveSPH(Image3D *input, const char *filename){
  FILE *out = fopen(filename,"wb");
    int len;
    int svType;
    int dType;
    int imax, jmax, kmax;
    float xorg, yorg, zorg;
    float xpitch, ypitch, zpitch;
    int step;
    float sphtime;
    svType = 1;
    dType = 1;
    
    imax = input->sx;
    jmax = input->sy;
    kmax = input->sz;
    
    //BBox bbox = grid->getBBox();
    
    xorg = 0.0f;//bbox.min[0];
    yorg = 0.0f;//bbox.min[1];
    zorg = 0.0f;//bbox.min[2];
    
    xpitch = ((float)(input->px));
    ypitch = ((float)(input->py));
    zpitch = ((float)(input->pz));
    
    step = 1;//time;
    sphtime = ((float)(step));
    
    len = 4 * 2;
    fwrite(&len, 4, 1,out);
    fwrite(&svType, 4, 1,out);
    fwrite(&dType, 4, 1,out);
    fwrite(&len, 4, 1,out);
    
    len = 4 * 3;
    fwrite(&len, 4, 1,out);
    fwrite(&imax, 4, 1,out);
    fwrite(&jmax, 4, 1,out);
    fwrite(&kmax, 4, 1,out);
    fwrite(&len, 4, 1,out);
    
    len = 4 * 3;
    fwrite(&len , 4, 1,out);
    fwrite(&xorg, 4, 1,out);
    fwrite(&yorg, 4, 1,out);
    fwrite(&zorg, 4, 1,out);
    fwrite(&len , 4, 1,out);
    
    len = 4 * 3;
    fwrite(&len   , 4, 1,out);
    fwrite(&xpitch, 4, 1,out);
    fwrite(&ypitch, 4, 1,out);
    fwrite(&zpitch, 4, 1,out);
    fwrite(&len   , 4, 1,out);
    
    len = 4 * 2;
    fwrite(&len    , 4, 1,out);
    fwrite(&step   , 4, 1,out);
    fwrite(&sphtime, 4, 1,out);
    fwrite(&len    , 4, 1,out);
    
    len = 4 * imax * jmax * kmax;
    fwrite(&len, 4, 1,out);
    int x, y, z;
    
    float data=0.0f;
	for (z=0; z<kmax; z++)
    {	
		for (y=0; y<jmax; y++)
		{
			for (x=0; x<imax; x++)
			{
				data = ((float)(input->img[z][y][x]));
				fwrite(&data, 4, 1,out);		  
			}
		  }	  
	}
    fwrite(&len, 4, 1,out);
    fclose(out);
}

 


};
#undef Native_ENDIAN
#undef Not_Native_ENDIAN
