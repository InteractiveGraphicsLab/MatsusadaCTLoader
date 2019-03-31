// SimpleVolumeViewerDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "SimpleVolumeViewer.h"
#include "SimpleVolumeViewerDlg.h"
#include "VolumeFileManager.h"
#include "SimpleImage.h"
#include "Tifio.h"
#include "TDlgVolExportParam.h"
#include "TLIB/tmath.h"

#define  PC_SIZE 900


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSimpleVolumeViewerDlg* CSimpleVolumeViewerDlg::m_myself = 0;

// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
	DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD){}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CSimpleVolumeViewerDlg �_�C�A���O

CSimpleVolumeViewerDlg::CSimpleVolumeViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSimpleVolumeViewerDlg::IDD, pParent)
{
	m_hIcon          = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_myself         = this ;
	m_bPictContInit  = false;
}

void CSimpleVolumeViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_DEPTH, m_slider_depth);
	DDX_Control(pDX, IDC_SLIDER_WINDOW_MIN, m_slider_winMin);
	DDX_Control(pDX, IDC_SLIDER_WINDOW_MAX, m_slider_winMax);
	DDX_Control(pDX, IDC_SPIN_CLIP_XMIN, m_spin_xmin);
	DDX_Control(pDX, IDC_SPIN_CLIP_XMAX, m_spin_xmax);
	DDX_Control(pDX, IDC_SPIN_CLIP_YMIN, m_spin_ymin);
	DDX_Control(pDX, IDC_SPIN_CLIP_YMAX, m_spin_ymax);
	DDX_Control(pDX, IDC_SPIN_CLIP_ZMIN, m_spin_zmin);
	DDX_Control(pDX, IDC_SPIN_CLIP_ZMAX, m_spin_zmax);
}

BEGIN_MESSAGE_MAP(CSimpleVolumeViewerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON_INIT_CLIPPING, &CSimpleVolumeViewerDlg::OnBnClickedButtonInitClipping)
	ON_WM_VSCROLL()
	ON_EN_CHANGE(IDC_EDIT_CLIP_XMIN, &CSimpleVolumeViewerDlg::OnEnChangeEditClipXmin)
	ON_EN_CHANGE(IDC_EDIT_CLIP_XMAX, &CSimpleVolumeViewerDlg::OnEnChangeEditClipXmax)
	ON_EN_CHANGE(IDC_EDIT_CLIP_YMIN, &CSimpleVolumeViewerDlg::OnEnChangeEditClipYmin)
	ON_EN_CHANGE(IDC_EDIT_CLIP_YMAX, &CSimpleVolumeViewerDlg::OnEnChangeEditClipYmax)
	ON_EN_CHANGE(IDC_EDIT_CLIP_ZMIN, &CSimpleVolumeViewerDlg::OnEnChangeEditClipZmin)
	ON_EN_CHANGE(IDC_EDIT_CLIP_ZMAX, &CSimpleVolumeViewerDlg::OnEnChangeEditClipZmax)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON_SAVETIF, &CSimpleVolumeViewerDlg::OnBnClickedButtonSavetif)
	ON_BN_CLICKED(IDC_BUTTON_SAVETRAW3D, &CSimpleVolumeViewerDlg::OnBnClickedButtonSavetraw3d)
END_MESSAGE_MAP()



// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CSimpleVolumeViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CSimpleVolumeViewerDlg::PreInitDialog()
{
	CDialog::PreInitDialog();
}

void CSimpleVolumeViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}



