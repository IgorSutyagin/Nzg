////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The nzg software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including nzg as 
//  long as they comply with the license.BSD 2 - Clause License
// 
//  Copyright(c) 2024, TOPCON, All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met :
// 
//  1. Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and /or other materials provided with the distribution.
// 
//  3. The software package includes some companion executive binaries or shared 
//  libraries necessary to execute APs on Windows. These licenses succeed to the 
//  original ones of these software.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// 	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// 	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// 	OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// 	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include "pch.h"

#include "Nzg.h"
#include "Plot3d.h"
#include "Settings.h"

using namespace nzg;

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plot3d camera position implementation
void Plot3dCamera::serialize(Archive& ar)
{
	DWORD dwVer = 1;
	if (ar.isStoring())
	{
		ar << dwVer;
		ar << rot;
		ar << rotTotal;
		ar << off;
		ar << offTotal;
		ar << scale;
	}
	else
	{
		ar >> dwVer;
		ar >> rot;
		ar >> rotTotal;
		ar >> off;
		ar >> offTotal;
		ar >> scale;
	}
}
// End of Plot3d camera position implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plot3dParams implementation
void Plot3dParams::serialize(Archive& ar)
{
	DWORD dwVer = 1;
	if (ar.isStoring())
	{
		ar << dwVer;
		ar << zMin;
		ar << zMax;
		ar << zStep;
		ar << isoStep;
		ar << colorStep;
		ar.write(&viewAt, sizeof(viewAt));
		ar.write(&coords, sizeof(coords));
		ar << bGrid0;
		ar << bRainbow;
		ar.write(&eLabels, sizeof(eLabels));
	}
	else
	{
		ar >> dwVer;
		ar >> zMin;
		ar >> zMax;
		ar >> zStep;
		ar >> isoStep;
		ar >> colorStep;
		ar.read(&viewAt, sizeof(viewAt));
		ar.read(&coords, sizeof(coords));
		ar >> bGrid0;
		ar >> bRainbow;
		ar.read(&eLabels, sizeof(eLabels));
	}
}
// End of Plot3dParams implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plot3d implementation

const Size3d Plot3dCamera::c_rots[ecMax][evaCustom] = {
	{ Size3d(-60, 0, -135), Size3d(-90, 0, 270), Size3d(0, 0, 0) },
	{ Size3d(-60, 0, 0), Size3d(-90, 0, 270), Size3d(0, 0, 0) }
};

//Plot3d::ViewAt Plot3d::c_lastView[2] = {
//	{ Size3d(0, 0, 0), c_rots[0][OglViewParams::evaDef], Size3d(0, 0, 0), Size3d(0, 0, 0), 1.0 },
//	{ Size3d(0, 0, 0), c_rots[1][OglViewParams::evaTop], Size3d(0, 0, 0), Size3d(0, 0, 0), 1.0 }
//};
//OglViewParams Plot3d::c_lastVp;

///////////////////////////
// Construction

Plot3d::Plot3d() : m_hDC(NULL), m_hRC(NULL), m_nPixelFormat(0), m_nSymbolListBase(0)
{
	WNDCLASS wndclass;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, IVSPLOT3D_CLASSNAME, &wndclass)))
	{
		wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ::DefWindowProc;
		wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInst;
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor(hInst, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = IVSPLOT3D_CLASSNAME;

		if (!AfxRegisterClass(&wndclass))
			AfxThrowResourceException();
	}

	m_axis[0] = nzg::OglAxis(nzg::OglAxis::X, 0, 360, 90);
	m_axis[1] = nzg::OglAxis(nzg::OglAxis::Y, 0, 90, 10);
	m_axis[2] = nzg::OglAxis(nzg::OglAxis::Z, -20, 20, 10);

	m_axis[0].title = "Azimuth (deg)";
	m_axis[1].title = "Zenith (deg)";

	//loadView();
	//m_va[m_vp.bPolar ? 1 : 0].rotTotal = Size3d(-60, 0, -135);
	//m_va[m_vp.bPolar ? 1 : 0].offTotal = c_lastView.offTotal;
	//m_scale = c_lastView.scale;
}

Plot3d::~Plot3d()
{
}

