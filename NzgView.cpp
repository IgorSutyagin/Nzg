// NzgView.cpp : implementation file
//

#include "pch.h"
#include "Nzg.h"
#include "NzgView.h"
#include "NzgDoc.h"
#include "MainFrm.h"


CFileView* getFileView()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	return &(pFrm->m_wndFileView);
}

CMainFrame* getMainFrame()
{
	CMainFrame* pFrm = (CMainFrame*)AfxGetMainWnd();

	return pFrm;
}

CNzgApp* getNzgApp()
{
	return (CNzgApp*)AfxGetApp();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CNzgCtrl implementation

///////////////////////////////
// Overrides
nzg::OglSurface::Grid CNzgCtrl::getGrid() const
{
	nzg::OglSurface::Grid grid;
	CNzgDoc* pd = m_pView->getDocument();
	if (pd->m_node == nullptr)
		return grid;

	nzg::NzgNode& n = *pd->m_node;

	return nzg::OglSurface::Grid(0, (float)n.sts.getCols()-1, n.sts.getCols(), 0, (float)n.sts.getRows()-1, n.sts.getRows());
}

nzg::Node* CNzgCtrl::getNode() const
{
	CNzgDoc* pd = m_pView->getDocument();
	return pd->m_node;
}

void CNzgCtrl::updateData()
{
	nzg::Node* pn = getNode();
	if (pn == nullptr)
		return;
	if (!pn->isNzg())
		return;

	updateScale();

	m_axis[2].title = nzg::stringFormat("%s", pn->getName().c_str());

	nzg::NzgNode* pnzg = (nzg::NzgNode*)pn;
	int nx = pnzg->sts.getCols();
	int ny = pnzg->sts.getRows();

	//updateSurface(1);
	m_oglColumns.reset();
	m_oglColumns.m_nVerts = nx * ny;
	m_oglColumns.m_pfVerts = new float[nx * ny * 3];
	m_oglColumns.m_pdwColors = new DWORD[nx * ny];
	m_oglColumns.m_size = nzg::Size2d(1, 1);

	for (int x = 0; x < nx; x++)
	{
		for (int y = 0; y < ny; y++)
		{
			double z = pnzg->sts(y, x)->totalScore;
			int nv = y * nx + x;
			m_oglColumns.m_pfVerts[nv * 3] = x + 0.5F;
			m_oglColumns.m_pfVerts[nv * 3+1] = y + 0.5F;
			m_oglColumns.m_pfVerts[nv * 3 + 2] = (float)z;
			m_oglColumns.m_pdwColors[nv] = pnzg->sts(y, x)->getColor();
		}
	}

	Invalidate();
}


double CNzgCtrl::getData(const nzg::Node* node, double x, double y) const
{
	nzg::NzgNode* pn = (nzg::NzgNode*)node;
	int col = (int)(x / pn->sts.getCols());
	int row = (int)(y / pn->sts.getRows());
	const nzg::Strat* ps = pn->sts(row, col).get();
	return ps->totalScore;
}

void CNzgCtrl::onViewChanged()
{
	nzg::Plot3d::onViewChanged();
	//m_pView->onViewChanged();
}

void CNzgCtrl::onDraw()
{
	m_oglColumns.draw();
	drawLegend();
}

void CNzgCtrl::updateScale()
{
	m_cubeScene = nzg::Cube3d(nzg::Point3d(DBL_MAX, DBL_MAX, DBL_MAX), nzg::Point3d(-DBL_MAX, -DBL_MAX, -DBL_MAX));

	nzg::OglSurface::Grid grid = getGrid();
	nzg::Node* node = getNode();
	//nzg::Gnss::Signal es = getSignal();
	//if (!grid.isValid() || node == nullptr || es == nzg::Gnss::esigInvalid)
	//	return;
	nzg::Point2d step = grid.getStep();

	for (double x = grid.xMin; x < grid.xMax + step.x / 2; x += step.x)
	{
		for (double y = grid.yMin; y < grid.yMax + step.y / 2; y += step.y)
		{
			double z = getData(node, x, y);
			m_cubeScene.onMinMax(nzg::Point3d(x, y, z));
		}
	}

	m_cubeScene.onMinMax(nzg::Point3d(0, 0, 1));

}

void CNzgCtrl::drawLegend()
{
	//if (!m_vp.bRainbow)
	//	return;

	glPushMatrix();

	CRect r;
	GetClientRect(r);
	float w = (float)r.Width() / 8;
	float h = (float)(r.Height());
	glViewport(0, 0, (int)w, (int)h); // m_rectClient.Width(), m_rectClient.Height());

	// Reset The Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	nzg::Size3d size(1, 1, 1); // = (m_ptMax - m_ptMin) / 2;
	if (w > h)
	{
		size.cx *= w / h;
		glOrtho(-size.cx, size.cx, -size.cy, size.cy, -100.0f, 100.0f);
	}
	else
	{
		size.cy *= h / w;
		glOrtho(-size.cx, size.cx, -size.cy, size.cy, -100.0f, 100.0f);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	glLoadIdentity();

	std::map<DWORD, std::string> types;

	nzg::NzgNode* node = (nzg::NzgNode *)getNode();
	for (int i = 0; i < node->sts.getRows(); i++)
	{
		for (int j = 0; j < node->sts.getCols(); j++)
		{
			nzg::Strat* p = node->sts(i, j).get();

			DWORD clr = p->getColor();
			if (types.find(clr) == types.end())
			{
				types[clr] = p->c_types[(int)p->type];
			}
		}
	}


	int nSec = types.size();
	double step = size.cy / (nSec + 2);

	int i = 0;
	for (auto& it : types)
	{
		nzg::Point2d pt(-size.cx * 0.8, i * step - size.cy * 0.5);

		//double d = i * valStep + range.first;
		DWORD dwColor = it.first;

		glColor4d(GetRValue(dwColor) / 255.0, GetGValue(dwColor) / 255.0, GetBValue(dwColor) / 255.0, 1.0);

		glBegin(GL_QUADS);
		glVertex3d(pt.x, pt.y, 0);
		glVertex3d(-size.cx * 0.3, pt.y, 0);
		glVertex3d(-size.cx * 0.3, pt.y + step, 0);
		glVertex3d(pt.x, pt.y + step, 0);
		glEnd();
		i++;

	}

	double fontScale = 0.3;
	//double fontHeight = m_oglFont.m_gmf['0'].gmfBlackBoxY * fontScale;

	i = 0;
	for (auto& it : types)
	{
		nzg::Point2d pt(-size.cx * 0.8, i * step - size.cy * 0.5);
		//double d = i * valStep + range.first;
		glPushMatrix();
		{
			glColor4d(0.0, 0.0, 0.0, 1.0);
			glTranslated(-size.cx * 0.2, pt.y, 0); // -0.5 * fontHeight, 0); //  + 0.2 * step
			glScaled(fontScale, fontScale, 0);
			//glListBase(1000);
			std::string str = it.second;
			m_oglFont.print(str, DT_BOTTOM);
			//glCallLists(str.length(), GL_UNSIGNED_BYTE, str.c_str());
		}
		glPopMatrix();

		i++;
	}

	glPopMatrix();

}


// End of CNzgCtrl implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////


// CNzgView

IMPLEMENT_DYNCREATE(CNzgView, CFormView)

CNzgView::CNzgView()
	: CFormView(IDD_FORMVIEW_NZG), m_wndPlot(this), m_bNeedInit(true), m_rows(10), m_cols(10)
{

}

CNzgView::~CNzgView()
{
}

void CNzgView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_PLOT, m_wndPlot);
	DDX_Text(pDX, IDC_EDIT_ROWS, m_rows);
	DDX_Text(pDX, IDC_EDIT_COLS, m_cols);
	//DDX_Control(pDX, IDC_COMBO_SIGNAL, m_cmbSignal);
	//DDX_Control(pDX, IDC_COMBO_VIEW_AT, m_cmbViewAt);
	//DDX_Control(pDX, IDC_COMBO_LABELS, m_cmbLabels);
}

