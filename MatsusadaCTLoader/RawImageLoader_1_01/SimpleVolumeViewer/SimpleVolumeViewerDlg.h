// SimpleVolumeViewerDlg.h : ヘッダー ファイル
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxext.h"

#include "resource.h"


#define SHOW_DEBUG_CONSOLE false
#define PROGRESS_RANGE     100


// CSimpleVolumeViewerDlg ダイアログ
class CSimpleVolumeViewerDlg : public CDialog
{
	int   m_pcX, m_pcY, m_pcH, m_pcW;
	bool m_bPictContInit;

	void paintPictureControl();
  bool gen3DimageForExport(  int    &outW,
  int    &outH,
  int    &outD,
  double &out_pitch,
  short* &out_img
 );

// コンストラクション
public:
	CSimpleVolumeViewerDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	enum { IDD = IDD_SIMPLEVOLUMEVIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	virtual void PreInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	static CSimpleVolumeViewerDlg* m_myself;
	void t_newDirLoaded();

	CStatusBar    m_statusBar     ;

	CSliderCtrl     m_slider_winMin ;
	CSliderCtrl     m_slider_winMax ;
	CSliderCtrl     m_slider_depth  ;
	CSpinButtonCtrl m_spin_xmin;
	CSpinButtonCtrl m_spin_xmax;
	CSpinButtonCtrl m_spin_ymin;
	CSpinButtonCtrl m_spin_ymax;
	CSpinButtonCtrl m_spin_zmin;
	CSpinButtonCtrl m_spin_zmax;

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedButtonInitClipping ();
	afx_msg void OnEnChangeEditClipXmin();
	afx_msg void OnEnChangeEditClipXmax();
	afx_msg void OnEnChangeEditClipYmin();
	afx_msg void OnEnChangeEditClipYmax();
	afx_msg void OnEnChangeEditClipZmin();
	afx_msg void OnEnChangeEditClipZmax();
	afx_msg void OnBnClickedButtonSavetif();
	afx_msg void OnBnClickedButtonSavetraw3d();


};
