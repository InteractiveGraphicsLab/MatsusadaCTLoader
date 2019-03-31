#include "StdAfx.h"

#include "TifIO.h"
#include <cmath>
#include <climits>


//modified by takashi 2013 11 4



using namespace std;

map<unsigned short,string> CTiffTag::IDTable;

TifIO::TifIO()
	: filename("")
	, m_fBigEndian(false)
{
	debug_msg("default constructer of class TifIO");
}

TifIO::TifIO(const char* in)
{
	debug_msg("TifIO::TifIO(const char* in)");
	setFilename(in);
}

TifIO::TifIO(const TifIO& rhs)
{
	debug_msg("TifIO::TifIO(const TifIO& rhs)");
	if(this==&rhs) return;
	setFilename(rhs.filename.c_str());
}

TifIO::~TifIO()
{
	debug_msg("destructer of class TifIO");
}

void TifIO::setFilename(const char* const in)
{
	debug_msg("TifIO::setFilename(const char* const in)");
	if(in==NULL) return;
	init();
	filename=in;
}


/*
!!!!! istream &read( char *バッファ, streamsize バイト数 );
read()は入力ストリームに対して使用し、バイト数 分をストリームから読み込んでバッファ に書き出す。
EOFに到達したらread()の処理は中止され、残りの文字数は入力時のバッファ の内容がそのまま残る: 


!!!!!!  pos_type tellg();

tellg()関数は入力ストリームに対して使用し、ストリーム内の現在の読み込み位置を返す。 


*/

int TifIO::getPageNumber(void)
{
	debug_msg("TifIO::getPageNumber(void)");
	if(filename=="") return 0;
	
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return 0;

	ifp.seekg(0,ios::end);
	data_t flen = ifp.tellg();//file length取得

	ifp.seekg(0,ios::beg);
	data_t head;

	if(!readHeader(ifp,head)) return 0;//header読む bigEndian or littleEndian確認(4byteだけ読んだ)

	data_t  off;
	tag_t  tags;
	m_Offsets.clear();
	while(true) 
	{
		readLong(ifp,off); //4byteだけ読む
		if(off>=flen){     // error
			m_Offsets.clear();
			return 0;
		}
		if(off==0) break; // successfully finished
		m_Offsets.push_back(off);//pageの先頭アドレス
		
		ifp.seekg(off,ios::beg);          //i-th pageの先頭へ移動 ！
		readShort(ifp,tags);              //tagの数を取得
		ifp.seekg(tags*IFD_SIZE,ios::cur);//tagを読み飛ばす        !
		debug_msg("page off="<<off);
	}
	ifp.close();
	return m_Offsets.size();
}

/*
上記のコードから、multi page tifは以下の感じになってるっぽい
header 4 byte MM042 or 420II
       4 byte 1個目のIFDのアドレス (4byte)
   
	
--> 1個目 IFD   2byte タグ数
	　　　　　　タグ1, タグ2...タグn (n*IFD_SIZE)
		  　　　次のpageのIFDのアドレス (最後のpageの時は0)
    2個目 IFD   2byte タグ数
	　　　　　　タグ1, タグ2...タグn (n*IFD_SIZE)
		        次のpageのIFDのアドレス
				。。。
*/

bool TifIO::isColor(int zID)
{
	debug_msg("TifIO::isColor(int zID): zID="<<zID);
	if(m_Offsets.empty()){
		if(getPageNumber()<=zID){
			return false;
		}
	}
	debug_msg("num of pages: "<<m_Offsets.size());
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	ifp.seekg(m_Offsets[zID],ios::beg);
	readIFD(ifp);
	debug_msg("readIFD fin.");
	bool ret=false;
	if(m_ttag[CTiffTag::StringToID("SAMPLESPERPIXEL")].addr>=2){
		ret=true;
	}
	ifp.close();
	return ret;
}

bool TifIO::isApplicable(std::string path)
{
	debug_msg("TifIO::isApplicable(std::string path) path="<<path);
	if(path!=""){
		init();
		filename=path;
	}
	if(m_Offsets.empty()){
		if(getPageNumber()==0){
			return false;
		}
	}
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	ifp.seekg(m_Offsets[0],ios::beg);
	readIFD(ifp);
	bool ret=true;
	// compressed TIFF
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1) ret=false;
	// pallet color, etc.
	if(m_ttag[CTiffTag::StringToID("PHOTOMETRIC")].addr>2) ret=false;
	// orientation topleft is only supported.
	if(m_ttag[CTiffTag::StringToID("ORIENTATION")].addr>1) ret=false;
	// planarconfig separate
	if(m_ttag[CTiffTag::StringToID("PLANARCONFIG")].addr>1) ret=false;
	// sampleformat void or complex
	if(m_ttag[CTiffTag::StringToID("SAMPLEFORMAT")].addr>3) ret=false;
	ifp.close();
	return ret;
}

void TifIO::readIFD(std::ifstream& ifp)
{
	debug_msg("readIFD(std::ifstream& ifp)");
	if(!m_ttag.empty()){
		m_ttag.clear();
	}
	tag_t tags;
	readShort(ifp,tags);
	CTiffTag t;
	debug_msg("tags="<<tags);
	for(int i=0;i<tags;i++){
		readShort(ifp,t.id);
		readShort(ifp,t.type);
		readLong(ifp,t.count);
		ifp.read(reinterpret_cast<char*>(&t.addr),sizeof(data_t));
		if(m_fBigEndian){
			if(t.getSize(t.type)*t.count>=sizeof(data_t)){
				swapBytes(&t.addr,sizeof(data_t));
			}
			else{
				swapBytes(&t.addr,t.getSize(t.type),t.count);
			}
		}
		m_ttag[t.id]=t;
		debug_msg(t.id<<", "<<t.IDToString(t.id)<<",\t"<<t.type<<", "<<t.count<<", "<<t.addr);
	}
	data_t pos=ifp.tellg();
	if(m_ttag[t.StringToID("BITSPERSAMPLE")].count>=3){
		ifp.seekg(m_ttag[t.StringToID("BITSPERSAMPLE")].addr,std::ios::beg);
		readShort(ifp,m_bps);
		ifp.seekg(pos,std::ios::beg);
	}
	else{
		m_bps=m_ttag[t.StringToID("BITSPERSAMPLE")].addr;
	}
}

