#pragma once

#include "Plot2d.h"


// CPlot2dView form view
class CPlot2dDoc;

class CPlot2dView : public CFormView
{
	DECLARE_DYNCREATE(CPlot2dView)

protected:
	CPlot2dView();           // protected constructor used by dynamic creation
	virtual ~CPlot2dView();

	CPlot2dDoc* GetDocument() const;

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_2DPLOT1 };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	bool m_bNeedInit;
	CRect m_rCrt;
	nzg::Plot2d m_wndPlot;
	CFont m_font;
	BOOL m_bMarker;

	double m_yMax;
	double m_yMin;
	double m_xMin;
	double m_xMax;
	BOOL m_byAuto;
	BOOL m_bxAuto;
	int m_yDivs;
	int m_xDivs;


protected:
	void updateCurves();
	void enableControls();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedCheckAuto();
	afx_msg void OnChangeEditXMinMax();
	afx_msg void OnChangeEditYMinMax();
	afx_msg void OnBnClickedButtonAddComment();
	afx_msg void OnBnClickedButtonRemoveComments();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonLoad();

	afx_msg void OnBnClickedButtonSetPlotTitle();

	afx_msg void OnCurveFormat();
public:
	afx_msg void OnBnClickedButtonUpdate();
};