void Plot3d::reset()
{
	for (int i = 0; i < (int)m_surfs.size(); i++)
	{
		delete m_surfs[i];
	}
	m_surfs.clear();
}

///////////////////////////////////////
// Operations
void Plot3d::setViewParams(const Plot3dParams& params) {
	m_vp = params;
	m_axis[2].fmin = params.zMin;
	m_axis[2].fmax = params.zMax;
	m_axis[2].step = params.zStep;
	Plot3dCamera& va = m_va[m_vp.coords]; // m_vp.bPolar ? m_va[1] : m_va[0];
	if (Plot3dCamera::evaDef <= m_vp.viewAt && m_vp.viewAt < Plot3dCamera::evaCustom)
	{
		m_va[m_vp.coords].rotTotal = Plot3dCamera::c_rots[m_vp.coords][params.viewAt];
	}
	saveView();
}

bool Plot3d::isTopView() const 
{
	return m_va[m_vp.coords].rotTotal == Plot3dCamera::c_rots[m_vp.coords][Plot3dCamera::evaTop];
}
bool Plot3d::isFrontView() const 
{
	return m_va[m_vp.coords].rotTotal == Plot3dCamera::c_rots[m_vp.coords][Plot3dCamera::evaFront];
}
bool Plot3d::isDefaultView() const 
{
	return m_va[m_vp.coords].rotTotal == Plot3dCamera::c_rots[m_vp.coords][Plot3dCamera::evaDef];
}

BEGIN_MESSAGE_MAP(Plot3d, CWnd)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void Plot3d::OnPaint()
{
	CPaintDC dc(this);

	if (m_hDC == NULL)
	{
		if (!setupPixelFormat())
			return;
	}

	wglMakeCurrent(m_hDC, m_hRC);

	setProjection();


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Size3d off = m_va[m_vp.coords].offTotal + m_va[m_vp.coords].off;
	Size3d rot = m_va[m_vp.coords].rotTotal + m_va[m_vp.coords].rot;


	glTranslated(off.cx, off.cy, off.cz);
	glRotated(rot.cx, 1, 0, 0);
	glRotated(rot.cy, 0, 1, 0); 
	glRotated(rot.cz, 0, 0, 1);

	setupLight();

	nzg::Point3d scale = getScale();
	glScaled(scale.x, scale.y, scale.z);


	onDraw();


	SwapBuffers(m_hDC);
}

void Plot3d::onDraw()
{
	Size3d rot = m_va[m_vp.coords].rotTotal + m_va[m_vp.coords].rot;
	nzg::Point3d scale = getScale();
	for (int i = 0; i < (int)m_surfs.size(); i++)
	{
		m_surfs[i]->draw(scale, rot, m_vp, m_oglFont);
	}

	glColor3f(0, 0, 0);
	if (m_vp.coords == Plot3dCamera::ecPolar)
		glLineWidth(2.0);
	else
		glLineWidth(1.0);
	if (m_vp.coords == Plot3dCamera::ecPolar)
	{
		m_axis[2].drawPolar(scale, rot, m_vp, m_oglFont);
	}
	else
	{
		for (int i = 0; i < 3; i++)
			m_axis[i].draw(m_axis, scale, rot, m_vp, m_oglFont);
	}

	drawRainbow();

}


void Plot3d::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);
	SetCapture();
	m_ptHit = point;
}


void Plot3d::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
	Plot3dCamera& va = m_va[m_vp.coords];
	va.rotTotal += va.rot;
	va.rot = Size3d(0, 0, 0);

	va.offTotal += va.off;
	//c_lastView[m_vp.bPolar].offTotal = va.offTotal;
	va.off = Size3d(0, 0, 0);
	ReleaseCapture();
	onViewChanged();
	saveView();
}


