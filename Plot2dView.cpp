// Plot2dView.cpp : implementation file
//

#include "pch.h"
#include "Nzg.h"
#include "Plot2dView.h"
#include "Plot2dDoc.h"

CPlot2dDoc* CPlot2dView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPlot2dDoc)));
	return (CPlot2dDoc*)m_pDocument;
}

// CPlot2dView

IMPLEMENT_DYNCREATE(CPlot2dView, CFormView)

CPlot2dView::CPlot2dView()
	: CFormView(IDD_FORMVIEW_2DPLOT)
	, m_yMax(1)
	, m_yMin(0)
	, m_xMin(0)
	, m_xMax(1)
	, m_byAuto(TRUE)
	, m_bxAuto(TRUE)
	, m_yDivs(10)
	, m_xDivs(10)
	, m_bMarker(FALSE)
	, m_bNeedInit(true)
{

}

CPlot2dView::~CPlot2dView()
{
}

void CPlot2dView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);

	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_yMax);
	DDX_Text(pDX, IDC_EDIT_Y_MIN, m_yMin);
	DDX_Text(pDX, IDC_EDIT_X_MIN, m_xMin);
	DDX_Text(pDX, IDC_EDIT_X_MAX, m_xMax);
	DDX_Check(pDX, IDC_CHECK_Y_AUTO, m_byAuto);
	DDX_Check(pDX, IDC_CHECK_X_AUTO, m_bxAuto);
	DDX_Text(pDX, IDC_EDIT_Y_DIVS, m_yDivs);
	DDX_Text(pDX, IDC_EDIT_X_DIVS, m_xDivs);
}

BEGIN_MESSAGE_MAP(CPlot2dView, CFormView)
	ON_WM_SIZE()
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK_Y_AUTO, &CPlot2dView::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_X_AUTO, &CPlot2dView::OnBnClickedCheckAuto)
	ON_EN_CHANGE(IDC_EDIT_X_MAX, &CPlot2dView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_MIN, &CPlot2dView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_X_DIVS, &CPlot2dView::OnChangeEditXMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MAX, &CPlot2dView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_MIN, &CPlot2dView::OnChangeEditYMinMax)
	ON_EN_CHANGE(IDC_EDIT_Y_DIVS, &CPlot2dView::OnChangeEditYMinMax)
	ON_BN_CLICKED(IDC_BUTTON_ADD_COMMENT, &CPlot2dView::OnBnClickedButtonAddComment)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_COMMENTS, &CPlot2dView::OnBnClickedButtonRemoveComments)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CPlot2dView::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CPlot2dView::OnBnClickedButtonLoad)

END_MESSAGE_MAP()


// CPlot2dView diagnostics

#ifdef _DEBUG
void CPlot2dView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPlot2dView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPlot2dView message handlers
void CPlot2dView::enableControls()
{
	BOOL bxAuto = m_bxAuto;
	BOOL byAuto = m_byAuto;

	double xMin = m_xMin;
	double xMax = m_xMax;
	int xDivs = m_xDivs;
	if (bxAuto)
	{
		xMin = m_wndPlot.m_xAxis.m_min;
		xMax = m_wndPlot.m_xAxis.m_max;
		xDivs = m_wndPlot.m_xAxis.m_divs;
	}

	double yMin = m_yMin;
	double yMax = m_yMax;
	int yDivs = m_yDivs;
	if (byAuto)
	{
		yMin = m_wndPlot.m_yAxis.m_min;
		yMax = m_wndPlot.m_yAxis.m_max;
		yDivs = m_wndPlot.m_yAxis.m_divs;
	}

	UpdateData(TRUE);

	m_xMin = xMin;
	m_xMax = xMax;
	m_xDivs = xDivs;

	m_yMin = yMin;
	m_yMax = yMax;
	m_yDivs = yDivs;

	//GetDlgItem(IDC_CHECK_X_AUTO)->EnableWindow(m_bTimeXAxis ? FALSE : TRUE);
	GetDlgItem(IDC_EDIT_X_MIN)->EnableWindow(!m_bxAuto);
	GetDlgItem(IDC_EDIT_X_MAX)->EnableWindow(!m_bxAuto);
	GetDlgItem(IDC_EDIT_Y_MIN)->EnableWindow(!m_byAuto);
	GetDlgItem(IDC_EDIT_Y_MAX)->EnableWindow(!m_byAuto);

	m_wndPlot.autoScale(m_bxAuto, m_byAuto, TRUE);

	//m_cmbDataType.ShowWindow(m_bShowDataType ? SW_SHOW : SW_HIDE);

	UpdateData(FALSE);
}