bool TifIO::readLine(ifstream& ifp,double* buf)
{
	tag_t bps = m_bps>>3;//unsigned short   Byte Per Sample

	data_t len=m_ttag[ CTiffTag::StringToID("IMAGEWIDTH")     ].addr;
	tag_t  spp=m_ttag[ CTiffTag::StringToID("SAMPLESPERPIXEL")].addr;
	if(spp>1){
		len*=spp;
	}
	tag_t fmt=m_ttag[CTiffTag::StringToID("SAMPLEFORMAT")].addr;
	int64 spix;
	float fpix;

	if(fmt==2){ // sampleformat int
		spix=0;

		for(data_t x=0;x<len;x++){
			ifp.read(reinterpret_cast<char*>(&spix),bps);
			if(m_fBigEndian) swapBytes(&spix,bps);
			buf[x]=static_cast<double>(spix);
		}

	}
	else if(fmt==3){ // sampleformat ieeefp
		if(bps==4)
		{
			for(data_t x=0;x<len;x++)
			{
				ifp.read(reinterpret_cast<char*>(&fpix),bps);
				if(m_fBigEndian) swapBytes(&fpix,bps);
				buf[x]=static_cast<double>(fpix);
			}
		}
		else if(bps==8)
		{
			ifp.read(reinterpret_cast<char*>(buf),bps*len);
			if(m_fBigEndian) swapBytes(buf,bps,len);
		}
		else{
			return false;
		}
	}
	else{ // sampleformat uint or tag not set
		uint64 upix=0;
		for(data_t x=0;x<len;x++)
		{
			ifp.read(reinterpret_cast<char*>(&upix),bps);//bps = 1, 2, 3, 4, 5, 6, 7, 8 byte imageに対応している 
			if(m_fBigEndian) swapBytes(&upix,bps);
			buf[x]=static_cast<double>(upix);
		}
	}
	return true;
}

bool TifIO::getImage(Image* img,int zID)
{
	debug_msg("TifIO::getImage(Image* img,int zID);");
	if(img==NULL) return false;
	if(isColor(zID)) return false;
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1){
		debug_msg("compressed TIFF is not supported");
		return false;
	}
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	img->sx=m_ttag[CTiffTag::StringToID("IMAGEWIDTH")].addr;
	img->sy=m_ttag[CTiffTag::StringToID("IMAGELENGTH")].addr;
	debug_msg("sx="<<img->sx);
	debug_msg("sy="<<img->sy);
	float fbuf;
	img->px=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHX")].addr);
	if(img->px==0) img->px=1;
	img->py=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHY")].addr);
	if(img->py==0) img->py=1;
	data_t rps=m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
	tag_t id=CTiffTag::StringToID("STRIPOFFSETS");
	data_t* soff=new data_t [m_ttag[id].count]; // strip offsets
	if(m_ttag[id].count==1){
		soff[0]=m_ttag[id].addr;
	}
	else{
		ifp.seekg(m_ttag[id].addr,ios::beg);
		for(data_t i=0;i<m_ttag[id].count;i++){
			readLong(ifp,soff[i]);
		}
	}
	debug_msg("rps="<<rps);
	bool ret=true;
	try{
		debug_msg("sy="<<img->sy);
		img->img=new double* [img->sy];
		data_t y;
		for(y=0;y<img->sy;y++){
			img->img[y]=new double[img->sx];
		}
		y=0;
		for(data_t i=0;i<m_ttag[id].count;i++){
			ifp.seekg(soff[i],ios::beg);
			for(data_t j=0;j<rps;j++){
				if(y>=img->sy) break;
				if(false==readLine(ifp,img->img[y])){
					throw(-1);
				}
				y++;
			}
		}
	}
	catch(...){
		ret=false;
	}
	if(ret) debug_msg("getImage(Image* img,int zID) is now successfully completed.");
	delete[] soff;
	return ret;
}


bool TifIO::getColorImage(Image* R, Image* G, Image* B, int zID)
{
	debug_msg("TifIO::getColorImage(Image* R, Image* G, Image* B, int zID)");
	if(R==NULL||G==NULL||B==NULL) return false;
	if(!isColor(zID)) return false;
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1){
		debug_msg("compressed TIFF is not supported");
		return false;
	}
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	R->sx=m_ttag[CTiffTag::StringToID("IMAGEWIDTH")].addr;
	G->sx=B->sx=R->sx;
	R->sy=m_ttag[CTiffTag::StringToID("IMAGELENGTH")].addr;
	G->sy=B->sy=R->sy;
	R->px=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHX")].addr);
	if(R->px==0) R->px=1;
	G->px=B->px=R->px;
	R->py=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHY")].addr);
	if(R->py==0) R->py=1;
	G->py=B->py=R->py;
	data_t rps=m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
	tag_t id=CTiffTag::StringToID("STRIPOFFSETS");
	data_t* soff=new data_t [m_ttag[id].count];
	if(m_ttag[id].count==1){
		soff[0]=m_ttag[id].addr;
	}
	else{
		ifp.seekg(m_ttag[id].addr,ios::beg);
		for(data_t i=0;i<m_ttag[id].count;i++){
			readLong(ifp,soff[i]);
		}
	}
	tag_t spp=m_ttag[CTiffTag::StringToID("SAMPLESPERPIXEL")].addr;
	double* buf;
	bool ret=true;
	try{
		buf=new double [R->sx*spp];
		R->img=new double* [R->sy];
		G->img=new double* [G->sy];
		B->img=new double* [B->sy];
		data_t y;
		for(y=0;y<R->sy;y++){
			R->img[y]=new double[R->sx];
			G->img[y]=new double[G->sx];
			B->img[y]=new double[B->sx];
		}
		y=0;
		for(data_t i=0;i<m_ttag[id].count;i++){
			ifp.seekg(soff[i],ios::beg);
			for(data_t j=0;j<rps;j++){
				if(y>=R->sy) break;
				if(false==readLine(ifp,buf)){
					throw(-1);
				}
				for(data_t x=0;x<R->sx;x++){
					R->img[y][x]=buf[x*spp+0];
					G->img[y][x]=buf[x*spp+1];
					B->img[y][x]=buf[x*spp+2];
				}
				y++;
			}
		}
	}
	catch(...){
		delete R;
		delete G;
		delete B;
		R=G=B=NULL;
		ret=false;
	}
	delete[] soff;
	delete[] buf;
	return ret;
}

bool TifIO::writeLine(fstream& ofp,double* buf,data_t len,data_t size,tag_t fmt)
{
	int64 spix;
	uint64 upix;
	float fpix;
	if(fmt==2){ // sampleformat int
		spix=0;
		for(data_t x=0;x<len;x++){
			spix=static_cast<int64>(buf[x]);
			if(m_fBigEndian) swapBytes(&spix,size);
			ofp.write(reinterpret_cast<char*>(&spix),size);
		}
	}
	else if(fmt==3){ // sampleformat ieeefp
		if(size==4){
			for(data_t x=0;x<len;x++){
				fpix=static_cast<float>(buf[x]);
				if(m_fBigEndian) swapBytes(&fpix,size);
				ofp.write(reinterpret_cast<char*>(&fpix),size);
			}
		}
		else if(size==8){
			for(data_t x=0;x<len;x++){
				if(m_fBigEndian) swapBytes(buf,size,len);
				ofp.write(reinterpret_cast<char*>(buf),size*len);
			}
		}
		else{
			return false;
		}
	}
	else{ // sampleformat uint or tag not set
		upix=0;
		for(data_t x=0;x<len;x++){
			upix=static_cast<uint64>(buf[x]);
			if(m_fBigEndian) swapBytes(&upix,size);
			ofp.write(reinterpret_cast<char*>(&upix),size);
		}
	}
	return true;
}