BEGIN_MESSAGE_MAP(CNzgView, CFormView)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CNzgView::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PLAY2, &CNzgView::OnBnClickedButtonPlay2)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CNzgView::OnBnClickedButtonReset)
	ON_EN_CHANGE(IDC_EDIT_ROWS, onChangeSize)
	ON_EN_CHANGE(IDC_EDIT_COLS, onChangeSize)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CNzgView diagnostics

#ifdef _DEBUG
void CNzgView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CNzgView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CNzgView message handlers
void CNzgView::OnSize(UINT nType, int cx, int cy)
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
			r1.bottom = m_rCrt.bottom - 30; // += dy;
			::DeferWindowPos(hDWP, pChild->m_hWnd, NULL, 0, 0,
				r1.Width(), r1.Height(),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		}

	}

	::EndDeferWindowPos(hDWP);

}


void CNzgView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(TRUE);

	UpdateData(FALSE);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	GetFont()->GetLogFont(&lf);

	m_wndPlot.onInitialUpdate();

	m_wndPlot.updateData();
}


void CNzgView::OnBnClickedButtonPlay()
{
	nzg::NzgNode* node = getNode();

	node->updateStrat();
	node->resetTotalScores();
	node->play();
	m_wndPlot.updateData();
	m_wndPlot.Invalidate();
}

void CNzgView::OnBnClickedButtonPlay2()
{
	nzg::NzgNode* node = getNode();

	for (int i = 0; i < 100; i++)
	{
		node->updateStrat();
		node->resetTotalScores();
		node->play();
		m_wndPlot.updateData();
		m_wndPlot.Invalidate();
		m_wndPlot.UpdateWindow();
	}
}

void CNzgView::OnBnClickedButtonReset()
{
	nzg::NzgNode* node = getNode();

	UpdateData(TRUE);

	node->reset(m_rows, m_cols, nzg::NzgNode::MapType::random);
	m_wndPlot.updateData();
	m_wndPlot.Invalidate();
}

void CNzgView::onChangeSize()
{
	SetTimer(1, 1000, NULL);
}

void CNzgView::OnTimer(UINT nTimerID)
{
	if (nTimerID == 1)
	{
		KillTimer(1);
		OnBnClickedButtonReset();
	}
}

