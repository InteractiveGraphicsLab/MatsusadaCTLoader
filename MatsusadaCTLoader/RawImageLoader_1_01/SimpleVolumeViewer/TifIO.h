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

		/// load/save�Ɏg�p����t�@�C����(����/�t��)�p�X��ݒ肷��.
		void setFilename(const char* const in);

		/// filename�Ŏw�肵��tif�t�@�C����page����Ԃ�.
		/** 1�ȏ�̎��R��. 1�Ȃ�2D�摜.
		  * TIFF�łȂ��A���Ă��铙�G���[�̏ꍇ��0��Ԃ�.
		  */
		int getPageNumber();

		/// filename�Ŏw�肵��tif�t�@�C�����J���[�摜�Ȃ�true���A�O���[�X�P�[���Ȃ�false��Ԃ�.
		bool isColor(int zID=0);

		/// path�Ɏw�肵��tif�t�@�C����TifIO�N���X�œǂݏo���\�Ȃ�true��Ԃ�.
		/** ����path���ȗ������ꍇfilename�����o�̒l���g�p����.
		  */
		bool isApplicable(std::string path="");

		// --------------------------------------------------------------------
		// 2D Image IO
		// --------------------------------------------------------------------
		/// filename�Ŏw�肵��tif�t�@�C����SimpleImage.h��Image�N���X�ɓǂݍ���.
		/** ������true,���s(�t�@�C���Ȃ��A�J���[�̏ꍇ��)��false��Ԃ�.
		  * �s�b�`(px,py),�T�C�Y(sx,sy),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		//bool getImage(Image* img);

		/// filename�Ŏw�肵��tif�t�@�C���̃J���[�摜��SimpleImage.h��Image�N���X��R,G,B���Ƃɓǂݍ���.
		/** ������true,���s(�t�@�C���Ȃ��A�J���[�摜�łȂ���)��false��Ԃ�.
		  * �s�b�`(px,py),�T�C�Y(sx,sy),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		//bool getColorImage(Image *R,Image *G,Image *B);

		/// filename�Ŏw�肵���}���`tif�t�@�C����zID�y�[�W�̉摜��SimpleImage.h��Image�N���X�ɓǂݍ���.
		/** ������true,���s(�t�@�C���Ȃ��A�J���[�̏ꍇ��)��false��Ԃ�.
		  * �s�b�`(px,py),�T�C�Y(sx,sy),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		bool getImage(Image* image,int zID=0);

		/// filename�Ŏw�肵���}���`tif�t�@�C����zID�y�[�W�̉摜��SimpleImage.h��Image�N���X��R,G,B���Ƃɓǂݍ���.
		/** ������true,���s(�t�@�C���Ȃ��A�J���[�摜�łȂ���)��false��Ԃ�.
		  * �s�b�`(px,py),�T�C�Y(sx,sy),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		
		bool getColorImage(Image *R,Image *G,Image *B,int zID=0);

		/// SimpleImage.h��Image�N���Xin�̒��g��filename�Ŏw�肵��tif�t�@�C���Ƃ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s��)��false��Ԃ�.
		  * fSaveAsbit=8�̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-255�Ɋ��蓖��BYTE(unsigned char)�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=16�̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-2^16-1�Ɋ��蓖��unsigned short�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=64�̏ꍇ/�w�薳���̏ꍇ��double�z������̂܂ܕۑ�����B
		  */
		bool saveImage(Image *in,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.h��Image�N���XR,G,B�̒��g��filename�Ŏw�肵���J���[tif�t�@�C���Ƃ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s��)��false��Ԃ�.
		   * fSaveAsbit=8�̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-255�Ɋ��蓖��BYTE(unsigned char)�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=16�̏ꍇ/�w�薳���̏ꍇ��smax,smin�ɂ���ċP�x�l��0-2^16-1�Ɋ��蓖��unsigned short�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=64�̏ꍇ��double�z������̂܂ܕۑ�����B
		  */
		bool saveColorImage(Image *R,Image *G,Image *B,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.h��Image�N���Xin�̒��g��filename�Ŏw�肵���}���`tif�t�@�C����zID�y�[�W�ڂƂ��ē���ւ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s�A�T�C�Y�Ⴂ��)��false��Ԃ�.
		  */
		bool saveImage(Image *in,int zID);

		/// SimpleImage.h��Image�N���XR,G,B�̒��g��filename�Ŏw�肵���J���[�}���`tif�t�@�C����zID�y�[�W�ڂƂ��ē���ւ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s�A�T�C�Y�Ⴂ��)��false��Ԃ�.
		  */
		bool saveColorImage(Image *R,Image *G,Image *B,int zID);

		// --------------------------------------------------------------------
		// 3D Image IO
		// --------------------------------------------------------------------
		/// filename�Ŏw�肵���}���`tif�t�@�C����SimpleImage.h��Image3D�N���X�ɓǂݍ���.
		/** ���s(�t�@�C���Ȃ��A�J���[�̏ꍇ��)��false��Ԃ�.
		  * �s�b�`(px,py,pz),�T�C�Y(sx,sy,sz),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		bool getImage(Image3D* image);
		bool t_getImage(Image3D* image);
		//bool t_getImage(int &width, int &height, int &depth, double& *img);�������̕���interface�Ƃ��Ă悳���������ǁE�E�E

		/// filename�Ŏw�肵���}���`tif�t�@�C���̃J���[�摜��SimpleImage.h��Image3D�N���X��R,G,B���Ƃɓǂݍ���.
		/** ������true,���s(�t�@�C���Ȃ��A�J���[�摜�łȂ���)��false��Ԃ�.
		  * �s�b�`(px,py,pz),�T�C�Y(sx,sy,sz),�P�ximg������.
		  * img�z���memory�̊m�ۂ͊֐����ōs��.
		  */
		bool getColorImage(Image3D *R,Image3D *G,Image3D *B);
		
		/// SimpleImage.h��Image3D�N���Xin�̒��g��filename�Ŏw�肵���}���`tif�t�@�C���Ƃ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s��)��NULL��Ԃ�.
		  * fSaveAsbit=8�̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-255�Ɋ��蓖��BYTE(unsigned char)�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=16�̏ꍇ/�w�薳���̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-2^16-1�Ɋ��蓖��unsigned short�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=64�̏ꍇ��double�z������̂܂ܕۑ�����B
		  */
		bool saveImage(Image3D *in,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);

		/// SimpleImage.h��Image3D�N���XR,G,B�̒��g��filename�Ŏw�肵���J���[�}���`tif�t�@�C���Ƃ��ĕۑ�����.
		/** ������true,���s(�t�@�C��open���s��)��NULL��Ԃ�.
		  * fSaveAsbit=8�̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-255�Ɋ��蓖��BYTE(unsigned char)�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=16�̏ꍇ/�w�薳���̏ꍇ�Asmax,smin�ɂ���ċP�x�l��0-2^16-1�Ɋ��蓖��unsigned short�z��Ƃ��ĕۑ�����B
		  * fSaveAsbit=64�̏ꍇ��double�z������̂܂ܕۑ�����B
		  */
		bool saveColorImage(Image3D *R,Image3D *G,Image3D *B,double smax=USHRT_MAX,double smin=0,int fSaveAsbit=16);
};