bool TifIO::saveImage(Image* in,double smax,double smin,int fSaveAsbit)
{
	debug_msg("TifIO::saveImage(Image* in,double smax,double smin,int fSaveAsbit)");
	ofstream ofp(filename.c_str(),ios::out|ios::binary|ios::trunc);
	if(!ofp.is_open()) return false;

	data_t size;
	if     ( fSaveAsbit ==8  ){  size = sizeof(uint8 ); }
	else if( fSaveAsbit ==64 ){  size = sizeof(double); }
	else                      {  size = sizeof(uint16); }

	data_t head=0x002a4949;
	writeLong(ofp,head);
	data_t off=8;
	writeLong(ofp,off);
	tag_t ntags=20;
	writeShort(ofp,ntags);
	
	data_t p;
	p=ofp.tellp();
	p+=IFD_SIZE*ntags+4;

	CTiffTag tag;
	tag.id=tag.StringToID("IMAGEWIDTH"   ); tag.type=3; tag.count=1; tag.addr=in->sx ;  writeTag(ofp,tag); // image width
	tag.id=tag.StringToID("IMAGELENGTH"  ); tag.type=3; tag.count=1; tag.addr=in->sy ;  writeTag(ofp,tag); // image length
	tag.id=tag.StringToID("BITSPERSAMPLE"); tag.type=3; tag.count=1; tag.addr=size<<3;  writeTag(ofp,tag); // bits per sample
	tag.id=tag.StringToID("COMPRESSION"  ); tag.type=3; tag.count=1; tag.addr=1;        writeTag(ofp,tag); // compression
	tag.id=tag.StringToID("PHOTOMETRIC"  ); tag.type=3; tag.count=1; tag.addr=1;        writeTag(ofp,tag); // photometric
	tag.id=tag.StringToID("STRIPOFFSETS" ); tag.type=4; tag.count=in->sy; tag.addr=p;   writeTag(ofp,tag); // strip offsets

	p+=tag.count*4;
	tag.id=tag.StringToID("SAMPLESPERPIXEL");tag.type=3; tag.count=1;      tag.addr=1; writeTag(ofp,tag); // samples per pixel
	tag.id=tag.StringToID("ROWSPERSTRIP"   );tag.type=3; tag.count=1;      tag.addr=1; writeTag(ofp,tag); // rows per strip
	tag.id=tag.StringToID("STRIPBYTECOUNTS");tag.type=4; tag.count=in->sy; tag.addr=p; writeTag(ofp,tag); // strip byte counts
	
	p+=tag.count*4;
	tag.id=tag.StringToID("PLANARCONFIG");  tag.type=3; tag.count=1;tag.addr=1;                    writeTag(ofp,tag); // planar config
	tag.id=tag.StringToID("ARTIST"      );  tag.type=2; tag.count=sizeof(BRICT_STRING);tag.addr=p; writeTag(ofp,tag); // artist
	
	p+=sizeof(BRICT_STRING);
	tag.id=tag.StringToID("SAMPLEFORMAT");  tag.type=3; tag.count=1;
	if(fSaveAsbit==8||fSaveAsbit==16) tag.addr=1;
	else tag.addr=3;
	writeTag(ofp,tag); // sampleformat
	
	tag.id=tag.StringToID("B_SIZEZ"     ); tag.type=3;  tag.count=1;tag.addr=1;  writeTag(ofp,tag); // size z
	tag.id=tag.StringToID("B_SIZET"     ); tag.type=3;  tag.count=1;tag.addr=1;  writeTag(ofp,tag); // size z


	float ftmp;
	tag.id=tag.StringToID("B_PITCHX"    ); tag.type=11; tag.count=1;tag.addr=0; writeTag(ofp,tag); // pitch x
	ofp.seekp(-4,ios::cur);
	ftmp=static_cast<float>(in->px);
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

	tag.id=tag.StringToID("B_PITCHY"    ); tag.type=11; tag.count=1;tag.addr=0; writeTag(ofp,tag); // pitch y
	ofp.seekp(-4,ios::cur);
	ftmp=static_cast<float>(in->py);
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	
	tag.id=tag.StringToID("B_PITCHZ"    ); tag.type=11; tag.count=1;tag.addr=0; writeTag(ofp,tag); // pitch z
	ofp.seekp(-4,ios::cur);
	ftmp=1.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	
	tag.id=tag.StringToID("B_PITCHT"    ); tag.type=11; tag.count=1;tag.addr=0; writeTag(ofp,tag); // pitch t
	ofp.seekp(-4,ios::cur);
	ftmp=1.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

	tag.id=tag.StringToID("B_TIMENUMBER"); tag.type=4 ; tag.count=1;tag.addr=0; writeTag(ofp,tag); // time #
	tag.id=tag.StringToID("B_ACTUALTIME"); tag.type=11; tag.count=1;tag.addr=0; writeTag(ofp,tag); // real time
	ofp.seekp(-4,ios::cur);
	ftmp=0.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

	off=0;
	writeLong(ofp,off);
	for(data_t y=0;y<in->sy;y++){ //strip offsets
		writeLong(ofp,p);
		p+=in->sx*size;
	}
	off=in->sx*size;
	for(data_t y=0;y<in->sy;y++){ //strip byte counts
		writeLong(ofp,off);
	}
	ofp.write(BRICT_STRING,sizeof(BRICT_STRING)); //artist







	//image bits
	if(fSaveAsbit==8){
		double sw=abs(smax-smin);
		if(sw<1.0) sw=1.0;
		double rt=UCHAR_MAX/sw;
		uint8* buf=new uint8 [in->sx];
		for(data_t y=0;y<in->sy;y++){
			for(data_t x=0;x<in->sx;x++){
				if(in->img[y][x]<smin) buf[x]=0;
				else if(in->img[y][x]>smax) buf[x]=UCHAR_MAX;
				else buf[x]=static_cast<uint8>(in->img[y][x]*rt);
			}
			ofp.write(reinterpret_cast<char*>(buf),in->sx*size);
		}
		delete[] buf;
	}else if(fSaveAsbit==64){
		for(data_t y=0;y<in->sy;y++){
			ofp.write(reinterpret_cast<char*>(in->img[y]),in->sx*size);
		}
	}else{
		double sw=abs(smax-smin);
		if(sw<1.0) sw=1.0;
		double rt = USHRT_MAX / sw;
		uint16* buf = new uint16 [in->sx];

		for(data_t y=0;y<in->sy;y++){
			for(data_t x=0;x<in->sx;x++){
				if(in->img[y][x]<smin) buf[x]=0;
				else if(in->img[y][x]>smax) buf[x]=USHRT_MAX;
				else buf[x]=static_cast<uint16>( in->img[y][x]*rt);
			}
			ofp.write(reinterpret_cast<char*>(buf),in->sx*size);
		}
		delete[] buf;

	}
	return true;
}

