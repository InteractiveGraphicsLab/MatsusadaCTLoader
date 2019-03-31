#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// TDlgVolExportParam ダイアログ

class TDlgVolExportParam : public CDialogEx
{
	DECLARE_DYNAMIC(TDlgVolExportParam)

private:
	const int m_W, m_H, m_D;

public:
	int  m_expW, m_expH, m_expD;
	bool m_bNearNei    ;
	bool m_bLinearInter;
	bool m_bFlipZ      ;

	void updateEditBoxInfo();


public:
	TDlgVolExportParam(int W, int H, int D, CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~TDlgVolExportParam();

// ダイアログ データ
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_slider_exportVolWidth;
	CButton m_check_nearest;
	CButton m_check_linear;
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckExpNearest();
	afx_msg void OnBnClickedCheckExpLinear ();
	CButton m_check_flipz;
	afx_msg void OnBnClickedCheckExpFlipz();
};
