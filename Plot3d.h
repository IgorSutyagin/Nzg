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

#include "OglEntity.h"
#include "Node.h"
#include "Angles.h"

namespace nzg
{
	////////////////////////////////////////////////////////////////////////////
	// 3D plot camera position
	struct Plot3dCamera
	{
		Plot3dCamera() : scale(1.0), rotTotal(c_rots[ecDecart][evaDef]) {}
		
		Size3d rot;
		Size3d rotTotal;
		Size3d off;
		Size3d offTotal;
		double scale;
		enum ViewAt
		{
			evaDef = 0,
			evaFront = 1,
			evaTop = 2,
			evaCustom = 3
		};
		enum Coords
		{
			ecDecart = 0,
			ecPolar = 1,
			ecMax
		};
		static const Size3d c_rots[ecMax][evaCustom];

		void serialize(Archive& ar);
	};
	// End of 3D plot camera position
	////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////
	// 3D plot parameters
	struct Plot3dParams
	{
		Plot3dParams() : zMin(-20), zMax(20), zStep(5), isoStep(5), colorStep(5),
			viewAt(Plot3dCamera::evaDef), coords(Plot3dCamera::ecDecart), bGrid0(true), 
			bRainbow(true), eLabels(elAll) {}
		enum Labels
		{
			elAll = 0,
			elHideInvisible = 1,
			elNone = 2
		};

		double zMin;
		double zMax;
		double zStep;
		double isoStep;
		double colorStep;
		Plot3dCamera::ViewAt viewAt;
		Plot3dCamera::Coords coords;
		bool bGrid0;
		bool bRainbow;
		Labels eLabels;

		bool isValid() const {
			if (zStep <= 1e-3)
				return false;
			if (isoStep <= 1e-3)
				return false;
			if (colorStep <= 1e-3)
				return false;
			if (zMin >= zMax)
				return false;
			return true;
		}

		void serialize(Archive& ar);
	};
	// End of 3D plot parameters
	////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////
	// Plot3d - Open GL 3d plot
	#define IVSPLOT3D_CLASSNAME "IvsPlot3d"
	
	class Plot3d : public CWnd
	{
	// Construction:
	public:
		Plot3d();
		virtual ~Plot3d();
		virtual void reset();

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnDestroy();
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void OnEditCopy();

	public:
		void setViewParams(const Plot3dParams& params);
		Plot3dParams& getViewParams() { return m_vp; }
		Plot3dParams getViewParams() const { return m_vp; }
		Plot3dCamera& getCamera() { return m_va[m_vp.coords]; }
		Plot3dCamera getCamera() const { return m_va[m_vp.coords]; }
		bool isTopView() const;
		bool isFrontView() const;
		bool isDefaultView() const;


		// Overrides:
	public:
		virtual void onInitialUpdate();
		virtual void onDraw();
		virtual bool isRainbow() const { return true; }
		virtual bool isLight() const { return true; }
		virtual nzg::OglSurface::Grid getGrid() const { return OglSurface::Grid(0, 360, 361, 0, 90, 91); }
		virtual nzg::Node* getNode() const { return nullptr; }
		//virtual nzg::Gnss::Signal getSignal() const { return Gnss::esigInvalid; }
		virtual double getData(const nzg::Node* node, double x, double y) const { return NAN; }
		virtual void updateData();
		virtual void updateSurface(int id);
		virtual void updateIsolines(int id);
		virtual void updateScale();
		virtual double getIsoStep() const { return m_vp.isoStep; }
		virtual int getMenuIndex() const { return -1; }
		virtual int getCopyWidthMM() const { return 84; }
		virtual std::pair<double, double> getColorRange() const { return std::pair<double, double>(m_axis[2].fmin, m_axis[2].fmax); }
		virtual void onViewChanged();

		void saveView() const;
		void loadView();

		// Implementation:
	protected:
		HDC m_hDC;
		HGLRC m_hRC;
		Cube3d m_cubeScene;
		int m_nSymbolListBase;
		std::vector<OglSurface*> m_surfs;
		OglColumns m_oglColumns;
		LOGFONT m_lf;
		OglFont m_oglFont;
		GLuint m_nPixelFormat;
		CPoint m_ptHit;
		CRect m_rectWalls;
		OglAxis m_axis[3];
		Plot3dParams m_vp;

		Plot3dCamera m_va[2];


		bool setupPixelFormat();
		void glInit() const;
		void setProjection();
		void setupLight() const;
		Point3d getScale() const;
		void snapClient();
		HANDLE snapToMemory();
		void drawRainbow();
	};
	// End of Plot3d interface
	////////////////////////////////////////////////////////////////////////
}