bool TifIO::saveColorImage(Image* R,Image* G,Image* B,double smax,double smin,int fSaveAsbit)
{
	debug_msg("TifIO::saveColorImage(Image* R,Image* G,Image* B,double smax,double smin,bool fSaveAsbit)");
	ofstream ofp(filename.c_str(),ios::out|ios::binary|ios::trunc);
	if(!ofp.is_open()) return false;
	tag_t size;
	if(fSaveAsbit==8){
	  size=sizeof(uint8);
	}else if(fSaveAsbit==64){
	  size = sizeof(double);
	}else{
	  size = sizeof(uint16);
	}
	data_t head=0x002a4949;
	writeLong(ofp,head);
	data_t off=8;
	writeLong(ofp,off);
	tag_t ntags=20;
	writeShort(ofp,ntags);
	data_t p;
	p=ofp.tellp();
	p+=IFD_SIZE*ntags+4;
	CTiffTag tag;
	tag.id=tag.StringToID("IMAGEWIDTH");
	tag.type=3;tag.count=1;tag.addr=R->sx;
	writeTag(ofp,tag); // image width
	tag.id=tag.StringToID("IMAGELENGTH");
	tag.type=3;tag.count=1;tag.addr=R->sy;
	writeTag(ofp,tag); // image length
	tag.id=tag.StringToID("BITSPERSAMPLE");
	tag.type=3;tag.count=3;tag.addr=p;
	writeTag(ofp,tag); // bits per sample
	p+=3*sizeof(tag_t);
	tag.id=tag.StringToID("COMPRESSION");
	tag.type=3;tag.count=1;tag.addr=1;
	writeTag(ofp,tag); // compression
	tag.id=tag.StringToID("PHOTOMETRIC");
	tag.type=3;tag.count=1;tag.addr=2;
	writeTag(ofp,tag); // photometric
	tag.id=tag.StringToID("STRIPOFFSETS");
	tag.type=4;tag.count=R->sy;tag.addr=p;
	writeTag(ofp,tag); // strip offsets
	p+=tag.count*4;
	tag.id=tag.StringToID("SAMPLESPERPIXEL");
	tag.type=3;tag.count=1;tag.addr=3;
	writeTag(ofp,tag); // samples per pixel
	tag.id=tag.StringToID("ROWSPERSTRIP");
	tag.type=3;tag.count=1;tag.addr=1;
	writeTag(ofp,tag); // rows per strip
	tag.id=tag.StringToID("STRIPBYTECOUNTS");
	tag.type=4;tag.count=R->sy;tag.addr=p;
	writeTag(ofp,tag); // strip byte counts
	p+=tag.count*4;
	tag.id=tag.StringToID("PLANARCONFIG");
	tag.type=3;tag.count=1;tag.addr=1;
	writeTag(ofp,tag); // planar config
	tag.id=tag.StringToID("ARTIST");
	tag.type=2;tag.count=sizeof(BRICT_STRING);tag.addr=p;
	writeTag(ofp,tag); // artist
	p+=sizeof(BRICT_STRING);
	tag.id=tag.StringToID("SAMPLEFORMAT");
	tag.type=3;tag.count=1;
	if(fSaveAsbit==8||fSaveAsbit==16) tag.addr=1;
	else tag.addr=3;
	writeTag(ofp,tag); // sampleformat
	tag.id=tag.StringToID("B_SIZEZ");
	tag.type=3;tag.count=1;tag.addr=1;
	writeTag(ofp,tag); // size z
	tag.id=tag.StringToID("B_SIZET");
	tag.type=3;tag.count=1;tag.addr=1;
	writeTag(ofp,tag); // size z
	float ftmp;
	tag.id=tag.StringToID("B_PITCHX");
	tag.type=11;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // pitch x
	ofp.seekp(-4,ios::cur);
	ftmp=static_cast<float>(R->px);
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	tag.id=tag.StringToID("B_PITCHY");
	tag.type=11;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // pitch y
	ofp.seekp(-4,ios::cur);
	ftmp=static_cast<float>(R->py);
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	tag.id=tag.StringToID("B_PITCHZ");
	tag.type=11;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // pitch z
	ofp.seekp(-4,ios::cur);
	ftmp=1.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	tag.id=tag.StringToID("B_PITCHT");
	tag.type=11;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // pitch t
	ofp.seekp(-4,ios::cur);
	ftmp=1.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
	tag.id=tag.StringToID("B_TIMENUMBER");
	tag.type=4;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // time #
	tag.id=tag.StringToID("B_ACTUALTIME");
	tag.type=11;tag.count=1;tag.addr=0;
	writeTag(ofp,tag); // real time
	ofp.seekp(-4,ios::cur);
	ftmp=0.0f;
	ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

	off=0;
	writeLong(ofp,off);
	tag_t bps=size<<3;
	for(data_t i=0;i<3;i++){ // bits per sample
		writeShort(ofp,bps);
	}
	for(data_t y=0;y<R->sy;y++){ //strip offsets
		writeLong(ofp,p);
		p+=R->sx*size*3;
	}
	off=R->sx*size*3;
	for(data_t y=0;y<R->sy;y++){ //strip byte counts
		writeLong(ofp,off);
	}
	ofp.write(BRICT_STRING,sizeof(BRICT_STRING)); //artist

	//image bits
	if(fSaveAsbit==8){
		uint8* buf=new uint8[R->sx*3];
		double sw=abs(smax-smin);
		if(sw<1.0) sw=1.0;
		double rt=UCHAR_MAX/sw;
		uint8* pb;
		for(data_t y=0;y<R->sy;y++){
			pb=buf;
			for(data_t x=0;x<R->sx;x++){
				if(R->img[y][x]<smin) *pb=0;
				else if(R->img[y][x]>smax) *pb=UCHAR_MAX;
				else *pb=static_cast<uint8>(R->img[y][x]*rt);
				pb++;
				if(G->img[y][x]<smin) *pb=0;
				else if(G->img[y][x]>smax) *pb=UCHAR_MAX;
				else *pb=static_cast<uint8>(G->img[y][x]*rt);
				pb++;
				if(B->img[y][x]<smin) *pb=0;
				else if(B->img[y][x]>smax) *pb=UCHAR_MAX;
				else *pb=static_cast<uint8>(B->img[y][x]*rt);
				pb++;
			}
			ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
		}
		delete[] buf;
	}else if(fSaveAsbit==64){
		double* buf=new double[R->sx*3];
		double* pb;
		for(data_t y=0;y<R->sy;y++){
			pb=buf;
			for(data_t x=0;x<R->sx;x++){
				*pb++=R->img[y][x];
				*pb++=G->img[y][x];
				*pb++=B->img[y][x];
			}
			ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
		}
		delete[] buf;
	}else{
	  uint16* buf=new uint16[R->sx*3];
	  double sw=abs(smax-smin);
	  if(sw<1.0) sw=1.0;
	  double rt=USHRT_MAX/sw;
	  uint16* pb;
	  for(data_t y=0;y<R->sy;y++){
	    pb=buf;
	    for(data_t x=0;x<R->sx;x++){
	      if(R->img[y][x]<smin) *pb=0;
	      else if(R->img[y][x]>smax) *pb=USHRT_MAX;
	      else *pb=static_cast<uint16>(R->img[y][x]*rt);
	      pb++;
	      if(G->img[y][x]<smin) *pb=0;
	      else if(G->img[y][x]>smax) *pb=USHRT_MAX;
	      else *pb=static_cast<uint16>(G->img[y][x]*rt);
	      pb++;
	      if(B->img[y][x]<smin) *pb=0;
	      else if(B->img[y][x]>smax) *pb=USHRT_MAX;
	      else *pb=static_cast<uint16>(B->img[y][x]*rt);
	      pb++;
	    }
	    ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
	  }
	  delete[] buf;
	  
	}
	return true;
}

