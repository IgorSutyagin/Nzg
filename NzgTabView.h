
// NzgView.h : interface of the CNzgTabView class
//

#pragma once
#include "NzgView.h"


class CNzgTabView : public CTabView
{
protected: // create from serialization only
	CNzgTabView() noexcept;
	DECLARE_DYNCREATE(CNzgTabView)

// Attributes
public:
	CNzgDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CNzgTabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CNzgView * m_pNzgView;

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // debug version in NzgView.cpp
inline CNzgDoc* CNzgTabView::GetDocument() const
   { return reinterpret_cast<CNzgDoc*>(m_pDocument); }
#endif

