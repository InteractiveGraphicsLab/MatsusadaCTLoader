#pragma once

#include <string>
#include <map>
#include <fstream>
#include <cstring>

// ----------------------------------------------------------------------
// definitions of string based functions
// ----------------------------------------------------------------------
/// change characters to capitals in a string.
inline void StringToLower(std::string& str){
	for(size_t i=0;i<str.length();i++){
		if(str[i]>='A'&&str[i]<='Z'){
			str[i]+='a'-'A';
		}
	}
}
/// change characters to lower-case in a string.
inline void StringToUpper(std::string& str){
	for(size_t i=0;i<str.length();i++){
		if(str[i]>='a'&&str[i]<='z'){
			str[i]-='a'-'A';
		}
	}
}
/// erase non-alphabetical characters
inline void chompString(std::string& str){
	std::string::size_type pre,post,n;
	pre=0;
	for(n=0;(str[n]<'A'||str[n]>'z');n++){
		pre=n+1;
	}
	for(n=0;n<str.length();n++){
		if(str[n]>='A'&&str[n]<='z'){
			post=n;
		}
	}
	str.erase(++post);
	str.erase(0,pre);
}

class CTiffTag
{
	typedef unsigned short tag_t ;
	typedef unsigned int   data_t;

	protected:
		/// hash: Tag-ID => Tag-Name
		static std::map<tag_t,std::string> IDTable;
		static void InitIDTable(void) {
			IDTable.clear();
			IDTable.insert(std::pair<tag_t,std::string>( 0, "UNKNOWN"));
			IDTable.insert(std::pair<tag_t,std::string>( 254, "SUBFILETYPE"));
			IDTable.insert(std::pair<tag_t,std::string>( 255, "OSUBFILETYPE"));
			IDTable.insert(std::pair<tag_t,std::string>( 256, "IMAGEWIDTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 257, "IMAGELENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 258, "BITSPERSAMPLE"));
			IDTable.insert(std::pair<tag_t,std::string>( 259, "COMPRESSION"));
			IDTable.insert(std::pair<tag_t,std::string>( 262, "PHOTOMETRIC"));
			IDTable.insert(std::pair<tag_t,std::string>( 263, "THRESHHOLDING"));
			IDTable.insert(std::pair<tag_t,std::string>( 264, "CELLWIDTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 265, "CELLLENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 266, "FILLORDER"));
			IDTable.insert(std::pair<tag_t,std::string>( 269, "DOCUMENTNAME"));
			IDTable.insert(std::pair<tag_t,std::string>( 270, "IMAGEDESCRIPTION"));
			IDTable.insert(std::pair<tag_t,std::string>( 271, "MAKE"));
			IDTable.insert(std::pair<tag_t,std::string>( 272, "MODEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 273, "STRIPOFFSETS"));
			IDTable.insert(std::pair<tag_t,std::string>( 274, "ORIENTATION"));
			IDTable.insert(std::pair<tag_t,std::string>( 277, "SAMPLESPERPIXEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 278, "ROWSPERSTRIP"));
			IDTable.insert(std::pair<tag_t,std::string>( 279, "STRIPBYTECOUNTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 280, "MINSAMPLEVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 281, "MAXSAMPLEVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 282, "XRESOLUTION"));
			IDTable.insert(std::pair<tag_t,std::string>( 283, "YRESOLUTION"));
			IDTable.insert(std::pair<tag_t,std::string>( 284, "PLANARCONFIG"));
			IDTable.insert(std::pair<tag_t,std::string>( 285, "PAGENAME"));
			IDTable.insert(std::pair<tag_t,std::string>( 286, "XPOSITION"));
			IDTable.insert(std::pair<tag_t,std::string>( 287, "YPOSITION"));
			IDTable.insert(std::pair<tag_t,std::string>( 288, "FREEOFFSETS"));
			IDTable.insert(std::pair<tag_t,std::string>( 289, "FREEBYTECOUNTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 290, "GRAYRESPONSEUNIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 291, "GRAYRESPONSECURVE"));
			IDTable.insert(std::pair<tag_t,std::string>( 292, "GROUP3OPTIONS"));
			IDTable.insert(std::pair<tag_t,std::string>( 292, "T4OPTIONS"));
			IDTable.insert(std::pair<tag_t,std::string>( 293, "GROUP4OPTIONS"));
			IDTable.insert(std::pair<tag_t,std::string>( 293, "T6OPTIONS"));
			IDTable.insert(std::pair<tag_t,std::string>( 296, "RESOLUTIONUNIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 297, "PAGENUMBER"));
			IDTable.insert(std::pair<tag_t,std::string>( 300, "COLORRESPONSEUNIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 301, "TRANSFERFUNCTION"));
			IDTable.insert(std::pair<tag_t,std::string>( 305, "SOFTWARE"));
			IDTable.insert(std::pair<tag_t,std::string>( 306, "DATETIME"));
			IDTable.insert(std::pair<tag_t,std::string>( 315, "ARTIST"));
			IDTable.insert(std::pair<tag_t,std::string>( 316, "HOSTCOMPUTER"));
			IDTable.insert(std::pair<tag_t,std::string>( 317, "PREDICTOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 318, "WHITEPOINT"));
			IDTable.insert(std::pair<tag_t,std::string>( 319, "PRIMARYCHROMATICITIES"));
			IDTable.insert(std::pair<tag_t,std::string>( 320, "COLORMAP"));
			IDTable.insert(std::pair<tag_t,std::string>( 321, "HALFTONEHINTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 322, "TILEWIDTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 323, "TILELENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 324, "TILEOFFSETS"));
			IDTable.insert(std::pair<tag_t,std::string>( 325, "TILEBYTECOUNTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 326, "BADFAXLINES"));
			IDTable.insert(std::pair<tag_t,std::string>( 327, "CLEANFAXDATA"));
			IDTable.insert(std::pair<tag_t,std::string>( 328, "CONSECUTIVEBADFAXLINES"));
			IDTable.insert(std::pair<tag_t,std::string>( 330, "SUBIFD"));
			IDTable.insert(std::pair<tag_t,std::string>( 332, "INKSET"));
			IDTable.insert(std::pair<tag_t,std::string>( 333, "INKNAMES"));
			IDTable.insert(std::pair<tag_t,std::string>( 334, "NUMBEROFINKS"));
			IDTable.insert(std::pair<tag_t,std::string>( 336, "DOTRANGE"));
			IDTable.insert(std::pair<tag_t,std::string>( 337, "TARGETPRINTER"));
			IDTable.insert(std::pair<tag_t,std::string>( 338, "EXTRASAMPLES"));
			IDTable.insert(std::pair<tag_t,std::string>( 339, "SAMPLEFORMAT"));
			IDTable.insert(std::pair<tag_t,std::string>( 340, "SMINSAMPLEVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 341, "SMAXSAMPLEVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 343, "CLIPPATH"));
			IDTable.insert(std::pair<tag_t,std::string>( 344, "XCLIPPATHUNITS"));
			IDTable.insert(std::pair<tag_t,std::string>( 345, "YCLIPPATHUNITS"));
			IDTable.insert(std::pair<tag_t,std::string>( 346, "INDEXED"));
			IDTable.insert(std::pair<tag_t,std::string>( 347, "JPEGTABLES"));
			IDTable.insert(std::pair<tag_t,std::string>( 351, "OPIPROXY"));
			IDTable.insert(std::pair<tag_t,std::string>( 512, "JPEGPROC"));
			IDTable.insert(std::pair<tag_t,std::string>( 513, "JPEGIFOFFSET"));
			IDTable.insert(std::pair<tag_t,std::string>( 514, "JPEGIFBYTECOUNT"));
			IDTable.insert(std::pair<tag_t,std::string>( 515, "JPEGRESTARTINTERVAL"));
			IDTable.insert(std::pair<tag_t,std::string>( 517, "JPEGLOSSLESSPREDICTORS"));
			IDTable.insert(std::pair<tag_t,std::string>( 518, "JPEGPOINTTRANSFORM"));
			IDTable.insert(std::pair<tag_t,std::string>( 519, "JPEGQTABLES"));
			IDTable.insert(std::pair<tag_t,std::string>( 520, "JPEGDCTABLES"));
			IDTable.insert(std::pair<tag_t,std::string>( 521, "JPEGACTABLES"));
			IDTable.insert(std::pair<tag_t,std::string>( 529, "YCBCRCOEFFICIENTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 530, "YCBCRSUBSAMPLING"));
			IDTable.insert(std::pair<tag_t,std::string>( 531, "YCBCRPOSITIONING"));
			IDTable.insert(std::pair<tag_t,std::string>( 532, "REFERENCEBLACKWHITE"));
			IDTable.insert(std::pair<tag_t,std::string>( 700, "XMLPACKET"));
			IDTable.insert(std::pair<tag_t,std::string>( 32781, "OPIIMAGEID"));
			IDTable.insert(std::pair<tag_t,std::string>( 32953, "REFPTS"));
			IDTable.insert(std::pair<tag_t,std::string>( 32954, "REGIONTACKPOINT"));
			IDTable.insert(std::pair<tag_t,std::string>( 32955, "REGIONWARPCORNERS"));
			IDTable.insert(std::pair<tag_t,std::string>( 32956, "REGIONAFFINE"));
			IDTable.insert(std::pair<tag_t,std::string>( 32995, "MATTEING"));
			IDTable.insert(std::pair<tag_t,std::string>( 32996, "DATATYPE"));
			IDTable.insert(std::pair<tag_t,std::string>( 32997, "IMAGEDEPTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 32998, "TILEDEPTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 33300, "PIXAR_IMAGEFULLWIDTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 33301, "PIXAR_IMAGEFULLLENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 33302, "PIXAR_TEXTUREFORMAT"));
			IDTable.insert(std::pair<tag_t,std::string>( 33303, "PIXAR_WRAPMODES"));
			IDTable.insert(std::pair<tag_t,std::string>( 33304, "PIXAR_FOVCOT"));
			IDTable.insert(std::pair<tag_t,std::string>( 33305, "PIXAR_MATRIX_WORLDTOSCREEN"));
			IDTable.insert(std::pair<tag_t,std::string>( 33306, "PIXAR_MATRIX_WORLDTOCAMERA"));
			IDTable.insert(std::pair<tag_t,std::string>( 33405, "WRITERSERIALNUMBER"));
			IDTable.insert(std::pair<tag_t,std::string>( 33432, "COPYRIGHT"));
			IDTable.insert(std::pair<tag_t,std::string>( 33723, "RICHTIFFIPTC"));
			IDTable.insert(std::pair<tag_t,std::string>( 34016, "IT8SITE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34017, "IT8COLORSEQUENCE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34018, "IT8HEADER"));
			IDTable.insert(std::pair<tag_t,std::string>( 34019, "IT8RASTERPADDING"));
			IDTable.insert(std::pair<tag_t,std::string>( 34020, "IT8BITSPERRUNLENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 34021, "IT8BITSPEREXTENDEDRUNLENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 34022, "IT8COLORTABLE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34023, "IT8IMAGECOLORINDICATOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 34024, "IT8BKGCOLORINDICATOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 34025, "IT8IMAGECOLORVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34026, "IT8BKGCOLORVALUE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34027, "IT8PIXELINTENSITYRANGE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34028, "IT8TRANSPARENCYINDICATOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 34029, "IT8COLORCHARACTERIZATION"));
			IDTable.insert(std::pair<tag_t,std::string>( 34030, "IT8HCUSAGE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34031, "IT8TRAPINDICATOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 34032, "IT8CMYKEQUIVALENT"));
			IDTable.insert(std::pair<tag_t,std::string>( 34232, "FRAMECOUNT"));
			IDTable.insert(std::pair<tag_t,std::string>( 34377, "PHOTOSHOP"));
			IDTable.insert(std::pair<tag_t,std::string>( 34665, "EXIFIFD"));
			IDTable.insert(std::pair<tag_t,std::string>( 34675, "ICCPROFILE"));
			IDTable.insert(std::pair<tag_t,std::string>( 34750, "JBIGOPTIONS"));
			IDTable.insert(std::pair<tag_t,std::string>( 34853, "GPSIFD"));
			IDTable.insert(std::pair<tag_t,std::string>( 34908, "FAXRECVPARAMS"));
			IDTable.insert(std::pair<tag_t,std::string>( 34909, "FAXSUBADDRESS"));
			IDTable.insert(std::pair<tag_t,std::string>( 34910, "FAXRECVTIME"));
			IDTable.insert(std::pair<tag_t,std::string>( 34911, "FAXDCS"));
			IDTable.insert(std::pair<tag_t,std::string>( 37439, "STONITS"));
			IDTable.insert(std::pair<tag_t,std::string>( 34929, "FEDEX_EDR"));
			IDTable.insert(std::pair<tag_t,std::string>( 40965, "INTEROPERABILITYIFD"));
			IDTable.insert(std::pair<tag_t,std::string>( 50706, "DNGVERSION"));
			IDTable.insert(std::pair<tag_t,std::string>( 50707, "DNGBACKWARDVERSION"));
			IDTable.insert(std::pair<tag_t,std::string>( 50708, "UNIQUECAMERAMODEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 50709, "LOCALIZEDCAMERAMODEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 50710, "CFAPLANECOLOR"));
			IDTable.insert(std::pair<tag_t,std::string>( 50711, "CFALAYOUT"));
			IDTable.insert(std::pair<tag_t,std::string>( 50712, "LINEARIZATIONTABLE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50713, "BLACKLEVELREPEATDIM"));
			IDTable.insert(std::pair<tag_t,std::string>( 50714, "BLACKLEVEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 50715, "BLACKLEVELDELTAH"));
			IDTable.insert(std::pair<tag_t,std::string>( 50716, "BLACKLEVELDELTAV"));
			IDTable.insert(std::pair<tag_t,std::string>( 50717, "WHITELEVEL"));
			IDTable.insert(std::pair<tag_t,std::string>( 50718, "DEFAULTSCALE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50719, "DEFAULTCROPORIGIN"));
			IDTable.insert(std::pair<tag_t,std::string>( 50720, "DEFAULTCROPSIZE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50721, "COLORMATRIX1"));
			IDTable.insert(std::pair<tag_t,std::string>( 50722, "COLORMATRIX2"));
			IDTable.insert(std::pair<tag_t,std::string>( 50723, "CAMERACALIBRATION1"));
			IDTable.insert(std::pair<tag_t,std::string>( 50724, "CAMERACALIBRATION2"));
			IDTable.insert(std::pair<tag_t,std::string>( 50725, "REDUCTIONMATRIX1"));
			IDTable.insert(std::pair<tag_t,std::string>( 50726, "REDUCTIONMATRIX2"));
			IDTable.insert(std::pair<tag_t,std::string>( 50727, "ANALOGBALANCE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50728, "ASSHOTNEUTRAL"));
			IDTable.insert(std::pair<tag_t,std::string>( 50729, "ASSHOTWHITEXY"));
			IDTable.insert(std::pair<tag_t,std::string>( 50730, "BASELINEEXPOSURE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50731, "BASELINENOISE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50732, "BASELINESHARPNESS"));
			IDTable.insert(std::pair<tag_t,std::string>( 50733, "BAYERGREENSPLIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 50734, "LINEARRESPONSELIMIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 50735, "CAMERASERIALNUMBER"));
			IDTable.insert(std::pair<tag_t,std::string>( 50736, "LENSINFO"));
			IDTable.insert(std::pair<tag_t,std::string>( 50737, "CHROMABLURRADIUS"));
			IDTable.insert(std::pair<tag_t,std::string>( 50738, "ANTIALIASSTRENGTH"));
			IDTable.insert(std::pair<tag_t,std::string>( 50739, "SHADOWSCALE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50740, "DNGPRIVATEDATA"));
			IDTable.insert(std::pair<tag_t,std::string>( 50741, "MAKERNOTESAFETY"));
			IDTable.insert(std::pair<tag_t,std::string>( 50778, "CALIBRATIONILLUMINANT1"));
			IDTable.insert(std::pair<tag_t,std::string>( 50779, "CALIBRATIONILLUMINANT2"));
			IDTable.insert(std::pair<tag_t,std::string>( 50780, "BESTQUALITYSCALE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50781, "RAWDATAUNIQUEID"));
			IDTable.insert(std::pair<tag_t,std::string>( 50827, "ORIGINALRAWFILENAME"));
			IDTable.insert(std::pair<tag_t,std::string>( 50828, "ORIGINALRAWFILEDATA"));
			IDTable.insert(std::pair<tag_t,std::string>( 50829, "ACTIVEAREA"));
			IDTable.insert(std::pair<tag_t,std::string>( 50830, "MASKEDAREAS"));
			IDTable.insert(std::pair<tag_t,std::string>( 50831, "ASSHOTICCPROFILE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50832, "ASSHOTPREPROFILEMATRIX"));
			IDTable.insert(std::pair<tag_t,std::string>( 50833, "CURRENTICCPROFILE"));
			IDTable.insert(std::pair<tag_t,std::string>( 50834, "CURRENTPREPROFILEMATRIX"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65535, "DCSHUESHIFTVALUES"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65536, "FAXMODE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65537, "JPEGQUALITY"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65538, "JPEGCOLORMODE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65539, "JPEGTABLESMODE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65540, "FAXFILLFUNC"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65549, "PIXARLOGDATAFMT"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65550, "DCSIMAGERTYPE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65551, "DCSINTERPMODE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65552, "DCSBALANCEARRAY"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65553, "DCSCORRECTMATRIX"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65554, "DCSGAMMA"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65555, "DCSTOESHOULDERPTS"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65556, "DCSCALIBRATIONFD"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65557, "ZIPQUALITY"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65558, "PIXARLOGQUALITY"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65559, "DCSCLIPRECTANGLE"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65560, "SGILOGDATAFMT"));
			//	IDTable.insert(std::pair<tag_t,std::string>( 65561, "SGILOGENCODE"));
			//brict original tag
			IDTable.insert(std::pair<tag_t,std::string>( 65003, "B_SIZEZ"));
			IDTable.insert(std::pair<tag_t,std::string>( 65004, "B_SIZET"));
			IDTable.insert(std::pair<tag_t,std::string>( 65005, "B_ZNUMBER"));
			IDTable.insert(std::pair<tag_t,std::string>( 65006, "B_PITCHX"));
			IDTable.insert(std::pair<tag_t,std::string>( 65007, "B_PITCHY"));
			IDTable.insert(std::pair<tag_t,std::string>( 65008, "B_PITCHZ"));
			IDTable.insert(std::pair<tag_t,std::string>( 65009, "B_PITCHUNIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 65012, "B_PITCHT"));
			IDTable.insert(std::pair<tag_t,std::string>( 65013, "B_TIMENUMBER"));
			IDTable.insert(std::pair<tag_t,std::string>( 65014, "B_ACTUALTIME"));
			IDTable.insert(std::pair<tag_t,std::string>( 65015, "B_TIMEUNIT"));
			IDTable.insert(std::pair<tag_t,std::string>( 65021, "B_ORIGINX"));
			IDTable.insert(std::pair<tag_t,std::string>( 65022, "B_ORIGINY"));
			IDTable.insert(std::pair<tag_t,std::string>( 65023, "B_ORIGINZ"));
			IDTable.insert(std::pair<tag_t,std::string>( 65024, "B_ORIGINT"));
			IDTable.insert(std::pair<tag_t,std::string>( 65030, "B_DATECREATED"));
		}

	public:

		tag_t  id;
		tag_t  type;
		data_t count;
		data_t addr;
		char*  data;

		CTiffTag():id(0),type(0),count(0),addr(0),data(NULL){}
		CTiffTag(const CTiffTag& rhs){
			id=rhs.id;
			type=rhs.type;
			count=rhs.count;
			addr=rhs.addr;
			data=new char[getSize(type)*count];
			memcpy(data,rhs.data,getSize(type)*count);
		}
		~CTiffTag(){
			delete data;
		}
		bool readTag(std::ifstream& ifp);
		bool readTag(const std::string& filename, const std::ifstream::pos_type& offset);
		CTiffTag& operator=(const CTiffTag& rhs){
			if(this==&rhs) return *this;
			id=rhs.id;
			type=rhs.type;
			count=rhs.count;
			addr=rhs.addr;
			if(data!=NULL){
				delete[] data;
				data=NULL;
			}
			if(rhs.data!=NULL){
				data=new char[getSize(type)*count];
				memcpy(data,rhs.data,getSize(type)*count);
			}
			return *this;
		}
		static const std::string& IDToString(const tag_t& id){
			if(CTiffTag::IDTable.empty()) InitIDTable();
			std::map<tag_t,std::string>::iterator p;
			p=CTiffTag::IDTable.find(id);
			if(p==CTiffTag::IDTable.end()){
				return CTiffTag::IDTable.begin()->second;
			}
			else {
				return p->second;
			}
		}
		static tag_t StringToID(const std::string& name){
			if(IDTable.empty()) InitIDTable();
			std::string str=name;
			chompString(str);
			StringToUpper(str);
			std::map<tag_t,std::string>::iterator p;
			for(p=IDTable.begin();p!=IDTable.end();p++){
				if(p->second==str){
					return p->first;
				}
			}
			return IDTable.begin()->first;
		}
		/// returns bytes of one value.
		static tag_t getSize(tag_t type){
			switch(type){
				case 0: //TIFF_NOTYPE	= 0,	/* placeholder */
					return 0;
				case 1: //TIFF_BYTE	= 1,	/* 8-bit unsigned integer */
				case 2: //TIFF_ASCII	= 2,	/* 8-bit bytes w/ last byte null */
				case 6: //TIFF_SBYTE	= 6,	/* !8-bit signed integer */
				case 7: //TIFF_UNDEFINED	= 7,	/* !8-bit untyped data */
					return 1;
				case 3: //TIFF_SHORT	= 3,	/* 16-bit unsigned integer */
				case 8: //TIFF_SSHORT	= 8,	/* !16-bit signed integer */
					return 2;
				case 4: //TIFF_LONG	= 4,	/* 32-bit unsigned integer */
				case 9: //TIFF_SLONG	= 9,	/* !32-bit signed integer */
				case 11: //TIFF_FLOAT	= 11,	/* !32-bit IEEE floating point */
				case 13: //TIFF_IFD	= 13	/* %32-bit unsigned integer (offset) */
					return 4;
				case 5: //TIFF_RATIONAL	= 5,	/* 64-bit unsigned fraction */
				case 10: //TIFF_SRATIONAL	= 10,	/* !64-bit signed fraction */
				case 12: //TIFF_DOUBLE	= 12,	/* !64-bit IEEE floating point */
					return 8;
				default:
					return 0;
			}
		}
		/// returns if the value is signed?.
		bool isSigned(tag_t type){
			return (type>=6&&type<=12);
		}
		/// returns if the value is floating?.
		bool isFloating(tag_t type){
			return (type==11||type==12);
		}
};

//std::map<unsigned short,std::string> CTiffTag::IDTable;

inline bool operator<(const CTiffTag& lhs,const CTiffTag& rhs)
{
	return lhs.id<rhs.id;
}