// CSimpleVolumeViewerDlg ���b�Z�[�W �n���h��
BOOL CSimpleVolumeViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	DragAcceptFiles();


	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	m_slider_depth.SetRange( 0, 0 );
	m_slider_depth.SetPos  ( 0    );


	//resize DIALOG and PICTURE CONTROL = �wPC_SIZE�x
	CWnd *h_PC = GetDlgItem(IDC_MAIN_PICTCONTROL);

	WINDOWPLACEMENT wplm_dlg, wplm_pc;
	this->GetWindowPlacement( &wplm_dlg );
	h_PC->GetWindowPlacement( &wplm_pc  );

	//resize dlg
	int dlgW = wplm_dlg.rcNormalPosition.right  - wplm_dlg.rcNormalPosition.left;
	int dlgH = wplm_dlg.rcNormalPosition.bottom - wplm_dlg.rcNormalPosition.top ;
	dlgW = max( dlgW, PC_SIZE + wplm_pc.rcNormalPosition.left + 30 );
	dlgH = max( dlgH, PC_SIZE + wplm_pc.rcNormalPosition.top  + 30 );

	this->SetWindowPos( &CWnd::wndNoTopMost, wplm_dlg.rcNormalPosition.left, wplm_dlg.rcNormalPosition.top, dlgW   , dlgH, SWP_NOACTIVATE | SWP_SHOWWINDOW  );
	h_PC->MoveWindow(                        wplm_pc .rcNormalPosition.left, wplm_pc.rcNormalPosition .top, PC_SIZE, PC_SIZE);

	m_bPictContInit = true;


	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}




//directry loading ����ɌĂ΂��
void CSimpleVolumeViewerDlg::t_newDirLoaded()
{
	VolumeFileManager *VFM = VolumeFileManager::getInst();

	//Loaded Image information
	CString strImageRes, strImageNum, strCubeX, strCubeY, strCubeZ;
	strImageRes.Format( "z�l:[%.2fmm, %.2fmm]   �s�b�`:%.4f", VFM ->m_minZpos, VFM->m_maxZpos, VFM ->m_pitch);
	strImageNum.Format( "�f�B���N�g����:%d   �摜��:%d ", VFM ->m_dirs.size(), VFM ->getTotalImgNum() );
	strCubeX   .Format( "X: 0-%d", VFM ->m_W - 1);
	strCubeY   .Format( "Y: 0-%d", VFM ->m_H - 1);
	strCubeZ   .Format( "Z: 0-%d", VFM ->m_D - 1);

	SetDlgItemText( IDC_TEXT_IMGRES, strImageRes );
	SetDlgItemText( IDC_TEXT_IMGNUM, strImageNum );
	SetDlgItemText( IDC_TEXT_CUBE_X, strCubeX);
	SetDlgItemText( IDC_TEXT_CUBE_Y, strCubeY);
	SetDlgItemText( IDC_TEXT_CUBE_Z, strCubeZ);

	//window level 
	m_slider_winMin.SetRange( VFM->m_minVal, VFM->m_maxVal);  m_slider_winMin.SetPos( VFM->m_minVal );
	m_slider_winMax.SetRange( VFM->m_minVal, VFM->m_maxVal);  m_slider_winMax.SetPos( VFM->m_maxVal );

	CString CStrMax; CStrMax.Format("%d", m_slider_winMax.GetPos() ); 
	CString CStrMin; CStrMin.Format("%d", m_slider_winMin.GetPos() ); 
	((CEdit*)GetDlgItem( IDC_EDIT_WINDOW_MAX ))->SetWindowTextA(CStrMax);
	((CEdit*)GetDlgItem( IDC_EDIT_WINDOW_MIN ))->SetWindowTextA(CStrMin);

	//clipping info (double�l��CubeSize�Ő���)
	const int W = VFM->m_W,  H = VFM->m_H, D = VFM->m_D;
	m_spin_xmin.SetRange32(0, W-1); m_spin_xmax.SetRange32(0, W-1);
	m_spin_ymin.SetRange32(0, H-1); m_spin_ymax.SetRange32(0, H-1);
	m_spin_zmin.SetRange32(0, D-1); m_spin_zmax.SetRange32(0, D-1);
	m_spin_xmin.SetPos32(0);        m_spin_xmax.SetPos32( W-1 );
	m_spin_ymin.SetPos32(0);        m_spin_ymax.SetPos32( H-1 );
	m_spin_zmin.SetPos32(0);        m_spin_zmax.SetPos32( D-1 );

	m_slider_depth.SetRange( 0, D-1 );
	m_slider_depth.SetPos( D / 2 ); 
	CString CStrZ  ; CStrZ  .Format("%d", m_slider_depth .GetPos()    ); 
	((CEdit*)GetDlgItem( IDC_EDIT_IMAGE_Z   ))->SetWindowTextA(CStrZ  );

	RedrawWindow();
}