bool TifIO::saveImage(Image* in,int zID)
{
	debug_msg("TifIO::saveImage(Image* in,int zID)");
	if(isColor(zID)) return false;
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	if(in->sx!=m_ttag[CTiffTag::StringToID("IMAGEWIDTH")].addr) return false;
	if(in->sy!=m_ttag[CTiffTag::StringToID("IMAGELENGTH")].addr) return false;
	data_t rps=m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
	tag_t id=CTiffTag::StringToID("STRIPOFFSETS");
	data_t* soff=new data_t [m_ttag[id].count];
	if(m_ttag[id].count==1){
		soff[0]=m_ttag[id].addr;
	}
	else{
		ifp.seekg(m_ttag[id].addr,ios::beg);
		for(data_t i=0;i<m_ttag[id].count;i++){
			readLong(ifp,soff[i]);
		}
	}
	ifp.close();
	fstream ofp(filename.c_str(),ios::out|ios::in|ios::binary);
	if(!ofp.is_open()) return false;
	bool ret=true;
	double* buf;
	try{
		buf=new double[in->sx];
		for(data_t i=0;i<m_ttag[id].count;i++){
			ofp.seekp(soff[i],ios::beg);
			for(data_t j=0;j<rps;j++){
				memcpy(buf,in->img[i*rps+j],in->sx*sizeof(double));
				if(false==writeLine(ofp,buf,in->sx,m_bps>>3,m_ttag[CTiffTag::StringToID("SAMPLEFORMAT")].addr)){
					throw(-1);
				}
			}
		}
	}
	catch(...){
		ret=false;
	}
	delete[] soff;
	delete[] buf;
	debug_msg("saveImage(Image*,int) successfully ends");
	return ret;
}

bool TifIO::saveColorImage(Image* R,Image* G,Image* B,int zID)
{
	debug_msg("TifIO::saveColorImage(Image* R,Image* G,Image* B,int zID)");
	if(!isColor(zID)) return false;
	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	if(R->sx!=m_ttag[CTiffTag::StringToID("IMAGEWIDTH")].addr) return false;
	if(R->sy!=m_ttag[CTiffTag::StringToID("IMAGELENGTH")].addr) return false;
	data_t rps=m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
	tag_t id=CTiffTag::StringToID("STRIPOFFSETS");
	data_t* soff=new data_t [m_ttag[id].count];
	if(m_ttag[id].count==1){
		soff[0]=m_ttag[id].addr;
	}
	else{
		ifp.seekg(m_ttag[id].addr,ios::beg);
		for(data_t i=0;i<m_ttag[id].count;i++){
			readLong(ifp,soff[i]);
		}
	}
	ifp.close();
	fstream ofp(filename.c_str(),ios::out|ios::in|ios::binary);
	if(!ofp.is_open()) return false;
	bool ret=true;
	double* buf;
	double* p;
	try{
		buf=new double[R->sx*3];
		for(data_t i=0;i<m_ttag[id].count;i++){
			ofp.seekp(soff[i],ios::beg);
			for(data_t j=0;j<rps;j++){
				p=buf;
				for(data_t x=0;x<R->sx;x++){
					*p++=R->img[i*rps+j][x];
					*p++=G->img[i*rps+j][x];
					*p++=B->img[i*rps+j][x];
				}
				if(false==writeLine(ofp,buf,3*R->sx,m_bps>>3,m_ttag[CTiffTag::StringToID("SAMPLEFORMAT")].addr)){
					throw(-1);
				}
			}
		}
	}
	catch(...){
		ret=false;
	}
	delete[] soff;
	delete[] buf;
	debug_msg("saveColorImage(Image*,Image*,Image*,int) successfully ends");
	return ret;
}


/*
fucciに対して14.5sec
*/
bool TifIO::getImage(Image3D* img)
{
	debug_msg("TifIO::getImage(Image3D* img)");
	if(img==NULL) return false;
	if(isColor(0)) return false;
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1){
		debug_msg("compressed TIFF is not supported");
		return false;
	}
	img->sx = m_ttag[ CTiffTag::StringToID("IMAGEWIDTH" ) ].addr;
	img->sy = m_ttag[ CTiffTag::StringToID("IMAGELENGTH") ].addr;
	img->sz = getPageNumber();

//	float fbuf;
	img->px =* reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHX")].addr); if(img->px==0) img->px=1;
	img->py =* reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHY")].addr); if(img->py==0) img->py=1;
	img->pz =* reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHZ")].addr); if(img->pz==0) img->pz=1;

	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;

	img->img = new double** [img->sz];
	bool ret = true;



	for(data_t z=0;z<img->sz;z++)
	{
		ifp.seekg(m_Offsets[z],ios::beg);
		if(isColor(z)) return false;
		data_t rps = m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
		tag_t  id  =        CTiffTag::StringToID("STRIPOFFSETS");
		data_t* soff=new data_t [m_ttag[id].count];

		if(m_ttag[id].count==1){
			soff[0]=m_ttag[id].addr;
			debug_msg("soff 0: "<<soff[0]);
		}
		else{
			ifp.seekg(m_ttag[id].addr,ios::beg);
			for(data_t i=0;i<m_ttag[id].count;i++){
				readLong(ifp,soff[i]);
				debug_msg("soff "<<i<<": "<<soff[i]);
			}
		}

		try{
			img->img[z]=new double* [img->sy];
			data_t y;
			for(y=0;y<img->sy;y++){
				img->img[z][y]=new double[img->sx];
			}
			y=0;
			for(data_t i=0;i<m_ttag[id].count;i++){
				ifp.seekg(soff[i],ios::beg);
				for(data_t j=0;j<rps;j++){
					if(y>=img->sy) break;
					if(false==readLine(ifp,img->img[z][y])){
						throw(-1);
					}
					y++;
				}
			}
		}
		catch(...)
		{
			delete img;
			img=NULL;
			ret=false;
	
		}
		delete[] soff;
	}

	return ret;
}