// CPlot2dView message handlers


void CPlot2dView::OnSize(UINT nType, int cx, int cy)
{
	CRect r1;
	CFormView::OnSize(nType, cx, cy);
	if (m_bNeedInit)
	{
		if (cx > 0 && cy > 0)
		{
			m_rCrt = CRect(0, 0, cx, cy);
			m_bNeedInit = false;
		}
		return;
	}

	int dx = cx - m_rCrt.Width();
	int dy = cy - m_rCrt.Height();
	GetClientRect(&m_rCrt);

	HDWP hDWP = ::BeginDeferWindowPos(5);

	CRect rectPlot;
	for (CWnd* pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		int nId = pChild->GetDlgCtrlID();
		if (nId == IDC_CUSTOM_PLOT)
		{
			pChild->GetWindowRect(&r1);
			ScreenToClient(r1);
			r1.right = m_rCrt.right - 20; // += dx;
			r1.bottom = m_rCrt.bottom - 60; // += dy;
			rectPlot = r1;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0,
				r1.Width(), r1.Height(),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		}

	}

	for (CWnd* pChild = GetWindow(GW_CHILD);
		pChild != NULL;
		pChild = pChild->GetWindow(GW_HWNDNEXT))
	{
		int nID = pChild->GetDlgCtrlID();
		pChild->GetWindowRect(&r1); ScreenToClient(&r1);

		if (nID == IDC_EDIT_X_MIN)
		{
			CRect r = r1;
			r.left = rectPlot.left;
			r.right = rectPlot.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_X_MAX)
		{
			CRect r = r1;
			r.left = rectPlot.right - r1.Size().cx;
			r.right = rectPlot.right;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_CHECK_X_AUTO)
		{
			CRect r = r1;
			r.left = (rectPlot.right + rectPlot.left) / 2 - r1.Size().cx / 2;
			r.right = (rectPlot.right + rectPlot.left) / 2 + r1.Size().cx / 2;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		if (nID == IDC_EDIT_Y_MIN)
		{
			CRect r = r1;
			r.top = rectPlot.bottom - r1.Size().cy;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_Y_MAX)
		{
			CRect r = r1;
			r.top = rectPlot.top;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_CHECK_Y_AUTO)
		{
			CRect r = r1;
			r.top = (rectPlot.top + rectPlot.bottom) / 2 - r1.Size().cy / 2;
			r.bottom = (rectPlot.top + rectPlot.bottom) / 2 + r1.Size().cy / 2;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_Y_DIVS)
		{
			CRect r = r1;
			r.top = (rectPlot.top + rectPlot.bottom) / 2 + (rectPlot.bottom - rectPlot.top) / 4 - r1.Size().cy / 2;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_EDIT_X_DIVS)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx / 2 - 15 - r1.Size().cx;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_SAVE)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx / 2;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_LOAD)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 - (rectPlot.right - rectPlot.left) / 4 + r1.Size().cx / 2 + 15;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_ADD_COMMENT)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 - r1.Size().cx - 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}
		else if (nID == IDC_BUTTON_REMOVE_COMMENTS)
		{
			CRect r = r1;
			r.left = (rectPlot.left + rectPlot.right) / 2 + (rectPlot.right - rectPlot.left) / 4 + 5;
			r.right = r.left + r1.Size().cx;
			r.top = rectPlot.bottom + 5;
			r.bottom = r.top + r1.Size().cy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		}

	}


	::EndDeferWindowPos(hDWP);

}


void CPlot2dView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(TRUE);

	UpdateData(FALSE);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	GetFont()->GetLogFont(&lf);

	lf.lfHeight = lf.lfHeight * 3 / 2;
	{
		HDC hdc = ::GetDC(NULL);
		int xLogPx = GetDeviceCaps(hdc, LOGPIXELSX);
		lf.lfHeight = (LONG)ceil(18.0 * xLogPx / 72.0);

		::ReleaseDC(NULL, hdc);
	}
	m_font.CreateFontIndirect(&lf);

	m_wndPlot.initControl(this);
	m_wndPlot.setLegendFont(&m_font);
	m_wndPlot.setMenuIDs(IDR_POPUP, 0, NULL);

	m_wndPlot.setAxisPrecision(TRUE, 0);
	m_wndPlot.setAxisPrecision(FALSE, 0);

	m_wndPlot.setTitleFont(lf);

	updateCurves();
}

