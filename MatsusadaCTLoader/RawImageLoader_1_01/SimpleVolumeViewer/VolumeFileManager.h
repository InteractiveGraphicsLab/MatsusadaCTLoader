#pragma once
#include <string>
#include <vector>
#include "tmath.h"



using namespace std;



//SliceXXXX �Ƃ����f�B���N�g������ .raw �t�@�C�� �Q���Ǘ�
//z�̒l�ƃt�@�C���p�X�݂̂�ێ�
class ImagesInDir
{

public:
	double m_zPosMin, m_zPosMax; //z pos
	vector<string> m_imgPathes ;

	ImagesInDir(){ m_zPosMin = m_zPosMax = 0; m_imgPathes.clear(); }
	~ImagesInDir(){ }
};


//������ SliceXXXX �f�B���N�g�����Ǘ����C
//z�̒l���N�G���Ƃ���2D slice image��Ԃ�

// �f�B���N�g���̎w��� loadDirectries ���s��
// �����f�B���N�g���̓��͂��\�����C�摜�T�C�Y W x H �� �����Ɖ��肷��
// �w��f�B���N�g���Ɠ���path�ɂ���2DParam.txt���摜�f�[�^�̓��e���擾����
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