void Plot3d::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);
	if (GetFocus() != this)
		SetFocus();

	if (GetCapture() == this && MK_LBUTTON & nFlags)
	{
		BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
		BOOL bControl = GetKeyState(VK_CONTROL) & 0x8000;
		BOOL bAlt = GetKeyState(VK_LMENU) & 0x8000 || GetKeyState(VK_RMENU) & 0x8000;

		CSize s = point - m_ptHit;
		Plot3dCamera& va = m_va[m_vp.coords];
		if (bShift)
		{
			va.off = Size3d(s.cx / 1000.0F, -s.cy / 1000.0F, 0);
		}
		else if (bControl && bAlt)
		{
			va.rot = Size3d(0, 0, s.cy / 10.0F);
		}
		else if (bAlt)
		{
			double sq = (point.x * m_ptHit.y - point.y * m_ptHit.x);
			double sina = sq / (sqrt(point.x * point.x + point.y * point.y) * sqrt(m_ptHit.x * m_ptHit.x + m_ptHit.y * m_ptHit.y));
			double a = asin(sina) * 180 / nzg::PI;
			va.rot = Size3d(a, 0, 0);
		}
		else if (bControl)
		{
			double sq = (point.x * m_ptHit.y - point.y * m_ptHit.x);
			double sina = sq / (sqrt(point.x * point.x + point.y * point.y) * sqrt(m_ptHit.x * m_ptHit.x + m_ptHit.y * m_ptHit.y));
			double a = asin(sina) * 180 / nzg::PI;
			va.rot = Size3d(0, a, 0);
		}
		else
		{
			va.rot = Size3d(s.cy / 10.0F, s.cx / 10.0F, 0);
		}
		Invalidate(FALSE);
		return;
	}
}


BOOL Plot3d::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nTimes = -zDelta / WHEEL_DELTA;
	ScreenToClient(&pt);

	m_va[m_vp.coords].scale *= (100.0 - nTimes) / 100.0;
	saveView();
	Invalidate(FALSE);

	return TRUE;
}


void Plot3d::OnDestroy()
{
	CWnd::OnDestroy();

	::wglDeleteContext(m_hRC);
}


void Plot3d::onInitialUpdate()
{
	CFont* pFont = GetParent()->GetFont();

	//LOGFONT lf;
	memset(&m_lf, 0, sizeof(m_lf));

	if (pFont != NULL)
		pFont->GetLogFont(&m_lf);
	else
	{
		NONCLIENTMETRICS ncm;
		memset(&ncm, 0, sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

		memmove(&m_lf, &ncm.lfMessageFont, sizeof(m_lf));
	}

	CFont font;
	strncpy_s(m_lf.lfFaceName, "Arial", LF_FACESIZE);
	m_lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
	BOOL bOk = font.CreateFontIndirect(&m_lf);

	font.GetLogFont(&m_lf);
}


void Plot3d::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}


void Plot3d::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPUP);
	int index = getMenuIndex();
	if (index < 0)
		return;
	HMENU hMenu = menu.GetSubMenu(index)->Detach();
	theApp.GetContextMenuManager()->ShowPopupMenu(hMenu, point.x, point.y, this, TRUE);
}


void Plot3d::OnEditCopy()
{
	snapClient();
}

////////////////////////////////
// Overrides

void Plot3d::updateData()
{
	Node* pn = getNode();
	if (pn != nullptr)
	{
		m_axis[2].title = stringFormat("%s", pn->getName().c_str());
	}
	updateSurface(1);
	Invalidate();
}

