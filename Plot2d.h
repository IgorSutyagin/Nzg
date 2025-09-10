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
#include "Archive.h"

namespace nzg
{
	class Plot2d;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Axis2d interface
	class Axis2d
	{
	public:
		enum Format
		{
			eaDefault = 0,
			eaTime,
			eaDateTime,
			eaDateHours,
			eaExp,
			eaInt,
			eaDateTimeGPS
		};
		enum Type
		{
			eatX = 1,
			eatY = 2
		};

	public:
		Axis2d();
		~Axis2d();

	// Attributes:
	public:
		Type m_type;
		bool m_auto;
		Format m_format;
		CRect m_r;
		std::string m_title;
		double m_min;
		double m_max;
		int m_divs;
		int m_precision;
		CFont m_font;
		static CSize c_snd;

		bool isTime() const {
			return m_format == eaTime || m_format == eaDateTime || m_format == eaDateTimeGPS || m_format == eaDateHours;
		}

		// Operations:
	public:
		virtual void onDataChanged(double dDataMin, double dDataMax);
		virtual void layout(CDC* pDC, CRect rClient, Plot2d* pPlot) = 0;
		virtual void draw(CDC* pDC, Plot2d* pPlot) = 0;
		void setPrecision(int nPrec) {
			m_precision = nPrec;
		}
	};
	// End of Axis2d interface
	///////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// XAxis2d interface
	class XAxis2d : public Axis2d
	{
		// Construction:
	public:
		XAxis2d();
		~XAxis2d();

		// Attributes:
	public:
		int m_nHalfFontHeight;

		// Operations:
	public:
		virtual void onDataChanged(double dataMin, double dataMax);
		void onDataChangedGPS(double dataMin, double dataMax);
		virtual void layout(CDC* pDC, CRect rClient, Plot2d* pPlot);
		virtual void draw(CDC* pDC, Plot2d* pPlot);
	};
	// End of XAxis2d interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// YAxis2d interface
	class YAxis2d : public Axis2d
	{
		// Construction:
	public:
		YAxis2d();
		~YAxis2d();

		// Attributes:
	public:

		// Operations:
	public:
		virtual void layout(CDC* pDC, CRect rClient, Plot2d* pPlot);
		virtual void draw(CDC* pDC, Plot2d* pPlot);
	};
	// End of YAxis2d interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// YAxis2dSecondary interface
	class YAxis2dSecondary : public Axis2d
	{
		// Construction:
	public:
		YAxis2dSecondary();
		~YAxis2dSecondary();

		// Attributes:
	public:

		// Operations:
	public:
		virtual void layout(CDC* pDC, CRect rClient, Plot2d* pPlot);
		virtual void draw(CDC* pDC, Plot2d* pPlot);
	};
	// End of YAxis2dSecondary interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Curve2d interface
	class Curve2d
	{
	public:
		enum Style
		{
			eSolid = 0,
			eLineAndPoints,
			ePoints,
			eSmallPoints,
			eAverage,
			eBar,
			eHistogram,
			eHistogramSolid,
			eDashed
		};

		enum PointStyle
		{
			epsNone = 0,
			epsRect,
			epsCircle,
			epsDiamond,
			epsCross,
			epsCrossRotated
		};

		struct Step
		{
			Step(double ax, double ayMin, double ayMax)
			{
				x = ax; yMin = ayMin, yMax = ayMax;
			}
			double x;
			double yMin;
			double yMax;
		};

		// Construction:
	public:
		Curve2d();
		~Curve2d();

		enum ArchType
		{
			eatCurve = 0,
			eatBarCurve = 1,
			eatBandCurve = 2,
			eatRectCurve = 3
		};

		virtual ArchType getArchType() const {
			return eatCurve;
		}
		//static IvsCurve* create(CArchive& ar);

		// Attributes:
	public:
		int m_id;
		bool m_bSecondaryAxis;
		std::string m_name;
		Point2d m_ptMin;
		Point2d m_ptMax;
		Style m_eStyle;
		bool m_bAutoColor;
		COLORREF m_rgb;
		int m_nWidth;
		int getCount() const { return m_pts.size(); }
		Point2d GetPoint(int i) { return m_pts[i]; }
		double m_dMaxPointDistance;
		CPen m_pen;
		bool m_bDrawCurve;
		CRect m_rCheckBox;
		bool m_bSel; // Set to true if the curve is selected
		PointStyle m_eps;
		bool m_bDirection; // Draw arrows to show direction of the curve


