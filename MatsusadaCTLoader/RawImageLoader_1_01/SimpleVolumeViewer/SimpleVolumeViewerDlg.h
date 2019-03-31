// SimpleVolumeViewerDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxext.h"

#include "resource.h"


#define SHOW_DEBUG_CONSOLE false
#define PROGRESS_RANGE     100


// CSimpleVolumeViewerDlg �_�C�A���O
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

// �R���X�g���N�V����
public:
	CSimpleVolumeViewerDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SIMPLEVOLUMEVIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g

// ����
protected:
	HICON m_hIcon;

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
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