void Plot3d::updateSurface(int id)
{
	updateScale();

	nzg::OglSurface::Grid grid = getGrid();
	nzg::Node* node = getNode();
	//nzg::Gnss::Signal es = getSignal();
	//if (!grid.isValid() || node == nullptr || es == nzg::Gnss::esigInvalid)
	//	return;
	nzg::OglSurface* ps = nullptr;
	auto its = std::find_if(m_surfs.begin(), m_surfs.end(), [=](const nzg::OglSurface* p) { return p->m_nID == id; });
	if (its != m_surfs.end())
	{
		(*its)->fromGrid(grid);
		ps = *its;
	}
	else
	{
		ps = new nzg::OglSurface(id, grid);
		m_surfs.push_back(ps);
	}

	nzg::Point2d step = grid.getStep();

	nzg::Point3d normScale(m_cubeScene.size());
	double minColor = 0;
	double maxColor = 1;
	std::pair<double, double> colorRange = getColorRange();

	int triangle = 0;
	int row = 0;
	double polRad = 100;
	for (double y = grid.yMin; y < grid.yMax + step.y / 2; y += step.y, row++)
	{
		int col = 0;
		for (double x = grid.xMin; x < grid.xMax + step.x / 2; x += step.x, col++)
		{
			double z = getData(node, x, y);
			if (m_vp.coords == Plot3dCamera::ecPolar)
			{
				Point3d pt = y > 0 ? ZenAz (y, x).dirCos() / sin(y*PI/180) * y / 90: Point3d(0, 0, 0);
				ps->m_pfVerts[3 * (row * grid.nx) + 3 * col] = (float)pt.x;
				ps->m_pfVerts[3 * (row * grid.nx) + 3 * col + 1] = (float)pt.y;
			}
			else
			{
				ps->m_pfVerts[3 * (row * grid.nx) + 3 * col] = (float)x;
				ps->m_pfVerts[3 * (row * grid.nx) + 3 * col + 1] = (float)y;
			}
			ps->m_pfVerts[3 * (row * grid.nx) + 3 * col + 2] = (float)z;
			ps->m_pdwColors[(row * grid.nx) + col] = nzg::rainbowColor(z, colorRange.first, colorRange.second, 0xFF, 120, 240, m_vp.colorStep);

			if (row > 0 && col > 0)
			{
				//
				// nCurVert-nx-1     nCurVert-nx
				//     X------------------X
				//     |                 /|
				//     |               /  |
				//     |             /    |
				//     |           /      |
				//     |         /        |
				//     |       /          |
				//     |     /            |
				//     |   /              |
				//     | /                |
				//     X------------------X
				// nCurVert-1          nCurVert
				//

				int nCurVert = (row * grid.nx) + col;
				int nIndex = triangle;
				ps->m_pnIndexes[triangle++] = 3 * (nCurVert - 1);
				ps->m_pnIndexes[triangle++] = 3 * (nCurVert - grid.nx - 1);
				ps->m_pnIndexes[triangle++] = 3 * (nCurVert - grid.nx);

				ps->m_pnIndexes[triangle++] = 3 * (nCurVert - 1);
				ps->m_pnIndexes[triangle++] = 3 * (nCurVert - grid.nx);
				ps->m_pnIndexes[triangle++] = 3 * (nCurVert);

				// Calc normals for every vertex as average of join rectangles
				float fn[3];
				ps->calcNormal(nIndex, TRUE, fn, normScale);

				ps->m_pfNorms[3 * (nCurVert - 1)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert - 1) + 1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert - 1) + 2] += fn[2];

				ps->m_pfNorms[3 * (nCurVert - grid.nx - 1)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert - grid.nx - 1) + 1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert - grid.nx - 1) + 2] += fn[2];

				ps->m_pfNorms[3 * (nCurVert - grid.nx)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert - grid.nx) + 1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert - grid.nx) + 2] += fn[2];


				ps->calcNormal(nIndex + 3, TRUE, fn, normScale);

				ps->m_pfNorms[3 * (nCurVert - 1)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert - 1) + 1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert - 1) + 2] += fn[2];

				ps->m_pfNorms[3 * (nCurVert - grid.nx)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert - grid.nx) + 1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert - grid.nx) + 2] += fn[2];

				ps->m_pfNorms[3 * (nCurVert)] += fn[0];
				ps->m_pfNorms[3 * (nCurVert)+1] += fn[1];
				ps->m_pfNorms[3 * (nCurVert)+2] += fn[2];

				if (col == grid.nx - 1)
				{
					float fx, fy, fz;
					fx = ps->m_pfNorms[3 * (nCurVert - grid.nx)] + ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1)];
					fy = ps->m_pfNorms[3 * (nCurVert - grid.nx) + 1] + ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1) + 1];
					fz = ps->m_pfNorms[3 * (nCurVert - grid.nx) + 2] + ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1) + 2];

					ps->m_pfNorms[3 * (nCurVert - grid.nx)] = ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1)] = fx;
					ps->m_pfNorms[3 * (nCurVert - grid.nx) + 1] = ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1) + 1] = fy;
					ps->m_pfNorms[3 * (nCurVert - grid.nx) + 2] = ps->m_pfNorms[3 * (nCurVert - grid.nx - grid.nx + 1) + 2] = fz;
				}
			}

		}
	}

	// Normalize normals:
	row = 0;
	for (double y = grid.yMin; y < grid.yMax + step.y / 2; y += step.y, row++)
	{
		int col = 0;
		for (double x = grid.xMin; x < grid.xMax + step.x / 2; x += step.x, col++)
		{
			float* pfNorm = ps->m_pfNorms + 3 * (row * grid.nx) + 3 * col;
			nzg::OglSurface::normalize(pfNorm);
		}
	}

	updateIsolines(id);
}

