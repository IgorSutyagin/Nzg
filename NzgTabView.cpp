
// NzgView.cpp : implementation of the CNzgTabView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Nzg.h"
#endif

#include "NzgDoc.h"
#include "NzgTabView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNzgTabView

IMPLEMENT_DYNCREATE(CNzgTabView, CTabView)

BEGIN_MESSAGE_MAP(CNzgTabView, CTabView)
	// Standard printing commands
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CNzgTabView construction/destruction

CNzgTabView::CNzgTabView() noexcept
{
	// TODO: add construction code here
	m_pNzgView = nullptr;
}

CNzgTabView::~CNzgTabView()
{
}

BOOL CNzgTabView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTabView::PreCreateWindow(cs);
}

// CNzgTabView drawing


// CNzgTabView printing



// CNzgTabView diagnostics

#ifdef _DEBUG
void CNzgTabView::AssertValid() const
{
	CView::AssertValid();
}

void CNzgTabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CNzgDoc* CNzgTabView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNzgDoc)));
	return (CNzgDoc*)m_pDocument;
}
#endif //_DEBUG


// CNzgTabView message handlers
int CNzgTabView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	AddView(RUNTIME_CLASS(CNzgView), "Nzg", 1);

	m_pNzgView = (CNzgView*)m_wndTabs.GetTabWnd(0);

	return 0;
}


BOOL CNzgTabView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CTabView::OnEraseBkgnd(pDC);
}


void CNzgTabView::OnInitialUpdate()
{
	CTabView::OnInitialUpdate();

	//getRtkBar()->onCreateView(this); // m_pView = this;
}