/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�`�惋�[�`��///////////////////////////////////////////////////////////////////////
void CSimpleVolumeViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width () - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else if( m_bPictContInit )
	{
		paintPictureControl();
		CDialog::OnPaint();
	}
}



static CBrush m_backBrush;

void CSimpleVolumeViewerDlg::paintPictureControl()
{
	if( VolumeFileManager::getInst()->m_dirs.empty() ) return;

	static bool first = true;
	if( first )
	{
		CWnd *cwndPC = GetDlgItem(IDC_MAIN_PICTCONTROL);
		first = false;
		CRect r;                   cwndPC->GetClientRect(&r);
		WINDOWPLACEMENT winplace;  cwndPC->GetWindowPlacement( &winplace );
		m_pcW = m_pcH = PC_SIZE;//-2
		m_pcX = winplace.rcNormalPosition.left;
		m_pcY = winplace.rcNormalPosition.top ;
		m_backBrush.CreateSolidBrush( RGB(50,50,50) );
	}


  //A. picture Control��CWnd���擾
  CWnd *pcWnd = GetDlgItem( IDC_MAIN_PICTCONTROL ); 
  CDC  *pcDC  = pcWnd->GetDC();  //pcDC->SetPixel( 3, 4, RGB(10, 10, 10)); ���ꂪ������Ή��̖����Ȃ��̂ɁE�E�E

  //C. �f�o�C�X�Ɉˑ����Ȃ��J���[�r�b�g�}�b�v���쐬
  BITMAPINFO binfo;
  ZeroMemory( &binfo, sizeof(binfo));
  binfo.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
  binfo.bmiHeader.biBitCount = 32;//1pixel 32-bit (4-byte)
  binfo.bmiHeader.biPlanes   =  1      ;
  binfo.bmiHeader.biWidth    =  PC_SIZE;
  binfo.bmiHeader.biHeight   = -PC_SIZE; //�����������ƁA���オ���_�ɂȂ�(���Ȃ獶�������_)

  byte    *bmpbits; //���� byte *bmpbits �� HBITMAP �̒��g�ւ̃|�C���^������
  HBITMAP  hbmp = CreateDIBSection( NULL, &binfo, DIB_RGB_COLORS, (void **)(&bmpbits), NULL, 0); 
            
  //D. hbmp�Ɋ֘A�t���� CBitmap�ƁACDC���쐬
  CBitmap *cbmp = CBitmap::FromHandle( hbmp );//hbmp��CBitmap cbmp�ɕϊ�
  CDC cbmpDC; 
  cbmpDC.CreateCompatibleDC( pcDC );          //cbmp�Ɋ֘A�Â���CDC������
  CBitmap *oldBmp = cbmpDC.SelectObject( cbmp ); 
        
  //E. hbmp(cbmp)�� �|�C���^���� (bmpbits) ���獂���ɃA�N�Z�X�ł���
	const int    imgW = VolumeFileManager::getInst()->m_W;
	const int    imgH = VolumeFileManager::getInst()->m_H;
	const int    imgD = VolumeFileManager::getInst()->m_D;
	const int    imgZ = m_slider_depth .GetPos();
	const short  vMax = m_slider_winMax.GetPos();
	const short  vMin = m_slider_winMin.GetPos();
	int x0 = m_spin_xmin.GetPos32(), x1 = m_spin_xmax.GetPos32();  t_cropI(x0,0,imgW-1);  t_cropI(x0,0,imgW-1);
	int y0 = m_spin_ymin.GetPos32(), y1 = m_spin_ymax.GetPos32();  t_cropI(y0,0,imgH-1);  t_cropI(y0,0,imgH-1);
	int z0 = m_spin_zmin.GetPos32(), z1 = m_spin_zmax.GetPos32();  t_cropI(z0,0,imgD-1);  t_cropI(z0,0,imgD-1);

	short *img2D = new short[imgW * imgH];
	VolumeFileManager::getInst()->getSliceImg_Zth_slice( imgZ, img2D );
	
	double xCoef = imgW / (double) PC_SIZE;
	double yCoef = imgH / (double) PC_SIZE;


	short &imgMinV = VolumeFileManager::getInst()->m_minVal;
	short &imgMaxV = VolumeFileManager::getInst()->m_maxVal; //���łɌv�Z

	if( img2D == 0 )
	{
		for( int y = 0; y < PC_SIZE; ++y)
    {
		  for( int x = 0; x < PC_SIZE; ++x)
      {
			  int bmpI = (x + y * PC_SIZE ) * 4;
			  bmpbits[ bmpI + 0 ] = bmpbits[ bmpI + 1 ] = bmpbits[ bmpI + 2 ] = 128;
		  }
    }
	}else{
		for( int y = 0; y < PC_SIZE; ++y)
    {
		  for( int x = 0; x < PC_SIZE; ++x)
		  {
			  int imgX = (int)( (x+0.5) * xCoef );
			  int imgY = (int)( (y+0.5) * yCoef );
			  int imgI = imgX + imgY * imgW      ; 
			  const short imgV = img2D[ imgI ];

			  imgMinV = min( imgMinV, imgV );
			  imgMaxV = max( imgMaxV, imgV );

			  int bmpI = (x + y * PC_SIZE ) * 4;
			  if( x0 <= imgX && imgX <= x1 && y0 <= imgY && imgY <= y1 && z0 <= imgZ && imgZ <= z1 )
			  { 
				  byte c  = (byte)( 255.0 * min(1, max(0, ( imgV - vMin) / (double)(vMax-vMin) ) ));
				  bmpbits[ bmpI + 0 ] = bmpbits[ bmpI + 1 ] = bmpbits[ bmpI + 2 ] = c;
			  }else{
				  bmpbits[ bmpI + 0 ] = 192; bmpbits[ bmpI + 1 ] =  bmpbits[ bmpI + 2 ] = 0;
			  }
		  }
    }
	}

    //F. CDC cbmpDC������A�N�Z�X�ł��Č��\�֗�
    //cbmpDC.MoveTo( 10, 10);
    //cbmpDC.LineTo( 10, 60);
    //cbmpDC.LineTo( 60, 60);
    //cbmpDC.LineTo( 60, 10);
    //cbmpDC.LineTo( 10, 10);

    //G. pcDC��hbmp���R�s�[����
    pcDC->BitBlt( 1,1, PC_SIZE-2, PC_SIZE-2, &cbmpDC, 0,0,SRCCOPY);

    //H. �����Y��Ȃ�
    cbmpDC.SelectObject( oldBmp );
    DeleteDC( cbmpDC );
    DeleteObject( hbmp );//HBITMAP�����
    pcWnd->ReleaseDC(pcDC);

	//�ő�l & �ŏ��l & slider�X�V
	m_slider_winMin.SetRange( imgMinV, imgMaxV);  m_slider_winMin.SetPos( vMin );
	m_slider_winMax.SetRange( imgMinV, imgMaxV);  m_slider_winMax.SetPos( vMax );

	delete[] img2D;
}





void CSimpleVolumeViewerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( *pScrollBar == m_slider_depth )
	{
		CString CStrZ; 
		CStrZ.Format("%d", m_slider_depth.GetPos() ); 
		((CEdit*)GetDlgItem( IDC_EDIT_IMAGE_Z))->SetWindowTextA( CStrZ);
		Invalidate( FALSE );
		UpdateWindow();
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CSimpleVolumeViewerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( *pScrollBar == m_slider_winMax)
	{
		CString CStrMax; CStrMax.Format("%d", m_slider_winMax.GetPos()); 
		((CEdit*)GetDlgItem( IDC_EDIT_WINDOW_MAX))->SetWindowTextA(CStrMax);
		Invalidate( FALSE );
		UpdateWindow();
	}
	if( *pScrollBar == m_slider_winMin)
	{
		CString CStrMin; CStrMin.Format("%d", m_slider_winMin.GetPos()); 
		((CEdit*)GetDlgItem( IDC_EDIT_WINDOW_MIN))->SetWindowTextA(CStrMin);
		Invalidate( FALSE );
		UpdateWindow();
	}	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}











//save as multi page tif file 

void CSimpleVolumeViewerDlg::OnBnClickedButtonInitClipping()
{
	m_spin_xmin.SetPos32(0); m_spin_xmax.SetPos32( VolumeFileManager::getInst()->m_W-1 );
	m_spin_ymin.SetPos32(0); m_spin_ymax.SetPos32( VolumeFileManager::getInst()->m_H-1 );
	m_spin_zmin.SetPos32(0); m_spin_zmax.SetPos32( VolumeFileManager::getInst()->m_D-1 );
	RedrawWindow();
}