void Plot3d::updateIsolines(int id)
{
	auto its = std::find_if(m_surfs.begin(), m_surfs.end(), [=](const nzg::OglSurface* p) { return p->m_nID == id; });
	if (its == m_surfs.end())
		return;
	nzg::OglSurface* ps = *its;

	ps->resetIsolines();
	double isoStep = getIsoStep();
	for (double z = 0; z <= m_cubeScene.pt1.z; z += isoStep)
	{
		ps->createIsoline((float)z, (float)isoStep / 20000, (float)isoStep / 250, 0.1, isoStep, 1.0);
	}

	for (double z = -isoStep; z >= m_cubeScene.pt0.z; z -= isoStep)
	{
		ps->createIsoline((float)z, (float)isoStep / 20000, (float)isoStep / 250, 0.1, isoStep, 1.0);
	}
}

void Plot3d::updateScale()
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
			if (m_vp.coords == Plot3dCamera::ecPolar)
			{
				double a = sin(y * PI / 180) * cos(PI * (90 - x) / 180);
				double b = sin(y * PI / 180) * sin(PI * (90 - x) / 180);
				m_cubeScene.onMinMax(nzg::Point3d(a, b, z));
			}
			else
				m_cubeScene.onMinMax(nzg::Point3d(x, y, z));
		}
	}

	if (m_vp.coords == Plot3dCamera::ecPolar)
	{
		Point3d ptMin = m_axis[2].getMinPt();
		m_cubeScene.onMinMax(Point3d(0, 0, ptMin.z));
		Point3d ptMax = m_axis[2].getMaxPt();
		m_cubeScene.onMinMax(Point3d(0, 0, ptMax.z));
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			m_cubeScene.onMinMax(m_axis[i].getMinPt());
			m_cubeScene.onMinMax(m_axis[i].getMaxPt());
		}
	}
}

void Plot3d::onViewChanged()
{
	if (isDefaultView())
		m_vp.viewAt = Plot3dCamera::evaDef;
	else if (isTopView())
		m_vp.viewAt = Plot3dCamera::evaTop;
	else if (isFrontView())
		m_vp.viewAt = Plot3dCamera::evaCustom;
	else
		m_vp.viewAt = Plot3dCamera::evaCustom;
}
////////////////////////////////
// Implementation:

void Plot3d::saveView() const
{
	gl_settings.savePlot3d(getNode(), this);
}

void Plot3d::loadView()
{
	gl_settings.loadPlot3d(getNode(), this);
	m_axis[2].fmin = m_vp.zMin;
	m_axis[2].fmax = m_vp.zMax;
	m_axis[2].step = m_vp.zStep;
	onViewChanged();
}

bool Plot3d::setupPixelFormat()
{
	GLuint	PixelFormat;
	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number (?)
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
		24,									// Select A 16Bit Color Depth
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored (?)
		0,									// No Alpha Buffer
		0,									// Shift Bit Ignored (?)
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored (?)
		16,									// 16Bit Z-Buffer (Depth Buffer)  
		0,									// No Stencil Buffer
		0,									// No Auxiliary Buffer (?)
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved (?)
		0, 0, 0								// Layer Masks Ignored (?)
	};


	m_hDC = ::GetDC(m_hWnd);				
	PixelFormat = ChoosePixelFormat(m_hDC, &pfd);

	if (!PixelFormat)
	{
		AfxMessageBox("Can't Find A Suitable PixelFormat.");
		return FALSE;
	}

	if (!SetPixelFormat(m_hDC, PixelFormat, &pfd))
	{
		AfxMessageBox("Can't Set The PixelFormat.");
		return FALSE;
	}
	m_nPixelFormat = PixelFormat;

	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
	{
		AfxMessageBox("Can't Create A GL Rendering Context.");
		return FALSE;
	}

	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		AfxMessageBox("Can't activate GLRC.");
		return FALSE;
	}

	m_oglFont.create(m_lf, m_hDC, 1000);

	glInit();

	return TRUE;

}

