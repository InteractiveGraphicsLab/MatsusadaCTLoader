#pragma once
#include <string>
#include <vector>
#include "tmath.h"



using namespace std;



//SliceXXXX というディレクトリ内の .raw ファイル 群を管理
//zの値とファイルパスのみを保持
class ImagesInDir
{

public:
	double m_zPosMin, m_zPosMax; //z pos
	vector<string> m_imgPathes ;

	ImagesInDir(){ m_zPosMin = m_zPosMax = 0; m_imgPathes.clear(); }
	~ImagesInDir(){ }
};


//複数の SliceXXXX ディレクトリを管理し，
//zの値をクエリとして2D slice imageを返す

// ディレクトリの指定は loadDirectries より行う
// 複数ディレクトリの入力が可能だが，画像サイズ W x H は 同じと仮定する
// 指定ディレクトリと同じpathにある2DParam.txtより画像データの内容を取得する
// 
// 
class VolumeFileManager
{
	VolumeFileManager(void);
public:
	~VolumeFileManager(void);
	inline static VolumeFileManager* getInst(){ static VolumeFileManager p; return &p;}

	//image size and value
	int    m_W, m_H, m_D; 
	double m_pitch, m_minZpos, m_maxZpos;
	short  m_minVal, m_maxVal;
	vector< ImagesInDir* > m_dirs;


	void loadDirectries(vector<string> &dirPathes);
	void getSliceImg_Zth_slice( int z, short *trgt2DimgArray  );


	int  getTotalImgNum()
  { 
		int total = 0;
		for( int i=0; i<(int)m_dirs.size(); ++i) {
      total += (int)m_dirs[i]->m_imgPathes.size(); 
    }
		return total;
	}

};







