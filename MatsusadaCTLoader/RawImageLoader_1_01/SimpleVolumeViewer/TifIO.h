#pragma once
//#pragma warning ( disable : 4018 ) 

#include <string>
#include <fstream>
#include <deque>
#include <map>
#include <climits>
#include "SimpleImage.h"
#include "TiffTags.h"

#ifndef int8
#define int8 signed char
#endif
#ifndef int16
#define int16 signed short int
#endif
#ifndef int32
#define int32 signed int
#endif
#ifndef int64
#define int64 signed long long int
#endif
#ifndef uint8
#define uint8 unsigned char
#endif
#ifndef uint16
#define uint16 unsigned short int
#endif
#ifndef uint32
#define uint32 unsigned int
#endif
#ifndef uint64
#define uint64 unsigned long long int
#endif

#ifdef DEBUG
#include <iostream>
/// write msg & src-file & line-number to STDOUT
/// to enable this function, compile with "DEBUG" definition
#define debug_msg(msg) (std::cout << msg << " @ " << __FILE__ << ":" << __LINE__ << std::endl)
#else
#define debug_msg(msg)
#endif

///////////////////////////////////////////////////////////////////////////////
// definitions of static values
///////////////////////////////////////////////////////////////////////////////
static const char BRICT_STRING[]="Bio-Research Infrastructure Construction Team, VCAD System Research Program, RIKEN, Japan.\0";
static const uint16 SAMPLE_R=0;
static const uint16 SAMPLE_G=1;
static const uint16 SAMPLE_B=2;

///////////////////////////////////////////////////////////////////////////////
/// definition of function reverse bytes array.
/** @param buf pointer to memory buffer.
  * @param size size of one valiable.
  * @param num num of values in the buffer.
  */ //////////////////////////////////////////////////////////////////////////