void Plot3d::glInit() const
{
	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	//glDepthRange(1.0f, 0.0f);
	glDepthRange(0.0f, 1.0f);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);

	//setProjection();

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glShadeModel(GL_SMOOTH);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
}

void Plot3d::setProjection()
{
	CRect rectClient;
	GetClientRect(rectClient);
	if (isRainbow())
		glViewport(rectClient.Width() / 8, 0, rectClient.Width(), rectClient.Height());
	else
		glViewport(0, 0, rectClient.Width(), rectClient.Height());

	float xf = rectClient.Width() > rectClient.Height() ? rectClient.Width() / (float)rectClient.Height() : 1.0F;
	float yf = rectClient.Width() > rectClient.Height() ? 1.0F : rectClient.Height() / (float)rectClient.Width();


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-1.0F * xf, 1.0F * xf, -1.0F * yf, 1.0F * yf, -2.5F, 2.5F);

	//m_oglFont.m_scale = rectClient.Width() / (2 * xf);

}


void Plot3d::setupLight() const
{
	if (isLight())
	{
		GLfloat LightAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat LightDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat LightSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat LightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };
		GLfloat LightDirection[] = { -1.0f, -1.0f, -1.0f };

		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT); // GL_SMOOTH);

		// Create a light and enable it.
		glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
		//glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);

		//glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);

		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, LightDirection);
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0f);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 128.0f);
		//glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);
		//glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1);
		//glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 1.0);

		GLfloat specref[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specref);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);

		glEnable(GL_LIGHT1);

		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // GL_SPECULAR); //  
		glEnable(GL_COLOR_MATERIAL);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1);
		//glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}

nzg::Point3d Plot3d::getScale() const
{
	nzg::Size3d s = m_cubeScene.size();
	return nzg::Point3d(1.0 / s.cx, 1.0 / s.cy, 1.0 / s.cz) * m_va[m_vp.coords].scale;
}

///////////////////////////////
// Implementation

#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif

void Plot3d::snapClient()
{
	BeginWaitCursor();

	HANDLE handle = snapToMemory();
	if (handle != NULL)
	{
		OpenClipboard();
		EmptyClipboard();
		SetClipboardData(CF_DIB, handle);
		CloseClipboard();
	}

	EndWaitCursor();
}

HANDLE Plot3d::snapToMemory()
{
	CRect rect;
	GetClientRect(&rect);
	CSize size(rect.Width(), rect.Height());
	TRACE("  client zone : (%d;%d)\n", size.cx, size.cy);
	size.cx -= size.cx % 4;
	rect.right = size.cx;
	rect.bottom = size.cy;

	int nWidthInMM = getCopyWidthMM();
	if (nWidthInMM > 0)
	{

		int nWidth = (int)(getSP().ppmm * nWidthInMM * 2);
		nWidth -= nWidth % 4;
		int nHeight = m_rectWalls.Height() * 20 / 19;
		CPoint ptCenter((m_rectWalls.left + m_rectWalls.right) / 2, (m_rectWalls.top + m_rectWalls.bottom) / 2);
		rect.left = ptCenter.x - nWidth / 2;
		rect.right = ptCenter.x + nWidth / 2;
		rect.top = ptCenter.y - nHeight / 2;
		rect.bottom = ptCenter.y + nHeight / 2;

		size = rect.Size();
	}

	TRACE("  final client zone : (%d;%d)-(%d;%d) Size(%d;%d)\n", rect.left, rect.top, rect.right, rect.bottom, size.cx, size.cy);

	Invalidate();
	UpdateWindow();

	CBitmap bitmap;
	CDC* pDC = GetDC();
	CDC MemDC;
	ASSERT(MemDC.CreateCompatibleDC(NULL));
	ASSERT(bitmap.CreateCompatibleBitmap(pDC, size.cx, size.cy));
	MemDC.SelectObject(&bitmap);

	int NbBytes = 3 * size.cx * size.cy;
	unsigned char* pPixelData = new unsigned char[NbBytes];

	::glReadPixels(rect.left, rect.top, size.cx, size.cy, GL_BGR, GL_UNSIGNED_BYTE, pPixelData);

	BITMAPINFOHEADER header;
	header.biWidth = size.cx;
	header.biHeight = size.cy;
	header.biSizeImage = NbBytes;
	header.biSize = 40;
	header.biPlanes = 1;
	header.biBitCount = 3 * 8;
	header.biCompression = 0;
	header.biXPelsPerMeter = (int)(getSP().ppmm * 2000);
	header.biYPelsPerMeter = (int)(getSP().ppmm * 2000);
	header.biClrUsed = 0;
	header.biClrImportant = 0;

	HANDLE handle = (HANDLE)::GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + NbBytes);
	if (handle != NULL)
	{
		char* pData = (char*) ::GlobalLock((HGLOBAL)handle);
		if (pData != nullptr)
		{
			memcpy(pData, &header, sizeof(BITMAPINFOHEADER));
			memcpy(pData + sizeof(BITMAPINFOHEADER), pPixelData, NbBytes);
		}
		::GlobalUnlock((HGLOBAL)handle);
	}

	MemDC.DeleteDC();
	bitmap.DeleteObject();
	delete[] pPixelData;

	return handle;
}

