#include "StdAfx.h"
#include "VolumeFileManager.h"
#include "SimpleVolumeViewerDlg.h"

#include "ImgLoadParamDlg.h"

#include <vector>
#include <algorithm>
#include <string>
#include "SimpleImage.h"

#pragma warning (disable:4786)
#pragma warning (disable:4996)

using namespace std;



VolumeFileManager::~VolumeFileManager(void){
	for( int i=0; i < m_dirs.size(); ++i) delete m_dirs[i];
}


VolumeFileManager::VolumeFileManager(void)
{
	m_minZpos =  DBL_MAX;
	m_maxZpos = -DBL_MAX;
	m_W = m_H = 0;
	m_pitch   = 0;
	m_minVal  = 0   ; 
	m_maxVal  = 1000; //レンダリング時に更新
	m_dirs.clear();
}








/*--------------------------------------------------------------------
//load raw image data 
size     : width * height
byteSize : 1 pixel 2 byte (signed short) 
imgVal   : pixel value (loaded from file) *already allocated 
--------------------------------------------------------------------*/
static inline bool t_readRawImageOnlyWH( const char *fname, int &W, int &H )
{
	//ファイルサイズ取得
	FILE *fp = fopen( fname, "rb");
	if(fp == 0 ) return false;

	int fsize = 0;
	for (; getc(fp) != EOF; fsize++) {}

	fclose(fp);

	W = H = (int)sqrt( fsize/2.0 ) ;
	fprintf( stderr, "aaaaaa %s %d %d %d\n", fname, fsize,W,H );

	return true;
}


static bool t_readRawImage( const char *fname, const int W, const int H, short* img)
{
	FILE *fp = fopen( fname, "rb"); 
  if(fp == 0 ) return false;
	
  int num = (int)fread( img, sizeof( short), W*H, fp);
	fclose(fp);
	
  return num == W*H;	
}


static void t_getAllRawFilesInDir( const char *dirpath, vector<string> &files)
{

	// ファイル探索
	// FindFirstFile(ファイル名, &fd);
	string ss = string(dirpath) + string("\\*.*"); //ワイルドカード
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFileA( ss.c_str(), &fd);


	// 検索失敗
	if(hFind == INVALID_HANDLE_VALUE)
  { 
		fprintf( stderr, "error %s\n", dirpath );
		return;
	}

  // ファイル名をリストに格納するためのループ
  do{
    // フォルダは除く
    if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
    {
      string targetFile = string(fd.cFileName);
      if( strcmp( targetFile.substr( targetFile.length() - 4, 4).c_str(), ".raw" ) == 0 || 
          strcmp( targetFile.substr( targetFile.length() - 4, 4).c_str(), ".RAW" ) == 0 )
      {
        files.push_back( string(fd.cFileName) ); //ファイル名をリストに格納
      }
    }
  }while( FindNextFile(hFind, &fd)); //次のファイルを探索
		
	// hFindのクローズ
	FindClose(hFind); 
}




//2DParam.txt を探してよみ, z値を取得する
/*
//2DParam.txtの内容      
"Do Not Edit This line", -1, 1, 0, 4095, 62, 12.4, 1, 0.00, 168.78, 165.69, 0.00, 165.69, 84.9, 0.0, -471.5, 0, 2, 3, 2, 0, 26.1111, 4.7418
Resolution Full=0.0383mm/Pixel
Resolution Half=0.0766mm/Pixel
X=0.00mm
Y=168.78mm
Z=165.69mm
Voltage=62kV
Power=12.4W
ImgAverage=4
FullScan
OffsetScan On
600View
Time=20m01s 
*/
static bool t_searchAndRead_2DParamtxt(const string fileDir, double &pitch, double &zPos  )
{
	string fileName1 = fileDir + "\\" + string("2DParam.txt");
	FILE *fp = fopen( fileName1.c_str(), "r");

	if( fp == 0 ){
		string::size_type index = fileDir.find_last_of( "\\" );
		if( index == string::npos ) return false;

		string fileName2 = fileDir.substr(0, index+1) + string("2DParam.txt") ;
		fp = fopen( fileName2.c_str(), "r");

		fprintf( stderr, "file name 2 = %s\n", fileName2.c_str() );

		if( fp == 0 ) return false;
	}

	bool bPitchRead = false;
	bool bZposRead  = false;

	//次にpitch情報を読む
	while( true ) 
	{
		char buf[1024];
		if( EOF == fscanf( fp, "%s", buf) ) break;
		string strBuf = string( buf );

		if( strcmp( strBuf.substr(0, 5).c_str(), "Half=" ) == 0 ) 
		{
			string::size_type index1 = strBuf.find_first_of( "=" );
			string::size_type index2 = strBuf.find_first_of( "m" );
			if( index1 == string::npos || index2 == string::npos || index2-index1 <= 0) { fclose(fp); return false; }

			string value = strBuf.substr( index1+1, index2-index1-1 );
			pitch = atof( value.c_str() );
			//fprintf( stderr, "%s  %s  %.10f\n", strBuf.c_str(), value.c_str(),pitch );

			bPitchRead = true;
		}

		//Z=195.64mm
		if( strcmp( strBuf.substr(0, 2).c_str(), "Z=" ) == 0 ) 
		{
			string::size_type index1 = strBuf.find_first_of( "=" );
			string::size_type index2 = strBuf.find_first_of( "m" );
			if( index1 == string::npos || index2 == string::npos || index2-index1 <= 0) { fclose(fp); return false; }

			string value = strBuf.substr( index1+1, index2-index1-1 );
			zPos = atof( value.c_str() );
			//fprintf( stderr, "%s  %s  %.10f\n", strBuf.c_str(), value.c_str(), zPos);

			bZposRead = true;
		}
		if( bPitchRead && bZposRead ) return true;
	}
	return false;
}