void CSimpleVolumeViewerDlg::OnEnChangeEditClipXmin(){ if( !m_bPictContInit ) return;  if( m_spin_xmin.GetPos32() >= m_spin_xmax.GetPos32() ) m_spin_xmin.SetPos( m_spin_xmax.GetPos32() - 1 ); Invalidate( FALSE );UpdateWindow();}
void CSimpleVolumeViewerDlg::OnEnChangeEditClipYmin(){ if( !m_bPictContInit ) return;  if( m_spin_ymin.GetPos32() >= m_spin_ymax.GetPos32() ) m_spin_ymin.SetPos( m_spin_ymax.GetPos32() - 1 ); Invalidate( FALSE );UpdateWindow();}
void CSimpleVolumeViewerDlg::OnEnChangeEditClipZmin(){ if( !m_bPictContInit ) return;  if( m_spin_zmin.GetPos32() >= m_spin_zmax.GetPos32() ) m_spin_zmin.SetPos( m_spin_zmax.GetPos32() - 1 );Invalidate( FALSE );UpdateWindow(); }
void CSimpleVolumeViewerDlg::OnEnChangeEditClipXmax(){ if( !m_bPictContInit ) return;  if( m_spin_xmin.GetPos32() >= m_spin_xmax.GetPos32() ) m_spin_xmax.SetPos( m_spin_xmin.GetPos32() + 1 ); Invalidate( FALSE );UpdateWindow();}
void CSimpleVolumeViewerDlg::OnEnChangeEditClipYmax(){ if( !m_bPictContInit ) return;  if( m_spin_ymin.GetPos32() >= m_spin_ymax.GetPos32() ) m_spin_ymax.SetPos( m_spin_ymin.GetPos32() + 1 ); Invalidate( FALSE );UpdateWindow();}
void CSimpleVolumeViewerDlg::OnEnChangeEditClipZmax(){ if( !m_bPictContInit ) return;  if( m_spin_zmin.GetPos32() >= m_spin_zmax.GetPos32() ) m_spin_zmax.SetPos( m_spin_zmin.GetPos32() + 1 ); Invalidate( FALSE );UpdateWindow();}
		


void CSimpleVolumeViewerDlg::OnDropFiles(HDROP hDropInfo)
{
	//�h���b�v���ꂽ�t�@�C���̌����擾
	UINT uiCount = DragQueryFile(hDropInfo,~0lu,NULL,0);

	vector<string> pathNames;
	for(UINT i=0; i<uiCount; i++){
		//�t�@�C�����̒������擾
		UINT uiLen=DragQueryFile(hDropInfo,i,NULL,0);

		//�t�@�C�������擾
		CString name;
		DragQueryFile(hDropInfo,i,name.GetBuffer(uiLen+1),uiLen+1);
		name.ReleaseBuffer();

		if( PathIsDirectoryA( name ) ){
			fprintf( stderr, "%s\n", name );
			pathNames.push_back( string( name ) );
		}
	}

	if( pathNames.size() != 0 ) VolumeFileManager::getInst()->loadDirectries( pathNames ) ;

	CDialog::OnDropFiles(hDropInfo);
}




