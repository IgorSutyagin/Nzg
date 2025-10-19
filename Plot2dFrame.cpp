// Plot2dFrame.cpp : implementation file
//

#include "pch.h"
#include "Nzg.h"
#include "Plot2dFrame.h"


// CPlot2dFrame

IMPLEMENT_DYNCREATE(CPlot2dFrame, CMDIChildWndEx)

CPlot2dFrame::CPlot2dFrame()
{

}

CPlot2dFrame::~CPlot2dFrame()
{
}


BEGIN_MESSAGE_MAP(CPlot2dFrame, CMDIChildWndEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CPlot2dFrame message handlers



BOOL CPlot2dFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	return CMDIChildWndEx::PreCreateWindow(cs);
}

int CPlot2dFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	return 0;
}