//
//  ひとつのフォルダについて 
//  z_piv_pos, pitch, 画像群 が読み込まれる
//   
//  このフォルダのファイルは，
//  z座標が， [z_min, z_max] をカバーする．
//  z_min = z_piv_pos - pitch[2] * f_num / 2 
//  z_max = z_piv_pos + pitch[2] * f_num / 2 
//
// また，zの座標から，スライス番号が計算できる
// slice_idx = z_idx = (int) ( (z_pos - z_min) / pitch[2] );
//
// zi番目のスライスの中心のz座標(z_pos)は
// z_pos = (zi + 0.5) * pitch[2] + z_min
// となる






class LessString {
public:
    bool operator()(const string& rsLeft, const string& rsRight) const {
		if(rsLeft.length() == rsRight.length()) return rsLeft < rsRight;
		else                                    return rsLeft.length() < rsRight.length();
    }
};


void VolumeFileManager::loadDirectries(vector<string> &dirPathes)
{
	for(int dirI = 0, s = (int)dirPathes.size(); dirI<s; ++dirI)
	{
		const string &dirPath = dirPathes[dirI];
		
		//param2D.txtの検索 / 読み込み
		double pitch, zPos;
		if( !t_searchAndRead_2DParamtxt( dirPath, pitch, zPos ) )
    { 
      fprintf( stderr, "error: 2Dparam.txtが見つからない　%s\n", dirPathes[dirI].c_str()); 
      continue;
    }
		if( m_dirs.empty() ) m_pitch = pitch;


		//.rawファイル全てを取得しソート
		vector<string> files;
		t_getAllRawFilesInDir( dirPathes[dirI].c_str(), files );
		sort( files.begin(), files.end(), LessString() );
		if( files.size() == 0 ) 
    {
      fprintf( stderr, "Rawファイルなし %s \n", dirPathes[dirI].c_str()); 
      continue; 
    }

		for( int k=0; k<(int)files.size();++k) files[k] = dirPath + "\\" + files[k];


		//rawファイルのサイズ取得
		if( m_dirs.size() == 0 ){
			t_readRawImageOnlyWH( files.front().c_str(), m_W, m_H );
		}


		//.rawファイルを読み込む
		m_dirs.push_back( new ImagesInDir() );
		ImagesInDir *ImgsDir = m_dirs.back();

		const int fNum = (int)files.size();
		ImgsDir->m_imgPathes = files;
		ImgsDir->m_zPosMin = zPos - m_pitch * fNum / 2.0 ;
		ImgsDir->m_zPosMax = zPos + m_pitch * fNum / 2.0 ;

		//後処理 
		m_minZpos = min( m_minZpos, ImgsDir->m_zPosMin );
		m_maxZpos = max( m_maxZpos, ImgsDir->m_zPosMax );
	}

	m_D = (int) ( ( m_maxZpos - m_minZpos) / m_pitch );

	fprintf( stderr, "pitch : %f\n", m_pitch);
	fprintf( stderr, "zMin max : %f %f\n", m_minZpos, m_maxZpos);
	fprintf( stderr, "%d %d %d\n", m_W, m_H, m_D );


	//更新をダイアログへ通知
	CSimpleVolumeViewerDlg::m_myself->t_newDirLoaded();
}








void VolumeFileManager::getSliceImg_Zth_slice( int z, short *trgt2DimgArray )
{
	double zPos = m_minZpos + (z+0.5) * m_pitch;
  
/*
  for( int i=0; i < 200; ++i )
  {
	  double zzz1 = m_minZpos + i * m_pitch + 0.001;
	  double zzz2 = m_minZpos + (i + 0.5) * m_pitch;
    fprintf( stderr, "%d %d %d %f %f\n", (int)( (zzz1 - m_dirs.front()->m_zPosMin ) / m_pitch ),
                                         (int)( (zzz2 - m_dirs.front()->m_zPosMin ) / m_pitch ), i, m_dirs.front()->m_zPosMin, m_minZpos);
  } 
*/

	//より境界からの値が遠いsliceを返す
	double maxDistFromEdge = -1 ;
	int    trgtDirI        = -1 ;
	int    trgtSliceI      = -1 ;

	for( int di = 0; di < (int)m_dirs.size(); ++di ) if( m_dirs[di]->m_zPosMin <= zPos && zPos <= m_dirs[di]->m_zPosMax )
	{
		double ofst = min( zPos - m_dirs[di]->m_zPosMin, m_dirs[di]->m_zPosMax - zPos );
		if( ofst > maxDistFromEdge )
		{
			maxDistFromEdge = ofst;
			trgtDirI   = di;
			trgtSliceI = (int)( (zPos - m_dirs[di]->m_zPosMin) / m_pitch );
			t_cropI( trgtSliceI, 0, (int)m_dirs[di]->m_imgPathes.size() - 1);
		}
	}

	if( trgtDirI != -1 && trgtSliceI != -1 ){
		if( t_readRawImage( m_dirs[trgtDirI]->m_imgPathes[trgtSliceI].c_str(), m_W,m_H,  trgt2DimgArray ) ) return;
	}

	trgt2DimgArray = 0;
}