bool TifIO::t_getImage(Image3D* img)
{
	if(img==NULL || isColor(0)) return false;
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1) return false;

	//resolution and pich
	img->sx = m_ttag[ CTiffTag::StringToID("IMAGEWIDTH")  ].addr;
	img->sy = m_ttag[ CTiffTag::StringToID("IMAGELENGTH") ].addr;
	img->sz = getPageNumber();

	img->px = *reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHX")].addr);  if(img->px==0) img->px=1;
	img->py = *reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHY")].addr);  if(img->py==0) img->py=1;
	img->pz = *reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHZ")].addr);  if(img->pz==0) img->pz=1;
	
	ifstream ifp( filename.c_str(), ios::in | ios::binary);
	if(!ifp.is_open()) return false;
	
	img->img    = new double** [img->sz];
	for(data_t z = 0; z < img->sz; ++z)
	{
		img->img[z] = new double* [img->sy];
		for(int y=0; y < img->sy; ++y) img->img[z][y]=new double[img->sx];
	}

	bool ret = true;

	for(data_t z = 0; z < img->sz; ++z)
	{
		//file先頭を基準に(beg), m_offsets[z]分だけseekする(これから読むpageのIFD)
		ifp.seekg( m_Offsets[z], ios::beg);

		if(isColor(z)) return false;//この中でtagの更新が行われる
		data_t rps   = m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
		tag_t  id    =        CTiffTag::StringToID("STRIPOFFSETS");
		data_t* soff = new data_t[ m_ttag[id].count ];

		if(m_ttag[id].count==1)
		{
			soff[0]=m_ttag[id].addr;
			debug_msg("soff 0: "<<soff[0]);
		}
		else{
			ifp.seekg(m_ttag[id].addr,ios::beg);
			for(data_t i=0;i<m_ttag[id].count;i++){
				readLong(ifp,soff[i]);
				debug_msg("soff "<<i<<": "<<soff[i]);
			}
		}

		try{
			data_t y = 0;

			for(data_t i=0; i < m_ttag[id].count; ++i)
			{
				ifp.seekg(soff[i],ios::beg);
				for(data_t j=0;j<rps;j++)
				{
					if(y>=img->sy) break;
					if(!readLine(ifp,img->img[z][y]))
					{
						throw(-1);
					}
					y++;
				}
			}
		}
		catch(...)
		{
			delete img;
			img=NULL;
			ret=false;
	
		}
		delete[] soff;
	}

	return ret;
}

bool TifIO::getColorImage(Image3D* R,Image3D* G,Image3D* B)
{
	debug_msg("TifIO::getColorImage(Image3D* R,Image3D* G,Image3D* B)");
	if(R==NULL||G==NULL||B==NULL) return false;
	if(!isColor(0)) return false;
	
	if(m_ttag[CTiffTag::StringToID("COMPRESSION")].addr>1)
	{
		debug_msg("compressed TIFF is not supported");
		return false;
	}

	R->sx=m_ttag[CTiffTag::StringToID("IMAGEWIDTH" )].addr;
	R->sy=m_ttag[CTiffTag::StringToID("IMAGELENGTH")].addr;
	R->sz=getPageNumber();

//	float fbuf;
	R->px=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHX")].addr);
	if(R->px==0) R->px=1;
	R->py=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHY")].addr);
	if(R->py==0) R->py=1;
	R->pz=*reinterpret_cast<float*>(&m_ttag[CTiffTag::StringToID("B_PITCHZ")].addr);
	if(R->pz==0) R->pz=1;

	G->sx = B->sx = R->sx;
	G->sy = B->sy = R->sy;
	G->sz = B->sz = R->sz;
	G->px = B->px = R->px;
	G->py = B->py = R->py;
	G->pz = B->pz = R->pz;

	debug_msg("R->sx="<<R->sx<<", R->sy="<<R->sy<<", R->sz="<<R->sz);
	debug_msg("R->px="<<R->px<<", R->py="<<R->py<<", R->pz="<<R->pz);

	ifstream ifp(filename.c_str(),ios::in|ios::binary);
	if(!ifp.is_open()) return false;
	tag_t spp=m_ttag[CTiffTag::StringToID("SAMPLESPERPIXEL")].addr;
	bool ret=true;
	double* buf;
	data_t* soff;

	try{
		R->img=new double** [R->sz];
		G->img=new double** [G->sz];
		B->img=new double** [B->sz];
		buf=new double [R->sx*spp];
		for(data_t z=0;z<R->sz;z++){
			ifp.seekg(m_Offsets[z],ios::beg);
			if(!isColor(z)) return false;
			data_t rps=m_ttag[CTiffTag::StringToID("ROWSPERSTRIP")].addr;
			tag_t id=CTiffTag::StringToID("STRIPOFFSETS");
			soff=new data_t [m_ttag[id].count];
			if(m_ttag[id].count==1){
				soff[0]=m_ttag[id].addr;
			}
			else{
				ifp.seekg(m_ttag[id].addr,ios::beg);
				for(data_t i=0;i<m_ttag[id].count;i++){
					readLong(ifp,soff[i]);
				}
			}
			R->img[z]=new double* [R->sy];
			G->img[z]=new double* [G->sy];
			B->img[z]=new double* [B->sy];
			data_t y;
			for(y=0;y<R->sy;y++){
				R->img[z][y]=new double[R->sx];
				G->img[z][y]=new double[G->sx];
				B->img[z][y]=new double[B->sx];
			}
			y=0;
			
			for(data_t i=0;i<m_ttag[id].count;i++)
			{
				ifp.seekg(soff[i],ios::beg);
				for(data_t j=0; j < rps; j++)
				{
					if(y>=R->sy) break;
					if(false==readLine(ifp,buf))
					{
						throw(-1);
					}
					for(data_t x=0;x<R->sx;x++){
						R->img[z][y][x]=buf[x*spp+0];
						G->img[z][y][x]=buf[x*spp+1];
						B->img[z][y][x]=buf[x*spp+2];
					}
					y++;
				}
			}
			delete[] soff; soff=NULL;
		}
	}
	catch(...){
		ret=false;
	}
	delete[] soff;
	delete[] buf;
	debug_msg("getColorImage(Image3D*,Image3D*,Image3D*) successfully ends");

	return ret;
}

