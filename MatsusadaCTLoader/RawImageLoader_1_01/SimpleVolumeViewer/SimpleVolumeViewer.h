// SimpleVolumeViewer.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CSimpleVolumeViewerApp:
// このクラスの実装については、SimpleVolumeViewer.cpp を参照してください。
//

class CSimpleVolumeViewerApp : public CWinApp
{
public:
	CSimpleVolumeViewerApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CSimpleVolumeViewerApp theApp;