inline void swapBytes(void* buf,size_t size,size_t num=1)
{
	if(buf==NULL) return;
	size_t buflen=size*num;
	unsigned char* s=static_cast<unsigned char*>(buf);
	unsigned char c;
	size_t p=0;
	while(p<buflen){
		for(size_t q=0;q<(size>>1);q++){
			c=*(s+p+q);
			*(s+p+q)=*(s+p+size-q-1);
			*(s+p+size-q-1)=c;
		}
		p+=size;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// definitions of class TifIO.
/** import pixels from tiff file to Image / Image3D class.
  * export pixels from Image / Image3D class to tiff file.
  * Image / Image3D class is in the "SimpleImage.h" file.
  */ //////////////////////////////////////////////////////////////////////////
class TifIO
{

	typedef unsigned short tag_t; /// 2-byte unsigned integer
	typedef unsigned int data_t;  /// 4-byte unsigned integer

	
protected:
	bool m_fBigEndian;                            //// is the file written in Big Endian?
	std::deque<std::fstream::pos_type> m_Offsets; //// offsets of pages
	tag_t m_bps;                                  //// bits per sample of the tif file
public:
	std::string filename; /// file path to read or write data
	std::map<tag_t,CTiffTag> m_ttag; /// tif tag list
	static const data_t IFD_SIZE=12; /// size of a tag

	protected:
		//// read TIFF Sign and check endian
		inline bool readHeader(std::ifstream& ifp, data_t& head)
		{
			ifp.read(reinterpret_cast<char*>(&head),sizeof(data_t));
			if     (head==0x002a4949) m_fBigEndian=false;
			else if(head==0x2a004d4d) m_fBigEndian=true;
			else return false;
			return true;
		}
		/// read 4-byte data from ifp
		void readLong(std::ifstream& ifp, data_t& off)
		{
			ifp.read(reinterpret_cast<char*>(&off),sizeof(data_t));
			if(m_fBigEndian) swapBytes(&off,sizeof(data_t));
		}

		/// read 2-byte data from ifp
		void readShort(std::ifstream& ifp, tag_t& tags){
			ifp.read(reinterpret_cast<char*>(&tags),sizeof(tag_t));
			if(m_fBigEndian) swapBytes(&tags,sizeof(tag_t));
		}
		/// write 4-byte data to ofp
		void writeLong(std::ofstream& ofp,data_t& val){
			ofp.write(reinterpret_cast<char*>(&val),sizeof(data_t));
		}
		/// write 2-byte data to ofp
		void writeShort(std::ofstream& ofp,tag_t& val){
			ofp.write(reinterpret_cast<char*>(&val),sizeof(tag_t));
		}
		/// write CTiffTag 12-byte data to ofp
		void writeTag(std::ofstream& ofp,CTiffTag& tag){
			writeShort(ofp,tag.id);
			writeShort(ofp,tag.type);
			writeLong(ofp,tag.count);
			writeLong(ofp,tag.addr);
		}
		/// read 12-byte data to CTiffTag from ifp
		void readIFD(std::ifstream& ifp);
		/// initialize(clear) this class
		void init(){
			filename="";
			m_ttag.clear();
		}
		/// read a scanline from ifp to buf
		bool readLine(std::ifstream& ifp,double* buf);
		/// write a scanline from buf to ofp
		bool writeLine(std::fstream& ofp,double* buf,data_t len,data_t size,tag_t fmt);
	public:
		TifIO();
		TifIO(const char* in);
		TifIO(const TifIO& rhs);
		~TifIO();

		/// load/saveに使用するファイル名(相対/フル)パスを設定する.
		void setFilename(const char* const in);

		/// filenameで指定したtifファイルのpage数を返す.
		/** 1以上の自然数. 1なら2D画像.
		  * TIFFでない、壊れている等エラーの場合は0を返す.
		  */
		int getPageNumber();

		/// filenameで指定したtifファイルがカラー画像ならtrueを、グレースケールならfalseを返す.
		bool isColor(int zID=0);

		/// pathに指定したtifファイルがTifIOクラスで読み出し可能ならtrueを返す.
		/** 引数pathを省略した場合filenameメンバの値を使用する.
		  */
		bool isApplicable(std::string path="");

		// --------------------------------------------------------------------
		// 2D Image IO
		// --------------------------------------------------------------------
		/// filenameで指定したtifファイルをSimpleImage.hのImageクラスに読み込む.
		/** 成功はtrue,失敗(ファイルなし、カラーの場合等)はfalseを返す.
		  * ピッチ(px,py),サイズ(sx,sy),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		//bool getImage(Image* img);

		/// filenameで指定したtifファイルのカラー画像をSimpleImage.hのImageクラスにR,G,Bごとに読み込む.
		/** 成功はtrue,失敗(ファイルなし、カラー画像でない等)はfalseを返す.
		  * ピッチ(px,py),サイズ(sx,sy),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		//bool getColorImage(Image *R,Image *G,Image *B);

		/// filenameで指定したマルチtifファイルのzIDページの画像をSimpleImage.hのImageクラスに読み込む.
		/** 成功はtrue,失敗(ファイルなし、カラーの場合等)はfalseを返す.
		  * ピッチ(px,py),サイズ(sx,sy),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		bool getImage(Image* image,int zID=0);

		/// filenameで指定したマルチtifファイルのzIDページの画像をSimpleImage.hのImageクラスにR,G,Bごとに読み込む.
		/** 成功はtrue,失敗(ファイルなし、カラー画像でない等)はfalseを返す.
		  * ピッチ(px,py),サイズ(sx,sy),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		
		bool getColorImage(Image *R,Image *G,Image *B,int zID=0);

		/// SimpleImage.hのImageクラスinの中身をfilenameで指定したtifファイルとして保存する.
		/** 成功はtrue,失敗(ファイルopen失敗等)はfalseを返す.
		  * fSaveAsbit=8の場合、smax,sminによって輝度値を0-255に割り当てBYTE(unsigned char)配列として保存する。
		  * fSaveAsbit=16の場合、smax,sminによって輝度値を0-2^16-1に割り当てunsigned short配列として保存する。
		  * fSaveAsbit=64の場合/指定無しの場合はdouble配列をそのまま保存する。
		  */
		bool saveImage(Image *in,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.hのImageクラスR,G,Bの中身をfilenameで指定したカラーtifファイルとして保存する.
		/** 成功はtrue,失敗(ファイルopen失敗等)はfalseを返す.
		   * fSaveAsbit=8の場合、smax,sminによって輝度値を0-255に割り当てBYTE(unsigned char)配列として保存する。
		  * fSaveAsbit=16の場合/指定無しの場合はsmax,sminによって輝度値を0-2^16-1に割り当てunsigned short配列として保存する。
		  * fSaveAsbit=64の場合はdouble配列をそのまま保存する。
		  */
		bool saveColorImage(Image *R,Image *G,Image *B,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.hのImageクラスinの中身をfilenameで指定したマルチtifファイルのzIDページ目として入れ替えて保存する.
		/** 成功はtrue,失敗(ファイルopen失敗、サイズ違い等)はfalseを返す.
		  */
		bool saveImage(Image *in,int zID);

		/// SimpleImage.hのImageクラスR,G,Bの中身をfilenameで指定したカラーマルチtifファイルのzIDページ目として入れ替えて保存する.
		/** 成功はtrue,失敗(ファイルopen失敗、サイズ違い等)はfalseを返す.
		  */
		bool saveColorImage(Image *R,Image *G,Image *B,int zID);

		// --------------------------------------------------------------------
		// 3D Image IO
		// --------------------------------------------------------------------
		/// filenameで指定したマルチtifファイルをSimpleImage.hのImage3Dクラスに読み込む.
		/** 失敗(ファイルなし、カラーの場合等)はfalseを返す.
		  * ピッチ(px,py,pz),サイズ(sx,sy,sz),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		bool getImage(Image3D* image);
		bool t_getImage(Image3D* image);
		//bool t_getImage(int &width, int &height, int &depth, double& *img);こっちの方がinterfaceとしてよさそうだけど・・・

		/// filenameで指定したマルチtifファイルのカラー画像をSimpleImage.hのImage3DクラスにR,G,Bごとに読み込む.
		/** 成功はtrue,失敗(ファイルなし、カラー画像でない等)はfalseを返す.
		  * ピッチ(px,py,pz),サイズ(sx,sy,sz),輝度imgを入れる.
		  * img配列のmemoryの確保は関数内で行う.
		  */
		bool getColorImage(Image3D *R,Image3D *G,Image3D *B);
		
		/// SimpleImage.hのImage3Dクラスinの中身をfilenameで指定したマルチtifファイルとして保存する.
		/** 成功はtrue,失敗(ファイルopen失敗等)はNULLを返す.
		  * fSaveAsbit=8の場合、smax,sminによって輝度値を0-255に割り当てBYTE(unsigned char)配列として保存する。
		  * fSaveAsbit=16の場合/指定無しの場合、smax,sminによって輝度値を0-2^16-1に割り当てunsigned short配列として保存する。
		  * fSaveAsbit=64の場合はdouble配列をそのまま保存する。
		  */
		bool saveImage(Image3D *in,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.hのImage3DクラスR,G,Bの中身をfilenameで指定したカラーマルチtifファイルとして保存する.
		/** 成功はtrue,失敗(ファイルopen失敗等)はNULLを返す.
		  * fSaveAsbit=8の場合、smax,sminによって輝度値を0-255に割り当てBYTE(unsigned char)配列として保存する。
		  * fSaveAsbit=16の場合/指定無しの場合、smax,sminによって輝度値を0-2^16-1に割り当てunsigned short配列として保存する。
		  * fSaveAsbit=64の場合はdouble配列をそのまま保存する。
		  */
		bool saveColorImage(Image3D *R,Image3D *G,Image3D *B,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);
};

