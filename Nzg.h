
// Nzg.h : main header file for the Nzg application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "Tools.h"
#include "Node.h"
#include "FileView.h"

class CNzgTabView;
class CMainFrame;
class CNzgApp;
class CPlot2dView;
// CNzgApp:
// See Nzg.cpp for the implementation of this class
//
CFileView* getFileView();
CMainFrame* getMainFrame();
CNzgApp* getNzgApp();

class CNzgApp : public CWinAppEx
{
public:
	CNzgApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	nzg::ScreenParams m_sp;
	std::vector <std::shared_ptr<nzg::Node>> m_nodes;
	CMultiDocTemplate* m_ptemplNzg;
	CMultiDocTemplate* m_ptemplPlot2d;

	void showNzgView(nzg::NzgNode* node);
	CNzgTabView* findNzgView(const nzg::NzgNode* node) const;

	void showPlot2dView(nzg::Plot2dNode* node);
	CPlot2dView* findPlot2dView(const nzg::Plot2dNode* node) const;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
	void onViewNzg(nzg::NzgNode* pn);
	void onViewPlot2d(nzg::Plot2dNode* pn);

	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNewNzg();
	afx_msg void OnNewPlot2d();
};

extern CNzgApp theApp;

inline const nzg::ScreenParams& getSP() {
	return theApp.m_sp;
}
