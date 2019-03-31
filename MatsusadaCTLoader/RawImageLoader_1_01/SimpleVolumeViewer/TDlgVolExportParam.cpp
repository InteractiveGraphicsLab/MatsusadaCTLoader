// TDlgVolExportParam.cpp : 実装ファイル
//

#include "stdafx.h"
#include "SimpleVolumeViewer.h"
#include "TDlgVolExportParam.h"
#include "afxdialogex.h"


// TDlgVolExportParam ダイアログ

IMPLEMENT_DYNAMIC(TDlgVolExportParam, CDialogEx)

TDlgVolExportParam::TDlgVolExportParam(int W, int H, int D, CWnd* pParent /*=NULL*/)
	: CDialogEx(TDlgVolExportParam::IDD, pParent),
	m_W( W ),
	m_H( H ),
	m_D( D )
{
	m_expW = W;
	m_expH = H;
	m_expD = D;
}

TDlgVolExportParam::~TDlgVolExportParam()
{
}

void TDlgVolExportParam::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_EXPVOLSIZE, m_slider_exportVolWidth);
	DDX_Control(pDX, IDC_CHECK_EXP_NEAREST, m_check_nearest);
	DDX_Control(pDX, IDC_CHECK_EXP_LINEAR, m_check_linear);
	DDX_Control(pDX, IDC_CHECK_EXP_FLIPZ, m_check_flipz);
}


BEGIN_MESSAGE_MAP(TDlgVolExportParam, CDialogEx)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_EXP_NEAREST, &TDlgVolExportParam::OnBnClickedCheckExpNearest)
	ON_BN_CLICKED(IDC_CHECK_EXP_LINEAR, &TDlgVolExportParam::OnBnClickedCheckExpLinear)
	ON_BN_CLICKED(IDC_CHECK_EXP_FLIPZ, &TDlgVolExportParam::OnBnClickedCheckExpFlipz)
END_MESSAGE_MAP()


// TDlgVolExportParam メッセージ ハンドラー

void TDlgVolExportParam::updateEditBoxInfo()
{
	double rate = m_slider_exportVolWidth.GetPos() / (double) m_W;
	m_expW = m_slider_exportVolWidth.GetPos();
	m_expH = (int)( m_H * rate );
	m_expD = (int)( m_D * rate );
	CString str;
	str.Format( "%d", m_expW ); ((CEdit*)GetDlgItem( IDC_EDIT_EXVOLSIZE_W ))->SetWindowTextA( str );
	str.Format( "%d", m_expH ); ((CEdit*)GetDlgItem( IDC_EDIT_EXVOLSIZE_H ))->SetWindowTextA( str );
	str.Format( "%d", m_expD ); ((CEdit*)GetDlgItem( IDC_EDIT_EXVOLSIZE_D ))->SetWindowTextA( str );

	double g = 2.0 * m_expW * m_expH * m_expD / 1024.0 / 1024.0 ;
	str.Format( "%.1f-MB",g);((CEdit*)GetDlgItem( IDC_EDIT_EXVOLSIZE_SIZE ))->SetWindowTextA( str );
}


BOOL TDlgVolExportParam::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	fprintf( stderr, "%d %d %d\n", m_W, m_H, m_D );
	m_slider_exportVolWidth.SetRange( m_W / 5, m_W);
	m_slider_exportVolWidth.SetPos  ( m_W );
	updateEditBoxInfo();

	m_bLinearInter = true ;
	m_bNearNei     = false;
	m_bFlipZ       = true ;
	m_check_linear .SetCheck(1);
	m_check_nearest.SetCheck(0);
	m_check_flipz  .SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


void TDlgVolExportParam::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( *pScrollBar == m_slider_exportVolWidth )
	{
		updateEditBoxInfo();
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void TDlgVolExportParam::OnBnClickedCheckExpNearest(){
	m_bNearNei     = true  ;
	m_bLinearInter = false ;
	m_check_nearest.SetCheck( m_bNearNei     );
	m_check_linear .SetCheck( m_bLinearInter );
}

void TDlgVolExportParam::OnBnClickedCheckExpLinear()
{
	m_bLinearInter =  true ;
	m_bNearNei     =  false;
	m_check_nearest.SetCheck( m_bNearNei     );
	m_check_linear .SetCheck( m_bLinearInter );
}


void TDlgVolExportParam::OnBnClickedCheckExpFlipz()
{
	m_bFlipZ = !m_bFlipZ;
	m_check_flipz.SetCheck( m_bFlipZ );
}