		void deletePens();
		void createPens(int nStyle, int nWidth, COLORREF clr);
		void createPens();
		bool hitTest(CPoint pt, const CRect& rectArea, Point2d ptMin, Point2d ptMax) const;
		std::string toString(LPCTSTR szArgName, Axis2d::Format eax) const;
		bool fromString(LPCTSTR szData);
		void drawPoint(CDC* pDC, CPoint pt);

	protected:
		CPen m_penGray;
		std::vector <Point2d> m_pts;
		std::vector<COLORREF> m_clrs;
		std::vector<COLORREF> m_clrs2;
		int m_nMarkSize;

		// Operations:
	public:
		void setData(int nID, LPCTSTR szName, std::vector <Point2d>& pts);
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);
		void drawPolar(CDC& dc, Plot2d& wndPlot);
		void drawLegendLine(CDC& dc, CRect rc);
		CPoint mapPoint(int nIndex, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const;
		CPoint mapPoint(const Point2d& ptF, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const;
		Gdiplus::Rect mapRect(const Rect2d& r, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const;
		void setMarks(std::vector<COLORREF>& clrs, int nMarkSize);
		void setMarks(std::vector<COLORREF>& clrs, std::vector<COLORREF>& clrs2, int nMarkSize);
		bool isSuccessiveArguments() const;
		double getY(double x) const;

		// Implementation:
	protected:
		void drawHistogram(CDC* PDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);
		void drawHistogramSolid(CDC* PDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);
	};
	// End of Curve2d interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// BarCurve2d
	class BarCurve2d : public Curve2d
	{
		// Construction:
	public:
		BarCurve2d();
		~BarCurve2d();

		virtual ArchType getArchType() const {
			return eatBarCurve;
		}

		// Attributes:
	public:
		std::vector <double> m_dbs;
		std::vector <double> m_dWidths; // Bar widths

		// Operations:
	public:
		void setData(int nID, LPCTSTR szName, std::vector <Point2d>& pts, std::vector<double>& dWidths, std::vector <double>& dbs);
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);
		//virtual void serialize(CArchive& ar);
	};
	// End of CIvsBarCurve interface
	/////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// BandCurve
	class BandCurve2d : public Curve2d
	{
	// Construction:
	public:
		BandCurve2d();
		~BandCurve2d();

		virtual ArchType getArchType() const {
			return eatBandCurve;
		}
	// Attributes:
	public:
		std::vector<Point2d> m_ptEdges;

	// Operations:
	public:
		void setData(int nID, LPCTSTR szName, std::vector <Point2d>& pts, std::vector<Point2d>& ptMinMax);
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);

	};
	// End of BandCurve interface
	//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// RectCurve2d interface - a number of rectangles
	class RectCurve2d : public Curve2d
	{
	// Consturction:
	public:
		RectCurve2d();
		~RectCurve2d();

		virtual ArchType getArchType() const {
			return eatRectCurve;
		}

	// Attributes:
	public:
		std::vector <Rect2d> m_rects;

	// Operations:
	public:
		void setData(int nID, LPCTSTR szName, std::vector<Rect2d>& rects);
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass);
	};
	// End of RectCurve2d interface
	//////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// SpecGrid interface
	class SpecGrid
	{
		// Construction:
	public:
		SpecGrid(int nID);
		~SpecGrid();

		enum ArchType
		{
			eatGrid = 0,
			eatCircle = 1
		};

		virtual ArchType getArchType() const {
			return eatGrid;
		}

		// Attributes:
	public:
		int m_id;

		// Operations
	public:
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax);
		CPoint mapPoint(const Point2d& ptF, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax);
		//virtual void serialize(CArchive& ar);
	};
	// End of SpecGrid interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// SpecGridCircle interface
	class SpecGridCircle : public SpecGrid
	{
		// Construction:
	public:
		SpecGridCircle(int nID, double x, double y, double rad, COLORREF clr, int nWidth, int nStyle);
		~SpecGridCircle();

		virtual ArchType getArchType() const {
			return eatCircle;
		}

		// Attributes:
	public:
		Point2d m_ptCenter;
		double m_dRad;
		CPen m_pen;

		// Operations
	public:
		virtual void draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax);
	};
	// End of SpecGridCircle interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Inscription interface
	class Inscription
	{
		// Construction:
	public:
		Inscription();
		~Inscription();

		enum Type
		{
			etRect,
			etCircle
		};

		// Attributes:
	public:
		int m_id;
		std::string m_text;
		int m_nAlign;
		COLORREF m_clr;
		Point2d m_ptPos;
		Type m_et;
		CFont* m_pFont;

		// Operations:
	public:
		void draw(CDC& dc, Plot2d* pPlot);
		//virtual void serialize(CArchive& ar);
	};
	// End of Inscription interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// PlotTitle interface
	class PlotTitle
	{
		// Construction:
	public:
		PlotTitle();
		~PlotTitle();

		enum Type
		{
			etTop
		};

		// Attributes:
	public:
		int m_id;
		std::string m_text;
		int m_nAlign;
		COLORREF m_clr;
		CRect m_rect;
		Type m_et;
		CFont m_font;

		// Operations:
	public:
		bool isEmpty() const {
			return m_text.empty();
		}
		void layout(CDC& dc, const CRect& rectClient);
		void draw(CDC& dc, Plot2d* pPlot);
		//virtual void serialize(CArchive& ar);
	};
	// End of PlotTitle interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// PlotTable - a table on the plot
	class PlotTable
	{
		// Construction:
	public:
		PlotTable();
		~PlotTable();

		enum ColType
		{
			ectText = 0,
			ectColor
		};

		struct Col
		{
			ColType ect;
			std::string str;
			COLORREF rgb;
			CRect rect;
		};

		struct Row
		{
			std::vector<Col> cols;
		};

		// Attributes:
	public:
		bool isEmpty() const {
			return m_rows.size() == 0;
		}
		std::vector<Row> m_rows;
		CRect m_rect;
		int m_nPos; // 0, 1, 2, 3

		// Operations:
	public:
		void layout(CDC* pDC, CRect rectClient, Plot2d* pPlot);
		void draw(CDC* pDC, Plot2d* pPlot);
	};
	// End of PlotTable interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// HistGroup - histogram with grouping
	class HistGroup
	{
	// Construction:
	public:
		HistGroup();
		~HistGroup();
		void reset() {
			groups.clear();
			types.clear();
		}

		struct Group
		{
			std::string name; // Name of the group to plot on X axis
			std::vector<std::pair<int, double>> vals; // Values type and values defining the heights of hist bars
			CRect rect; // Rectangle of the group

			double getVal(int key) const {
				auto it = std::find_if(vals.begin(), vals.end(), [&](auto& a) { return a.first == key; });
				if (it == vals.end())
					return NAN;
				return it->second;
			}
		};

		struct Type
		{
			Type() : key(0), clr(0) {}
			Type(int key_, const char* name_, COLORREF clr_) : key(key_), name(name_), clr(clr_) {}
			int key;
			std::string name;
			COLORREF clr;
			CRect rectLegend;
		};

	// Attributes:
	public:
		std::string name;
		std::map<int, Type> types; // Types of values
		std::vector<Group> groups;
		int colWidth; // The width of the column for values
		int space; // space between columns
		int maxCols; // Maximum number of columns in groups

		bool isEmpty() const {
			return groups.size() == 0;
		}

		void getMinMax(double& vmin, double& vmax) const;

		void addGroup(const char* name, const double* pv, const char** valNames, const COLORREF * pclrs, int valCount);
		void addGroup(const char* name, const std::vector<double>& vs, const std::vector<const char *>& valNames, const std::vector<COLORREF>& clrs);

		void layout(CDC* pDC, CRect rectClient, Plot2d* pPlot);
		void draw(CDC* pDC, Plot2d* pPlot, std::vector<HistGroup>& hgs, int nPass);
	};
	// End if HistGroup interface
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Plot2d window
	#define IVSPLOT2D_CLASSNAME "IvsPlot2d"
	#define CURVES_MAX		40

	#define IVSPLOT_FIRST               0x0010
	#define IVSPLOT_LAST                0x0020


	#define IVSPLOT_HITPOINT			IVSPLOT_FIRST + 1

	#define ID_EXPORT_TO_EXCEL			IVSPLOT_FIRST + 1
	#define ID_PLOT_COPY_TO_CLIPBOARD	IVSPLOT_FIRST + 2
	#define ID_PLOT_COPY_CURVE			IVSPLOT_FIRST + 3
	#define ID_PLOT_PASTE_CURVE			IVSPLOT_FIRST + 4
	#define ID_PLOT_COPY_DATA			IVSPLOT_FIRST + 5

	#define IVSPLOT_CLIPBOARD_CURVE		"IvsPlotClipboardCurve"

	class Plot2d : public CWnd
	{
	public:
		struct CMarker
		{
			int nID;
			BOOL bX;
			double dCoord;
			double dCoordOld;
			COLORREF clr;
		};

		struct CEvent
		{
			int nID;
			double x;
			COLORREF clr;
			LPCTSTR szTitle;
		};

		struct CArea
		{
			int nID;
			double x0;
			double x1;
			COLORREF clr;
			LPCTSTR szTitle;
		};

		enum CMode
		{
			emDecart = 0,
			emPolar = 1
		};

		struct CEllipse
		{
			int nID;
			Point2d pt;
			Size2d size;
			COLORREF clr;

			void serialize(Archive& ar)
			{
				DWORD dwVer = 1;
				if (ar.isStoring())
				{
					ar << dwVer;
					ar << nID;
					ar << pt;
					ar << size;
					ar << clr;
				}
				else
				{
					ar >> dwVer;
					ar >> nID;
					ar >> pt;
					ar >> size;
					ar >> clr;
				}
			}
		};

		struct PlotRange
		{
			double xMin;
			double xMax;
			int xDivs;
			double yMin;
			double yMax;
			int yDivs;
			std::vector<BOOL> drawCurves;
		};

		// Construction
	public:
		Plot2d();
		void reset();

		// Attributes
	public:
		CMode m_eMode;
		//CPen * m_penCurves[CURVES_MAX];
		COLORREF m_clrCurves[CURVES_MAX];
		static const int c_stdColorsNum = 15;
		static COLORREF c_stdColors[c_stdColorsNum];
		static COLORREF getStdColor(int nc) {
			return c_stdColors[nc % c_stdColorsNum];
		}

		bool m_bSubclassFromCreate;
		bool m_bDoubleBuffer;
		bool m_bFirstPainting;
		COLORREF m_clrBack;
		//CFont m_fontX;
		//CFont m_fontY;
		CBrush m_brushBack;
		CPen m_penGrid;
		CPen m_penAxis;
		std::vector <Curve2d*> m_curves;
		std::vector <Curve2d*> m_curves2;
		std::vector <SpecGrid*> m_grids;
		std::vector <Inscription*> m_inscs;
		std::vector <CEllipse> m_ellipses;

		double m_dCurveMaxPointDistance;

		XAxis2d m_xAxis;
		YAxis2d m_yAxis;
		YAxis2dSecondary m_yAxis2;
		bool m_bEnableSecondaryAxis;

		//CAxisFormat m_eaxFormat[2];
		//BOOL m_bxAuto;
		//BOOL m_byAuto;
		Size2d m_fsizeScale;
		CPoint m_ptPlotOrg;
		Rect2d m_frectPlotArea;
		Rect2d m_frectInitArea;

		Size2d m_fsizeScale2;
		Rect2d m_frectPlotArea2;
		Rect2d m_frectInitArea2;

		//double m_dPolarMin;
		//double m_dPolarMax;
		CRect m_rectPolar;
		CPoint m_ptPolarCenter;
		//int m_nPolarGrids;


		void setInitArea(BOOL bX = TRUE, BOOL bY = TRUE);
		void resetInitArea();
		void resetZoom();
		//int m_nxDiv;
		//int m_nyDiv;
		//CString m_csxTitle;
		//CString m_csyTitle;

		Point2d m_ptLastHit;

		std::vector <CMarker> m_marks;
		std::vector <CEvent> m_events;
		std::vector <CArea> m_areas;
		PlotTable m_table;
		PlotTitle m_plotTitle;

		void addTableItem(int row, int col, COLORREF clr) {
			while (row >= (int)m_table.m_rows.size())
				m_table.m_rows.push_back(PlotTable::Row());
			while (col >= (int)m_table.m_rows[row].cols.size())
				m_table.m_rows[row].cols.push_back(PlotTable::Col());
			m_table.m_rows[row].cols[col].ect = PlotTable::ectColor;
			m_table.m_rows[row].cols[col].rgb = clr;
		}
		void addTableItem(int row, int col, LPCTSTR szText) {
			while (row >= (int)m_table.m_rows.size())
				m_table.m_rows.push_back(PlotTable::Row());
			while (col >= (int)m_table.m_rows[row].cols.size())
				m_table.m_rows[row].cols.push_back(PlotTable::Col());
			m_table.m_rows[row].cols[col].ect = PlotTable::ectText;
			m_table.m_rows[row].cols[col].str = szText;
		}

		void setMenuIDs(int nResID, int nMenuIndex, CWnd* pOwner = NULL, int nCurveMenuIndex = -1) {
			m_nPopupMenuResourceID = nResID; m_nPopupMenuIndex = nMenuIndex; m_nPopupMenuCurveIndex = nCurveMenuIndex; if (pOwner != NULL) m_pPopupOwner = pOwner;
		}
		int m_nPopupMenuResourceID;
		int m_nPopupMenuIndex;
		int m_nPopupMenuCurveIndex;
		CWnd* m_pPopupOwner;
		Curve2d* m_pSelCurveToCopy;

		//CToolTipCtrl m_ttc;
		CSize m_sizeBmp;
		CBitmap m_bmpAll;
		CBitmap m_bmpTip;

		CPoint m_ptLastTip;
		Point2d m_ptTip;
		Point2d m_ptTip2;
		bool m_bTipPainted;
		bool m_bCursorInPlot;
		CFont* m_pLegendFont;

		std::vector<HistGroup> m_hgs;

		void setLegend(bool bLegend, bool bSecondary = false, bool bCheck = true) {
			if (bSecondary)
			{
				m_bLegend2 = bLegend;
				m_bCheckLegend2 = bCheck;
			}
			else
			{
				m_bLegend = bLegend;
				m_bCheckLegend = bCheck;
			}
		}
		void setLegendFont(CFont* pFont)
		{
			m_pLegendFont = pFont;
		}
		bool m_bLegend, m_bLegend2;
		bool m_bCheckLegend, m_bCheckLegend2;
		enum CLegendAlign
		{
			elaTopLeft = 0,
			elaTopRight = 1,
			elaBottomLeft = 2,
			elaBottomRight = 3,
			elaMax
		} m_eLegendAlign, m_eLegendAlign2;

		enum CZoomMode
		{
			ezmOnlyX,
			ezmOnlyY,
			ezmXY
		} m_eZoomMode;
		void setZoomMode(CZoomMode ezm) {
			m_eZoomMode = ezm;
		}

		// Operations
	public:
		bool initControl(CWnd* pParent);

		void removeCurve(int nID);
		void removeAllCurves();
		void addCurve(int nID, LPCTSTR szName, std::vector<Point2d>& pts, int nWidth = 1, Curve2d::Style eStyle = Curve2d::eSolid, COLORREF rgb = 0xFFFFFFFF, bool bSecondary = false, bool bInvalidate = true);
		void addPolarCurve(int nID, LPCTSTR szName, std::vector<Point2d>& pts, int nWidth = 1, Curve2d::Style eStyle = Curve2d::eSolid, COLORREF rgb = 0xFFFFFFFF);
		void addBar(int nID, LPCTSTR szName, std::vector <Point2d>& pts, std::vector <double>& dWidths, std::vector <double>& dbs, int nWidth = 1, Curve2d::Style eStyle = Curve2d::eSolid, COLORREF rgb = 0xFFFFFFFF, bool bSecondary = false);
		void addBand(int nID, LPCTSTR szName, std::vector<Point2d>& pts, std::vector<Point2d>& ptMinMax, int nWidth = 1, COLORREF rgb = 0xFFFFFFFF, bool bSecondary = false);
		void addRect(int nID, LPCTSTR szName, std::vector<Rect2d>& rets, int nWidth = 1, COLORREF rgb = 0xFFFFFFFF, bool bSecondary = false);
		void setCurveMaxPointDistance(double dMaxDistance, int nID, bool bSecondary);
		void setCurveStyle(int nID, Curve2d::Style eStyle, int nWidth, bool bSecondary);
		void addCircle(int nID, double x, double y, double rad, COLORREF clr = 0, int nWidth = 1, int nStyle = PS_SOLID);
		void delCircle(int nID);
		void removeAllCircles();
		void addEllipse(int nID, double x, double y, double dx, double dy, COLORREF clr);
		void removeAllEllipses();
		void addHistGroup(const HistGroup& hg);
		void removeHistGroup();
		void addEvent(int nID, double x, COLORREF clr, LPCTSTR szTitle);
		void clearEvents();
		void addArea(int nID, double x0, double x1, COLORREF clr, LPCTSTR szTitle);
		void clearAreas();
		void addInscription(int nID, LPCTSTR szText, Point2d& ptPos, int nAlign, COLORREF clr, Inscription::Type et = Inscription::etRect, CFont* pFont = nullptr);
		void deleteInscription(int nID = 0);
		void clearInscriptions();
		void addPlotTitle(const char* szText, COLORREF clr = RGB(0, 0, 0));
		void setTitleFont(const LOGFONT& lf);
		void addCurveMarks(int nID, std::vector<COLORREF>& clrs, int nMarkSize);
		void addCurveMarks(int nID, std::vector<COLORREF>& clrs, std::vector<COLORREF>& clrs2, int nMarkSize);
		Curve2d* getCurve(int nID) const;

		void drawBackground(CDC* pDC);
		void drawGrid(CDC* pDC);
		void drawCurves(CDC* pDC, int nPass);
		void drawAxis(CDC* pDC);
		void drawMarkers(CDC* pDC, BOOL bPaint = FALSE);
		void drawEvents(CDC* pDC);
		void drawAreas(CDC* pDC);
		void drawInscriptions(CDC* pDC);
		void drawTip(CDC* pDC);
		void drawLegend(CDC* pDC, BOOL bPrimary);
		void drawHgLegend(CDC* pDC);

		void drawPolarGrid(CDC& dc);
		void drawPolarGridLabels(CDC& dc);
		void drawPolarGridLabelsInt(CDC& dc);
		void drawPolarLegend(CDC& dc);

		void paintTip();
		void setAxisTitle(BOOL bX, LPCTSTR szTitle, bool bSecondary) { if (bX) m_xAxis.m_title = szTitle; else if (bSecondary) m_yAxis2.m_title = szTitle; else m_yAxis.m_title = szTitle; }
		void setAxisFormat(BOOL bX, Axis2d::Format efmt, bool bSecondary = false)
		{
			if (bX)
				m_xAxis.m_format = efmt;
			else
			{
				if (bSecondary)
					m_yAxis2.m_format = efmt;
				else
					m_yAxis.m_format = efmt;
			}
		}
		void setAxis(bool bX, double dMin, double dMax, int nDivisions, bool bSecondary = false);
		void setAxisDivisions(bool bX, int nDivisions, bool bPrimary = true);
		void setAxisPrecision(bool bX, int nPrecision, bool bPrimary = true);
		void autoScale(bool bxAuto, bool byAuto, bool byAuto2);
		void computeScale(CDC* pDC, const CRect& rectClient);
		CPoint scale(double x, double y, bool bSecondary) const;
		CSize scaleSize(double dx, double dy, bool bSecondary) const;
		Point2d reverseScale(int x, int y, bool bSecondary) const;
		void setMarker(int nID, double dCoord, bool bX = true, COLORREF clr = RGB(255, 0, 0));
		void clearMarkers();
		bool isArgsEqual();
		bool hitTestLegend(bool bPrimary, CPoint pt) const;
		Curve2d* hitTestCurve(CPoint pt) const;
		void clearSelCurve();
		int getMaxCurveId() const;
		CRect getPlotRect() const;
		Point2d getPlotTopLeft() const {
			return Point2d(m_frectPlotArea.pt.x, m_frectPlotArea.pt.y + m_frectPlotArea.s.cy);
		}

		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(Plot2d)
		//}}AFX_VIRTUAL
		virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

		PlotRange getPlotRange() const;
		void setPlotRange(const PlotRange& pf);

		void serialize(Archive& ar);

		// Implementation
	public:
		virtual ~Plot2d();
		bool getAutoValues(double dMin, double dMax, double& dAutoMin, double& dAutoMax, int& nDiv);

		// Generated message map functions
	protected:
		//{{AFX_MSG(Plot2d)
		afx_msg void OnPaint();
		void paintPolar(CDC& dc);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		//}}AFX_MSG
		BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnCopyToClipboard();
		afx_msg void OnCopyData();
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	};

	// End of Plot2d interface
	/////////////////////////////////////////////////////////////////////////////

}