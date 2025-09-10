#pragma once

#include "Plot3d.h"
#include "NzgNode.h"
#include "NzgDoc.h"

class CNzgView;
class CNzgDoc;

class CNzgCtrl : public nzg::Plot3d
{
	// Construction:
public:
	CNzgCtrl(CNzgView* pv) : m_pView(pv) {}
	virtual ~CNzgCtrl() {}

	// Attributes:
public:
	CNzgView* m_pView;

	// Overrides:
public:
	virtual nzg::OglSurface::Grid getGrid() const;
	virtual nzg::Node* getNode() const;
	virtual void updateData();
	virtual double getData(const nzg::Node* node, double x, double y) const;
	virtual void onViewChanged();
	virtual void onDraw();
	virtual void updateScale();
	void drawLegend();

};


// CNzgView form view

class CNzgView : public CFormView
{
	DECLARE_DYNCREATE(CNzgView)

protected:
	CNzgView();           // protected constructor used by dynamic creation
	virtual ~CNzgView();
	bool m_bNeedInit;
	CRect m_rCrt;

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_NZG };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CNzgDoc* getDocument() {
		return (CNzgDoc*)GetDocument();
	}
	nzg::NzgNode* getNode() {
		return getDocument()->m_node;
	}

	CNzgCtrl m_wndPlot;
	int m_rows;
	int m_cols;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonPlay2();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void onChangeSize();
	afx_msg void OnTimer(UINT nID);
};


