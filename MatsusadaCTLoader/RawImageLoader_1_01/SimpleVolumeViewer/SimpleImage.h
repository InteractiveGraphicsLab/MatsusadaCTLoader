#pragma once

//2D Image class
class Image{
 public:
  int sx,sy;
  double px,py;
  double **img;
  int graylevel;
  double maxgray;
  Image (){
    img=NULL;
    sx=sy=0;
    px=py=1.0;
    graylevel=0;maxgray=0.0;
  }
  Image(int dx,int dy){
    sx = dx;
    sy = dy;
    px=py=1.0;maxgray=0.0;
    graylevel=0;
    int i;
    img = new double* [sy];
    for(i=0;i<sy;i++){
      img[i] = new double[dx];
    }
  }
  void setMaxMin(double *dmax,double *dmin){
    (*dmax)=img[0][0];
    (*dmin)=img[0][0];
    
    int i,j;
    for(i=0;i<sy;i++)
      for(j=0;j<sx;j++){
	if((*dmax)<img[i][j])(*dmax)=img[i][j];
	if((*dmin)>img[i][j])(*dmin)=img[i][j];
      }
  }
  void Copy(Image *in){
    int i,j;
    for(i=0;i<sy;i++)
      for(j=0;j<sx;j++)img[i][j] = in->img[i][j];
    graylevel = in->graylevel;
    maxgray = in->maxgray;
    
  }
  
  void setMaxgray(){
    int i,j;maxgray=0.0;
    for(i=0;i<sy;i++)
      for(j=0;j<sx;j++){
	if(maxgray<img[i][j])maxgray = img[i][j];
      }
    if(maxgray<=0.0)maxgray=1.0;
  }
  virtual ~Image(){
    int i;
    for(i=0;i<sy;i++){
      delete [] img[i];
    }
    delete [] img;
  }
  
	void clear()
	{
		if( img!=NULL)
		{
			for(int i=0;i<sy;i++) delete [] img[i];
			delete [] img;
		}
		img=NULL;
		sx=sy=0;
		px=py=1.0;
		graylevel=0;maxgray=0.0;
	}
};

//3D Image class
class Image3D{
 public:
  int sx,sy,sz;
  double ***img;
  int    graylevel;
  double px,py,pz;
  double maxgray;
 
  Image3D()
  {
    graylevel=0;
    px=py=pz=1.0;
    img=NULL;
    sx=sy=sz=0;maxgray=0.0;
  }
  Image3D(int dx,int dy,int dz){
    sx = dx;
    sy = dy;
    sz = dz;
    graylevel=0;maxgray=0.0;
    px=py=pz=1.0;
    int i,j;
    img = new double** [sz];
    for(i=0;i<sz;i++){
      img[i] = new double* [dy];
      for(j=0;j<sy;j++)img[i][j] = new double[sx]; 
    }
  }
  Image3D(Image3D *in){
    sx = in->sx;
    sy = in->sy;
    sz = in->sz;
    graylevel=0;maxgray=0.0;
    px=in->px;
    py=in->py;
    pz=in->pz;
    int i,j;
    img = new double** [sz];
    for(i=0;i<sz;i++){
      img[i] = new double* [sy];
      for(j=0;j<sy;j++)img[i][j] = new double[sx]; 
    }
  }
  
  void setMaxMin(double *dmax,double *dmin){
    (*dmax)=img[0][0][0];
    (*dmin)=img[0][0][0];
    
    int i,j,k;
    for(i=0;i<sz;i++)
      for(j=0;j<sy;j++)
		for(k=0;k<sx;k++){
		  if((*dmax)<img[i][j][k])(*dmax)=img[i][j][k];
		  if((*dmin)>img[i][j][k])(*dmin)=img[i][j][k];
		}
      
  }
  void Copy(Image3D *in){
    int i,j,k;
    for(i=0;i<sz;i++)
      for(j=0;j<sy;j++)
	for(k=0;k<sx;k++)img[i][j][k] = in->img[i][j][k];
    graylevel = in->graylevel;
    maxgray = in->maxgray;
    
  }
  
  void setMaxgray(){
    int i,j,k;
    maxgray=0.0;
    for(i=0;i<sz;i++)
      for(j=0;j<sy;j++)
	for(k=0;k<sx;k++){
	  if(maxgray<img[i][j][k])maxgray = img[i][j][k];
      }
    if(maxgray<=0.0)maxgray=1.0;
  }


  void scaling( double scale )
  {
	for(int i=0;i<sz;i++)
    for(int j=0;j<sy;j++)
	for(int k=0;k<sx;k++)
		img[i][j][k] *= scale; 
  }

  virtual ~Image3D()
  {
	  if(img != 0)
	  {
		  for(int i=0;i<sz;i++)
		  {
			  for(int j=0;j<sy;j++) delete [] img[i][j];			
			  delete [] img[i];
		  }	
		  delete [] img;
	  }
  }
};
