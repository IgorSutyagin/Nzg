#pragma once


// CPlot2dFrame

class CPlot2dFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CPlot2dFrame)

public:
	CPlot2dFrame();
	virtual ~CPlot2dFrame();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