bool TifIO::saveImage(Image3D* in,double smax,double smin,int fSaveAsbit)
{
	debug_msg("TifIO::saveImage(Image3D* in,double smax,double smin,int fSaveAsbit)");
	ofstream ofp(filename.c_str(),ios::out|ios::binary|ios::trunc);
	if(!ofp.is_open()) return false;
	data_t size;
	if(fSaveAsbit==8){
	  size=sizeof(uint8);
	}else if(fSaveAsbit==64){
	  size = sizeof(double);
	}else{
	  size = sizeof(uint16);
	}
	

	data_t head=0x002a4949;
	writeLong(ofp,head);
	data_t off=8;
	writeLong(ofp,off);
	data_t p;
	for(data_t z=0;z<in->sz;z++){
		tag_t ntags=20;
		writeShort(ofp,ntags);
		p=ofp.tellp();
		p+=IFD_SIZE*ntags+4;
		CTiffTag tag;
		tag.id=tag.StringToID("IMAGEWIDTH");
		tag.type=3;tag.count=1;tag.addr=in->sx;
		writeTag(ofp,tag); // image width
		tag.id=tag.StringToID("IMAGELENGTH");
		tag.type=3;tag.count=1;tag.addr=in->sy;
		writeTag(ofp,tag); // image length
		tag.id=tag.StringToID("BITSPERSAMPLE");
		tag.type=3;tag.count=1;tag.addr=size<<3;
		writeTag(ofp,tag); // bits per sample
		tag.id=tag.StringToID("COMPRESSION");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // compression
		tag.id=tag.StringToID("PHOTOMETRIC");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // photometric
		tag.id=tag.StringToID("STRIPOFFSETS");
		tag.type=4;tag.count=in->sy;tag.addr=p;
		writeTag(ofp,tag); // strip offsets
		p+=tag.count*4;
		tag.id=tag.StringToID("SAMPLESPERPIXEL");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // samples per pixel
		tag.id=tag.StringToID("ROWSPERSTRIP");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // rows per strip
		tag.id=tag.StringToID("STRIPBYTECOUNTS");
		tag.type=4;tag.count=in->sy;tag.addr=p;
		writeTag(ofp,tag); // strip byte counts
		p+=tag.count*4;
		tag.id=tag.StringToID("PLANARCONFIG");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // planar config
		tag.id=tag.StringToID("ARTIST");
		tag.type=2;tag.count=sizeof(BRICT_STRING);tag.addr=p;
		writeTag(ofp,tag); // artist
		p+=sizeof(BRICT_STRING);
		tag.id=tag.StringToID("SAMPLEFORMAT");
		tag.type=3;tag.count=1;
		if(fSaveAsbit==8||fSaveAsbit==16) tag.addr=1;
		else tag.addr=3;
		writeTag(ofp,tag); // sampleformat
		tag.id=tag.StringToID("B_SIZEZ");
		tag.type=3;tag.count=1;tag.addr=in->sz;
		writeTag(ofp,tag); // size z
		tag.id=tag.StringToID("B_SIZET");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // size z
		float ftmp;
		tag.id=tag.StringToID("B_PITCHX");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch x
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(in->px);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHY");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch y
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(in->py);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHZ");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch z
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(in->pz);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHT");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch t
		ofp.seekp(-4,ios::cur);
		ftmp=1.0f;
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_TIMENUMBER");
		tag.type=4;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // time #
		tag.id=tag.StringToID("B_ACTUALTIME");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // real time
		ofp.seekp(-4,ios::cur);
		ftmp=0.0f;
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

		if(z!=in->sz-1){
			off=(data_t)ofp.tellp()+4+in->sy*8+sizeof(BRICT_STRING)+in->sy*in->sx*size;
		}
		else {
			off=0;
		}
		writeLong(ofp,off);
		for(data_t y=0;y<in->sy;y++){ //strip offsets
			writeLong(ofp,p);
			p+=in->sx*size;
		}
		off=in->sx*size;
		for(data_t y=0;y<in->sy;y++){ //strip byte counts
			writeLong(ofp,off);
		}
		ofp.write(BRICT_STRING,sizeof(BRICT_STRING)); //artist

		//image bits
		if(fSaveAsbit==8)
		{
			double sw = abs( smax - smin );
			if( sw < 1.0) sw = 1.0;
			double rt = UCHAR_MAX / sw;
			uint8* buf = new uint8 [in->sx];
			for( data_t y = 0; y < in->sy; y++)
			{
				for( data_t x = 0; x < in->sx; x++)
				{
					if     ( in->img[z][y][x] < smin ) buf[x] = 0        ;
					else if( in->img[z][y][x] > smax ) buf[x] = UCHAR_MAX;
					else                               buf[x] = static_cast<uint8>( in->img[z][y][x] * rt );
				}
				ofp.write(reinterpret_cast<char*>(buf),in->sx*size);
			}
			delete[] buf;
		}
		else if( fSaveAsbit == 64 )
		{
			for(data_t y=0;y<in->sy;y++)
			{
				ofp.write(reinterpret_cast<char*>(in->img[z][y]),in->sx*sizeof(double));
			}
		}else{
		  double sw=abs(smax-smin);
			if(sw<1.0) sw=1.0;
			double rt=USHRT_MAX/sw;
			uint16* buf=new uint16 [in->sx];
			for(data_t y=0;y<in->sy;y++){
			  for(data_t x=0;x<in->sx;x++){
			    if(in->img[z][y][x]<smin) buf[x]=0;
			    else if(in->img[z][y][x]>smax) buf[x]=USHRT_MAX;
			    else buf[x]=static_cast<uint16>(in->img[z][y][x]*rt);
			  }
			  ofp.write(reinterpret_cast<char*>(buf),in->sx*size);
			}
			delete[] buf;
			
		}
	}
	return true;
}

