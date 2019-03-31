#pragma once
#include "afxcmn.h"


// ImgLoadParamDlg ダイアログ

class ImgLoadParamDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ImgLoadParamDlg)

public:
	ImgLoadParamDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~ImgLoadParamDlg();

// ダイアログ データ
	enum { IDD = IDD_DIALOG_IMGPARAM };

	bool DoModal_getParam( int &width, int &height, int &pixByte);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	int m_width, m_height, m_pixByte;


	DECLARE_MESSAGE_MAP()
public:
	CSpinButtonCtrl m_spin_width;
	CSpinButtonCtrl m_spin_height;
	CSpinButtonCtrl m_spin_PixByte;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
