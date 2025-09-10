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

#include "Tools.h"
#include "Angles.h"
#include "OglEntity.h"
#include "Plot3d.h"
//#include "GnssModel.h"


using namespace nzg;

namespace nzg
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglEntity implementation

	/////////////////////////////
	// Construction:
	OglEntity::OglEntity()
	{
	}

	OglEntity::~OglEntity()
	{
	}

	/////////////////
	// Overrides:
	void OglEntity::move(const Point3d& ptTo)
	{
		m_ptCenter = ptTo;
	}

	//////////////////////////////////
	// Implementation
	void OglEntity::onBeginDraw()
	{
		glPushMatrix();

		glTranslated(m_ptCenter.x, m_ptCenter.y, m_ptCenter.z);
	}

	void OglEntity::onEndDraw()
	{
		glPopMatrix();
	}
	// End of Entity implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglFont implementation

	////////////////////////////
	// Construction:
	OglFont::OglFont() : m_listBase(0)
	{
		memset(m_gmf, 0, sizeof(GLYPHMETRICSFLOAT) * 256);
	}

	OglFont::~OglFont()
	{
	}

	bool OglFont::create(const LOGFONT& lf, HDC hDC, int listBase)
	{
		HDC hdc = wglGetCurrentDC();
		CDC* pdc = CDC::FromHandle(hdc);

		CFont font;
		font.CreateFontIndirect(&lf);
		CFont* pOldFont = (CFont*)pdc->SelectObject(&font);

		BOOL bOk = wglUseFontOutlines(hDC, 0, 255, listBase, 0.0f, 0.02f, WGL_FONT_POLYGONS, m_gmf);

		if (!bOk)
		{
			DWORD dwErr = GetLastError();
			TRACE("Err: %lu", dwErr);
			return false;
		}

		pdc->SelectObject(pOldFont);
		m_listBase = listBase;
		return true;
	}

	Size2d OglFont::size(const char* str) const
	{
		Size2d s = Size2d(0, 0);

		for (const char* p = str; *p; p++)
		{
			s.cx += m_gmf[*p].gmfCellIncX;
			if (m_gmf[*p].gmfBlackBoxY > s.cy)
				s.cy = m_gmf[*p].gmfBlackBoxY;
		}
		return s;
	}

	void OglFont::print(const char* str, size_t len, unsigned int format) const 
	{
		if (len < 0)
			len = strlen(str);

		if (format != 0)
		{
			Size2d s = size(str);
			if (format & DT_CENTER)
				glTranslated(-s.cx / 2, 0, 0);
			if (format & DT_VCENTER)
				glTranslated(0, -s.cy / 2, 0);
			if (format & DT_RIGHT)
				glTranslated(-s.cx, 0, 0);
		}
		glListBase(m_listBase);
		glCallLists(len, GL_UNSIGNED_BYTE, str);
	}

	// End of OglFont implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglIsoline implementation

	////////////////////////////
	// Construction:
	OglIsoline::OglIsoline() : m_bLabelAlongX (true)
	{
		m_nVerts = 0;
		m_pfVerts = NULL;
		m_dLength = 0;
		m_bClosed = false;
		m_nLabelPoint = -1;
		m_dValue = NAN;
	}

	OglIsoline::OglIsoline(const OglIsoline& a)
	{
		*this = a;
	}

	OglIsoline::~OglIsoline()
	{
		reset();
	}

	void OglIsoline::reset()
	{
		if (m_pfVerts != NULL)
			delete[] m_pfVerts;
		m_pfVerts = NULL;
		m_nVerts = 0;
		m_dLength = 0;
		m_bClosed = false;
		m_nLabelPoint = -1;
		m_dValue = NAN;
	}

	static double getDist(Point3d& pt, float* pf)
	{
		double dx = pt.x - pf[0];
		double dy = pt.y - pf[1];
		return sqrt(dx * dx + dy * dy);
	}

	void OglIsoline::createLabels(double dMinLabelLen, double dZScale, std::vector<OglIsoline*>& isos, int nDigs)
	{
		computeLength();

		m_strLabel.clear();
		if (m_dLength < dMinLabelLen)
			return; 

		int nOurIndex = -1;
		for (int i = 0; i < (int)isos.size(); i++)
		{
			if (isos[i] == this)
			{
				nOurIndex = i;
				break;
			}
		}

		if (nOurIndex < 0)
			return;

		std::vector <Point3d> pts;
		for (int i = 0; i < (int)isos.size(); i++)
		{
			if (i == nOurIndex)
				continue;
			if (abs(i - nOurIndex) <= 10)
			{
				OglIsoline* pi = isos[i];
				if (pi->m_nLabelPoint >= 0)
				{
					Point3d pt = pi->getLabelPoint();
					pts.push_back(pt);
				}
			}
		}

		double dRatio = m_rect.height() / m_rect.width();
		bool bRound = (0.8 <= dRatio && dRatio <= 1.2);
		m_bLabelAlongX = dRatio < 1.0 && !bRound;

		double dLen = 0;
		m_nLabelPoint = 0;
		for (int i = 1; i < m_nVerts; i++)
		{
			float* pf0 = getPoint(i - 1);
			float* pf1 = getPoint(i);

			double dx = pf1[0] - pf0[0];
			double dy = pf1[1] - pf0[1];

			dLen += sqrt(dx * dx + dy * dy);
			if (dLen > m_dLength / 2)
			{
				m_nLabelPoint = i - 1;
				break;
			}
		}
		if (pts.size() > 0)
		{
			double dMin = DBL_MAX;
			float* pfMid = getPoint(m_nLabelPoint);
			for (int i = 0; i < (int)pts.size(); i++)
			{
				double d = getDist(pts[i], pfMid);
				if (d < dMin)
					dMin = d;
			}
			if (dMin < dMinLabelLen / 2)
			{
				int nFirst = m_nVerts / 2;
				int nLast = m_nVerts;
				double dMax = -DBL_MAX;
				for (int i = nFirst; i < nLast; i++)
				{
					float* pf = getPoint(i);
					double dMin = DBL_MAX;
					for (int j = 0; j < (int)pts.size(); j++)
					{
						double d = getDist(pts[j], pf);
						if (d < dMin)
						{
							dMin = d;
						}
					}


					if (dMin > dMax)
					{
						dMax = dMin;
						m_nLabelPoint = i;
					}

					if (dMax > dMinLabelLen)
					{
						break;
					}
				}
			}
		}



		double dValue = m_dValue;
		m_strLabel = stringFormat("%.*f", nDigs, dValue * dZScale);
		rtrim(m_strLabel);
		rtrim(m_strLabel);
	}

	/////////////////////////////
	// Operations:
	OglIsoline& OglIsoline::operator=(const OglIsoline& a)
	{
		reset();
		m_nVerts = a.m_nVerts;
		m_pfVerts = new float[m_nVerts * 3];
		memmove(m_pfVerts, a.m_pfVerts, sizeof(float) * m_nVerts * 3);
		m_dValue = a.m_dValue;
		m_dLength = a.m_dLength;
		m_strLabel = a.m_strLabel;
		m_bClosed = a.m_bClosed;
		m_bLabelAlongX = a.m_bLabelAlongX;
		m_nLabelPoint = a.m_nLabelPoint;

		return *this;
	}

	void OglIsoline::allocate(int nPoints)
	{
		reset();
		m_nVerts = nPoints;
		m_pfVerts = new float[nPoints * 3];
	}

	void OglIsoline::computeLength()
	{
		m_dLength = 0;
		Point2d ptBase = Point2d(DBL_MAX, DBL_MAX);
		Point2d ptTop = Point2d(-DBL_MAX, -DBL_MAX);

		for (int i = 1; i < m_nVerts; i++)
		{
			float* pf0 = getPoint(i - 1);
			float* pf1 = getPoint(i);
			double dx = pf1[0] - pf0[0];
			double dy = pf1[1] - pf0[1];
			m_dLength += sqrt(dx * dx + dy * dy);

			if ((double)pf0[0] < ptBase.x)
				ptBase.x = pf0[0];
			if ((double)pf0[1] < ptBase.y)
				ptBase.y = pf0[1];
			if ((double)pf0[0] > ptTop.x)
				ptTop.x = pf0[0];
			if ((double)pf0[1] > ptTop.y)
				ptTop.y = pf0[1];

			if (i == m_nVerts - 1)
			{
				if ((double)pf1[0] < ptBase.x)
					ptBase.x = pf1[0];
				if ((double)pf1[1] < ptBase.y)
					ptBase.y = pf1[1];
				if ((double)pf1[0] > ptTop.x)
					ptTop.x = pf1[0];
				if ((double)pf1[1] > ptTop.y)
					ptTop.y = pf1[1];
			}
		}

		{
			float* pf0 = getPoint(0);
			float* pf1 = getPoint(m_nVerts - 1);
			double dx = pf1[0] - pf0[0];
			double dy = pf1[1] - pf0[1];
			double d = sqrt(dx * dx + dy * dy);
			if (d < 0.0001)
				m_bClosed = true;
			else
				m_bClosed = false;

		}
	}

	void OglIsoline::draw() const
	{
		float fWidthOld;
		glGetFloatv(GL_LINE_WIDTH, &fWidthOld);
		glLineWidth(2.0F);
		glBegin(GL_LINE_STRIP); 

		for (int i = 0; i < m_nVerts; i++)
		{
			glVertex3fv(m_pfVerts + 3 * i);
		}
		glEnd();
		glLineWidth(fWidthOld);
	}

	void OglIsoline::drawText(const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font)
	{
		if (m_strLabel.empty() || m_nLabelPoint < 0 || m_nLabelPoint >= m_nVerts || viewParams.eLabels == Plot3dParams::elNone)
			return;

		const Point3d textScale(0.05, 0.05, 0.05);

		float* pf = getPoint(m_nLabelPoint);
		double zOffset = m_dOffset;
		if (viewParams.eLabels != Plot3dParams::elHideInvisible)
			glDisable(GL_DEPTH_TEST);
		else
			zOffset = 0.25;

		glPushMatrix();
		{
			glPushAttrib(GL_LIST_BIT);
			glDisable(GL_LIGHT1);

			if (m_bLabelAlongX)
				glTranslatef(pf[0], pf[1] + 0.01F, pf[2] + (float)zOffset); 
			else
				glTranslatef(pf[0] + 0.02F, pf[1], pf[2] + (float)zOffset); 
			glScaled(1/scale.x, 1/scale.y, 1/scale.z);
			glRotated(rot.cz, 0.0, 0.0, -1.0);
			glRotated(rot.cy, 0.0, -1.0, 0.0);
			glRotated(rot.cx, -1.0, 0.0, 0.0);

			glScaled(textScale.x, textScale.y, textScale.z);

			glColor4d(0.0, 0.0, 0.0, 1.0);
			font.print(m_strLabel, DT_CENTER | DT_VCENTER);

			glEnable(GL_LIGHT1);
			glPopAttrib();
		}
		glPopMatrix();

		if (viewParams.eLabels != Plot3dParams::elHideInvisible)
			glEnable(GL_DEPTH_TEST);
	}

	/*
	void OglIsoline::serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			DWORD dwVer = 1;
			ar << dwVer;

			ar << m_nVerts;
			ar.Write(m_pfVerts, sizeof(float) * m_nVerts);
			ar << m_dValue;
			ar << m_dLength;
			ar << m_rect;
			ar << m_strLabel;
			ar << m_bClosed;
			ar << m_bLabelAlongX;
			ar << m_nLabelPoint;
			ar << m_dOffset;
		}
		else
		{
			reset();
			DWORD dwVer = 1;
			ar >> dwVer;

			ar >> m_nVerts;
			m_pfVerts = new float[m_nVerts];
			ar.Read(m_pfVerts, sizeof(float) * m_nVerts);
			ar >> m_dValue;
			ar >> m_dLength;
			ar >> m_rect;
			ar >> m_strLabel;
			ar >> m_bClosed;
			ar >> m_bLabelAlongX;
			ar >> m_nLabelPoint;
			ar >> m_dOffset;
		}
	}
	*/
	// End if OglIsoline implmenetation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglSurface implementation

	OglSurface::Edge::Edge()
	{
		pfVerts[0] = pfVerts[1] = NULL;
	}

	OglSurface::Edge::Edge(float* pf1, float* pf2)
	{
		pfVerts[0] = pf1;
		pfVerts[1] = pf2;
	}

	bool OglSurface::Edge::isNull() const
	{
		return pfVerts[0] == NULL || pfVerts[1] == NULL;
	}

	bool OglSurface::Edge::isIntersect(float h) const
	{
		float fMin = std::min(pfVerts[0][2], pfVerts[1][2]);
		float fMax = std::max(pfVerts[0][2], pfVerts[1][2]);
		if (fMin < h && h < fMax)
			return true;
		return false;
	}

	void OglSurface::Edge::appendIntPoint(float h, std::vector<float>& fPts) const
	{
		float f[3];
		float d = (h - pfVerts[0][2]) / (pfVerts[1][2] - pfVerts[0][2]);
		f[0] = d * (pfVerts[1][0] - pfVerts[0][0]) + pfVerts[0][0];
		f[1] = d * (pfVerts[1][1] - pfVerts[0][1]) + pfVerts[0][1];
		f[2] = d * (pfVerts[1][2] - pfVerts[0][2]) + pfVerts[0][2];
		fPts.push_back(f[0]);
		fPts.push_back(f[1]);
		fPts.push_back(f[2]);
	}

	OglSurface::Triangle::Triangle()
	{
		nTriangle = -1;
		memset(nVerts, 0, sizeof(int) * 3);
		memset(pfVerts, 0, sizeof(float) * 3);
		bMarked = false;
	}

	OglSurface::Triangle::Triangle(int nIndex, int* pnIndexes, float* pfs)
	{
		nTriangle = nIndex;
		nVerts[0] = pnIndexes[nIndex];
		nVerts[1] = pnIndexes[nIndex + 1];
		nVerts[2] = pnIndexes[nIndex + 2];
		pfVerts[0] = pfs + nVerts[0];
		pfVerts[1] = pfs + nVerts[1];
		pfVerts[2] = pfs + nVerts[2];
		bMarked = false;
	}

	OglSurface::Edge OglSurface::Triangle::getEdge(int i) const {
		ASSERT(0 <= i && i < 3);
		if (i < 2)
			return Edge(pfVerts[i], pfVerts[i + 1]);
		else
			return Edge(pfVerts[2], pfVerts[0]);
	}

	void OglSurface::Triangle::getIntEdges(float h, Edge& e1, Edge& e2) const
	{
		for (int i = 0; i < 3; i++)
		{
			Edge e = getEdge(i);
			if (!e.isIntersect(h))
				continue;
			if (e1.isNull())
				e1 = e;
			else if (e2.isNull())
				e2 = e;
			else
			{
				ASSERT(FALSE);
			}
		}
	}

	bool OglSurface::Triangle::hasEdge(const Edge& edge, float h, Edge* peNext) const
	{
		bool bOk = false;
		for (int i = 0; i < 3; i++)
		{
			Edge e = getEdge(i);
			if (e == edge)
			{
				bOk = true;
				break;
			}
		}

		if (!bOk)
			return false;

		for (int i = 0; i < 3; i++)
		{
			Edge e = getEdge(i);
			if (e == edge)
				continue;
			if (e.isIntersect(h))
			{
				*peNext = e;
				return true;
			}
		}

		return false;
	}

	bool OglSurface::Triangle::isValid() const
	{
		for (int i = 0; i < 3; i++)
		{
			float* pfv = pfVerts[i];
			if (_isnan(pfv[0]) || _isnan(pfv[1]) || _isnan(pfv[2]))
				return false;
		}

		return true;
	}

	//////////////////////////
	// Costruction:
	OglSurface::OglSurface() : m_pfVerts(nullptr), m_pfNorms(nullptr), m_pdwColors(nullptr), m_pnIndexes(nullptr)
	{
		m_bNet = FALSE;
		m_nID = 0;
		m_nVerts = 0;
		m_nIndexes = 0;
		m_bUseVertexColors = true;
	}

	OglSurface::OglSurface(const OglSurface& a) : m_pfVerts(nullptr), m_pfNorms(nullptr), m_pdwColors(nullptr), m_pnIndexes(nullptr)
	{
		m_bNet = false;
		m_nID = 0;
		m_nVerts = 0;
		m_nIndexes = 0;
		*this = a;
	}

	OglSurface::OglSurface(int id, const Grid& grid) : m_pfVerts(nullptr), m_pfNorms(nullptr), m_pdwColors(nullptr), m_pnIndexes (nullptr)
	{
		fromGrid(grid);
		m_nID = id;
		m_bUseVertexColors = true;
	}

	void OglSurface::fromGrid(const Grid& grid)
	{
		m_bNet = false;
		if (!(m_grid == grid))
		{
			reset();
			m_grid = grid;
			m_nVerts = grid.nx * grid.ny;
			m_pfVerts = new float[3 * m_nVerts];
			m_pfNorms = new float[3 * m_nVerts];
			m_pdwColors = new DWORD[m_nVerts];
			memset(m_pfNorms, 0, sizeof(float) * 3 * m_nVerts);
			m_nIndexes = (grid.nx - 1) * (grid.ny - 1) * 2 * 3;
			m_pnIndexes = new int[m_nIndexes];
		}
	}

	OglSurface::~OglSurface()
	{
		reset();
	}

	void OglSurface::reset()
	{
		if (m_pfVerts != nullptr)
			delete[] m_pfVerts;

		if (m_pnIndexes != nullptr)
			delete[] m_pnIndexes;

		if (m_pfNorms != nullptr)
			delete[] m_pfNorms;

		if (m_pdwColors != nullptr)
			delete[] m_pdwColors;

		m_bNet = false;
		m_nID = 0;
		m_nVerts = 0;
		m_pfVerts = nullptr;
		m_pfNorms = nullptr;
		m_nIndexes = 0;
		m_pnIndexes = nullptr;
		m_pdwColors = nullptr;
		m_bUseVertexColors = true;

		resetIsolines();
	}

	void OglSurface::resetIsolines()
	{
		for (int i = 0; i < (int)m_isos.size(); i++)
		{
			delete m_isos[i];
		}
		m_isos.clear();
	}

	/////////////////
	// Operations:
	OglSurface& OglSurface::operator=(const OglSurface& a)
	{
		reset();

		*(OglEntity*)this = a;

		m_strName = a.m_strName;
		m_nID = 0;
		m_bNet = a.m_bNet;
		m_grid = a.m_grid;
		m_nVerts = a.m_nVerts;
		m_pfVerts = new float[m_nVerts*3];
		memmove(m_pfVerts, a.m_pfVerts, sizeof(float) * m_nVerts*3);
		m_nIndexes = a.m_nIndexes;
		m_pnIndexes = new int[m_nIndexes];
		memmove(m_pnIndexes, a.m_pnIndexes, sizeof(int) * m_nIndexes);
		m_pfNorms = new float[m_nVerts*3];
		memmove(m_pfNorms, a.m_pfNorms, sizeof(float) * m_nVerts*3);
		m_pdwColors = new DWORD[m_nVerts];
		memmove(m_pdwColors, a.m_pdwColors, sizeof(DWORD) * m_nVerts);
		m_bUseVertexColors = a.m_bUseVertexColors;
		return *this;
	}

	void OglSurface::draw(const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font)
	{
		bool bTestNormals = false;
		m_bNet = false;

		if (!m_bNet && !bTestNormals)
			glBegin(GL_TRIANGLES);

		for (int i = 0; i < m_nIndexes; i += 3)
		{
			if (m_bNet || bTestNormals)
				glBegin(GL_LINE_LOOP);

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i]);
			DWORD dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i] / 3] : m_dwSingleColor;
			//if (m_pdwColors != NULL)
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i] / 3];
				float fa = getAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}
			glVertex3fv(m_pfVerts + m_pnIndexes[i]); 

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i + 1]);
			dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i+1] / 3] : m_dwSingleColor;
			//if (m_pdwColors != NULL)
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i + 1] / 3];
				float fa = getAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}
			glVertex3fv(m_pfVerts + m_pnIndexes[i + 1]); 

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i + 2]);

			dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i+2] / 3] : m_dwSingleColor;
			//if (m_pdwColors != NULL)
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i + 2] / 3];
				float fa = getAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}
			glVertex3fv(m_pfVerts + m_pnIndexes[i + 2]);

			if (m_bNet || bTestNormals)
				glEnd();
		}

		if (!m_bNet && !bTestNormals)
			glEnd();

		glColor4f(0.0F, 0.0F, 0.0F, 1.0F);
		for (auto iso : m_isos)
		{
			iso->draw();
			iso->drawText(scale, rot, viewParams, font);
		}

		// Draw normals
		if (bTestNormals)
		{
			glColor4f(0.0F, 0.0F, 0.0F, 1.0F);
			glBegin(GL_LINES);
			for (int i = 0; i < m_nIndexes; i++)
			{
				glVertex3fv(m_pfVerts + m_pnIndexes[i]);
				float f[3];
				for (int j = 0; j < 3; j++)
				{
					f[j] = *(m_pfVerts + m_pnIndexes[i] + j);
					f[j] += 10 * (*(m_pfNorms + m_pnIndexes[i] + j));
				}
				glVertex3fv(f);
			}
			glEnd();
		}

	}

	static BYTE GetAValue(DWORD dwArgb)
	{
		BYTE c = ((BYTE)((dwArgb) >> 24));
		return c;
	}


	void OglSurface::draw()
	{
		onBeginDraw();

		bool bTestNormals = false;

		if (!m_bNet && !bTestNormals)
			glBegin(GL_TRIANGLES);

		for (int i = 0; i < m_nIndexes; i += 3)
		{
			//glBegin (GL_TRIANGLES); //(GL_LINE_LOOP); //(GL_TRIANGLES);
			if (m_bNet || bTestNormals)
				glBegin(GL_LINE_LOOP);

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i]);

			DWORD dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i] / 3] : m_dwSingleColor;
			//if ()
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i] / 3];
				float fa = GetAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}

			glVertex3fv(m_pfVerts + m_pnIndexes[i]); //, m_pfVerts[m_pnIndexes[i]+1], m_pfVerts[m_pnIndexes[i]+2]);

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i + 1]);


			dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i+1] / 3] : m_dwSingleColor;
			//if (m_pdwColors != NULL)
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i + 1] / 3];
				//float fa = GetAValue (dwColor)/255.0F;
				float fa = GetAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}
			glVertex3fv(m_pfVerts + m_pnIndexes[i + 1]); //, m_pfVerts[m_pnIndexes[i+1]+1], m_pfVerts[m_pnIndexes[i+1]+2]);

			if (!m_bNet && !bTestNormals)
				glNormal3fv(m_pfNorms + m_pnIndexes[i + 2]);

			dwColor = (m_pdwColors != NULL && m_bUseVertexColors) ? m_pdwColors[m_pnIndexes[i + 2] / 3] : m_dwSingleColor;
			//if (m_pdwColors != NULL)
			{
				//DWORD dwColor = m_pdwColors[m_pnIndexes[i + 2] / 3];
				//float fa = GetAValue (dwColor)/255.0F;
				float fa = GetAValue(dwColor) / 255.0F;
				float fr = GetRValue(dwColor) / 255.0F;
				float fg = GetGValue(dwColor) / 255.0F;
				float fb = GetBValue(dwColor) / 255.0F;
				glColor4f(fr, fg, fb, fa);
			}
			glVertex3fv(m_pfVerts + m_pnIndexes[i + 2]); //, m_pfVerts[m_pnIndexes[i+2]+1], m_pfVerts[m_pnIndexes[i+2]+2]);

			if (m_bNet || bTestNormals)
				glEnd();
		}

		if (!m_bNet && !bTestNormals)
			glEnd();

		glColor4f(0.0F, 0.0F, 0.0F, 1.0F);
		for (int i = 0; i < (int)m_isos.size(); i++)
		{
			m_isos[i]->draw();
		}

		// Draw normals
		if (bTestNormals)
		{
			glColor4f(0.0F, 0.0F, 0.0F, 1.0F);
			glBegin(GL_LINES);
			for (int i = 0; i < m_nIndexes; i++)
			{
				glVertex3fv(m_pfVerts + m_pnIndexes[i]); //, m_pfVerts[m_pnIndexes[i]+1], m_pfVerts[m_pnIndexes[i]+2]);
				float f[3];
				for (int j = 0; j < 3; j++)
				{
					f[j] = *(m_pfVerts + m_pnIndexes[i] + j);
					f[j] += 10 * (*(m_pfNorms + m_pnIndexes[i] + j));
				}
				glVertex3fv(f);
			}
			glEnd();
		}

		onEndDraw();
	}

	static bool compVerts(float f1[3], float f2[3])
	{
		return f1[0] == f2[0] && f1[1] == f2[1] && f1[2] == f2[2];
	}

	bool OglSurface::createIsoline(float fValue, float fDelta, float fOffset, double dMinLabelLen, double isoStep, double dZScale)
	{
		double dValue = fValue;
		int nFirstNewIso = (int)m_isos.size();

	#pragma region ShiftValue
		while (true)
		{
			bool bFound = false;
			for (int i = 0; i < m_nVerts; i++)
			{
				if (fValue == m_pfVerts[3 * i + 2])
				{
					fValue -= fDelta;
					bFound = true;
					break;
				}
			}

			if (!bFound)
				break;
		}
	#pragma endregion // ShiftValue

	#pragma region SelectCrossedTriangles
		int nTriangles = m_nIndexes / 3;
		std::vector <Triangle> trs;
		for (int i = 0; i < m_nIndexes; i += 3)
		{
			float* pvs[3] = { m_pfVerts + m_pnIndexes[i], m_pfVerts + m_pnIndexes[i + 1], m_pfVerts + m_pnIndexes[i + 2] };
			float fMin = FLT_MAX;
			float fMax = -FLT_MAX;
			for (int j = 0; j < 3; j++)
			{
				float f = *(pvs[j] + 2);
				if (f > fMax)
					fMax = f;
				if (f < fMin)
					fMin = f;
			}

			if ((fMin < fValue) && (fValue < fMax))
			{
				trs.push_back(Triangle(i, m_pnIndexes, m_pfVerts));
			}
		}
	#pragma endregion

		for (int nTr = 0; nTr < (int)trs.size(); nTr++)
		{
			Triangle& t = trs[nTr];
			if (t.bMarked)
				continue;

			if (!t.isValid())
				continue;

			t.mark(true);

			OglIsoline* iso = new OglIsoline();


			Edge e1, e2;
			t.getIntEdges(fValue, e1, e2);
			ASSERT(!e1.isNull() && !e2.isNull());

			std::vector <float> fPts1;

			e1.appendIntPoint(fValue, fPts1);

			Edge eNext;
			while (getNextTriangle(e1, trs, fValue, &eNext))
			{
				eNext.appendIntPoint(fValue, fPts1);
				e1 = eNext;
			}

			std::vector <float> fPts2;
			while (getNextTriangle(e2, trs, fValue, &eNext))
			{
				eNext.appendIntPoint(fValue, fPts2);
				e2 = eNext;
			}

	#pragma region CopyPoints
			int nPoints = (int)(fPts1.size() / 3 + fPts2.size() / 3);
			iso->allocate(nPoints);
			int nVert = 0;
			for (int i = (int)fPts2.size() - 3; i >= 0; i -= 3)
			{
				iso->m_pfVerts[3 * nVert] = fPts2[i];
				iso->m_pfVerts[3 * nVert + 1] = fPts2[i + 1];
				iso->m_pfVerts[3 * nVert + 2] = fPts2[i + 2] + fOffset;
				nVert++;
			}

			for (int i = 0; i < (int)fPts1.size(); i += 3)
			{
				iso->m_pfVerts[3 * nVert] = fPts1[i];
				iso->m_pfVerts[3 * nVert + 1] = fPts1[i + 1];
				iso->m_pfVerts[3 * nVert + 2] = fPts1[i + 2] + fOffset;
				nVert++;
			}
	#pragma endregion

			iso->m_dValue = dValue;
			iso->m_dOffset = fOffset;
			m_isos.push_back(iso);
		}

		int nDigs = 0;
		for (double is = isoStep; floor(is) != is; is *= 10)
		{
			nDigs++;
			if (nDigs > 3)
				break;
		}

		for (int i = nFirstNewIso; i < (int)m_isos.size(); i++)
		{
			m_isos[i]->createLabels(dMinLabelLen, dZScale, m_isos, nDigs);
		}

		return true; 
	}

	void OglSurface::getMinMax(Point3d& ptMin, Point3d& ptMax) const
	{
		if (m_nVerts <= 0)
			return;

		ptMin = Point3d(DBL_MAX, DBL_MAX, DBL_MAX);
		ptMax = Point3d(-DBL_MAX, -DBL_MAX, -DBL_MAX);
		for (int i = 0; i < m_nVerts; i++)
		{
			double x = m_pfVerts[3 * i];
			double y = m_pfVerts[3 * i + 1];
			double z = m_pfVerts[3 * i + 2];

			if (ptMin.x > x)
				ptMin.x = x;
			if (ptMin.y > y)
				ptMin.y = y;
			if (ptMin.z > z)
				ptMin.z = z;

			if (ptMax.x < x)
				ptMax.x = x;
			if (ptMax.y < y)
				ptMax.y = y;
			if (ptMax.z < z)
				ptMax.z = z;
		}
	}

	/*
	void OglSurface::serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			DWORD dwVer = 1;
			ar << dwVer;
			ar << m_strName;
			ar << m_bNet;
			ar << m_nID;
			ar.Write(&m_grid, sizeof(m_grid));
			ar << m_nVerts;
			ar.Write(m_pfVerts, sizeof(float) * 3 * m_nVerts);
			ar.Write(m_pfNorms, sizeof(float) * 3 * m_nVerts);
			ar.Write(m_pdwColors, sizeof(DWORD) * m_nVerts);

			ar << m_nIndexes;
			ar.Write(m_pnIndexes, sizeof(int) * m_nIndexes);

			int nSize = m_isos.GetSize();
			ar << nSize;
			for (int i = 0; i < nSize; i++)
			{
				m_isos[i]->serialize(ar);
			}
		}
		else
		{
			Reset();

			DWORD dwVer = 1;
			ar >> dwVer;
			ar >> m_strName;
			ar >> m_bNet;
			ar >> m_nID;
			ar.Read(&m_grid, sizeof(m_grid));
			ar >> m_nVerts;
			m_pfVerts = new float[3 * m_nVerts];
			ar.Read(m_pfVerts, sizeof(float) * 3 * m_nVerts);
			m_pfNorms = new float[3 * m_nVerts];
			ar.Read(m_pfNorms, sizeof(float) * 3 * m_nVerts);
			m_pdwColors = new DWORD[m_nVerts];
			ar.Read(m_pdwColors, sizeof(DWORD) * m_nVerts);

			ar >> m_nIndexes;
			ar.Read(m_pnIndexes, sizeof(int) * m_nIndexes);

			int nSize = 0;
			ar >> nSize;
			for (int i = 0; i < nSize; i++)
			{
				CIvs3dIsoline* pi = new CIvs3dIsoline();
				pi->serialize(ar);
				m_isos.Add(pi);
			}
		}
	}
	*/

	void OglSurface::calcNormal(int i, BOOL bClockwise, float fn[3], double dZScale) const
	{
		const int x = 0;
		const int y = 1;
		const int z = 2;

		float v1[3], v2[3];

		v1[x] = m_pfVerts[m_pnIndexes[i]] - m_pfVerts[m_pnIndexes[i + 1]];
		v1[y] = m_pfVerts[m_pnIndexes[i] + 1] - m_pfVerts[m_pnIndexes[i + 1] + 1];
		v1[z] = (m_pfVerts[m_pnIndexes[i] + 2] - m_pfVerts[m_pnIndexes[i + 1] + 2]) / (float)dZScale;

		v2[x] = m_pfVerts[m_pnIndexes[i + 1]] - m_pfVerts[m_pnIndexes[i + 2]];
		v2[y] = m_pfVerts[m_pnIndexes[i + 1] + 1] - m_pfVerts[m_pnIndexes[i + 2] + 1];
		v2[z] = (m_pfVerts[m_pnIndexes[i + 1] + 2] - m_pfVerts[m_pnIndexes[i + 2] + 2]) / (float)dZScale;

		fn[x] = v1[y] * v2[z] - v1[z] * v2[y];
		fn[y] = v1[z] * v2[x] - v1[x] * v2[z];
		fn[z] = v1[x] * v2[y] - v1[y] * v2[x];

		if (!bClockwise)
		{
			fn[x] = -fn[x];
			fn[y] = -fn[y];
			fn[z] = -fn[z];
		}

		normalize(fn);
	}

	void OglSurface::calcNormal(int i, BOOL bClockwise, float fn[3], Point3d scale) const
	{
		const int x = 0;
		const int y = 1;
		const int z = 2;

		float v1[3], v2[3];

		v1[x] = (m_pfVerts[m_pnIndexes[i]] - m_pfVerts[m_pnIndexes[i + 1]]) / (float)scale.x;
		v1[y] = (m_pfVerts[m_pnIndexes[i] + 1] - m_pfVerts[m_pnIndexes[i + 1] + 1]) / (float)scale.y;
		v1[z] = (m_pfVerts[m_pnIndexes[i] + 2] - m_pfVerts[m_pnIndexes[i + 1] + 2]) / (float)scale.z;

		v2[x] = (m_pfVerts[m_pnIndexes[i + 1]] - m_pfVerts[m_pnIndexes[i + 2]]) / (float)scale.x;
		v2[y] = (m_pfVerts[m_pnIndexes[i + 1] + 1] - m_pfVerts[m_pnIndexes[i + 2] + 1]) / (float)scale.y;
		v2[z] = (m_pfVerts[m_pnIndexes[i + 1] + 2] - m_pfVerts[m_pnIndexes[i + 2] + 2]) / (float)scale.z;

		fn[x] = v1[y] * v2[z] - v1[z] * v2[y];
		fn[y] = v1[z] * v2[x] - v1[x] * v2[z];
		fn[z] = v1[x] * v2[y] - v1[y] * v2[x];

		if (!bClockwise)
		{
			fn[x] = -fn[x];
			fn[y] = -fn[y];
			fn[z] = -fn[z];
		}

		normalize(fn);
	}

	void OglSurface::normalize(float v[3], float fScale)
	{
		float length = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

		if (length == 0)
			length = 1.0f;

		v[0] /= length;
		v[1] /= length;
		v[2] /= length;

		v[0] *= fScale;
		v[1] *= fScale;
		v[2] *= fScale;
	}

	DWORD OglSurface::getColorFromRange(double dVal, double dMin, double dMax, BYTE cAlpha)
	{
		return rainbowColor(dVal, dMin, dMax, cAlpha);
	}

	bool OglSurface::getNextTriangle(const Edge& e, std::vector <Triangle>& trs, float h, Edge* peNext)
	{
		for (int nTr1 = 0; nTr1 < (int)trs.size(); nTr1++)
		{
			Triangle& t1 = trs[nTr1];
			if (t1.bMarked)
				continue;

			Edge eNext;
			if (t1.hasEdge(e, h, peNext))
			{
				t1.mark(true);
				return true;
			}
		}

		return false;
	}
	// End of OglSurface implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglPointCloud implementation

	////////////////////////////////
	// Construction:
	OglPointCloud::OglPointCloud()
	{
		m_nID = 0;
		m_nVerts = 0;
		m_pfVerts = NULL;
		m_pdwColors = NULL;
	}

	OglPointCloud::~OglPointCloud()
	{
		reset();
	}

	void OglPointCloud::reset()
	{
		if (m_pfVerts != NULL)
			delete[] m_pfVerts;

		if (m_pdwColors != NULL)
			delete[] m_pdwColors;

		m_nID = 0;
		m_nVerts = 0;
		m_pfVerts = NULL;
		m_pdwColors = NULL;
	}

	////////////////////////////
	// Operations:
	void OglPointCloud::draw()
	{
		glBegin(GL_POINTS);
		for (size_t n = 0; n < (size_t)m_nVerts; n++) {
			DWORD dwColor = m_pdwColors[n];
			float fa = getAValue(dwColor) / 255.0F;
			float fr = GetRValue(dwColor) / 255.0F;
			float fg = GetGValue(dwColor) / 255.0F;
			float fb = GetBValue(dwColor) / 255.0F;
			glColor4f(fr, fg, fb, fa);
			glVertex3f(m_pfVerts[3 * n], m_pfVerts[3 * n + 1], m_pfVerts[3 * n + 2]);
		}
		glEnd();
	}

	DWORD OglPointCloud::getColorFromRange(double dVal, double dMin, double dMax, BYTE cAlpha)
	{
		return rainbowColor(dVal, dMin, dMax, cAlpha);
	}

	// End of OglPointCloud implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglColumns

	OglColumns::OglColumns()
	{
	}

	OglColumns::~OglColumns()
	{
	}

	void OglColumns::draw()
	{
		for (int i = 0; i < m_nVerts; i++)
		{
			drawColumn(m_pfVerts[3 * i], m_pfVerts[3 * i + 1], m_pfVerts[3 * i + 2], m_pdwColors[i]);
		}
	}

	void OglColumns::drawColumn(float x, float y, float z, DWORD dwColor)
	{
		double verts[] = {
			x - m_size.cx / 2, y + m_size.cy / 2, 0, // Bottom
			x + m_size.cx / 2, y + m_size.cy / 2, 0,
			x + m_size.cx / 2, y - m_size.cy / 2, 0,
			x - m_size.cx / 2, y + m_size.cy / 2, 0,
			x + m_size.cx / 2, y - m_size.cy / 2, 0,
			x - m_size.cx / 2, y - m_size.cy / 2, 0,

			x - m_size.cx / 2, y - m_size.cy / 2, z, // Top
			x + m_size.cx / 2, y - m_size.cy / 2, z,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x - m_size.cx / 2, y - m_size.cy / 2, z,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x - m_size.cx / 2, y + m_size.cy / 2, z,

			x - m_size.cx / 2, y + m_size.cy / 2, z, // Front
			x - m_size.cx / 2, y - m_size.cy / 2, z,
			x - m_size.cx / 2, y + m_size.cy / 2, 0,
			x - m_size.cx / 2, y - m_size.cy / 2, 0,
			x - m_size.cx / 2, y + m_size.cy / 2, z,
			x - m_size.cx / 2, y - m_size.cy / 2, z,
			x + m_size.cx / 2, y - m_size.cy / 2, 0, // Back
			x + m_size.cx / 2, y + m_size.cy / 2, 0,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x + m_size.cx / 2, y - m_size.cy / 2, 0,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x + m_size.cx / 2, y - m_size.cy / 2, z,
			x - m_size.cx / 2, y - m_size.cy / 2, 0, // Left
			x + m_size.cx / 2, y - m_size.cy / 2, 0,
			x + m_size.cx / 2, y - m_size.cy / 2, z,
			x - m_size.cx / 2, y - m_size.cy / 2, 0,
			x + m_size.cx / 2, y - m_size.cy / 2, z,
			x - m_size.cx / 2, y - m_size.cy / 2, z,
			x - m_size.cx / 2, y + m_size.cy / 2, 0, // Right
			x + m_size.cx / 2, y + m_size.cy / 2, 0,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x - m_size.cx / 2, y + m_size.cy / 2, 0,
			x + m_size.cx / 2, y + m_size.cy / 2, z,
			x - m_size.cx / 2, y + m_size.cy / 2, z
		};

		glBegin(GL_TRIANGLES);

		for (int i = 0; i < 36; i++)
		{
			float fa = getAValue(dwColor) / 255.0F;
			float fr = GetRValue(dwColor) / 255.0F;
			float fg = GetGValue(dwColor) / 255.0F;
			float fb = GetBValue(dwColor) / 255.0F;
			glColor4f(fr, fg, fb, fa);
			glVertex3dv(verts + 3 * i);

			/*
			if (i < 6)
				glNormal3d(0, 0, -1);
			else if (i < 12)
				glNormal3d(0, 0, 1);
			else if (i < 18)
				glNormal3d(-1, 0, 0);
			else if (i < 24)
				glNormal3d(1, 0, 0);
			else if (i < 30)
				glNormal3d(0, -1, 0);
			else
				glNormal3d(0, 1, 0);
				*/
		}
		glEnd();
	}
	// End of OglColumns
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglAxis implementation
	void OglAxis::draw(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font) const
	{
		const Point3d titleScale(0.07, 0.07, 0.07);
		const Point3d labelScale(0.05, 0.05, 0.05);

		glPushMatrix();
		{
			glBegin(GL_LINE_STRIP);
			Point3d pt0 = getMinPt();
			Point3d pt1 = getMaxPt();
			glVertex3d(pt0.x, pt0.y, pt0.z);
			glVertex3d(pt1.x, pt1.y, pt1.z);
			glEnd();

			drawArrow(axis, getMaxPt()[type] * 0.05, 0.009, 12);

			pt1 = getMaxValuePt();

			for (int i = 0; i < 3; i++)
			{
				if (axis[i].type == type)
					continue;

				bool bGrid0 = viewParams.bGrid0 || !((type == X && axis[i].type == Y) || (type == Y && axis[i].type == X));
				if (!bGrid0)
					glDisable(GL_DEPTH_TEST);

				for (double y = axis[i].fmin; y < axis[i].fmax + axis[i].step / 2; y += axis[i].step)
				{
					glBegin(GL_LINE_STRIP);
					Point3d pt = pt0;
					pt[axis[i].type] = y;
					glVertex3d(pt.x, pt.y, pt.z);
					pt = pt1;
					pt[axis[i].type] = y;
					glVertex3d(pt.x, pt.y, pt.z);
					glEnd();
				}

				if (!bGrid0)
					glEnable(GL_DEPTH_TEST);
			}

		}
		glPopMatrix();

		drawTitles(axis, scale, rot, titleScale, viewParams, font);
		drawLabels(axis, scale, rot, labelScale, viewParams, font);
	}

	void OglAxis::draw(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Const3dParams& viewParams, const OglFont& font) const
	{
		const Point3d titleScale(0.07, 0.07, 0.07);
		const Point3d labelScale(0.05, 0.05, 0.05);

		glPushMatrix();
		{
			glBegin(GL_LINE_STRIP);
			Point3d pt0 = getMinPt();
			Point3d pt1 = getMaxPt();
			glVertex3d(pt0.x, pt0.y, pt0.z);
			glVertex3d(pt1.x, pt1.y, pt1.z);
			glEnd();

			drawArrow(axis, getMaxPt()[type] * 0.05, 0.009, 12);

			pt1 = getMaxValuePt();

			for (int i = 0; i < 3; i++)
			{
				if (axis[i].type == type)
					continue;

				bool bGrid0 = false; // viewParams.bGrid0 || !((type == X && axis[i].type == Y) || (type == Y && axis[i].type == X));
				if (!bGrid0)
					glDisable(GL_DEPTH_TEST);

				for (double y = axis[i].fmin; y < axis[i].fmax + axis[i].step / 2; y += axis[i].step)
				{
					glBegin(GL_LINE_STRIP);
					Point3d pt = pt0;
					pt[axis[i].type] = y;
					glVertex3d(pt.x, pt.y, pt.z);
					pt = pt1;
					pt[axis[i].type] = y;
					glVertex3d(pt.x, pt.y, pt.z);
					glEnd();
				}

				if (!bGrid0)
					glEnable(GL_DEPTH_TEST);
			}

		}
		glPopMatrix();

		//drawTitles(axis, scale, rot, titleScale, viewParams, font);
		//drawLabels(axis, scale, rot, labelScale, viewParams, font);
	}

	void OglAxis::drawPolar(const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font) const
	{
		const Point3d titleScale(0.07, 0.07, 0.07);
		const Point3d labelScale(0.05, 0.05, 0.05);

		if (!viewParams.bGrid0)
			glDisable(GL_DEPTH_TEST);

		glPushMatrix();
		{
			double daz = 1;
			Point3d pt0(0, 1.2, 0);
			for (double az = 0; az < 360; az += daz)
			{
				glBegin(GL_LINE_STRIP);
				Point3d pt1 = ZenAz(90, az + daz).dirCos() * 1.2; // (1.2 * cos(PI * ((az + daz) - 90) / 180), 1.2 * sin(PI * ((az + daz) - 90) / 180), 0);
				glVertex3d(pt0.x, pt0.y, pt0.z);
				glVertex3d(pt1.x, pt1.y, pt1.z);
				glEnd();

				if (floor(az / 30) * 30 == az)
				{
					Point3d ptc(0, 0, 0);
					glBegin(GL_LINE_STRIP);
					glVertex3d(ptc.x, ptc.y, ptc.z);
					glVertex3d(pt0.x, pt0.y, pt0.z);
					glEnd();

					glPushMatrix();
					glPushAttrib(GL_LIST_BIT);
					glDisable(GL_LIGHT1);
					glColor4d(0.0, 0.0, 0.0, 1.0);

					{
						glTranslated(pt0.x*1.1, pt0.y*1.1, 0);
						glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
						glRotated(rot.cz, 0, 0, -1);
						glRotated(rot.cy, 0, -1, 0);
						glRotated(rot.cx, -1, 0, 0);
						glScaled(labelScale.x, labelScale.y, labelScale.z);

						if (viewParams.viewAt != Plot3dCamera::evaFront)
						{
							std::string label = stringFormat("%.0f", az);
							font.print(label, DT_VCENTER | DT_CENTER);
						}
					}

					glEnable(GL_LIGHT1);
					glPopAttrib();
					glPopMatrix();
				}

				pt0 = pt1;

			}

			for (double z = 30; z <= 90; z += 30)
			{
				glBegin(GL_LINE_STRIP);
				Point3d pt0 = ZenAz(z, 0).dirCos() / sin(z*PI/180) * z / 90;
				glVertex3d(pt0.x, pt0.y, 0);
				for (double az = 0; az <= 360; az+=1)
				{
					Point3d pt1 = ZenAz(z, az).dirCos() / sin(z*PI/180) * z / 90;
					glVertex3d(pt1.x, pt1.y, 0);
					pt0 = pt1;
				}
				glEnd();
			}
		}
		glPopMatrix();

		if (!viewParams.bGrid0)
			glEnable(GL_DEPTH_TEST);

		glPushMatrix();
		glPushAttrib(GL_LIST_BIT);
		glDisable(GL_LIGHT1);
		glColor4d(0.0, 0.0, 0.0, 1.0);
		GLfloat modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		if (type == Z)
		{

			if (modelview[0] > 0)
				glTranslated(0, 1.6, 0);
			else
				glTranslated(0, 1.6, 0);
			glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
			if (modelview[0] < 0)
			{
				glRotated(180, 0, 1, 0);
				glRotated(180, 1, 0, 0);
			}

			//glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
			glScaled(titleScale.x, titleScale.y, titleScale.z);

			//Size2d s = font.size(title.c_str()); // , titleScale);
			//glTranslated(-s.cx / 2, 0, 0);

			font.print(title, DT_CENTER);
		}
		glEnable(GL_LIGHT1);
		glPopAttrib();
		glPopMatrix();

	}

	void OglAxis::drawArrow(const OglAxis axis[3], double height, double rad, int n) const
	{
		const Type otherTypes[3][2] = { { Y, Z }, { Z, X }, { X, Y } };
		Point3d ptEnd = getMaxPt();
		Point3d ptBase = ptEnd;
		ptBase[type] -= height;
		double r1 = rad * axis[otherTypes[type][0]].getMaxValuePt()[otherTypes[type][0]];
		double r2 = rad * axis[otherTypes[type][1]].getMaxValuePt()[otherTypes[type][1]];
		std::vector<Point3d> pts;
		for (int i = 0; i <= n; i++)
		{
			double a = 2*PI / n * i;
			Point3d pt;
			pt[type] = ptBase[type];
			pt[otherTypes[type][0]] = r1 * sin(a);
			pt[otherTypes[type][1]] = r2 * cos(a);
			pts.push_back(pt);
		}

		// draw cone top 
		glBegin(GL_TRIANGLE_FAN);
		glVertex3d(ptEnd.x, ptEnd.y, ptEnd.z);
		for (int i = 0; i <= n; ++i) {
			glVertex3d(pts[i].x, pts[i].y, pts[i].z);
		}
		glEnd();
	}

	void OglAxis::drawTitles(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Point3d& textScale, const Plot3dParams& viewParams, const OglFont& font) const
	{
		glPushMatrix();
		glPushAttrib(GL_LIST_BIT);
		glDisable(GL_LIGHT1);
		glColor4d(0.0, 0.0, 0.0, 1.0);
		GLfloat modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		if (type == X)
		{

			if (modelview[0] > 0)
				glTranslated(fmax * 0.5, -0.15 * axis[Y].fmax, axis[Z].fmax * 1.2);
			else
				glTranslated(fmax * 0.5, -0.15 * axis[Y].fmax, axis[Z].fmax * 1.2);
			glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
			if (modelview[0] < 0)
			{
				glRotated(180, 0, 1, 0);
				glRotated(180, 1, 0, 0);
			}
			glScaled(textScale.x, textScale.y, textScale.z);
			if (viewParams.viewAt != Plot3dCamera::evaFront)
				font.print(title, DT_CENTER);
		}
		else if (type == Y)
		{
			if (viewParams.viewAt == Plot3dCamera::evaFront)
				glTranslated(-0.15 * axis[X].fmax, fmax * 0.5, -axis[Z].fmax * 1.2);
			else if (viewParams.viewAt == Plot3dCamera::evaTop)
				glTranslated(-0.15 * axis[X].fmax, fmax * 0.5, axis[Z].fmax * 1.2);
			else
				glTranslated(-0.15 * axis[X].fmax, fmax * 0.5, axis[Z].fmax * 1.2);
			glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
			if (viewParams.viewAt == Plot3dCamera::evaFront)
				glRotated(-90, 0, 1, 0);
			else
				glRotated(180, 0, 1, 0);
			glRotated(180, 1, 0, 0);
			glRotated(-90, 0, 0, 1);
			glScaled(textScale.x, textScale.y, textScale.z);

			if (viewParams.viewAt == Plot3dCamera::evaFront)
				font.print(title, DT_CENTER|DT_VCENTER);
			else
				font.print(title, DT_CENTER);
		}
		else if (type == Z)
		{
			if (viewParams.viewAt == Plot3dCamera::evaFront)
				glTranslated(-0.1 * axis[X].fmax, 0.5 * axis[Y].fmax, axis[Z].fmax * 1.2);
			else if (viewParams.viewAt == Plot3dCamera::evaTop)
				glTranslated(0.5 * axis[X].fmax, 1.1 * axis[Y].fmax, axis[Z].fmax * 1.2);
			else if (modelview[0] > 0)
				glTranslated(-0.1 * axis[X].fmax, -0.1 * axis[Y].fmax, axis[Z].fmax * 1.2);
			else
				glTranslated(-0.1 * axis[X].fmax, -0.1 * axis[Y].fmax, axis[Z].fmax * 1.2);
		

			glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
			glRotated(rot.cz, 0, 0, -1);
			glRotated(rot.cx, -1, 0, 0);
			glRotated(rot.cy, 0, -1, 0);
			glScaled(textScale.x, textScale.y, textScale.z);

			font.print(title, DT_CENTER);
		}
		glEnable(GL_LIGHT1);
		glPopAttrib();
		glPopMatrix();
	}

	void OglAxis::drawLabels(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Point3d& textScale, const Plot3dParams& viewParams, const OglFont& font) const
	{
		glPushAttrib(GL_LIST_BIT);
		glColor4d(0.0, 0.0, 0.0, 1.0);
		if (type == X || type == Y)
		{
			double z = axis[Z].fmax;

			for (double val = fmin; val < fmax + step / 2; val += step)
			{
				if (type == X && viewParams.viewAt == Plot3dCamera::evaFront)
					continue;

				glLineStipple(1, 0x8888);
				glEnable(GL_LINE_STIPPLE);
				glBegin(GL_LINE_STRIP);
				if (type == X)
				{
					glVertex3d(val, 0.0, z);
					glVertex3d(val, -0.04*axis[Y].fmax, z);
				}
				else
				{
					glVertex3d(0.0, val, z);
					glVertex3d(-0.04 * axis[X].fmax, val, z);
				}
				glEnd();
				glDisable(GL_LINE_STIPPLE);

				glPushMatrix();
				if (type == X)
				{
					glTranslated(val, -0.06 * axis[Y].fmax, z);
					glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);

					glRotated(rot.cz, 0, 0, -1);
					glRotated(rot.cx, -1, 0, 0);
					glRotated(rot.cy, 0, -1, 0);

					glScaled(textScale.x, textScale.y, textScale.z);

					std::string str = ::stringFormat("%.0f", val);
					if (viewParams.viewAt == Plot3dCamera::evaFront)
						font.print(str, DT_CENTER);
					else if (viewParams.viewAt == Plot3dCamera::evaTop)
						font.print(str, DT_CENTER);
					else
						font.print(str, DT_VCENTER | DT_RIGHT);
				}
				else
				{
					if (viewParams.viewAt == Plot3dCamera::evaFront)
						glTranslated(-0.1 * axis[X].fmax, val, -z-axis[Z].fmax*0.1);
					else if (viewParams.viewAt == Plot3dCamera::evaTop)
						glTranslated(-0.05 * axis[X].fmax, val, z + axis[Z].fmax * 0.02);
					else
						glTranslated(-0.1*axis[X].fmax, val, z);
					glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);

					glRotated(rot.cz, 0, 0, -1);
					glRotated(rot.cx, -1, 0, 0);
					glRotated(rot.cy, 0, -1, 0);

					glScaled(textScale.x, textScale.y, textScale.z);

					std::string str = ::stringFormat("%.0f", val);
					if (viewParams.viewAt == Plot3dCamera::evaFront)
						font.print(str, DT_CENTER);
					else
						font.print(str, DT_VCENTER | DT_RIGHT);
				}

				//glListBase(1000);
				//glCallLists(str.length(), GL_UNSIGNED_BYTE, str.c_str());
				glPopMatrix();
			}
		}
		else if (type == Z && viewParams.viewAt != Plot3dCamera::evaTop)
		{
			for (double val = fmin; val < fmax + step / 2; val += step)
			{
				std::string str = stringFormat("%.0f", val);

				glPushMatrix();
				glTranslated(0.01*axis[X].fmax, axis[Y].fmax * 1.11, val);
				glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
				glScaled(textScale.x, textScale.y, textScale.z);
				glRotated(90.0, 0.0, 0.0, 1.0);
				glRotated(-270.0, 1.0, 0.0, 0.0);

				font.print(str, DT_VCENTER);
				glPopMatrix();

				if (viewParams.viewAt != Plot3dCamera::evaFront)
				{
					glPushMatrix();
					glTranslated(1.15 * axis[X].fmax, axis[Y].fmax * 0.01, val);
					glScaled(1 / scale.x, 1 / scale.y, 1 / scale.z);
					glScaled(textScale.x, textScale.y, textScale.z);
					glRotated(90.0, 1.0, 0.0, 0.0);
					glRotated(180.0, 0.0, 1.0, 0.0);

					font.print(str, DT_VCENTER);

					glPopMatrix();
				}
			}
		}
		glPopAttrib();
	}

	// End of OglAxis implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OglShp implementation

	/////////////////
	// Construction
	OglSph::OglSph()
	{
		m_bUseVertexColors = false;
	}

	OglSph::~OglSph()
	{
		reset();
	}

	void OglSph::reset()
	{
		OglSurface::reset();
	}



	bool OglSph::create(const ShProjection& shp, COLORREF clr, BYTE transparency, int depth, const Point3d& center, const Point3d& norm, double gammaDeg)
	{
		reset();

		m_bUseVertexColors = false;
		m_dwSingleColor = clr;

		m_ptCenter = center;

		std::vector<Point3d> sphere;
		std::vector<int> indices;

		initialize_sphere(sphere, indices, depth);

		DWORD dwAlpha = transparency;
		m_nVerts = sphere.size();
		m_pfVerts = new float[3 * m_nVerts];
		m_pfNorms = new float[3 * m_nVerts];
		m_pdwColors = new DWORD[m_nVerts];
		for (int i = 0; i < m_nVerts; i++)
		{
			m_pfVerts[i * 3 + 0] = (float)sphere[i].x;
			m_pfVerts[i * 3 + 1] = (float)sphere[i].y;
			m_pfVerts[i * 3 + 2] = (float)sphere[i].z;
			m_pfNorms[i * 3 + 0] = m_pfVerts[i * 3 + 0];
			m_pfNorms[i * 3 + 1] = m_pfVerts[i * 3 + 1];
			m_pfNorms[i * 3 + 2] = m_pfVerts[i * 3 + 2];
			m_pdwColors[i] = (dwAlpha << 24) | (0x00FFFFFF & clr);
		}

		m_nIndexes = indices.size();
		m_pnIndexes = new int[m_nIndexes];
		for (int i = 0; i < m_nIndexes; i++)
		{
			m_pnIndexes[i] = indices[i];
		}

		double valMin = DBL_MAX;
		double valMax = -DBL_MAX;
		for (int i = 0; i < m_nVerts; i++)
		{
			Point3d pt(m_pfVerts[3 * i + 0], m_pfVerts[3 * i + 1], m_pfVerts[3 * i + 2]);
			EleAz ea(pt);

			double val = shp.evaluate(ea);
			if (val < valMin)
				valMin = val;
			if (val > valMax)
				valMax = val;
		}

		for (int i = 0; i < m_nVerts; i++)
		{
			Point3d pt(m_pfVerts[3 * i + 0], m_pfVerts[3 * i + 1], m_pfVerts[3 * i + 2]);
			EleAz ea(pt);

			double val = shp.evaluate(ea);
			pt *= fabs(val);
			m_pfVerts[3 * i + 0] = (float)pt.x;
			m_pfVerts[3 * i + 1] = (float)pt.y;
			m_pfVerts[3 * i + 2] = (float)pt.z;

			//m_pdwColors[i] = getColorFromRange(val, valMin, valMax, transparency);
		}


		return true;
	}

	void OglSph::scale(double scale)
	{
		for (int i = 0; i < m_nVerts; i++)
		{
			m_pfVerts[3 * i + 0] *= (float)scale;
			m_pfVerts[3 * i + 1] *= (float)scale;
			m_pfVerts[3 * i + 2] *= (float)scale;
		}
	}

	///////////////////////////
	// Implementation
	void OglSph::subdivide(const Point3d& v1, const Point3d& v2, const Point3d& v3, std::vector<Point3d>& sphere_points, std::vector<int>& indices, unsigned int depth)
	{
		if (depth == 0) {
			indices.push_back(sphere_points.size() * 3);
			sphere_points.push_back(v1);
			indices.push_back(sphere_points.size() * 3);
			sphere_points.push_back(v2);
			indices.push_back(sphere_points.size() * 3);
			sphere_points.push_back(v3);
			return;
		}
		const Point3d v12 = (v1 + v2).normalize();
		const Point3d v23 = (v2 + v3).normalize();
		const Point3d v31 = (v3 + v1).normalize();
		subdivide(v1, v12, v31, sphere_points, indices, depth - 1);
		subdivide(v2, v23, v12, sphere_points, indices, depth - 1);
		subdivide(v3, v31, v23, sphere_points, indices, depth - 1);
		subdivide(v12, v23, v31, sphere_points, indices, depth - 1);
	}

	void OglSph::initialize_sphere(std::vector<Point3d>& sphere_points, std::vector<int>& indices, unsigned int depth)
	{
		const double X = 0.525731112119133606;
		const double Z = 0.850650808352039932;
		const Point3d vdata[12] = {
			{-X, 0.0, Z}, { X, 0.0, Z }, { -X, 0.0, -Z }, { X, 0.0, -Z },
			{ 0.0, Z, X }, { 0.0, Z, -X }, { 0.0, -Z, X }, { 0.0, -Z, -X },
			{ Z, X, 0.0 }, { -Z, X, 0.0 }, { Z, -X, 0.0 }, { -Z, -X, 0.0 }
		};
		int tindices[20][3] = {
			{0, 4, 1}, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
			{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
			{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
			{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
		};
		for (int i = 0; i < 20; i++)
			subdivide(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], sphere_points, indices, depth);
	}


	// End of OglSph implementation
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}