bool TifIO::saveColorImage(Image3D* R,Image3D* G,Image3D* B,double smax,double smin,int fSaveAsbit)
{
	debug_msg("TifIO::saveColorImage(Image3D* R,Image3D* G,Image3D* B,double smax,double smin,int fSaveAsbit)");
	ofstream ofp(filename.c_str(),ios::out|ios::binary|ios::trunc);
	if(!ofp.is_open()) return false;
	tag_t size;
	if(fSaveAsbit==8){
	  size=sizeof(uint8);
	}else if(fSaveAsbit==64){
	  size = sizeof(double);
	}else{
	  size = sizeof(uint16);
	}

	data_t head=0x002a4949;
	writeLong(ofp,head);
	data_t off=8;
	writeLong(ofp,off);
	data_t p;
	for(data_t z=0;z<R->sz;z++){
		tag_t ntags=20;
		writeShort(ofp,ntags);
		p=ofp.tellp();
		p+=IFD_SIZE*ntags+4;
		CTiffTag tag;
		tag.id=tag.StringToID("IMAGEWIDTH");
		tag.type=3;tag.count=1;tag.addr=R->sx;
		writeTag(ofp,tag); // image width
		tag.id=tag.StringToID("IMAGELENGTH");
		tag.type=3;tag.count=1;tag.addr=R->sy;
		writeTag(ofp,tag); // image length
		tag.id=tag.StringToID("BITSPERSAMPLE");
		tag.type=3;tag.count=3;tag.addr=p;
		writeTag(ofp,tag); // bits per sample
		p+=3*sizeof(tag_t);
		tag.id=tag.StringToID("COMPRESSION");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // compression
		tag.id=tag.StringToID("PHOTOMETRIC");
		tag.type=3;tag.count=1;tag.addr=2;
		writeTag(ofp,tag); // photometric
		tag.id=tag.StringToID("STRIPOFFSETS");
		tag.type=4;tag.count=R->sy;tag.addr=p;
		writeTag(ofp,tag); // strip offsets
		p+=tag.count*4;
		tag.id=tag.StringToID("SAMPLESPERPIXEL");
		tag.type=3;tag.count=1;tag.addr=3;
		writeTag(ofp,tag); // samples per pixel
		tag.id=tag.StringToID("ROWSPERSTRIP");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // rows per strip
		tag.id=tag.StringToID("STRIPBYTECOUNTS");
		tag.type=4;tag.count=R->sy;tag.addr=p;
		writeTag(ofp,tag); // strip byte counts
		p+=tag.count*4;
		tag.id=tag.StringToID("PLANARCONFIG");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // planar config
		tag.id=tag.StringToID("ARTIST");
		tag.type=2;tag.count=sizeof(BRICT_STRING);tag.addr=p;
		writeTag(ofp,tag); // artist
		p+=sizeof(BRICT_STRING);
		tag.id=tag.StringToID("SAMPLEFORMAT");
		tag.type=3;tag.count=1;
		if(fSaveAsbit==8||fSaveAsbit==16) tag.addr=1;
		else tag.addr=3;
		writeTag(ofp,tag); // sampleformat
		tag.id=tag.StringToID("B_SIZEZ");
		tag.type=3;tag.count=1;tag.addr=R->sz;
		writeTag(ofp,tag); // size z
		tag.id=tag.StringToID("B_SIZET");
		tag.type=3;tag.count=1;tag.addr=1;
		writeTag(ofp,tag); // size z
		float ftmp;
		tag.id=tag.StringToID("B_PITCHX");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch x
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(R->px);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHY");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch y
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(R->py);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHZ");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch z
		ofp.seekp(-4,ios::cur);
		ftmp=static_cast<float>(R->pz);
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_PITCHT");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // pitch t
		ofp.seekp(-4,ios::cur);
		ftmp=1.0f;
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));
		tag.id=tag.StringToID("B_TIMENUMBER");
		tag.type=4;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // time #
		tag.id=tag.StringToID("B_ACTUALTIME");
		tag.type=11;tag.count=1;tag.addr=0;
		writeTag(ofp,tag); // real time
		ofp.seekp(-4,ios::cur);
		ftmp=0.0f;
		ofp.write(reinterpret_cast<char*>(&ftmp),sizeof(float));

		if(z!=R->sz-1){
			off=(data_t)ofp.tellp()+4+3*2+R->sy*8+sizeof(BRICT_STRING)+3*R->sy*R->sx*size;
		}
		else {
			off=0;
		}
		writeLong(ofp,off);
		tag_t bps=size<<3;
		for(data_t i=0;i<3;i++){ // bits per sample
			writeShort(ofp,bps);
		}
		for(data_t y=0;y<R->sy;y++){ //strip offsets
			writeLong(ofp,p);
			p+=R->sx*size*3;
		}
		off=R->sx*size*3;
		for(data_t y=0;y<R->sy;y++){ //strip byte counts
			writeLong(ofp,off);
		}
		ofp.write(BRICT_STRING,sizeof(BRICT_STRING)); //artist

		//image bits
		if(fSaveAsbit==8)
		{
			uint8* buf=new uint8[R->sx*3];
			double sw=abs(smax-smin);
			if(sw<1.0) sw=1.0;
			double rt=UCHAR_MAX/sw;
			uint8* pb;
			for(data_t y=0;y<R->sy;y++){
				pb=buf;
				for(data_t x=0;x<R->sx;x++){
					if     (R->img[z][y][x]<smin) *pb=0;
					else if(R->img[z][y][x]>smax) *pb=UCHAR_MAX;
					else *pb=static_cast<uint8>(R->img[z][y][x]*rt);
					pb++;
					if     (G->img[z][y][x]<smin) *pb=0;
					else if(G->img[z][y][x]>smax) *pb=UCHAR_MAX;
					else *pb=static_cast<uint8>(G->img[z][y][x]*rt);
					pb++;
					if     (B->img[z][y][x]<smin) *pb=0;
					else if(B->img[z][y][x]>smax) *pb=UCHAR_MAX;
					else *pb=static_cast<uint8>(B->img[z][y][x]*rt);
					pb++;
				}
				ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
			}
			delete[] buf;
		}else if(fSaveAsbit==64){
			double* buf=new double[R->sx*3];
			double* pb;
			for(data_t y=0;y<R->sy;y++){
				pb=buf;
				for(data_t x=0;x<R->sx;x++){
					*pb++=R->img[z][y][x];
					*pb++=G->img[z][y][x];
					*pb++=B->img[z][y][x];
				}
				ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
			}
			delete[] buf;
		}else{
		  uint16* buf=new uint16[R->sx*3];
			double sw=abs(smax-smin);
			if(sw<1.0) sw=1.0;
			double rt=USHRT_MAX/sw;
			uint16* pb;
			for(data_t y=0;y<R->sy;y++){
			  pb=buf;
			  for(data_t x=0;x<R->sx;x++){
			    if(R->img[z][y][x]<smin) *pb=0;
			    else if(R->img[z][y][x]>smax) *pb=USHRT_MAX;
			    else *pb=static_cast<uint16>(R->img[z][y][x]*rt);
			    pb++;
			    if(G->img[z][y][x]<smin) *pb=0;
			    else if(G->img[z][y][x]>smax) *pb=USHRT_MAX;
			    else *pb=static_cast<uint16>(G->img[z][y][x]*rt);
			    pb++;
			    if(B->img[z][y][x]<smin) *pb=0;
			    else if(B->img[z][y][x]>smax) *pb=USHRT_MAX;
			    else *pb=static_cast<uint16>(B->img[z][y][x]*rt);
			    pb++;
			  }
			  ofp.write(reinterpret_cast<char*>(buf),3*R->sx*size);
			}
			delete[] buf;
			
		}
	}
	return true;
}

// vim:set foldmethod=syntax:
