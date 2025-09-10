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
#pragma once

#include "Points.h"
#include "Sph.h"
//#include "GnssTime.h"
//#include "NavIDs.h"
//#include "LeoModel.h"

#ifndef ARGB
#define ARGB(a, r, g, b) ((a << 24) | RGB(r, g, b))
#endif

namespace nzg
{
	struct Plot3dParams;
	struct Const3dParams;
	class GnssModel;

	//////////////////////////////////////////////////////////////////////////////
	// OglEntity interface
	class OglEntity
	{
		// Construction:
	public:
		OglEntity();
		virtual ~OglEntity();

		// Attributes:
	public:
		Point3d m_ptCenter;

		// Overrides:
	public:
		virtual void draw() = 0;
		virtual void move(const Point3d& ptTo);

		// Implementation:
	protected:
		void onBeginDraw();
		void onEndDraw();
	};
	// End of OglEntity interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// Font for OpenGL
	class OglFont
	{
	// Construction:
	public:
		OglFont();
		~OglFont();
		bool create(const LOGFONT& lf, HDC hDC, int listBase);

	// Attributes:
	public:
		int m_listBase;
		GLYPHMETRICSFLOAT m_gmf[256];

	// Operations:
	public:
		Size2d size(const char* str) const;
		void print(const char* str, size_t len, unsigned int format) const;
		void print(const std::string& str, unsigned int format=0) const {
			print(str.c_str(), str.length(), format);
		}

	};
	// End of OglFont interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// Isoline interface
	class OglIsoline
	{
		// Attributes:
	public:
		OglIsoline();
		OglIsoline(const OglIsoline& a);
		~OglIsoline();
		void reset();
		void createLabels(double dMinLabelLen, double dZScale, std::vector<OglIsoline*>& isos, int nDigs);

		// Attributes:
	public:
		int m_nVerts;
		float* m_pfVerts;
		float* getPoint(int i) const {
			return m_pfVerts + 3 * i;
		}
		double m_dValue;
		double m_dLength;
		Rect2d m_rect;
		std::string m_strLabel;
		bool m_bClosed;
		bool m_bLabelAlongX; // If label aligned with X axis
		int m_nLabelPoint;
		double m_dOffset; // Shift label up by this value 
		bool isClosed() const {
			if (m_nVerts <= 0)
				return false;
			float* pf0 = m_pfVerts;
			float* pf1 = getPoint(m_nVerts - 1);
			return pf0[0] == pf1[0] && pf0[1] == pf1[1];
		}

		// Operations:
	public:
		Point3d getLabelPoint() const {
			if (m_nLabelPoint < 0 || m_nLabelPoint >= m_nVerts)
				return Point3d(NAN, NAN, NAN);
			float* pf = getPoint(m_nLabelPoint);
			return Point3d(pf[0], pf[1], pf[2]);
		}
		OglIsoline& operator=(const OglIsoline& a);
		void allocate(int nPoints);
		void computeLength();
		void draw() const;
		void drawText(const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font);

	};
	// End of OglIsoline interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// OglSurface interface
	class OglSurface : public OglEntity
	{
		// Grid params
	public:
		struct Grid
		{
			Grid() { xMin = xMax = yMin = yMax = 0; nx = ny = 0; }
			Grid(float x0, float x1, int anx, float y0, float y1, int any) { xMin = x0; xMax = x1; nx = anx; yMin = y0; yMax = y1; ny = any; }
			float xMin;
			float xMax;
			int nx;
			float yMin;
			float yMax;
			int ny;

			BOOL operator==(const Grid& a) { return xMin == a.xMin && xMax == a.xMax && nx == a.nx && yMin == a.yMin && yMax == a.yMax && ny == a.ny; }
			Point2d getStep() const { return Point2d((xMax - xMin) / (nx - 1), (yMax - yMin) / (ny - 1)); }
			bool isValid() const {	return xMax > xMin && nx > 1 && yMax > yMin && ny > 1;	}
		};

		struct Edge
		{
			Edge();
			Edge(float* pf1, float* pf2);

			float* pfVerts[2];
			bool operator==(const Edge& e) {
				return (pfVerts[0] == e.pfVerts[1] && pfVerts[1] == e.pfVerts[0]) || (pfVerts[0] == e.pfVerts[0] && pfVerts[1] == e.pfVerts[1]);
			}
			bool isNull() const;
			bool isIntersect(float h) const;
			void appendIntPoint(float h, std::vector<float>& fPts) const;
			Edge& operator=(const Edge& a) {
				pfVerts[0] = a.pfVerts[0]; pfVerts[1] = a.pfVerts[1];
				return *this;
			}
		};

		struct Triangle
		{
			Triangle();
			Triangle(int nIndex, int* pnIndexes, float* pfs);
			int nTriangle;
			int nVerts[3];
			float* pfVerts[3];
			bool bMarked;
			void mark(bool b) {
				bMarked = b;
			}
			Edge getEdge(int i) const;
			void getIntEdges(float h, Edge& e1, Edge& e2) const;
			bool hasEdge(const Edge& edge, float h, Edge* peNext) const;
			bool isValid() const;
		};

		// Construction:
	public:
		OglSurface();
		OglSurface(const OglSurface& a);
		OglSurface(int id, const Grid& grid);
		~OglSurface();
		void reset();
		void fromGrid(const Grid& grid);
		void resetIsolines();