void Plot3d::drawRainbow()
{
	if (!m_vp.bRainbow)
		return;

	glPushMatrix();

	CRect r;
	GetClientRect(r);
	float w = (float)r.Width() / 8;
	float h = (float)(r.Height());
	glViewport(0, 0, (int)w, (int)h); // m_rectClient.Width(), m_rectClient.Height());

	// Reset The Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	Size3d size(1, 1, 1); // = (m_ptMax - m_ptMin) / 2;
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

	int nSec = (int)floor((m_vp.colorStep > 0 ? (m_vp.zMax - m_vp.zMin) / m_vp.colorStep :
		m_vp.isoStep > 0 ? (m_vp.zMax - m_vp.zMin) / m_vp.isoStep :
		m_vp.zStep > 0 ? (m_vp.zMax - m_vp.zMin) / m_vp.zStep : 1.0) + 0.5);
	double step = size.cy / (nSec+2);

	std::pair<double, double> range(m_vp.zMin, m_vp.zMax);
	double valStep = (m_vp.zMax - m_vp.zMin) / nSec; // 2 * dRange / (nSec - 1); // 14.0;

	for (int i = -1; i < nSec+1; i++)
	{
		Point2d pt(-size.cx * 0.8, i * step - size.cy*0.5);

		double d = i * valStep + range.first;
		DWORD dwColor = rainbowColor(d, range.first, range.second, 255, 120, 240, m_vp.colorStep);

		glColor4d(GetRValue(dwColor) / 255.0, GetGValue(dwColor) / 255.0, GetBValue(dwColor) / 255.0, 1.0);

		glBegin(GL_QUADS);
		glVertex3d(pt.x, pt.y, 0);
		glVertex3d(-size.cx * 0.3, pt.y, 0);
		glVertex3d(-size.cx * 0.3, pt.y + step, 0);
		glVertex3d(pt.x, pt.y + step, 0);
		glEnd();

	}

	double fontScale = 0.3;
	//double fontHeight = m_oglFont.m_gmf['0'].gmfBlackBoxY * fontScale;

	for (int i = 0; i <= nSec; i++)
	{
		Point2d pt(-size.cx * 0.8, i * step - size.cy*0.5);
		double d = i * valStep + range.first;
		glPushMatrix();
		{
			glColor4d(0.0, 0.0, 0.0, 1.0);
			glTranslated(-size.cx * 0.2, pt.y, 0); // -0.5 * fontHeight, 0); //  + 0.2 * step
			glScaled(fontScale, fontScale, 0);
			//glListBase(1000);
			std::string str = stringFormat("%.1f", d);
			m_oglFont.print(str, DT_VCENTER);
			//glCallLists(str.length(), GL_UNSIGNED_BYTE, str.c_str());
		}
		glPopMatrix();
	}

	glPopMatrix();

}

// End of Plot3d implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////