bool CSimpleVolumeViewerDlg::gen3DimageForExport
(
  int    &outW,
  int    &outH,
  int    &outD,
  double &out_pitch,
  short* &out_img
)
{
	VolumeFileManager* VFM = VolumeFileManager::getInst();
	
	//source volume �Ɋւ���l��  in_*
	//output volume �Ɋւ���l�� out_*
	const int in_x0 = max(0, min(VFM->m_W-1, m_spin_xmin.GetPos32() )),  in_x1 = max(0, min(VFM->m_W-1, m_spin_xmax.GetPos32() )); 
	const int in_y0 = max(0, min(VFM->m_H-1, m_spin_ymin.GetPos32() )),  in_y1 = max(0, min(VFM->m_H-1, m_spin_ymax.GetPos32() )); 
	const int in_z0 = max(0, min(VFM->m_D-1, m_spin_zmin.GetPos32() )),  in_z1 = max(0, min(VFM->m_D-1, m_spin_zmax.GetPos32() )); 
  const int in_W = in_x1 - in_x0 + 1;
  const int in_H = in_y1 - in_y0 + 1;
  const int in_D = in_z1 - in_z0 + 1;
	
	//�𑜓x && ��Ԗ@ �擾
	TDlgVolExportParam paramDlg( in_W, in_H, in_D);
	if( paramDlg.DoModal() != IDOK ) return false;

	outW     = paramDlg.m_expW;
	outH     = paramDlg.m_expH;
	outD     = paramDlg.m_expD;
	const bool bNearest = paramDlg.m_bNearNei    ;
	const bool blinear  = paramDlg.m_bLinearInter;
	const bool bFlipZ   = paramDlg.m_bFlipZ      ;

  //�V����pitch���v�Z out_pitch * outW = in_pitch * inW ���
	out_pitch = VFM->m_pitch * (double)in_W / (double)outW;
	fprintf( stderr, "%d %d %d   rate   %f\n" , outW, outH, outD, (double)outW / (double)in_W);


	//�摜��������
	short *inImg2D1 = new short[ VFM->m_W * VFM->m_H ]; //��Ԃ̂���2�p��
	short *inImg2D2 = new short[ VFM->m_W * VFM->m_H ]; 
	double zRate = in_D / (double) outD; // in ��� out�̂ق����������̂� rate ��1���傫��
	double yRate = in_H / (double) outH;
	double xRate = in_W / (double) outW;

	const int inImgOrigW = VFM->m_W;
  
  out_img = new short[ outW * outH * outD ];

	if( bNearest )
  {
		for(int z = 0; z < outD; ++z )
		{ 
			int inZ = (int)( ( z + 0.5 ) * zRate + in_z0 ) ;
			VFM->getSliceImg_Zth_slice( inZ, inImg2D1 );

			for(int y = 0; y < outH; ++y )
			{ 
				int inY = (int)( ( y + 0.5 ) * yRate + in_y0 );
				for(int x = 0; x < outW; ++x )
				{
					int inX = (int)( ( x + 0.5 ) * xRate + in_x0 );
          out_img[x + y*outW + z*outW*outH] = inImg2D1[ inX + inY * inImgOrigW ];
				}
			}
			fprintf( stderr, "(%d/%d/%d)\n", z, outD, inZ);
		}
	}
	else if( blinear )
	{
		for ( int z = 0; z < outD; ++z )
		{
			double zPos = (z + 0.5 ) * zRate + in_z0;
      //nearest�Ȃ� zi = (int)zPos;�ƂȂ邪�C���̈��ƂЂƂ悪�~����
      //�� zPos = 3.2�Ȃ�  zPre = 2, zNex=3, zT = 0.7 
      //�� zPos = 3.8�Ȃ�  zPre = 3, zNex=4, zT = 0.3  (���� zi + 0.5 ����f���S�Ȃ̂�) 

			int inZpre = (int)( zPos - 0.5 ); t_cropI( inZpre, in_z0, in_z1);
			int inZnex = (int)( zPos + 0.5 ); t_cropI( inZnex, in_z0, in_z1);
			double tZ = zPos - (inZpre + 0.5);
			VFM->getSliceImg_Zth_slice( inZpre, inImg2D1 );
			VFM->getSliceImg_Zth_slice( inZnex, inImg2D2 );

			for ( int y = 0; y < outH; ++y )
			{ 
				double yPos   = ( y + 0.5 ) * yRate + in_y0;
				int    inYpre = (int)( yPos - 0.5 ); t_cropI( inYpre, in_y0, in_y1);
				int    inYnex = (int)( yPos + 0.5 ); t_cropI( inYnex, in_y0, in_y1);

				for ( int x = 0; x < outW; ++x )
				{
					double xPos   = ( x + 0.5 ) * xRate + in_x0;
					int    inXpre = (int)( xPos - 0.5 ); t_cropI( inXpre, in_x0, in_x1);
					int    inXnex = (int)( xPos + 0.5 ); t_cropI( inXnex, in_x0, in_x1);

					double tY = yPos - (inYpre + 0.5);
					double tX = xPos - (inXpre + 0.5);

					//  c1   c2  x��
					//           y��
					//  c3   c4   
					double c1 = (1-tZ) * inImg2D1[ inXpre + inYpre * inImgOrigW ]   +   tZ * inImg2D2[ inXpre + inYpre * inImgOrigW ] ;
					double c2 = (1-tZ) * inImg2D1[ inXnex + inYpre * inImgOrigW ]   +   tZ * inImg2D2[ inXnex + inYpre * inImgOrigW ] ;
					double c3 = (1-tZ) * inImg2D1[ inXpre + inYnex * inImgOrigW ]   +   tZ * inImg2D2[ inXpre + inYnex * inImgOrigW ] ;
					double c4 = (1-tZ) * inImg2D1[ inXnex + inYnex * inImgOrigW ]   +   tZ * inImg2D2[ inXnex + inYnex * inImgOrigW ] ;
					double c13   = (1-tY) * c1  +  tY * c3 ; 
					double c24   = (1-tY) * c2  +  tY * c4 ; 
					double c1324 = (1-tX) * c13 +  tX * c24;
          out_img[x + y*outW + z*outW*outH] = (short) c1324;
				}
			}
			fprintf( stderr, "(%d/%d)d", z,outD);
		}
	}
  
  if ( bFlipZ )
  {
    short *tmp = new short[outW * outH];
    const int outWH = outW * outH;
    for (int z = 0; z < outD / 2; ++z)
    {
      memcpy( tmp                         , &out_img[z * outWH]         , sizeof(short) * outWH);
      memcpy( &out_img[z * outWH ]        , &out_img[(outD-1-z) * outWH], sizeof(short) * outWH);
      memcpy( &out_img[(outD-1-z) * outWH], tmp                         , sizeof(short) * outWH);
    }
    delete[] tmp;
  }

	delete[] inImg2D1;
	delete[] inImg2D2;

  return true;

}