		// Attributes:
	public:
		std::string m_strName;
		std::string m_strX; // X argument name
		std::string m_strY; // Y argument name
		bool m_bNet;
		int m_nID;
		Grid m_grid;
		int m_nVerts;
		float* m_pfVerts;
		int m_nIndexes;
		int* m_pnIndexes;
		float* m_pfNorms;
		DWORD* m_pdwColors;
		bool m_bUseVertexColors;
		DWORD m_dwSingleColor;

		std::vector <OglIsoline*> m_isos;

		// Operations:
	public:
		OglSurface& operator=(const OglSurface& a);
		void draw(const Point3d& scale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font);
		bool createIsoline(float fValue, float fDelta, float fOffset, double dMinLabelLen, double isoStep, double dZScale=1.0);
		void getMinMax(Point3d& ptMin, Point3d& ptMax) const;

		void serialize(CArchive& ar);

		// Overrides:
	public:
		virtual void draw();

		// Implementation:
	public:
		void calcNormal(int i, BOOL bClockwise, float fn[3], double dZScale = 1.0) const;
		void calcNormal(int i, BOOL bClockwise, float fn[3], Point3d scale) const;
		static void normalize(float v[3], float fScale = 1.0);
		static DWORD getColorFromRange(double dVal, double dMin, double dMax, BYTE cAlpha);
		static bool getNextTriangle(const Edge& e, std::vector<Triangle>& trs, float h, Edge* peNext);
	};
	// End of OglSurface interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// OglPointCloud interface
	class OglPointCloud
	{
		// Construction:
	public:
		OglPointCloud();
		virtual ~OglPointCloud();
		void reset();

		// Attributes:
	public:
		int m_nID;
		int m_nVerts;
		float* m_pfVerts;
		DWORD* m_pdwColors;

		// Operations:
	public:
		void draw();

		static DWORD getColorFromRange(double dVal, double dMin, double dMax, BYTE cAlpha);

	};
	// End of OglPointCloud interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// OglColumns
	class OglColumns : public OglPointCloud
	{
	// Construction:
	public:
		OglColumns();
		virtual ~OglColumns();

	// Attributes:
	public:
		Size2d m_size;

	// Operations:
	public:
		void draw();

	// Implementation:
	protected:
		void drawColumn(float x, float y, float z, DWORD dwColor);
	};
	// End of OglColumns
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// OglAxis interface
	class OglAxis
	{
	// Construction:
	public:
		enum Type
		{
			X = 0,
			Y = 1,
			Z = 2
		};

		OglAxis() : type(X), fmin(0), fmax(1), step(0.1) {}
		OglAxis(OglAxis::Type type_, double fmin_, double fmax_, double step_) : type(type_), fmin(fmin_), fmax(fmax_), step(step_) {}
		~OglAxis() {}

	// Attributes:
	public:
		Type type;
		double fmin;
		double fmax;
		double step;
		std::string title;

		Point3d getMinPt() const {
			return type == X ? Point3d(fmin, 0, 0) : type == Y ? Point3d(0, fmin, 0) : Point3d(0, 0, fmin);
		}
		Point3d getMaxValuePt() const {
			return type == X ? Point3d(fmax, 0, 0) : type == Y ? Point3d(0, fmax, 0) : Point3d(0, 0, fmax);
		}
		Point3d getMaxPt() const {
			return getMaxValuePt() * 1.1;
		}

	// Operations:
	public:
		void draw(const OglAxis axis[3], const Point3d& ptScale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font) const;
		void draw(const OglAxis axis[3], const Point3d& ptScale, const Size3d& rot, const Const3dParams& viewParams, const OglFont& font) const;
		void drawPolar(const Point3d& ptScale, const Size3d& rot, const Plot3dParams& viewParams, const OglFont& font) const;


	// Implementation:
	protected:
		// height - the height of the arrow
		// rad - radius of the base of the arrow in percents of maximum value
		// n - number of segments for the arrow cone
		void drawArrow(const OglAxis axis[3], double height, double rad, int n) const;

		void drawTitles(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Point3d& textScale, const Plot3dParams& viewParams, const OglFont& font) const;

		void drawLabels(const OglAxis axis[3], const Point3d& scale, const Size3d& rot, const Point3d& textScale, const Plot3dParams& viewParams, const OglFont& font) const;

	};
	// End of OglAxis interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// OglSph - spherical harmonics
	class OglSph : public OglSurface
	{
		// Construction:
	public:
		OglSph();
		virtual ~OglSph();
		void reset();

		// Create a spherical projection with the center, orientation norm and rotation gammaDeg
		// Gedesic sphere triangulation based on Icosahedral, 
		// the 'depth' is the power of 2 during Icosahedral triangles division
		bool create(const ShProjection& shp, COLORREF clr, BYTE transparency, int depth = 2, const Point3d& center = { 0, 0 ,0 }, const Point3d& norm = { 0, 0, 1 }, double gammaDeg = 0.0);
		void scale(double scale);

		// Implementation:
	public:
		static void subdivide(const Point3d& v1, const Point3d& v2, const Point3d& v3, std::vector<Point3d>& sphere_points, std::vector<int>& indices, unsigned int depth);
		static void initialize_sphere(std::vector<Point3d>& sphere_points, std::vector<int>& indices, unsigned int depth);

		//Point3d m_ptCenter;
	};
	// End of OglSph interface
	//////////////////////////////////////////////////////////////////////////////

}