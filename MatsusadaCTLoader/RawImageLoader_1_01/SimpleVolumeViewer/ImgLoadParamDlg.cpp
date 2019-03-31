// ImgLoadParamDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "SimpleVolumeViewer.h"
#include "ImgLoadParamDlg.h"
#include "afxdialogex.h"


// ImgLoadParamDlg ダイアログ

IMPLEMENT_DYNAMIC(ImgLoadParamDlg, CDialogEx)

ImgLoadParamDlg::ImgLoadParamDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ImgLoadParamDlg::IDD, pParent)
{

}

ImgLoadParamDlg::~ImgLoadParamDlg()
{
}

void ImgLoadParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN_WIDTH  , m_spin_width);
	DDX_Control(pDX, IDC_SPIN_HEIGHT , m_spin_height);
	DDX_Control(pDX, IDC_SPIN_PIXBYTE, m_spin_PixByte);
}


BEGIN_MESSAGE_MAP(ImgLoadParamDlg, CDialogEx)
END_MESSAGE_MAP()


// ImgLoadParamDlg メッセージ ハンドラー


bool ImgLoadParamDlg::DoModal_getParam( int &width, int &height, int &pixByte)
{

	if( DoModal() == IDOK )
	{
		width   = m_width  ;
		height  = m_height ;
		pixByte = m_pixByte;
		return true;
	}
	return false;

}


BOOL ImgLoadParamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	m_spin_width  .SetRange32(1,10000);
	m_spin_height .SetRange32(1,10000);
	m_spin_PixByte.SetRange32(1,8    );

	m_spin_width  .SetPos32( 1024 );
	m_spin_height .SetPos32( 1024 );
	m_spin_PixByte.SetPos32(  4   );

	m_width = m_height = 1024;
	m_pixByte  = 4;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


void ImgLoadParamDlg::OnOK()
{
	m_width   = m_spin_width  .GetPos32();
	m_height  = m_spin_height .GetPos32();
	m_pixByte = m_spin_PixByte.GetPos32();

	CDialogEx::OnOK();
}