void CPlot2dView::updateCurves()
{
	CWaitCursor wc;

	if (!IsWindow(m_wndPlot.GetSafeHwnd()))
		return;

	UpdateData(TRUE);
	CPlot2dDoc* pDoc = GetDocument();
	nzg::Plot2dNode* pn = pDoc->m_pn;

	std::vector<nzg::Point2d> pts;
	pn->getCurve(pts);

	m_wndPlot.removeAllCurves();
	m_wndPlot.removeAllCircles();
	m_wndPlot.clearMarkers();

	m_wndPlot.setAxisTitle(FALSE, "A function", FALSE);
	m_wndPlot.resetInitArea();

	m_wndPlot.addCurve(1, "1", pts, 1, nzg::Curve2d::eLineAndPoints, RGB(255, 0, 0));

	m_wndPlot.setInitArea();
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
}


void CPlot2dView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
	{
		KillTimer(nIDEvent);
		updateCurves();
	}
	else if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		enableControls();
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
	else if (nIDEvent == 3)
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		m_wndPlot.setAxis(TRUE, m_xMin, m_xMax, m_xDivs);
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
	else if (nIDEvent == 4)
	{
		KillTimer(nIDEvent);
		UpdateData(TRUE);
		m_wndPlot.setAxis(FALSE, m_yMin, m_yMax, m_yDivs);
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}

	CFormView::OnTimer(nIDEvent);
}

void CPlot2dView::OnBnClickedCheckAuto()
{
	KillTimer(1);
	SetTimer(1, 10, NULL);
}

void CPlot2dView::OnChangeEditXMinMax()
{
	KillTimer(3);
	SetTimer(3, 2000, NULL);
}


void CPlot2dView::OnChangeEditYMinMax()
{
	KillTimer(4);
	SetTimer(4, 2000, NULL);
}

void CPlot2dView::OnBnClickedButtonAddComment()
{
	/*
	CPromptTitleDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	if (dlg.m_strTitle.IsEmpty())
		return;

	CString str = dlg.m_strTitle;
	m_wndPlot.AddInscription(1, str, nzg::Point2d(gl_dNan, gl_dNan), DT_TOP, RGB(0, 0, 0), CInscription::etRect, &m_font);
	*/
}


void CPlot2dView::OnBnClickedButtonRemoveComments()
{
	m_wndPlot.deleteInscription(0);
}


void CPlot2dView::OnBnClickedButtonSave()
{
	AfxMessageBox("Not implemented yet");
	/*
	CFileDialog dlg(FALSE, ".plt", m_strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Plot files (*.plt)|*.plt|All files (*.*)|*.*||", this);

	if (dlg.DoModal() != IDOK)
		return;
	m_strFileName = dlg.GetPathName();

	std::ofstream ofs(m_strFileName);
	if (!ofs.good())
	{
		AfxMessageBox("Can't save to file");
		return;
	}

	try
	{
		nzg::Archive ar(ofs, nzg::Archive::store);
		m_wndPlot.serialize(ar);
	}
	catch (std::exception e)
	{
		AfxMessageBox(e.what());
	}
	*/
}


void CPlot2dView::OnBnClickedButtonLoad()
{
	AfxMessageBox("Not implemented yet");
	/*
	CFileDialog dlg(TRUE, ".plt", m_strFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Plot files (*.plt)|*.plt|All files (*.*)|*.*||", this);

	if (dlg.DoModal() != IDOK)
		return;
	m_strFileName = dlg.GetPathName();

	std::ifstream ifs(m_strFileName);
	if (!ifs.good())
	{
		AfxMessageBox("Can't open source file");
		return;
	}

	try
	{
		nzg::Archive ar(ifs, nzg::Archive::load);
		m_wndPlot.serialize(ar);
		m_wndPlot.Invalidate();
	}
	catch (std::exception e)
	{
		AfxMessageBox(e.what());
	}
	*/
}




void CPlot2dView::OnCurveFormat()
{
	//static COLORREF clrs[9] = { RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255),
	//	RGB(200, 0, 200), RGB(125, 125, 0), RGB(0, 255, 255),
	//	RGB(120, 0, 0), RGB(0, 120, 0), RGB(255, 128, 0) };

	/*
	std::vector<COLORREF> colors;
	for (int i = 0; i < nzg::Plot2d::c_stdColorsNum; i++)
		colors.push_back(nzg::Plot2d::getStdColor(i));

	CCurveFormatPg pgCurve;
	pgCurve.m_pPlot = &m_wndPlot;
	pgCurve.m_colors = colors;

	CPropertySheet dlg("Plot properties");


	dlg.AddPage(&pgCurve);

	if (dlg.DoModal() != IDOK)
		return;
	m_wndPlot.Invalidate();
	m_wndPlot.UpdateWindow();
	*/
}