void CSimpleVolumeViewerDlg::OnBnClickedButtonSavetif()
{
	//file���擾
	CString filter("multipage tif (*tif)|*.tif||");
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_CREATEPROMPT, filter);
	if( dlg.DoModal() != IDOK ) return;
	string fname( dlg.GetPathName() );
	if(  strcmp( dlg.GetFileExt(), "tif") != 0) fname = fname + ".tif";

  int outW, outH, outD;
  double out_pitch;
  short *out_img;
  if( !this->gen3DimageForExport(outW,outH,outD,out_pitch,out_img) ) return;

	Image3D out_img3d( outW, outH, outD );


	for(int z = 0; z < outD; ++z )
	{ 
		for(int y = 0; y < outH; ++y )
		{ 
			for(int x = 0; x < outW; ++x )
			{
				out_img3d.img[z][y][x] = out_img[ x + y * outW + z * outW * outH ];
			}
		}
	}

	TifIO tifio( fname.c_str() );
	tifio.saveImage( &out_img3d );
  delete[] out_img;

}



void CSimpleVolumeViewerDlg::OnBnClickedButtonSavetraw3d()
{
	//file���擾	
	CString filter("traw signed short 3D (*traw3D_ss)|*.traw3D_ss||");
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_CREATEPROMPT, filter);
	if( dlg.DoModal() != IDOK ) return;
	string fname( dlg.GetPathName() );
	if(  strcmp( dlg.GetFileExt(), "traw3D_ss") != 0) fname = fname + ".traw3D_ss";

  int outW, outH, outD;
  double out_pitch;
  short *out_img;
  if( !this->gen3DimageForExport(outW,outH,outD,out_pitch,out_img) ) return;


	FILE *fp = fopen( fname.c_str(), "wb") ;
	if( fp ==0 ) return;

	//header info   W, H, D,  pitchX, pitchY, pitchZ 
	fwrite( &outW     , sizeof( int    ), 1, fp );
	fwrite( &outH     , sizeof( int    ), 1, fp );
	fwrite( &outD     , sizeof( int    ), 1, fp );
	fwrite( &out_pitch, sizeof( double ), 1, fp );
	fwrite( &out_pitch, sizeof( double ), 1, fp );
	fwrite( &out_pitch, sizeof( double ), 1, fp );
	fwrite( out_img, sizeof( short ), outW*outH*outD, fp );
	fclose(fp);

  delete[] out_img;
  
}



