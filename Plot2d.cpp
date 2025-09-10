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

#include "Angles.h"
#include "Tools.h"
#include "Plot2d.h"

using namespace nzg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Axis2d implementation
#define MAX_ROUNDS 8
double gl_dRounds[MAX_ROUNDS] = { 0.10F, 0.15F, 0.20F, 0.25F, 0.30F, 0.40F, 0.50F, 0.60F };


CSize Axis2d::c_snd = CSize(2, 2);

//////////////////////////
// Construction:
Axis2d::Axis2d()
{
	m_type = eatX;
	m_auto = true;
	m_format = eaDefault;
	m_min = 0.0;
	m_max = 1.0;
	m_divs = 5;
	m_precision = 2;
}

Axis2d::~Axis2d()
{
}

void Axis2d::onDataChanged(double dDataMin, double dDataMax)
{
	if (!m_auto)
		return;

	if (_isnan(dDataMin) || _isnan(dDataMax))
		return;

	if (isinf(dDataMin) || isinf(dDataMax))
		return;

	double fD;

	//ASSERT(dDataMin <= dDataMax);
	//if (dDataMin <= dDataMax)
	//	return;

	// 1. Choose Data Range in the first approximation
	fD = dDataMax - dDataMin;

	if (fD <= 0)
	{
		TRACE("Can't change axis parameters in GetXAxisMinMax\n");
		return;
	}

	int nDeg;
	{
		nDeg = (int)log10(fD);
		fD = fD / pow(10.0, nDeg);
		if (fD > 3.0)
		{
			fD /= 10;
			nDeg++;
		}
		if (fD <= 0.3)
		{
			fD *= 10;
			nDeg--;
		}
	}

	// 2. Choose Label distance
	double dLabelDistance = 0;
	//if (m_bLabelDistanceAuto)
	{
		double tmp, tmpmin = DBL_MAX;
		int k, j;

		for (int i = 0; i < MAX_ROUNDS; i++)
		{
			tmp = fD / gl_dRounds[i];
			k = (int)ceil(tmp);
			tmp = fabs(tmp - floor(tmp)) * gl_dRounds[i];
			if ((tmpmin > tmp) && (k >= 2) && (k <= 6))
			{
				j = i;
				tmpmin = tmp;
			}
		}
		dLabelDistance = gl_dRounds[j] * (float)pow(10.0, (double)nDeg);
	}

	// 3. Choose MinAxis & MaxAxis
	m_min = floor(dDataMin / dLabelDistance) * dLabelDistance;
	m_max = ceil(dDataMax / dLabelDistance) * dLabelDistance;
	m_divs = (int)((m_max - m_min) / dLabelDistance);
}

// End of Axis2d implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XAxis2d implementation

XAxis2d::XAxis2d()
{
	m_type = eatX;
	m_nHalfFontHeight = 0;
}

XAxis2d::~XAxis2d()
{
}

void XAxis2d::onDataChanged(double dDataMin, double dDataMax)
{
	if (m_format != eaDateTime && m_format != eaDateHours)
	{
		Axis2d::onDataChanged(dDataMin, dDataMax);
		return;
	}
	else if (m_format == eaDateTimeGPS)
	{
		onDataChangedGPS(dDataMin, dDataMax);
		return;
	}

	if (!m_auto)
		return;

	if (_isnan(dDataMin) || _isnan(dDataMax))
		return;

	if (isinf(dDataMin) || isinf(dDataMax))
		return;

	if (dDataMin > dDataMax)
	{
		dDataMin = (double)getDate(CTime::GetCurrentTime()).GetTime();
		dDataMax = 24 * 60 * 60;
	}

	CTime tMin((time_t)dDataMin);
	tMin = CTime(tMin.GetYear(), tMin.GetMonth(), tMin.GetDay(), tMin.GetHour(), 0, 0);
	CTime tMax((time_t)dDataMax);
	int nHour = tMax.GetHour();
	if (nHour == 23)
		tMax = CTime(tMax.GetYear(), tMax.GetMonth(), tMax.GetDay(), 0, 0, 0) + CTimeSpan(1, 0, 0, 0);
	else
		tMax = CTime(tMax.GetYear(), tMax.GetMonth(), tMax.GetDay(), tMax.GetHour() + 1, 0, 0);

	CTimeSpan ts = tMax - tMin;
	int nTotHours = (int)ts.GetTotalHours();

	if (nTotHours <= 0)
	{
		int nTotMinutes = (int)ts.GetTotalMinutes();
		if (nTotMinutes <= 0)
		{
			int nTotSeconds = (int)ts.GetTotalSeconds();
			m_divs = 1;
		}
		else
		{
			m_divs = 1;
		}
	}
	else if (nTotHours < 6)
	{
		m_divs = nTotHours;
	}
	else
	{
		m_divs = 6;
		int nd = nTotHours / m_divs;
		if (nd * m_divs != nTotHours)
			tMax += CTimeSpan(0, 1, 0, 0);
	}

	m_min = (double)tMin.GetTime();
	m_max = (double)tMax.GetTime();
}

void XAxis2d::onDataChangedGPS(double dataMin, double dataMax)
{
	if (!m_auto)
		return;

	if (_isnan(dataMin) || _isnan(dataMax))
		return;

	if (isinf(dataMin) || isinf(dataMax))
		return;

	if (dataMin > dataMax)
	{
		dataMin = (double)getDate(CTime::GetCurrentTime()).GetTime();
		dataMax = 24 * 60 * 60;
	}

	CTimeEx tMin((time_t)dataMin);
	tMin = CTime(tMin.getYearGmt(), tMin.getMonthGmt(), tMin.getDayGmt(), tMin.getHourGmt(), 0, 0, CTimeEx::etrGmt);
	CTimeEx tMax((__time32_t)dataMax);
	int nHour = tMax.getHourGmt();
	if (nHour == 23)
		tMax = CTime(tMax.getYearGmt(), tMax.getMonthGmt(), tMax.getDayGmt(), 0, 0, 0, CTimeEx::etrGmt) + CTimeSpan(1, 0, 0, 0);
	else
		tMax = CTime(tMax.getYearGmt(), tMax.getMonthGmt(), tMax.getDayGmt(), tMax.getHourGmt() + 1, 0, 0, CTimeEx::etrGmt);

	CTimeSpan ts = tMax - tMin;
	int nTotHours = (int)ts.GetTotalHours();

	if (nTotHours <= 0)
	{
		int nTotMinutes = (int)ts.GetTotalMinutes();
		if (nTotMinutes <= 0)
		{
			int nTotSeconds = (int)ts.GetTotalSeconds();
			m_divs = 1;
		}
		else
		{
			m_divs = 1;
		}
	}
	else if (nTotHours < 6)
	{
		m_divs = nTotHours;
	}
	else
	{
		m_divs = 6;
		int nd = nTotHours / m_divs;
		if (nd * m_divs != nTotHours)
			tMax += CTimeSpan(0, 1, 0, 0);
	}

	m_min = (double)tMin.GetTime();
	m_max = (double)tMax.GetTime();
}

void XAxis2d::layout(CDC* pDC, CRect rClient, Plot2d* pPlot)
{
	int nHeight = 0;

	nHeight += c_snd.cy;

	CFont* pOldFont = (CFont*)pDC->SelectObject(&m_font);

	CRect r(0, 0, 0, 0);
	pDC->DrawText("0123456789", r, DT_CALCRECT | DT_SINGLELINE);
	m_nHalfFontHeight = r.Height() / 2;
	nHeight += r.Height() * 3 / 2 + c_snd.cy;

	if (m_format == eaDateTime || m_format == eaDateHours || m_format == eaDateTimeGPS)
	{
		nHeight += r.Height() + c_snd.cy;
	}

	if (!m_title.empty())
	{
		CRect r(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), r, DT_CALCRECT | DT_SINGLELINE);
		nHeight += r.Height() + c_snd.cy;
	}

	m_r = rClient;
	m_r.top = m_r.bottom - nHeight;

	pDC->SelectObject(pOldFont);
}

void XAxis2d::draw(CDC* pDC, Plot2d* pPlot)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject(&m_font);

	double dx = (m_max - m_min) / m_divs; //(m_frectPlotArea.ptTop.x - m_frectPlotArea.ptBase.x) / m_nxDiv;

	BOOL bSeveralDates = FALSE;
	if (m_format == eaDateTime || m_format == eaDateHours || m_format == eaDateTimeGPS)
	{
		CTime t0((time_t)m_min);
		CTime t1((time_t)m_max);
		if (t0.GetYear() == t1.GetYear() && t0.GetMonth() == t1.GetMonth() && t0.GetDay() == t1.GetDay())
			bSeveralDates = FALSE;
		else
			bSeveralDates = TRUE;
	}

	for (double x = m_min; (x <= m_max + dx / 5) && (dx != 0); x += dx)
	{
		CPoint pt = pPlot->scale(x, pPlot->m_frectPlotArea.pt.y, FALSE);
		pt.y += c_snd.cy + m_nHalfFontHeight;
		CString cs;
		switch (m_format)
		{
		default:
		case eaDefault:
			cs.Format("%.*f", m_precision, x);
			break;
		case eaExp:
			cs.Format("%.*g", m_precision, x);
			break;
			/*
		case eaxDefault:
			cs.Format("%.2f", x);
			break;
		case eaxExp:
			cs.Format("%.4g", x);
			break;
			*/
		case eaInt:
			cs.Format("%.0f", x);
			break;
		case eaTime:
		case eaDateTime:
		{
			CTime t((time_t)x);
			cs = t.Format("%H:%M:%S");
		}
		break;
		case eaDateTimeGPS:
		{
			CTime t((time_t)x);
			cs = t.FormatGmt("%H:%M:%S");
		}
		break;
		case eaDateHours:
		{
			CTime t((time_t)x);
			cs = t.Format("%H:%M");
		}
		break;
		}
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(cs, &rect, DT_CALCRECT);
		CSize size = rect.Size();
		rect.OffsetRect(CPoint(pt.x - size.cx / 2, pt.y));
		pDC->DrawText(cs, &rect, DT_CENTER | DT_SINGLELINE);

		CPoint ptTop = pPlot->scale(x, pPlot->m_frectPlotArea.top(), FALSE);
		pDC->MoveTo(ptTop);
		pDC->LineTo(pt);

		if (m_format == eaDateTime || m_format == eaDateHours)
		{
			CTime t((time_t)x);
			if (//bSeveralDates ||
				(t.GetHour() == 0 && t.GetMinute() == 0 && t.GetSecond() == 0) ||
				(x == m_min) ||
				(x + dx > m_max + dx / 5))
			{
				cs = t.Format("%d.%m.%y");
				CRect r(0, 0, 0, 0);
				pDC->DrawText(cs, &r, DT_CALCRECT);
				CSize size = r.Size();
				r.OffsetRect(CPoint(pt.x - size.cx / 2, pt.y + size.cy + c_snd.cy));
				//rect.OffsetRect(0, size.cy + c_snd.cy);
				pDC->DrawText(cs, &r, DT_CENTER | DT_SINGLELINE);
			}
		}
		else if (m_format == eaDateTimeGPS)
		{
			CTimeEx t((time_t)x);
			if (//bSeveralDates ||
				(t.getHourGmt() == 0 && t.GetMinute() == 0 && t.GetSecond() == 0) ||
				(x == m_min) ||
				(x + dx > m_max + dx / 5))
			{
				cs = t.FormatGmt("%d.%m.%y");
				CRect r(0, 0, 0, 0);
				pDC->DrawText(cs, &r, DT_CALCRECT);
				CSize size = r.Size();
				r.OffsetRect(CPoint(pt.x - size.cx / 2, pt.y + size.cy + c_snd.cy));
				//rect.OffsetRect(0, size.cy + c_snd.cy);
				pDC->DrawText(cs, &r, DT_CENTER | DT_SINGLELINE);
			}
		}
	}

	if (!m_title.empty())
	{
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), &rect, DT_CALCRECT);
		CSize size = rect.Size();
		double dMid = (m_min + m_max) / 2;
		CPoint pt = pPlot->scale(dMid, pPlot->m_frectPlotArea.bottom(), FALSE);
		//pt.y += c_snd.cy;
		pt.y = m_r.bottom - size.cy - c_snd.cy;
		rect.OffsetRect(pt.x - size.cx / 2, pt.y);
		pDC->DrawText(m_title.c_str(), &rect, DT_SINGLELINE);
	}

	pDC->SelectObject(pOldFont);
}

// End of XAxis2d implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// YAxis2d implementation

YAxis2d::YAxis2d()
{
	m_type = eatY;
	m_precision = 2;
}

YAxis2d::~YAxis2d()
{
}

void YAxis2d::layout(CDC* pDC, CRect rClient, Plot2d* pPlot)
{
	int nWidth = 0;

	nWidth += c_snd.cx;

	CFont* pOldFont = (CFont*)pDC->SelectObject(&(pPlot->m_xAxis.m_font));

	int nMaxWidth = 0;
	double dy = (m_max - m_min) / m_divs; //(m_frectPlotArea.ptTop.x - m_frectPlotArea.ptBase.x) / m_nxDiv;
	//for (double y = m_min; (y <= m_max + dy / 5) && (dy != 0); y += dy)
	for (int nDiv = 0; nDiv < m_divs + 1; nDiv++)
	{
		double y = m_min + nDiv * dy;
		CString cs;
		switch (m_format)
		{
		default:
		case eaDefault:
			cs.Format("%.*f", m_precision, y);
			break;
		case eaExp:
			cs.Format("%.*g", m_precision, y);
			break;
		case eaInt:
			cs.Format("%.0f", y);
			break;
		case eaTime:
		case eaDateTime:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M:%S");
		}
		break;
		case eaDateTimeGPS:
		{
			CTime t((time_t)y);
			cs = t.FormatGmt("%H:%M:%S");
		}
		break;
		case eaDateHours:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M");
		}
		break;
		}
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(cs, &rect, DT_CALCRECT);
		CSize size = rect.Size();
		if (size.cx > nMaxWidth)
			nMaxWidth = size.cx;
	}

	nWidth += nMaxWidth + c_snd.cy;

	if (!m_title.empty())
	{
		pDC->SelectObject(&m_font);
		CRect r(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), r, DT_CALCRECT | DT_SINGLELINE);
		nWidth += r.Height() * 3 / 2 + c_snd.cx;
	}

	m_r = rClient;
	m_r.right = m_r.left + nWidth;

	pDC->SelectObject(pOldFont);
}

void YAxis2d::draw(CDC* pDC, Plot2d* pPlot)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject(&(pPlot->m_xAxis.m_font));
	//CFont * pOldFont = (CFont *)pDC->SelectObject(&m_font);

	double dy = (m_max - m_min) / m_divs; //(m_frectPlotArea.ptTop.x - m_frectPlotArea.ptBase.x) / m_nxDiv;
	for (double y = m_min; (y <= m_max + dy / 5) && (dy != 0); y += dy)
	{
		CPoint pt = pPlot->scale(pPlot->m_frectPlotArea.left(), y, false);
		pt.x -= c_snd.cx;
		CString cs;
		switch (m_format)
		{
		default:
		case eaDefault:
			cs.Format("%.*f", m_precision, y);
			break;
		case eaExp:
			cs.Format("%.*g", m_precision, y);
			break;
		case eaInt:
			cs.Format("%.0f", y);
			break;
		case eaTime:
		case eaDateTime:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M:%S");
		}
		break;
		case eaDateTimeGPS:
		{
			CTime t((time_t)y);
			cs = t.FormatGmt("%H:%M:%S");
		}
		break;
		case eaDateHours:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M");
		}
		break;
		}
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(cs, &rect, DT_CALCRECT);
		CSize size = rect.Size();
		rect.OffsetRect(CPoint(pt.x - size.cx, pt.y - size.cy / 2));
		pDC->DrawText(cs, &rect, DT_CENTER | DT_SINGLELINE);

		CPoint ptTop = pPlot->scale(pPlot->m_frectPlotArea.right(), y, false);
		pDC->MoveTo(ptTop);
		pDC->LineTo(pt);
	}

	if (!m_title.empty())
	{
		pDC->SelectObject(&m_font);
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), &rect, DT_CALCRECT);
		CSize size = rect.Size();
		double dMid = (m_min + m_max) / 2;
		CPoint pt = pPlot->scale(pPlot->m_frectPlotArea.left(), dMid, false);
		pt.x = m_r.left + c_snd.cx;
		pDC->TextOut(pt.x, pt.y + rect.Size().cx / 2, m_title.c_str(), m_title.length());
	}

	pDC->SelectObject(pOldFont);
}

// End of YAxis2d implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// YAxis2dSecondary implementation

YAxis2dSecondary::YAxis2dSecondary()
{
	m_type = eatY;
}

YAxis2dSecondary::~YAxis2dSecondary()
{
}

void YAxis2dSecondary::layout(CDC* pDC, CRect rClient, Plot2d* pPlot)
{
	int nWidth = 0;

	nWidth += c_snd.cx;

	CFont* pOldFont = (CFont*)pDC->SelectObject(&(pPlot->m_xAxis.m_font));

	int nMaxWidth = 0;
	double dy = (m_max - m_min) / m_divs; //(m_frectPlotArea.ptTop.x - m_frectPlotArea.ptBase.x) / m_nxDiv;
	for (double y = m_min; (y <= m_max + dy / 5) && (dy != 0); y += dy)
	{
		CString cs;
		switch (m_format)
		{
		default:
		case eaDefault:
			cs.Format("%.*f", m_precision, y);
			break;
		case eaExp:
			cs.Format("%.*g", m_precision, y);
			break;
		case eaInt:
			cs.Format("%.0f", y);
			break;
		case eaTime:
		case eaDateTime:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M:%S");
		}
		break;
		case eaDateTimeGPS:
		{
			CTime t((time_t)y);
			cs = t.FormatGmt("%H:%M:%S");
		}
		break;
		case eaDateHours:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M");
		}
		break;
		}
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(cs, &rect, DT_CALCRECT);
		CSize size = rect.Size();
		if (size.cx > nMaxWidth)
			nMaxWidth = size.cx;
	}

	nWidth += nMaxWidth + c_snd.cy;

	if (!m_title.empty())
	{
		pDC->SelectObject(&m_font);
		CRect r(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), r, DT_CALCRECT | DT_SINGLELINE);
		nWidth += r.Height() * 3 / 2 + c_snd.cx;
	}

	m_r = rClient;
	m_r.left = m_r.right - nWidth;

	pDC->SelectObject(pOldFont);
}

void YAxis2dSecondary::draw(CDC* pDC, Plot2d* pPlot)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject(&(pPlot->m_xAxis.m_font));

	double dy = (m_max - m_min) / m_divs; 
	for (double y = m_min; (y <= m_max + dy / 5) && (dy != 0); y += dy)
	{
		CPoint pt = pPlot->scale(pPlot->m_frectPlotArea.right(), y, true);
		pt.x += c_snd.cx;
		CString cs;
		switch (m_format)
		{
		default:
		case eaDefault:
			cs.Format("%.*f", m_precision, y);
			break;
		case eaExp:
			cs.Format("%.*g", m_precision, y);
			break;
		case eaInt:
			cs.Format("%.0f", y);
			break;
		case eaTime:
		case eaDateTime:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M:%S");
		}
		break;
		case eaDateTimeGPS:
		{
			CTime t((time_t)y);
			cs = t.FormatGmt("%H:%M:%S");
		}
		break;
		case eaDateHours:
		{
			CTime t((time_t)y);
			cs = t.Format("%H:%M");
		}
		break;
		}
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(cs, &rect, DT_CALCRECT);
		CSize size = rect.Size();
		rect.OffsetRect(CPoint(pt.x + c_snd.cx, pt.y - size.cy / 2)); //CPoint (pt.x - size.cx, pt.y - size.cy / 2));
		pDC->DrawText(cs, &rect, DT_SINGLELINE | DT_RIGHT); // DT_CENTER | 

		CPoint ptTop = pPlot->scale(pPlot->m_frectPlotArea.right(), y, true);
		pDC->MoveTo(ptTop);
		pDC->LineTo(pt);
	}

	if (!m_title.empty())
	{
		pDC->SelectObject(&m_font);
		CRect rect(0, 0, 0, 0);
		pDC->DrawText(m_title.c_str(), &rect, DT_CALCRECT);
		CSize size = rect.Size();
		double dMid = (m_min + m_max) / 2;
		CPoint pt = pPlot->scale(pPlot->m_frectPlotArea.right(), dMid, true);
		pDC->DrawText(m_title.c_str(), &rect, DT_SINGLELINE);
		pt.x = m_r.right + c_snd.cx - rect.Height();
		pDC->TextOut(pt.x, pt.y + rect.Size().cx / 2, m_title.c_str(), m_title.length());
	}

	pDC->SelectObject(pOldFont);
}

// End of YAxis2dSecondary implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Curve2d interface

///////////////////
// Construction:
Curve2d::Curve2d()
{
	m_id = 0;
	m_bSecondaryAxis = false;
	m_eStyle = eSolid;
	m_bAutoColor = true;
	m_rgb = RGB(0, 0, 0);
	m_nWidth = 1;
	m_dMaxPointDistance = NAN;
	m_bDrawCurve = true;
	m_bSel = false;
	m_bDirection = false;
	m_eps = epsCircle;
}

Curve2d::~Curve2d()
{
}

void Curve2d::deletePens()
{
	if (m_pen.m_hObject != NULL)
		m_pen.DeleteObject();
	if (m_penGray.m_hObject != NULL)
		m_penGray.DeleteObject();
}

void Curve2d::createPens(int nStyle, int nWidth, COLORREF clr)
{
	deletePens();

	m_rgb = clr;
	m_nWidth = nWidth;

	m_pen.CreatePen(nStyle, nWidth, clr);

	//unsigned short h, l, s;
	ColorHLS hls = ColorRGB(clr); // rgb2hls(clr, h, l, s);

	hls.l += (255 - hls.l) / 3;
	clr = hls; // hls2rgb(h, l, s);
	m_penGray.CreatePen(nStyle, 1, clr); // nStyle
}

void Curve2d::createPens()
{
	deletePens();
	m_pen.CreatePen(m_eStyle == eDashed ? PS_DASH : PS_SOLID, m_nWidth, m_rgb);
}

bool Curve2d::hitTest(CPoint ptClient, const CRect& rectArea, Point2d ptMin, Point2d ptMax) const
{
	CRect rc(ptClient.x - 5, ptClient.y - 5, ptClient.x + 5, ptClient.y + 5);
	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		CPoint pt = mapPoint(i, rectArea, ptMin, ptMax);
		if (rc.PtInRect(pt))
			return true;
	}

	return false;
}

std::string Curve2d::toString(LPCTSTR szArgName, Axis2d::Format eax) const
{
	std::string strData = stringFormat("%s\t%s\r\n", szArgName, m_name.c_str());
	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		Point2d pt = m_pts[i];
		std::string str;
		if (eax == Axis2d::eaTime)
			str = stringFormat("%s\t%f\r\n", (LPCTSTR)CTime((time_t)pt.x).Format("%H:%M:%S"), pt.y);
		else if (eax == Axis2d::eaDateHours || eax == Axis2d::eaDateTime)
			str = stringFormat("%s\t%f\r\n", (LPCTSTR)CTime((time_t)pt.x).Format("%d.%m.%Y %H:%M:%S"), pt.y);
		else if (eax == Axis2d::eaDateTimeGPS)
		{
			double ms = (pt.x - floor(pt.x)) * 1000;
			str = stringFormat("%s %.0f\t%f\r\n", (LPCTSTR)CTime((time_t)pt.x).FormatGmt("%d.%m.%Y %H:%M:%S"), ms, pt.y);
		}
		else
			str = stringFormat("%f\t%f\r\n", pt.x, pt.y);
		strData += str;
	}

	return strData;
}

bool Curve2d::fromString(LPCTSTR szData)
{
	LPCTSTR p = szData;
	LPCTSTR szSeps = "\t\r\n";
	while (*p)
	{
		double x = atof(p);
		p += strcspn(p, szSeps);
		p += strspn(p, szSeps);
		double y = atof(p);
		p += strspn(p, szSeps);
		m_pts.push_back(Point2d(x, y));
	}

	return m_pts.size() > 0;
}

/////////////////////
// Operations:
void Curve2d::setData(int nID, LPCTSTR szName, std::vector<Point2d>& pts)
{
	m_pts.resize(pts.size());
	if (m_pts.size() == 0)
	{
		m_ptMin = Point2d(0, 0);
		m_ptMax = Point2d(1, 1);
	}

	m_ptMin = Point2d(DBL_MAX, DBL_MAX);
	m_ptMax = Point2d(-DBL_MAX, -DBL_MAX);

	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		m_pts[i] = pts[i];
		if (m_ptMin.x > pts[i].x)
			m_ptMin.x = pts[i].x;
		if (m_ptMin.y > pts[i].y)
			m_ptMin.y = pts[i].y;
		if (m_ptMax.x < pts[i].x)
			m_ptMax.x = pts[i].x;
		if (m_ptMax.y < pts[i].y)
			m_ptMax.y = pts[i].y;
	}
}

void Curve2d::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	if (getCount() <= 0)
		return;

	if (!m_bDrawCurve)
		return;

	if (m_eStyle == eHistogram)
	{
		drawHistogram(pDC, rectArea, ptMin, ptMax, nPass);
		return;
	}
	else if (m_eStyle == eHistogramSolid)
	{
		drawHistogramSolid(pDC, rectArea, ptMin, ptMax, nPass);
		return;
	}

	if (nPass > 0)
	{
		if (m_eStyle != eBar)
			return;
	}

	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_pen);


	Point2d ptAverage(0, 0);
	pDC->MoveTo(mapPoint(0, rectArea, ptMin, ptMax));
	int i = 0;
	for (i = 0; i < (int)m_pts.size(); i++)
	{
		if (m_eStyle == eBar)
		{
			CPoint pt0 = mapPoint(Point2d(m_pts[i].x, ptMin.y), rectArea, ptMin, ptMax);
			CPoint pt1 = mapPoint(i, rectArea, ptMin, ptMax);

			if (nPass == 0)
			{
				CRect r(pt1.x - m_nWidth / 2, pt1.y, pt0.x + m_nWidth / 2, pt0.y);
				pDC->FillSolidRect(r, m_rgb);
			}
			else
			{
				CString cs;
				cs.Format("%s %.1f", m_name.c_str(), m_pts[i].y);
				CRect r;
				pDC->DrawText(cs, r, DT_CALCRECT | DT_SINGLELINE);

				int nd = 2;
				r = CRect(pt1.x - r.Width() / 2 - nd, pt1.y - nd - r.Height(), pt1.x + r.Width() / 2 + nd, pt1.y - nd);
				pDC->DrawText(cs, r, DT_SINGLELINE | DT_CENTER);
			}
		}
		else if (i && (m_eStyle == eSolid || m_eStyle == eLineAndPoints))
		{
			if (_isnan(m_pts[i - 1].y) || _isnan(m_pts[i - 1].x))
			{
				if (!_isnan(m_pts[i].y) && !_isnan(m_pts[i].x))
					pDC->MoveTo(mapPoint(i, rectArea, ptMin, ptMax));
				//continue;
			}
			else
			{

				CPoint pt0 = mapPoint(i - 1, rectArea, ptMin, ptMax);
				pDC->MoveTo(pt0);

				if (_isnan(m_pts[i].x) || _isnan(m_pts[i].y))
					continue;

				if (!_isnan(m_dMaxPointDistance))
				{
					double dDist = m_pts[i].x - m_pts[i - 1].x;
					if (fabs(dDist) > m_dMaxPointDistance)
					{
						pDC->SelectObject(&m_penGray);
						CPoint pt2 = mapPoint(i, rectArea, ptMin, ptMax);
						CPoint pt1(pt2.x, pt0.y);
						//pDC->LineTo(pt1);
						//pDC->LineTo(pt2);
						pDC->MoveTo(pt2);
						pDC->SelectObject(&m_pen);
						//continue;
					}
					else
						pDC->LineTo(mapPoint(i, rectArea, ptMin, ptMax));
				}
				else
					pDC->LineTo(mapPoint(i, rectArea, ptMin, ptMax));

				if (m_bDirection)
				{
					CPoint p0 = pt0;
					CPoint p1 = mapPoint(i, rectArea, ptMin, ptMax);
					Line2d seg(Point2d(p0.x, p0.y), Point2d(p1.x, p1.y));
					double len = seg.getLen();

					if (len > 5)
					{

						Line2d per = seg.getPerp(0.4, m_nWidth * 2);

						Point2d pp0 = per.getPoint(1);
						Point2d pp1 = per.getPoint(-1);
						Point2d ppm = seg.getPoint(0.5);
						pDC->MoveTo((int)pp0.x, (int)pp0.y);
						pDC->LineTo((int)ppm.x, (int)ppm.y);
						pDC->LineTo((int)pp1.x, (int)pp1.y);

						pDC->MoveTo(mapPoint(i, rectArea, ptMin, ptMax));
					}
				}
			}
		}

		if (m_eStyle == ePoints || m_eStyle == eLineAndPoints)
		{
			if (!_isnan(m_pts[i].x) && !_isnan(m_pts[i].y))
			{
				CPoint pt = mapPoint(i, rectArea, ptMin, ptMax);
				ptAverage.x += m_pts[i].x;
				ptAverage.y += m_pts[i].y;
				drawPoint(pDC, pt);
				/*
				CPoint pts[4];
				pts[0] = CPoint(pt.x - m_nWidth * 2, pt.y - m_nWidth * 2);
				pts[1] = CPoint(pt.x + m_nWidth * 2, pt.y + m_nWidth * 2);
				pts[2] = CPoint(pt.x - m_nWidth * 2, pt.y + m_nWidth * 2);
				pts[3] = CPoint(pt.x + m_nWidth * 2, pt.y - m_nWidth * 2);
				pDC->MoveTo(pts[0]); pDC->LineTo(pts[1]);
				pDC->MoveTo(pts[2]); pDC->LineTo(pts[3]);
				*/
			}
		}
		else if (m_eStyle == eSmallPoints)
		{
			if (!_isnan(m_pts[i].x) && !_isnan(m_pts[i].y))
			{
				CPoint pt = mapPoint(i, rectArea, ptMin, ptMax);
				ptAverage.x += m_pts[i].x;
				ptAverage.y += m_pts[i].y;
				pDC->Ellipse(pt.x - 1, pt.y - 1, pt.x + 1, pt.y + 1);
			}
		}
	}

	if ((m_eStyle == eAverage) && (i > 1))
	{
		ptAverage.x /= i;
		ptAverage.y /= i;
		CPoint pt = mapPoint(ptAverage, rectArea, ptMin, ptMax);
		pDC->Ellipse(pt.x - 4, pt.y - 4, pt.x + 4, pt.y + 4);
	}

	if (m_clrs2.size() == m_pts.size())
	{
		for (int i = 0; i < (int)m_pts.size(); i++)
		{
			CPoint pt = mapPoint(m_pts[i], rectArea, ptMin, ptMax);
			CPen pen0(PS_SOLID, 0, m_clrs[i]);
			CBrush br0(m_clrs[i]);
			CPen pen1(PS_SOLID, 0, m_clrs2[i]);
			CBrush br1(m_clrs2[i]);

			CPen* pOldPen = (CPen*)pDC->SelectObject(&pen0);
			CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&br0);

			//pDC->Ellipse(pt.x - m_nMarkSize, pt.y - m_nMarkSize, pt.x + m_nMarkSize, pt.y + m_nMarkSize);
			pDC->Pie(pt.x - m_nMarkSize, pt.y - m_nMarkSize, pt.x + m_nMarkSize, pt.y + m_nMarkSize,
				pt.x, pt.y - m_nMarkSize, pt.x, pt.y + m_nMarkSize);

			pDC->SelectObject(&pen1);
			pDC->SelectObject(&br1);

			pDC->Pie(pt.x - m_nMarkSize, pt.y - m_nMarkSize, pt.x + m_nMarkSize, pt.y + m_nMarkSize,
				pt.x, pt.y + m_nMarkSize, pt.x, pt.y - m_nMarkSize);

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
	}
	else if (m_clrs.size() == m_pts.size())
	{
		for (int i = 0; i < (int)m_pts.size(); i++)
		{
			CPoint pt = mapPoint(m_pts[i], rectArea, ptMin, ptMax);
			CPen pen(PS_SOLID, 1, m_clrs[i]);
			CBrush br(m_clrs[i]);

			CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);
			CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&br);

			pDC->Ellipse(pt.x - m_nMarkSize, pt.y - m_nMarkSize, pt.x + m_nMarkSize, pt.y + m_nMarkSize);

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
	}


	if (m_bSel)
	{
		int nStep = m_pts.size() / 10;
		if (nStep == 0)
			nStep = 1;
		for (i = 0; i < (int)m_pts.size(); i++)
		{
			if ((i % nStep) == 0)
			{
				CPoint pt0 = mapPoint(i, rectArea, ptMin, ptMax);
				CRect r(pt0.x - 5, pt0.y - 5, pt0.x + 5, pt0.y + 5);
				pDC->MoveTo(r.TopLeft());
				pDC->LineTo(r.right, r.top);
				pDC->LineTo(r.right, r.bottom);
				pDC->LineTo(r.left, r.bottom);
				pDC->LineTo(r.TopLeft());
			}
		}
	}

	pDC->SelectObject(pOldPen);
}

void Curve2d::drawPoint(CDC* pDC, CPoint pt)
{
	int nSize = m_nWidth; // 5
	if (m_eps == epsRect)
	{
		pDC->MoveTo(pt.x - nSize, pt.y - nSize);
		pDC->LineTo(pt.x + nSize, pt.y - nSize);
		pDC->LineTo(pt.x + nSize, pt.y + nSize);
		pDC->LineTo(pt.x - nSize, pt.y + nSize);
		pDC->LineTo(pt.x - nSize, pt.y - nSize);
	}
	else if (m_eps == epsCircle)
	{
		CBrush* pOld = (CBrush*)pDC->SelectStockObject(HOLLOW_BRUSH);
		pDC->Ellipse(pt.x - nSize, pt.y - nSize, pt.x + nSize, pt.y + nSize);
		pDC->SelectObject(pOld);
	}
}

void Curve2d::drawPolar(CDC& dc, Plot2d& wndPlot)
{
	if (m_eStyle == eSolid)
	{
		CPen* pOldPen = (CPen*)dc.SelectObject(&m_pen);

		int i = 0;
		bool bMove = true;
		for (i = 0; i < (int)m_pts.size(); i++)
		{
			Point2d ptf = m_pts[i];
			if (_isnan(ptf.y))
			{
				bMove = true;
				continue;
			}
			double r = ptf.y - wndPlot.m_yAxis.m_min;
			double a = ptf.x;

			int nx = (int)(r * sin(a * PI / 180) * wndPlot.m_fsizeScale.cy);
			int ny = (int)(-r * cos(a * PI / 180) * wndPlot.m_fsizeScale.cy);

			CPoint pt = wndPlot.m_ptPolarCenter + CPoint(nx, ny);
			if (bMove)
			{
				dc.MoveTo(pt);
				bMove = false;
			}
			else
				dc.LineTo(pt);
		}


		dc.SelectObject(pOldPen);
	}
}

void Curve2d::drawLegendLine(CDC& dc, CRect rc)
{
	if (m_eStyle == eSolid)
	{
		CPen* pOldPen = (CPen*)dc.SelectObject(&m_pen);

		dc.MoveTo(rc.left, (rc.top + rc.bottom) / 2);
		dc.LineTo(rc.right, (rc.top + rc.bottom) / 2);

		dc.SelectObject(pOldPen);
	}
}

void Curve2d::drawHistogram(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	ASSERT(m_eStyle == eHistogram);

	if (nPass > 0)
	{
		return;
	}

	int nColWidth = 2;
	if (m_pts.size() > 1)
	{
		CPoint pt0 = mapPoint(0, rectArea, ptMin, ptMax);
		CPoint pt1 = mapPoint(1, rectArea, ptMin, ptMax);
		nColWidth = pt1.x - pt0.x;
	}

	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_pen);

	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		CPoint pt0 = mapPoint(Point2d(m_pts[i].x, ptMin.y), rectArea, ptMin, ptMax);
		CPoint pt1 = mapPoint(i, rectArea, ptMin, ptMax);
		CRect rect(pt0.x - nColWidth / 2, pt1.y, pt0.x + nColWidth / 2, pt0.y);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right, rect.top);
		pDC->LineTo(rect.right, rect.bottom);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->LineTo(rect.left, rect.top);
	}

	pDC->SelectObject(pOldPen);
}

void Curve2d::drawHistogramSolid(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	ASSERT(m_eStyle == eHistogramSolid);

	if (nPass > 0)
	{
		return;
	}

	int nColWidth = 2;
	if (m_pts.size() > 1)
	{
		CPoint pt0 = mapPoint(0, rectArea, ptMin, ptMax);
		CPoint pt1 = mapPoint(1, rectArea, ptMin, ptMax);
		nColWidth = pt1.x - pt0.x;
	}

	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_pen);
	CBrush br(m_rgb);
	//CBrush * pOldBrush = (CBrush *)pDC->SelectObject(&br);

	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		CPoint pt0 = mapPoint(Point2d(m_pts[i].x, ptMin.y), rectArea, ptMin, ptMax);
		CPoint pt1 = mapPoint(i, rectArea, ptMin, ptMax);
		CRect rect(pt0.x - nColWidth / 2 + 1, pt1.y, pt0.x + nColWidth / 2 - 1, pt0.y);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right, rect.top);
		pDC->LineTo(rect.right, rect.bottom);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->LineTo(rect.left, rect.top);
		pDC->FillRect(rect, &br);
	}

	pDC->SelectObject(pOldPen);
	//pDC->SelectObject(pOldBrush);
}

CPoint Curve2d::mapPoint(int nIndex, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const
{
	int x = (int)(rectArea.left + (rectArea.right - rectArea.left) / (ptMax.x - ptMin.x) * (m_pts[nIndex].x - ptMin.x));
	int y = (int)(rectArea.bottom - (rectArea.bottom - rectArea.top) / (ptMax.y - ptMin.y) * (m_pts[nIndex].y - ptMin.y));
	return CPoint(x, y);
}

CPoint Curve2d::mapPoint(const Point2d& ptF, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const
{
	int x = (int)(rectArea.left + (rectArea.right - rectArea.left) / (ptMax.x - ptMin.x) * (ptF.x - ptMin.x));
	int y = (int)(rectArea.bottom - (rectArea.bottom - rectArea.top) / (ptMax.y - ptMin.y) * (ptF.y - ptMin.y));
	return CPoint(x, y);
}

Gdiplus::Rect Curve2d::mapRect(const Rect2d& r, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax) const
{
	CPoint pt0 = mapPoint(Point2d(r.left(), r.top()), rectArea, ptMin, ptMax);
	CPoint pt1 = mapPoint(Point2d(r.right(), r.bottom()), rectArea, ptMin, ptMax);
	return Gdiplus::Rect(Gdiplus::Point(pt0.x, pt0.y), Gdiplus::Size(pt1.x - pt0.x, pt1.y - pt0.y));
}

void Curve2d::setMarks(std::vector<COLORREF>& clrs, int nMarkSize)
{
	ASSERT(m_pts.size() == clrs.size());
	m_clrs = clrs;
	m_nMarkSize = nMarkSize;
}

void Curve2d::setMarks(std::vector<COLORREF>& clrs, std::vector<COLORREF>& clrs2, int nMarkSize)
{
	ASSERT(m_pts.size() == clrs.size());
	ASSERT(m_pts.size() == clrs2.size());
	m_clrs = clrs;
	m_clrs2 = clrs2;
	m_nMarkSize = nMarkSize;
}

bool Curve2d::isSuccessiveArguments() const
{
	for (int np = 1; np < (int)m_pts.size(); np++)
	{
		if (m_pts[np - 1].x >= m_pts[np].x)
			return false;
	}

	return true;
}

double Curve2d::getY(double x) const
{
	auto it = std::find_if(m_pts.begin(), m_pts.end(), [&](const Point2d& a) { return a.x == x; });
	if (it == m_pts.end())
		return NAN;
	return it->y;
}

// End of Curve2d implementation
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// BarCurve2d implementation

////////////////////////
// Construction:
BarCurve2d::BarCurve2d()
{
}

BarCurve2d::~BarCurve2d()
{
}

///////////////////////////////
// Operaitons:

void BarCurve2d::setData(int nID, LPCTSTR szName, std::vector <Point2d>& pts, std::vector<double>& dWidths, std::vector <double>& dbs)
{
	Curve2d::setData(nID, szName, pts);

	m_dbs.resize(dbs.size());
	for (int i = 0; i < (int)dbs.size(); i++)
		m_dbs[i] = dbs[i];

	m_dWidths.resize(dWidths.size());
	for (int i = 0; i < (int)dWidths.size(); i++)
	{
		m_dWidths[i] = dWidths[i];
	}
}

void BarCurve2d::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	if (getCount() <= 0)
		return;

	if (nPass > 0)
	{
		if (m_eStyle != eBar)
			return;
	}

	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_pen);

	Point2d ptAverage(0, 0);
	pDC->MoveTo(mapPoint(0, rectArea, ptMin, ptMax));
	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		if (m_eStyle == eBar)
		{
			CPoint pt0 = mapPoint(Point2d(m_pts[i].x, m_dbs[i]), rectArea, ptMin, ptMax);
			CPoint pt1 = mapPoint(Point2d(m_pts[i].x, m_dbs[i] + m_pts[i].y), rectArea, ptMin, ptMax);

			if (nPass == 0)
			{
				COLORREF clrOld = pDC->GetBkColor();
				CRect r(pt1.x - m_nWidth / 2, pt1.y, pt0.x + m_nWidth / 2, pt0.y);
				pDC->FillSolidRect(r, m_rgb);
				pDC->SetBkColor(clrOld);
			}
			else
			{
				CString cs;
				cs.Format("%s %.1f", m_name.c_str(), m_pts[i].y);
				CRect r(0, 0, 0, 0);
				pDC->DrawText(cs, r, DT_CALCRECT | DT_SINGLELINE);

				int nd = 2;
				r = CRect(pt1.x - r.Width() / 2 - nd, pt1.y - nd - r.Height(), pt1.x + r.Width() / 2 + nd, pt1.y - nd);
				pDC->DrawText(cs, r, DT_SINGLELINE | DT_CENTER);
			}
		}
	}

	pDC->SelectObject(pOldPen);
}

// End of BarCurve2d implementation
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// BandCurve2d implementation

/////////////////////////
// Construction:
BandCurve2d::BandCurve2d()
{
}

BandCurve2d::~BandCurve2d()
{
}

/////////////////////////////
// Operations:
void BandCurve2d::setData(int nID, LPCTSTR szName, std::vector <Point2d>& pts, std::vector<Point2d>& ptMinMax)
{
	ASSERT(pts.size() == ptMinMax.size());

	Curve2d::setData(nID, szName, pts);

	for (int i = 0; i < (int)m_pts.size(); i++)
	{
		if (m_ptMin.y > ptMinMax[i].x)
			m_ptMin.y = ptMinMax[i].x;
		if (m_ptMax.y < ptMinMax[i].y)
			m_ptMax.y = ptMinMax[i].y;
	}


	m_ptEdges.clear();
	m_ptEdges.push_back(pts[0]);
	for (int i = 0; i < (int)pts.size(); i++)
	{
		if (isnan(ptMinMax[i].y))
			continue;
		
		m_ptEdges.push_back(Point2d (pts[i].x, ptMinMax[i].y));
	}
	m_ptEdges.push_back(pts[pts.size() - 1]);
	for (int i = pts.size() - 1; i >= 0; i--)
	{
		if (isnan(ptMinMax[i].x))
			continue;

		m_ptEdges.push_back(Point2d(pts[i].x, ptMinMax[i].x));
	}
	m_ptEdges.push_back(pts[0]);
}

void BandCurve2d::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	using namespace Gdiplus;

	if (!m_bDrawCurve)
		return;

	if (nPass == 0)
	{
		Graphics grp(pDC->m_hDC);

		GraphicsPath path;
		path.StartFigure();
		Point pt0;
		for (int i = 0; i < (int)m_ptEdges.size(); i++)
		{
			CPoint cpt = mapPoint(m_ptEdges[i], rectArea, ptMin, ptMax);
			Point pt(cpt.x, cpt.y);
			if (i == 0)
			{
				pt0 = pt;
				continue;
			}

			path.AddLine(pt0, pt);
			pt0 = pt;
		}

		//Rect r(120, 120, 40, 40);
		//path.AddRectangle(r);
		path.CloseFigure();

		SolidBrush br(Color(100, GetRValue(m_rgb), GetGValue(m_rgb), GetBValue(m_rgb)));
		//SolidBrush br(Color(255, 0, 0, 200));
		//Pen pen(Color(255, 0, 0), 1);
		grp.FillPath(&br, &path);
		//grp.DrawRectangle(&pen, r);
	}
	Curve2d::draw(pDC, rectArea, ptMin, ptMax, nPass);


}

// End of BandCurve2d implementation
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// RectCurve2d implementation

/////////////////////////
// Construction:
RectCurve2d::RectCurve2d()
{
}

RectCurve2d::~RectCurve2d()
{
}

/////////////////////////////
// Operations:
void RectCurve2d::setData(int nID, LPCTSTR szName, std::vector<Rect2d>& rects)
{
	std::vector <Point2d> pts;
	Curve2d::setData(nID, szName, pts);

	m_rects = rects;
	for (int i = 0; i < (int)m_rects.size(); i++)
	{
		const Rect2d& r = m_rects[i];
		for (int j = 0; j < 4; j++)
		{
			Point2d p = r.corner(j);
			if (m_ptMin.y > p.y)
				m_ptMin.y = p.y;
			if (m_ptMax.y < p.y)
				m_ptMax.y = p.y;
			if (m_ptMin.x > p.x)
				m_ptMin.x = p.x;
			if (m_ptMax.x < p.x)
				m_ptMax.x = p.x;
		}
	}
}

void RectCurve2d::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax, int nPass)
{
	using namespace Gdiplus;

	if (!m_bDrawCurve)
		return;

	if (nPass == 0)
	{
		Graphics grp(pDC->m_hDC);

		GraphicsPath path;
		for (int i = 0; i < (int)m_rects.size(); i++)
		{
			path.StartFigure();
			Rect r = mapRect(m_rects[i], rectArea, ptMin, ptMax);
			path.AddRectangle(r);
			path.CloseFigure();
		}

		SolidBrush br(Color(100, GetRValue(m_rgb), GetGValue(m_rgb), GetBValue(m_rgb)));
		//SolidBrush br(Color(255, 0, 0, 200));
		//Pen pen(Color(255, 0, 0), 1);
		grp.FillPath(&br, &path);
		//grp.DrawRectangle(&pen, r);
	}
}

// End of BandCurve2d implementation
//////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// SpecGrid implemntation

////////////////////////
// Construction:
SpecGrid::SpecGrid(int nID)
{
	m_id = nID;
}

SpecGrid::~SpecGrid()
{
}

/////////////////
// Operations:
void SpecGrid::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax)
{
}

CPoint SpecGrid::mapPoint(const Point2d& ptF, const CRect& rectArea, const Point2d& ptMin, const Point2d& ptMax)
{
	int x = (int)(rectArea.left + (rectArea.right - rectArea.left) / (ptMax.x - ptMin.x) * (ptF.x - ptMin.x));
	int y = (int)(rectArea.bottom - (rectArea.bottom - rectArea.top) / (ptMax.y - ptMin.y) * (ptF.y - ptMin.y));
	return CPoint(x, y);
}

// End of SpecGrid implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// SpecGridCircle implemntation

////////////////////////
// Construction:
SpecGridCircle::SpecGridCircle(int nID, double x, double y, double rad, COLORREF clr, int nWidth, int nStyle) : SpecGrid(nID)
{
	m_ptCenter = Point2d(x, y);
	m_dRad = rad;
	m_pen.CreatePen(nStyle, nWidth, clr);
}

SpecGridCircle::~SpecGridCircle()
{
}

/////////////////
// Operations:
void SpecGridCircle::draw(CDC* pDC, const CRect& rectArea, Point2d ptMin, Point2d ptMax)
{
	CPoint pt = mapPoint(m_ptCenter, rectArea, ptMin, ptMax);
	CPoint pt1 = mapPoint(Point2d(m_ptCenter.x - m_dRad, m_ptCenter.y + m_dRad), rectArea, ptMin, ptMax);
	CPoint pt2 = mapPoint(Point2d(m_ptCenter.x + m_dRad, m_ptCenter.y - m_dRad), rectArea, ptMin, ptMax);
	CPen* pPen = (CPen*)pDC->SelectObject(&m_pen);

	CRect rect(pt1, pt2);
	pDC->Arc(rect, pt1, pt2);
	pDC->Arc(rect, pt2, pt1);

	pDC->SelectObject(pPen);

}

// End of SpecGrid implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inscription implementation

/////////////////////////
// Construction:
Inscription::Inscription()
{
	m_id = 0;
	m_nAlign = DT_CENTER | DT_VCENTER;
	m_et = etRect;
	m_pFont = nullptr;
}

Inscription::~Inscription()
{
}


////////////////////////////
// Operations:
void Inscription::draw(CDC& dc, Plot2d* pPlot)
{
	COLORREF clr = dc.GetTextColor();
	dc.SetTextColor(m_clr);
	COLORREF clrBack = dc.GetBkColor();
	int nbk = dc.SetBkMode(TRANSPARENT);

	if (_isnan(m_ptPos.x) || _isnan(m_ptPos.y))
	{
		CRect rectIns;
		CPoint pt0 = pPlot->scale(pPlot->m_frectPlotArea.left(), pPlot->m_frectPlotArea.bottom(), false);
		CPoint pt1 = pPlot->scale(pPlot->m_frectPlotArea.right(), pPlot->m_frectPlotArea.top(), false);
		rectIns = m_nAlign == DT_LEFT ? CRect(pt0.x + 2, pt1.y + 2, pt1.x, pt0.y) : 
			CRect(pt0.x, pt1.y + 5, pt1.x, pt0.y);

		dc.SetBkMode(OPAQUE);
		//dc.DrawText(m_csText, m_csText.GetLength(), rectIns, m_nAlign);
		CFont* pOldFont = m_pFont != nullptr ? (CFont*)dc.SelectObject(m_pFont) : nullptr;
		dc.DrawText(m_text.c_str(), m_text.length(), rectIns, m_nAlign);
		if (pOldFont != nullptr)
			dc.SelectObject(pOldFont);
	}
	else
	{
		CPoint ptPos = pPlot->scale(m_ptPos.x, m_ptPos.y, false);
		CRect rectTmp = CRect(0, 0, 0, 0);
		CFont* pOldFont = m_pFont != nullptr ? (CFont*)dc.SelectObject(m_pFont) : nullptr;
		int nHeight = dc.DrawText(m_text.c_str(), rectTmp, DT_CALCRECT);
		if (pOldFont != nullptr)
			dc.SelectObject(pOldFont);

		CRect rectIns(ptPos.x - rectTmp.Width() / 2, ptPos.y - nHeight / 2, ptPos.x + rectTmp.Width() / 2, ptPos.y + nHeight / 2);

		rectIns.InflateRect(6, 2);
		//dc.DrawEdge(rectIns, EDGE_RAISED, BF_RECT); // | BF_MIDDLE
		if (m_et == etRect)
		{
			dc.FillSolidRect(rectIns, pPlot->m_clrBack);
			dc.Draw3dRect(rectIns, m_clr, m_clr);
		}
		else if (m_et == etCircle)
		{
			CPen pen(PS_SOLID, 1, m_clr);
			CBrush br(pPlot->m_clrBack);
			CPen* pOldPen = (CPen*)dc.SelectObject(&pen);
			CBrush* pOldBrush = (CBrush*)dc.SelectObject(&br);
			int nRad = rectIns.Width() / 2 + rectIns.Height() / 2;
			CRect rEllipse(ptPos.x - nRad, ptPos.y - nRad, ptPos.x + nRad, ptPos.y + nRad);
			dc.Ellipse(rEllipse);

			dc.SelectObject(pOldPen);
			dc.SelectObject(pOldBrush);
		}

		rectIns.DeflateRect(6, 2);

		pOldFont = m_pFont != nullptr ? (CFont*)dc.SelectObject(m_pFont) : nullptr;
		dc.DrawText(m_text.c_str(), rectIns, DT_SINGLELINE | m_nAlign);
		if (pOldFont != nullptr)
			dc.SelectObject(pOldFont);

	}

	dc.SetBkMode(nbk);
	dc.SetTextColor(clr);
	dc.SetBkColor(clrBack);
}

// End of Inscription implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PlotTitle implementation

/////////////////////////////
// Construction:
PlotTitle::PlotTitle()
{
	m_id = 0;
	m_nAlign = DT_CENTER;
	m_rect = CRect(0, 0, 0, 0);
	m_et = etTop;
}

PlotTitle::~PlotTitle()
{
}

void PlotTitle::layout(CDC& dc, const CRect& rectClient)
{
	CRect rc = rectClient;
	CRect r = rc;

	r.left += 50;
	r.right -= 50;

	CFont* pOldFont = (CFont*)dc.SelectObject(&m_font);
	int nHeight = dc.DrawText(m_text.c_str(), m_text.length(), &r, DT_CALCRECT| DT_WORDBREAK);
	dc.SelectObject(pOldFont);

	m_rect.left = rc.left;
	m_rect.top = rc.top;
	m_rect.bottom = rc.top + nHeight + 20;
	m_rect.right = rc.right;
}

void PlotTitle::draw(CDC& dc, Plot2d* pPlot)
{
	if (isEmpty())
		return;

	COLORREF clrOld = dc.SetTextColor(m_clr);
	CFont* pOldFont = (CFont*)dc.SelectObject(&m_font);
	int nHeight = dc.DrawText(m_text.c_str(), m_text.length(), &m_rect, DT_CENTER | DT_WORDBREAK | DT_TOP);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(clrOld);
}

// End of PlotTitle implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PlotTable implementaion

////////////////////////
// Construction:
PlotTable::PlotTable()
{
	m_nPos = 0;
}

PlotTable::~PlotTable()
{
}


///////////////////////////////////
// Operations:
void PlotTable::layout(CDC* pDC, CRect rectClient, Plot2d* pPlot)
{
	int nCols = 0;
	for (int i = 0; i < (int)m_rows.size(); i++)
	{
		Row& row = m_rows[i];
		if (nCols < (int)row.cols.size())
			nCols = row.cols.size();

		for (int j = 0; j < (int)row.cols.size(); j++)
		{
			Col& col = row.cols[j];

			col.rect = CRect(0, 0, 0, 12);
			if (col.ect != ectText)
				continue;

			pDC->DrawText(col.str.c_str(), col.rect, DT_CALCRECT | DT_SINGLELINE);
			col.rect.right += 5;
			col.rect.bottom += 2;
		}
	}

	// Adjsut columns widths and heights
	int nRowHeight = 0;
	for (int i = 0; i < (int)m_rows.size(); i++)
	{
		Row& row = m_rows[i];
		for (int j = 0; j < (int)row.cols.size(); j++)
		{
			Col& col = row.cols[j];
			if (col.rect.Height() > nRowHeight)
				nRowHeight = col.rect.Height();
		}
	}

	int x = 0;
	for (int c = 0; c < nCols; c++)
	{
		int nColWidth = 0;
		for (int i = 0; i < (int)m_rows.size(); i++)
		{
			Row& row = m_rows[i];
			if ((int)row.cols.size() <= c)
				continue;
			Col& col = row.cols[c];
			if (col.rect.Width() > nColWidth)
				nColWidth = col.rect.Width();
			if (col.rect.Height() > nRowHeight)
				nRowHeight = col.rect.Height();
		}

		if (nColWidth == 0)
			nColWidth = nRowHeight;

		for (int i = 0; i < (int)m_rows.size(); i++)
		{
			Row& row = m_rows[i];
			if ((int)row.cols.size() <= c)
				continue;
			Col& col = row.cols[c];
			col.rect.left = x;
			col.rect.right = x + nColWidth;
		}
		x += nColWidth;
	}

	int y = 0;
	for (int i = 0; i < (int)m_rows.size(); i++)
	{
		Row& row = m_rows[i];
		for (int j = 0; j < (int)row.cols.size(); j++)
		{
			Col& col = row.cols[j];
			col.rect.top = y;
			col.rect.bottom = y + nRowHeight;
		}
		y += nRowHeight;
	}

	CSize size(x, y);

	CRect rectPlot = pPlot->getPlotRect();

	CPoint pt;
	switch (m_nPos)
	{
	case 0:
		pt = rectPlot.TopLeft() + CSize(10, 10);
		break;
	case 1:
		pt = CPoint(rectPlot.right - size.cx - 10, rectPlot.top + 10);
		break;
	case 2:
		pt = CPoint(rectPlot.right - size.cx - 10, rectPlot.bottom - size.cy - 10);
		break;
	case 3:
		pt = CPoint(rectPlot.left + size.cx + 10, rectPlot.bottom - size.cy - 10);
		break;
	}

	m_rect = CRect(pt, size);
	for (int i = 0; i < (int)m_rows.size(); i++)
	{
		Row& row = m_rows[i];
		for (int j = 0; j < (int)row.cols.size(); j++)
		{
			Col& col = row.cols[j];
			col.rect += pt;
		}
	}
}

void PlotTable::draw(CDC* pDC, Plot2d* pPlot)
{
	for (int i = 0; i < (int)m_rows.size(); i++)
	{
		Row& row = m_rows[i];
		for (int j = 0; j < (int)row.cols.size(); j++)
		{
			Col& col = row.cols[j];

			if (col.ect == ectText)
			{
				pDC->DrawText(col.str.c_str(), col.rect, DT_SINGLELINE);
			}
			else if (col.ect == ectColor)
			{
				CBrush br(col.rgb);
				CPen pen(PS_SOLID, 1, col.rgb);
				CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&br);
				CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);
				pDC->Ellipse(col.rect);
				pDC->SelectObject(pOldBrush);
				pDC->SelectObject(pOldPen);
			}
		}
	}
}

// End of PlotTable implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HistGroup implementation

HistGroup::HistGroup()
{
	colWidth = 0;
	space = 4;
	maxCols = 0;
}

HistGroup::~HistGroup()
{
}

/////////////////////////////////
// Operations:
void HistGroup::getMinMax(double& vmin, double& vmax) const
{
	vmin = DBL_MAX;
	vmax = -DBL_MAX;
	for (int i = 0; i < (int)groups.size(); i++)
	{
		const Group& g = groups.at(i);
		for (int j = 0; j < (int)g.vals.size(); j++)
		{
			if (g.vals[j].second > vmax)
				vmax = g.vals[j].second;
			if (g.vals[j].second < vmin)
				vmin = g.vals[j].second;
		}
	}
}

void HistGroup::addGroup(const char* name, const double* pv, const char** valNames, const COLORREF* pclrs, int valCount)
{
	Group g;
	g.name = name;
	for (int i = 0; i < valCount; i++)
	{
		int valKey = -1;
		int maxKey = 0;
		std::for_each(types.begin(), types.end(), [&](const std::pair<int, Type>& a) {
			if (a.second.name == valNames[i])
				valKey = a.first;
			if (a.first > maxKey)
				maxKey = a.first;
		});

		if (valKey < 0)
		{
			valKey = maxKey + 1;
			Type t(valKey, valNames[i], pclrs[i]);
			types[valKey] = t;
		}

		g.vals.push_back(std::pair<int, double>(valKey, pv[i]));
	}

	groups.push_back(g);
}

void HistGroup::addGroup(const char* name, const std::vector<double>& vs, const std::vector<const char *>& valNames, const std::vector<COLORREF>& clrs)
{
	ASSERT(vs.size() == valNames.size() && vs.size() == clrs.size());
	Group g;
	g.name = name;
	int valCount = vs.size();
	for (int i = 0; i < valCount; i++)
	{
		int valKey = -1;
		int maxKey = 0;
		std::for_each(types.begin(), types.end(), [&](const std::pair<int, Type>& a) {
			if (a.second.name == valNames[i])
				valKey = a.first;
			if (a.first > maxKey)
				maxKey = a.first;
		});

		if (valKey < 0)
		{
			valKey = maxKey + 1;
			Type t(valKey, valNames[i], clrs[i]);
			types[valKey] = t;
		}

		g.vals.push_back(std::pair<int, double>(valKey, vs[i]));
	}

	groups.push_back(g);
}


void HistGroup::layout(CDC* pDC, CRect rectClient, Plot2d* pPlot)
{
	if (groups.size() == 0)
		return;
	int cx = rectClient.Width() / groups.size();
	maxCols = 0;
	for (int i = 0; i < (int)groups.size(); i++)
	{
		groups[i].rect = CRect(cx * i, rectClient.top, cx * (i + 1), rectClient.bottom);
		if (groups[i].vals.size() > (size_t)maxCols)
			maxCols = groups[i].vals.size();
	}

	colWidth = (cx - (maxCols + 1) * space) / maxCols;
}

void HistGroup::draw(CDC* pDC, Plot2d* pPlot, std::vector<HistGroup>& hgs, int nPass)
{
	CPoint pt0 = pPlot->scale(0, 0, false);
	//CPoint pt1 = pPlot->scale(1, 0, false);
	CRect rectPlot;
	rectPlot.TopLeft() = pPlot->scale(pPlot->m_frectPlotArea.pt.x, pPlot->m_frectPlotArea.top(), false);
	rectPlot.BottomRight() = pPlot->scale(pPlot->m_frectPlotArea.right(), pPlot->m_frectPlotArea.pt.y, false);
	int cx = rectPlot.Width() / groups.size();
	colWidth = (cx - (maxCols - 1) * space - 4 * space) / maxCols;

	CPen * pOldPen = (CPen *)pDC->SelectObject(&pPlot->m_penAxis);
	CFont* pOldFont = (CFont*)pDC->SelectObject(pPlot->m_pLegendFont);

	auto getVal = [](int ng, int nv, std::vector<HistGroup>& hgs, int nPass) {
		std::vector<std::pair<int, std::pair<int, double>>> vals;
		for (int i = 0; i < (int)hgs.size(); i++)
			vals.push_back(std::pair<int, std::pair<int, double>>{ i, hgs[i].groups[ng].vals[nv] });

		std::sort(vals.begin(), vals.end(), [](const std::pair<int, std::pair<int, double>>& a, const std::pair<int, std::pair<int, double>>& b) {
			if (_isnan(a.second.second))
				return false;
			return fabs(a.second.second) > fabs(b.second.second);
		});
		return vals[nPass];
	};

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	pPlot->m_pLegendFont->GetLogFont(&lf);
	int nFontHeight = lf.lfHeight;
	for (int i = 0; i < (int)groups.size(); i++)
	{
		Group& g = hgs[0].groups[i];

		int x0 = i * (3*space + (colWidth + space) * maxCols) + rectPlot.left;
		int x1 = x0 + 3*space + (colWidth + space) * maxCols;
		
		COLORREF clrBack = pDC->GetBkColor();
		for (int j = 0; j < (int)g.vals.size(); j++)
		{
			int x = x0 + 2*space + (colWidth + space) * j;
			std::pair<int, std::pair<int, double>> p = getVal(i, j, hgs, nPass); // g.vals[j].second;
			COLORREF clr = hgs[p.first].types[p.second.first].clr;
			double val = p.second.second;
			if (!isnan(val) && pPlot->m_frectPlotArea.bottom() <= val && val <= pPlot->m_frectPlotArea.top())
			{
				CPoint ptv = pPlot->scale(0, val, false);
				CRect r(x, ptv.y, x + colWidth, pt0.y);
				pDC->FillSolidRect(r, clr);
			}
			else if (!isnan(val))
			{
				CPoint ptv = val < pPlot->m_frectPlotArea.bottom() ?
					pPlot->scale(0, pPlot->m_frectPlotArea.bottom(), false) :
					pPlot->scale(0, pPlot->m_frectPlotArea.top(), false);
				CRect r(x, ptv.y, x + colWidth, pt0.y);
				pDC->FillSolidRect(r, clr);

				double val2 = val < pPlot->m_frectPlotArea.bottom() ?
					pPlot->m_frectPlotArea.bottom() - pPlot->m_frectPlotArea.height() / 30 :
					pPlot->m_frectPlotArea.top() + pPlot->m_frectPlotArea.height() / 30;
				CPoint ptv2 = pPlot->scale(0, val2, false);

				CRect r2(x, ptv.y, x + colWidth, ptv2.y);
				pDC->FillSolidRect(r2, RGB(230, 230, 230));

			}
			else
			{
				CPoint ptv = pPlot->scale(0, pPlot->m_frectPlotArea.top(), false);
				CRect r(x, ptv.y, x + colWidth, pt0.y);
				pDC->FillSolidRect(r, RGB(240, 240, 240));
			}
		}
		pDC->SetBkColor(clrBack);

		pDC->MoveTo(x0, rectPlot.top);
		pDC->LineTo(x0, rectPlot.bottom);

		int nOldMode = pDC->SetBkMode(TRANSPARENT);
		CRect rt(x0, pPlot->m_xAxis.m_r.top, x1, pPlot->m_xAxis.m_r.top + nFontHeight + space);
		pDC->DrawText(g.name.c_str(), rt, DT_CENTER);
		pDC->SetBkMode(nOldMode);
	}

	pDC->MoveTo(rectPlot.right, rectPlot.top);
	pDC->LineTo(rectPlot.right, rectPlot.bottom);

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
}
// End of HistGroup implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plot2d

COLORREF Plot2d::c_stdColors[c_stdColorsNum] =
{
	RGB(0xFF, 0x00, 0x00),
	RGB(0x00, 0xB0, 0x00),
	RGB(0x00, 0x00, 0xFF),
	RGB(0xFF, 0x00, 0xFF),
	RGB(0x00, 0xB0, 0xC0),
	RGB(0xFF, 0xB0, 0x00),
	RGB(0xB0, 0x40, 0x00),
	RGB(0xB0, 0xB0, 0x00),
	RGB(0xA0, 0xA0, 0xA0),
	RGB(166, 166, 230),
	RGB(226, 159, 226),
	RGB(190, 226, 190),
	RGB(128, 0, 128),
	RGB(255, 128, 64),
	RGB(0, 0, 0)
};

Plot2d::Plot2d()
{
	// Register the window class if it has not already been registered.
	WNDCLASS wndclass;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, IVSPLOT2D_CLASSNAME, &wndclass)))
	{
		// Otherwise we need to register a new class
		wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ::DefWindowProc;
		wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInst;
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor(hInst, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = IVSPLOT2D_CLASSNAME;

		if (!AfxRegisterClass(&wndclass))
			AfxThrowResourceException();
	}

	m_eMode = emDecart;
	m_bSubclassFromCreate = FALSE;
	m_bDoubleBuffer = TRUE;
	m_clrBack = RGB(255, 255, 255);
	m_brushBack.CreateSolidBrush(RGB(255, 255, 255));
	m_penGrid.CreatePen(PS_DOT, 1, RGB(0, 0, 0));
	m_penAxis.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	m_bEnableSecondaryAxis = FALSE;

	for (int i = 0; i < CURVES_MAX; i++)
	{
		short hue = (short)((50 - 30) / CURVES_MAX * i + 30);
		if (hue > 80)
			hue = 80;
		else if (hue < 0)
			hue = 0;

		COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((unsigned short)hue, 120, 240);

		m_clrCurves[i] = clr;
		//m_penCurves[i] = new CPen (PS_SOLID, 1, clr);
	}


	m_frectPlotArea = Rect2d(0, 0, 1, 1);

	//m_bxAuto = TRUE;
	//m_byAuto = TRUE;
	//m_nxDiv = 6;
	//m_nyDiv = 4;
	//m_eaxFormat [0] = m_eaxFormat[1] = eaxDefault;

	m_nPopupMenuResourceID = -1;
	m_nPopupMenuIndex = -1;

	m_pPopupOwner = this;

	m_bFirstPainting = TRUE;

	m_dCurveMaxPointDistance = NAN;

	m_bTipPainted = TRUE;
	m_bCursorInPlot = FALSE;

	m_bLegend = TRUE;
	m_bLegend2 = TRUE;
	m_bCheckLegend = TRUE;
	m_bCheckLegend2 = TRUE;

	m_eLegendAlign = elaBottomRight;
	m_eLegendAlign2 = elaTopRight;
	m_eZoomMode = ezmOnlyX;

	m_pLegendFont = nullptr;
}

Plot2d::~Plot2d()
{
	reset();
}

void Plot2d::reset()
{
	for (int i = 0; i < (int)m_curves.size(); i++)
		delete m_curves[i];

	m_curves.resize(0);

	for (int i = 0; i < (int)m_curves2.size(); i++)
		delete m_curves2[i];

	m_curves2.resize(0);

	for (int i = 0; i < (int)m_grids.size(); i++)
		delete m_grids[i];

	m_grids.resize(0);
	m_marks.resize(0);
	m_events.resize(0);
	m_areas.resize(0);

	clearInscriptions();
}

void Plot2d::setInitArea(BOOL bX, BOOL bY)
{
	BOOL bxAuto = m_xAxis.m_auto;
	double xMin = m_xAxis.m_min;
	double xMax = m_xAxis.m_max;
	int xDivs = m_xAxis.m_divs;
	BOOL byAuto = m_yAxis.m_auto;
	double yMin = m_yAxis.m_min;
	double yMax = m_yAxis.m_max;
	int yDivs = m_yAxis.m_divs;
	BOOL byAuto2 = m_yAxis2.m_auto;
	double yMin2 = m_yAxis2.m_min;
	double yMax2 = m_yAxis2.m_max;
	int yDivs2 = m_yAxis2.m_divs;

	autoScale(true, true, true);

	if (bX) {
		m_frectInitArea = m_frectPlotArea;
		m_frectInitArea2 = m_frectPlotArea2;
	}
	if (bY) {
		m_frectInitArea = m_frectPlotArea;
		m_frectInitArea2 = m_frectPlotArea2;
	}

	if (!bxAuto)
	{
		m_xAxis.m_auto = bxAuto;
		m_xAxis.m_min = xMin;
		m_xAxis.m_max = xMax;
		m_xAxis.m_divs = xDivs;
		m_frectPlotArea.pt.x = xMin;
		m_frectPlotArea.s.cx = xMax - xMin;
		m_frectPlotArea2.pt.x = xMin;
		m_frectPlotArea2.s.cx = xMax - xMin;
	}
	if (!byAuto)
	{
		m_yAxis.m_auto = byAuto;
		m_yAxis.m_min = yMin;
		m_yAxis.m_max = yMax;
		m_yAxis.m_divs = yDivs;
		m_frectPlotArea.pt.y = yMin;
		m_frectPlotArea.s.cy = yMax - yMin;
	}
	if (!byAuto2)
	{
		m_yAxis2.m_auto = byAuto2;
		m_yAxis2.m_min = yMin2;
		m_yAxis2.m_max = yMax2;
		m_yAxis2.m_divs = yDivs2;
		m_frectPlotArea2.pt.y = yMin2;
		m_frectPlotArea2.s.cy = yMax2 - yMin2;
	}

	autoScale(bxAuto, byAuto, byAuto2);
	if (IsWindow(m_hWnd))
		Invalidate(FALSE);
}

void Plot2d::resetInitArea()
{
}

void Plot2d::resetZoom()
{
	if (m_frectInitArea.area() > 0)
	{
		m_frectPlotArea = m_frectInitArea;
	}

	if (m_frectInitArea2.area() > 0)
	{
		m_frectPlotArea2 = m_frectInitArea2;
	}

	autoScale(TRUE, TRUE, TRUE);

	Invalidate(FALSE);
}

/////////////////////////
// Operations:
bool Plot2d::initControl(CWnd* pParent)
{
	if (m_xAxis.m_font.m_hObject != NULL)
		return FALSE;


	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	CFont* pFont = pParent->GetFont();
	if (pFont != NULL)
	{
		pFont->GetLogFont(&lf);
	}
	else
	{
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		memmove(&lf, &ncm.lfMessageFont, sizeof(LOGFONT));
	}
	{
		HDC hdc = ::GetDC(NULL);
		int xLogPx = GetDeviceCaps(hdc, LOGPIXELSX);
		lf.lfHeight = (LONG)ceil(18.0 * xLogPx / 72.0);

		::ReleaseDC(NULL, hdc);
	}
	m_xAxis.m_font.CreateFontIndirect(&lf);

	strncpy_s(lf.lfFaceName, "Arial", LF_FACESIZE);
	lf.lfOrientation = lf.lfEscapement = 900;
	lf.lfCharSet = RUSSIAN_CHARSET;
	m_yAxis.m_font.CreateFontIndirect(&lf);

	strncpy_s(lf.lfFaceName, "Arial", LF_FACESIZE);
	lf.lfOrientation = lf.lfEscapement = 900;
	lf.lfCharSet = RUSSIAN_CHARSET;
	m_yAxis2.m_font.CreateFontIndirect(&lf);

	return true;
}

CPoint Plot2d::scale(double x, double y, bool bSecondary) const
{
	CPoint pt;
	if (bSecondary)
	{
		pt.x = (long)((x - m_frectPlotArea2.pt.x) * m_fsizeScale2.cx + m_ptPlotOrg.x);
		pt.y = (long)((y - m_frectPlotArea2.pt.y) * m_fsizeScale2.cy + m_ptPlotOrg.y);
	}
	else
	{
		pt.x = (long)((x - m_frectPlotArea.pt.x) * m_fsizeScale.cx + m_ptPlotOrg.x);
		pt.y = (long)((y - m_frectPlotArea.pt.y) * m_fsizeScale.cy + m_ptPlotOrg.y);
	}
	return pt;
}

CSize Plot2d::scaleSize(double dx, double dy, bool bSecondary) const
{
	CPoint pt0 = scale(0, 0, bSecondary);
	CPoint pt1 = scale(dx, dy, bSecondary);
	return CSize(pt1.x - pt0.x, pt0.y - pt1.y);
}

Point2d Plot2d::reverseScale(int x, int y, bool bSecondary) const
{
	Point2d pt;

	if (bSecondary)
	{
		pt.x = (x - m_ptPlotOrg.x) / m_fsizeScale2.cx + m_frectPlotArea2.pt.x;
		pt.y = (y - m_ptPlotOrg.y) / m_fsizeScale2.cy + m_frectPlotArea2.pt.y;
	}
	else
	{
		pt.x = (x - m_ptPlotOrg.x) / m_fsizeScale.cx + m_frectPlotArea.pt.x;
		pt.y = (y - m_ptPlotOrg.y) / m_fsizeScale.cy + m_frectPlotArea.pt.y;
	}
	return pt;
}

void Plot2d::setMarker(int nID, double dCoord, bool bX, COLORREF clr)
{
	BOOL bFound = FALSE;
	for (int i = 0; i < (int)m_marks.size(); i++)
	{
		if (m_marks[i].nID != nID)
			continue;

		double dCoordOld = m_marks[i].dCoord;
		m_marks[i].dCoord = dCoord;
		m_marks[i].bX = bX;
		m_marks[i].clr = clr;
		bFound = TRUE;

		if (bX)
		{
			Point2d ptNewf(dCoord, m_frectPlotArea.pt.y);
			Point2d ptOldf(dCoordOld, m_frectPlotArea.pt.y);

			CPoint ptNew = scale(ptNewf.x, ptNewf.y, false);
			CPoint ptOld = scale(ptOldf.x, ptOldf.y, false);

			if (ptNew.x == ptOld.x)
				return;
		}
		else
		{
			Point2d ptNewf(m_frectPlotArea.pt.x, dCoord);
			Point2d ptOldf(m_frectPlotArea.pt.x, dCoordOld);

			CPoint ptNew = scale(ptNewf.x, ptNewf.y, FALSE);
			CPoint ptOld = scale(ptOldf.x, ptOldf.y, FALSE);

			if (ptNew.y == ptOld.y)
				return;
		}
		m_marks[i].dCoordOld = dCoordOld;

		break;
	}

	if (!bFound)
	{
		CMarker m;
		m.nID = nID;
		m.bX = bX;
		m.dCoord = dCoord;
		m.dCoordOld = NAN;
		m.clr = clr;
		m_marks.push_back(m);
	}

	//CClientDC dc (this);
	Invalidate(FALSE);
	//DrawMarkers (&dc, FALSE);
	//CRect r;
	//GetClientRect (r);
	//ValidateRect (r);
}

void Plot2d::clearMarkers()
{
	m_marks.resize(0);
	Invalidate(FALSE);
}

void Plot2d::removeCurve(int nID)
{
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		Curve2d* pCurve = m_curves[i];
		if (pCurve->m_id == nID)
		{
			m_curves.erase(m_curves.begin() + i); // RemoveAt(i);
			delete pCurve;
			autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);
			Invalidate();
			return;
		}
	}

	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		Curve2d* pCurve = m_curves2[i];
		if (pCurve->m_id == nID)
		{
			m_curves2.erase(m_curves2.begin() + i);
			delete pCurve;
			autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);
			Invalidate();
			return;
		}
	}
}

void Plot2d::removeAllCurves()
{
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		delete m_curves[i];
	}
	m_curves.resize(0);
	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		delete m_curves2[i];
	}
	m_curves2.resize(0);
}

void Plot2d::addCurve(int nID, LPCTSTR szName, std::vector<Point2d>& pts, int nWidth, Curve2d::Style eStyle, COLORREF rgb, bool bSecondary, bool bInvalidate)
{
	// Get the number of auto-color curves
	int nAutoColorNum = 0;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (m_curves[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		if (m_curves2[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	if (pts.size() <= 0)
	{
		removeCurve(nID);
		return;
	}

	BOOL bAutoColor = (rgb == 0xFFFFFFFF);
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;

	if (bSecondary && pts.size())
		m_bEnableSecondaryAxis = TRUE;


	int nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];
		if (pCurve->m_id == nID)
		{
			pCurve->setData(nID, szName, pts);
			pCurve->m_eStyle = eStyle;
			pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
			pCurve->deletePens();

			if (bAutoColor && !pCurve->m_bAutoColor)
				nAutoColorNum++;

			pCurve->m_bAutoColor = bAutoColor;
			pCurve->m_rgb = rgb;
			pCurve->m_nWidth = nWidth;
			pCurve->m_name = szName;

			if (bAutoColor)
			{
				short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
				if (hue < 0)
					hue = 0;
				COLORREF clr = ColorHLS((byte)hue, 120, 200); // hls2rgb((unsigned short)hue, 120, 200); // 120
				pCurve->createPens(eStyle == Curve2d::eDashed ? PS_DASH : PS_SOLID, nWidth, clr);
			}
			else
			{
				pCurve->createPens(eStyle == Curve2d::eDashed ? PS_DASH : PS_SOLID, nWidth, rgb);
			}

			autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);
			Invalidate();
			return;
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}

	if (curves.size() >= CURVES_MAX)
	{
		TRACE("Too many curves\n");
		return;
	}

	Curve2d* pCurve = new Curve2d();
	pCurve->m_id = nID;
	pCurve->m_bAutoColor = bAutoColor;
	pCurve->m_rgb = rgb;
	pCurve->m_name = szName;
	pCurve->m_nWidth = nWidth;
	pCurve->setData(nID, szName, pts);
	pCurve->m_eStyle = eStyle;
	pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
	curves.push_back(pCurve);

	autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);

	nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];

		if (pCurve->m_bAutoColor)
		{
			pCurve->deletePens();

			short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
			if (hue < 0)
				hue = 0;

			COLORREF clr = ColorHLS((byte)hue, 120, 200); // hls2rgb((WORD)hue, 120, 200); // 120, 240

			pCurve->createPens(pCurve->m_eStyle == Curve2d::eDashed ? PS_DASH : PS_SOLID, pCurve->m_nWidth, clr); // m_pen.CreatePen (
		}
		else
		{
			//CPen * pPen = &(pCurve->m_pen); //m_penCurves[i];
			//pPen->DeleteObject ();
			pCurve->deletePens();
			pCurve->createPens(pCurve->m_eStyle == Curve2d::eDashed ? PS_DASH : PS_SOLID, pCurve->m_nWidth, pCurve->m_rgb);
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}
	if (bInvalidate && IsWindow(m_hWnd))
		Invalidate();
}

void Plot2d::addPolarCurve(int nID, LPCTSTR szName, std::vector<Point2d>& pts, int nWidth, Curve2d::Style eStyle, COLORREF rgb)
{
	m_eMode = emPolar;
	addCurve(nID, szName, pts, nWidth, eStyle, rgb);
}

void Plot2d::addBar(int nID, LPCTSTR szName, std::vector<Point2d>& pts, std::vector<double>& dWidths, std::vector <double>& dbs, int nWidth, Curve2d::Style eStyle, COLORREF rgb, bool bSecondary)
{
	// Get the number of auto-color curves
	int nAutoColorNum = 0;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (m_curves[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		if (m_curves2[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	if (pts.size() <= 0)
	{
		removeCurve(nID);
		return;
	}

	BOOL bAutoColor = (rgb == 0xFFFFFFFF);
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;

	if (bSecondary && pts.size())
		m_bEnableSecondaryAxis = TRUE;


	int nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		BarCurve2d* pCurve = (BarCurve2d*)curves[i];
		if (pCurve->m_id == nID)
		{
			pCurve->setData(nID, szName, pts, dWidths, dbs);
			pCurve->m_eStyle = eStyle;
			pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
			pCurve->deletePens();

			if (bAutoColor && !pCurve->m_bAutoColor)
				nAutoColorNum++;

			pCurve->m_bAutoColor = bAutoColor;
			pCurve->m_rgb = rgb;
			pCurve->m_nWidth = nWidth;

			if (bAutoColor)
			{
				short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
				if (hue < 0)
					hue = 0;
				COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((WORD)hue, 120, 240);
				pCurve->createPens(PS_SOLID, nWidth, clr);
			}
			else
			{
				pCurve->createPens(PS_SOLID, nWidth, rgb);
			}

			autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);
			Invalidate();
			return;
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}

	if (curves.size() >= CURVES_MAX)
	{
		TRACE("Too many curves\n");
		return;
	}

	BarCurve2d* pCurve = new BarCurve2d();
	pCurve->m_id = nID;
	pCurve->m_bAutoColor = bAutoColor;
	pCurve->m_rgb = rgb;
	pCurve->m_name = szName;
	pCurve->m_nWidth = nWidth;
	pCurve->setData(nID, szName, pts, dWidths, dbs);
	pCurve->m_eStyle = eStyle;
	pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
	curves.push_back(pCurve);

	autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);

	nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];

		if (pCurve->m_bAutoColor)
		{
			pCurve->deletePens();

			short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
			if (hue < 0)
				hue = 0;

			COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((WORD)hue, 120, 240);

			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, clr); // m_pen.CreatePen (
		}
		else
		{
			//CPen * pPen = &(pCurve->m_pen); //m_penCurves[i];
			//pPen->DeleteObject ();
			pCurve->deletePens();
			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, pCurve->m_rgb);
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}
	Invalidate();
}

void Plot2d::addBand(int nID, LPCTSTR szName, std::vector<Point2d>& pts, std::vector<Point2d>& ptMinMax, int nWidth, COLORREF rgb, bool bSecondary)
{
	// Get the number of auto-color curves
	int nAutoColorNum = 0;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (m_curves[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		if (m_curves2[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	removeCurve(nID);
	if (pts.size() <= 0)
	{
		return;
	}

	BOOL bAutoColor = (rgb == 0xFFFFFFFF);
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;

	if (bSecondary && pts.size())
		m_bEnableSecondaryAxis = TRUE;


	int nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		BarCurve2d* pCurve = (BarCurve2d*)curves[i];
		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}

	if (curves.size() >= CURVES_MAX)
	{
		TRACE("Too many curves\n");
		return;
	}

	BandCurve2d* pCurve = new BandCurve2d();
	pCurve->m_id = nID;
	pCurve->m_bAutoColor = bAutoColor;
	pCurve->m_rgb = rgb;
	pCurve->m_name = szName;
	pCurve->m_nWidth = nWidth;
	pCurve->setData(nID, szName, pts, ptMinMax);
	pCurve->m_eStyle = Curve2d::eSolid;
	pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
	curves.push_back(pCurve);

	autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);

	nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];

		if (pCurve->m_bAutoColor)
		{
			pCurve->deletePens();

			short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
			if (hue < 0)
				hue = 0;

			COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((WORD)hue, 120, 240);

			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, clr); // m_pen.CreatePen (
		}
		else
		{
			//CPen * pPen = &(pCurve->m_pen); //m_penCurves[i];
			//pPen->DeleteObject ();
			pCurve->deletePens();
			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, pCurve->m_rgb);
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}
	Invalidate();
}

void Plot2d::addRect(int nID, LPCTSTR szName, std::vector<Rect2d>& rects, int nWidth, COLORREF rgb, bool bSecondary)
{
	// Get the number of auto-color curves
	int nAutoColorNum = 0;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (m_curves[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		if (m_curves2[i]->m_bAutoColor)
			nAutoColorNum++;
	}

	removeCurve(nID);
	if (rects.size() <= 0)
	{
		return;
	}

	BOOL bAutoColor = (rgb == 0xFFFFFFFF);
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;

	if (bSecondary && rects.size())
		m_bEnableSecondaryAxis = TRUE;


	int nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		BarCurve2d* pCurve = (BarCurve2d*)curves[i];
		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}

	if (curves.size() >= CURVES_MAX)
	{
		TRACE("Too many curves\n");
		return;
	}

	RectCurve2d* pCurve = new RectCurve2d();
	pCurve->m_id = nID;
	pCurve->m_bAutoColor = bAutoColor;
	pCurve->m_rgb = rgb;
	pCurve->m_name = szName;
	pCurve->m_nWidth = nWidth;
	pCurve->setData(nID, szName, rects);
	pCurve->m_eStyle = Curve2d::eSolid;
	pCurve->m_dMaxPointDistance = m_dCurveMaxPointDistance;
	curves.push_back(pCurve);

	autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);

	nAutoColor = 0;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];

		if (pCurve->m_bAutoColor)
		{
			pCurve->deletePens();

			short hue = (short)((220.0 - 0.0) / nAutoColorNum * nAutoColor + 0);
			if (hue < 0)
				hue = 0;

			COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((WORD)hue, 120, 240);

			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, clr); // m_pen.CreatePen (
		}
		else
		{
			//CPen * pPen = &(pCurve->m_pen); //m_penCurves[i];
			//pPen->DeleteObject ();
			pCurve->deletePens();
			pCurve->createPens(PS_SOLID, pCurve->m_nWidth, pCurve->m_rgb);
		}

		if (pCurve->m_bAutoColor)
			nAutoColor++;
	}
	Invalidate();
}

void Plot2d::setCurveMaxPointDistance(double dMaxDistance, int nID, bool bSecondary)
{
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];
		if (nID == 0)
		{
			pCurve->m_dMaxPointDistance = dMaxDistance;
			m_dCurveMaxPointDistance = dMaxDistance;

		}
		else
		{
			if (pCurve->m_id == nID)
			{
				pCurve->m_dMaxPointDistance = dMaxDistance;
				return;
			}
		}
	}
}

void Plot2d::setCurveStyle(int nID, Curve2d::Style eStyle, int nWidth, bool bSecondary)
{
	std::vector <Curve2d*>& curves = bSecondary ? m_curves2 : m_curves;

	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];
		if (pCurve->m_id == nID)
		{
			pCurve->m_eStyle = eStyle;
			pCurve->m_nWidth = nWidth;
			pCurve->deletePens();

			short hue = (short)((220.0 - 0.0) / curves.size() * i + 0);
			if (hue < 0)
				hue = 0;

			COLORREF clr = ColorHLS((byte)hue, 120, 240); // hls2rgb((WORD)hue, 120, 240);

			pCurve->createPens(eStyle == Curve2d::eDashed ? PS_DASH : PS_SOLID, nWidth, clr);
			Invalidate();
			return;
		}
	}
}

void Plot2d::addCircle(int nID, double x, double y, double rad, COLORREF clr, int nWidth, int nStyle)
{
	delCircle(nID);
	SpecGridCircle* p = new SpecGridCircle(nID, x, y, rad, clr, nWidth, nStyle);
	m_grids.push_back(p);
}

void Plot2d::delCircle(int nID)
{
	for (int i = 0; i < (int)m_grids.size(); i++)
	{
		if (m_grids[i]->m_id == nID)
		{
			delete m_grids[i];
			m_grids.erase(m_grids.begin() + i);
			return;
		}
	}
}

void Plot2d::removeAllCircles()
{
	for (int i = 0; i < (int)m_grids.size(); i++)
	{
		delete m_grids[i];
	}

	m_grids.clear();
}

void Plot2d::addEllipse(int nID, double x, double y, double dx, double dy, COLORREF clr)
{
	CEllipse el;
	el.nID = nID;
	el.pt = Point2d(x, y);
	el.size = Size2d(dx, dy);
	el.clr = clr;
	for (int i = 0; i < (int)m_ellipses.size(); i++)
	{
		if (m_ellipses[i].nID == nID)
		{
			m_ellipses[i] = el;
			Invalidate();
			return;
		}
	}

	m_ellipses.push_back(el);
	Invalidate();
}

void Plot2d::removeAllEllipses()
{
	m_ellipses.clear();
	Invalidate();
}

void Plot2d::addHistGroup(const HistGroup& hg)
{
	m_hgs.push_back(hg);
}

void Plot2d::removeHistGroup()
{
	m_hgs.clear();
}

void Plot2d::addEvent(int nID, double x, COLORREF clr, LPCTSTR szTitle)
{
	CEvent ev;
	ev.nID = nID;
	ev.clr = clr;
	ev.szTitle = szTitle;
	ev.x = x;
	for (int i = 0; i < (int)m_events.size(); i++)
	{
		if (m_events[i].nID == nID)
		{
			m_events[i] = ev;
			return;
		}
	}

	m_events.push_back(ev);
}

void Plot2d::clearEvents()
{
	m_events.resize(0);
}

void Plot2d::addArea(int nID, double x0, double x1, COLORREF clr, LPCTSTR szTitle)
{
	CArea a;
	a.nID = nID;
	a.clr = clr;
	a.szTitle = szTitle;
	a.x0 = x0;
	a.x1 = x1;
	for (int i = 0; i < (int)m_areas.size(); i++)
	{
		if (m_areas[i].nID == nID)
		{
			m_areas[i] = a;
			return;
		}
	}

	m_areas.push_back(a);
}

void Plot2d::clearAreas()
{
	m_areas.resize(0);
}

void Plot2d::addInscription(int nID, LPCTSTR szText, Point2d& ptPos, int nAlign, COLORREF clr, Inscription::Type et, CFont* pFont)
{
	for (int i = 0; i < (int)m_inscs.size(); i++)
	{
		Inscription* pIns = m_inscs[i];
		if (pIns->m_id == nID)
		{
			pIns->m_text = szText;
			pIns->m_ptPos = ptPos;
			pIns->m_nAlign = nAlign;
			pIns->m_clr = clr;
			pIns->m_et = et;
			pIns->m_pFont = pFont;
			return;
		}
	}

	Inscription* pIns = new Inscription();
	pIns->m_id = nID;
	pIns->m_text = szText;
	pIns->m_ptPos = ptPos;
	pIns->m_nAlign = nAlign;
	pIns->m_clr = clr;
	pIns->m_et = et;
	pIns->m_pFont = pFont;
	m_inscs.push_back(pIns);
}

void Plot2d::deleteInscription(int nID)
{
	if (nID != 0)
	{
		for (int i = 0; i < (int)m_inscs.size(); i++)
		{
			Inscription* pIns = m_inscs[i];
			if (pIns->m_id == nID)
			{
				delete pIns;
				m_inscs.erase(m_inscs.begin() + i);
				return;
			}
		}
	}
	else
	{
		for (int i = 0; i < (int)m_inscs.size(); i++)
		{
			Inscription* pIns = m_inscs[i];
			delete pIns;
		}
		m_inscs.resize(0);
	}
}

void Plot2d::clearInscriptions()
{
	for (int i = 0; i < (int)m_inscs.size(); i++)
	{
		delete m_inscs[i];
	}
	m_inscs.resize(0);
}

void Plot2d::addPlotTitle(const char* szText, COLORREF clr)
{
	m_plotTitle.m_text = szText;
	m_plotTitle.m_clr = clr;
}

void Plot2d::setTitleFont(const LOGFONT& lf)
{
	if (m_plotTitle.m_font.m_hObject != NULL)
		m_plotTitle.m_font.DeleteObject();

	m_plotTitle.m_font.CreateFontIndirect(&lf);
}

void Plot2d::addCurveMarks(int nID, std::vector<COLORREF>& clrs, int nMarkSize)
{
	Curve2d* pc = getCurve(nID);
	if (pc == nullptr)
		return;

	pc->setMarks(clrs, nMarkSize);
}

void Plot2d::addCurveMarks(int nID, std::vector<COLORREF>& clrs, std::vector<COLORREF>& clrs2, int nMarkSize)
{
	Curve2d* pc = getCurve(nID);
	if (pc == nullptr)
		return;

	pc->setMarks(clrs, clrs2, nMarkSize);
}

Curve2d* Plot2d::getCurve(int nID) const
{
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (m_curves[i]->m_id == nID)
			return m_curves[i];
	}

	return nullptr;
}

void Plot2d::drawBackground(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	pDC->FillRect(&rectClient, &m_brushBack);
}

void Plot2d::drawGrid(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	pDC->SelectObject(&m_penAxis);

	CPoint pt[4];
	pt[0] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.pt.y, false);
	pt[1] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	pt[2] = scale(m_frectPlotArea.right(), m_frectPlotArea.top(), false);
	pt[3] = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	pDC->MoveTo(pt[0]);
	for (int i = 1; i < 4; i++)
	{
		pDC->LineTo(pt[i]);
	}
	pDC->LineTo(pt[0]);

	CRect rectPlot;
	rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	for (int i = 0; i < (int)m_grids.size(); i++)
		m_grids[i]->draw(pDC, rectPlot, m_frectPlotArea.pt, m_frectPlotArea.topRight());
}

void Plot2d::drawCurves(CDC* pDC, int nPass)
{
	CRect rectPlot;
	rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	if (m_eMode == emPolar)
	{
		for (int i = 0; i < (int)m_curves.size(); i++)
		{
			Curve2d* pCurve = m_curves[i]; 
			pCurve->drawPolar(*pDC, *this);
		}
		return;
	}

	for (int na = 0; na < 2; na++)
	{
		for (int i = 0; i < (int)m_curves.size(); i++)
		{
			Curve2d* pCurve = m_curves[i];
			if (na == 0 && pCurve->getArchType() != Curve2d::eatBandCurve)
				continue;
			else if (na == 1 && pCurve->getArchType() == Curve2d::eatBandCurve)
				continue;

			pCurve->draw(pDC, rectPlot, m_frectPlotArea.pt, m_frectPlotArea.topRight(), nPass);
		}

		if (m_bEnableSecondaryAxis)
		{
			for (int i = 0; i < (int)m_curves2.size(); i++)
			{
				Curve2d* pCurve = m_curves2[i];
				if (na == 0 && pCurve->getArchType() != Curve2d::eatBandCurve)
					continue;
				else if (na == 1 && pCurve->getArchType() == Curve2d::eatBandCurve)
					continue;

				pCurve->draw(pDC, rectPlot, m_frectPlotArea2.pt, m_frectPlotArea2.topRight(), nPass);
			}
		}
	}

	for (int i = 0; i < (int)m_ellipses.size(); i++)
	{
		CEllipse& el = m_ellipses[i];
		CBrush br(el.clr);
		CPen pen(PS_SOLID, 1, el.clr);
		CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&br);
		CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);

		CPoint pt0 = scale(el.pt.x, el.pt.y, false);
		CPoint pt1 = scale(el.pt.x + el.size.cx, el.pt.y + el.size.cy, false);
		CPoint pt2 = scale(el.pt.x - el.size.cx, el.pt.y - el.size.cy, false);

		CRect r(std::min(pt1.x, pt2.x), std::min(pt1.y, pt2.y), std::max(pt1.x, pt2.x), std::max(pt1.y, pt2.y));
		if (r.Width() < 4)
		{
			r.left = pt0.x - 2;
			r.right = pt0.x + 2;
		}

		if (r.Height() < 4)
		{
			r.top = pt0.y - 2;
			r.bottom = pt0.y + 2;
		}

		pDC->Ellipse(r);

		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}

}

void Plot2d::drawAxis(CDC* pDC)
{
	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_penGrid);
	if (m_hgs.size() == 0)
		m_xAxis.draw(pDC, this);

	m_yAxis.draw(pDC, this);
	if (m_bEnableSecondaryAxis)
		m_yAxis2.draw(pDC, this);
	pDC->SelectObject(pOldPen);

}

void Plot2d::drawMarkers(CDC* pDC, BOOL bPaint)
{
	if (m_marks.size() == 0)
		return;

	int nOldRop = pDC->GetROP2();

	if (!bPaint)
		pDC->SetROP2(R2_NOT);

	for (int i = 0; i < (int)m_marks.size(); i++)
	{
		CMarker& m = m_marks[i];
		CPen pen(PS_SOLID, 1, bPaint ? m.clr : RGB(0, 0, 0));

		CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);

		if (m.bX)
		{
			if (!_isnan(m.dCoordOld) && !bPaint) // && m.dCoordOld != m,dCoord)
			{
				Point2d pt1f(m.dCoordOld, m_frectPlotArea.pt.y);
				Point2d pt2f(m.dCoordOld, m_frectPlotArea.top());

				CPoint pt1 = scale(pt1f.x, pt1f.y, false);
				CPoint pt2 = scale(pt2f.x, pt2f.y, false);

				pDC->MoveTo(pt1);
				pDC->LineTo(pt2);
				m.dCoordOld = m.dCoord;
			}

			Point2d pt1f(m.dCoord, m_frectPlotArea.pt.y);
			Point2d pt2f(m.dCoord, m_frectPlotArea.top());

			CPoint pt1 = scale(pt1f.x, pt1f.y, false);
			CPoint pt2 = scale(pt2f.x, pt2f.y, false);

			pDC->MoveTo(pt1);
			pDC->LineTo(pt2);
		}
		else
		{
			if (!_isnan(m.dCoordOld) && !bPaint)
			{
				Point2d pt1f(m_frectPlotArea.pt.x, m.dCoordOld);
				Point2d pt2f(m_frectPlotArea.right(), m.dCoordOld);

				CPoint pt1 = scale(pt1f.x, pt1f.y, false);
				CPoint pt2 = scale(pt2f.x, pt2f.y, false);

				pDC->MoveTo(pt1);
				pDC->LineTo(pt2);
				m.dCoordOld = m.dCoord;
			}

			Point2d pt1f(m_frectPlotArea.pt.x, m.dCoord);
			Point2d pt2f(m_frectPlotArea.right(), m.dCoord);

			CPoint pt1 = scale(pt1f.x, pt1f.y, false);
			CPoint pt2 = scale(pt2f.x, pt2f.y, false);

			pDC->MoveTo(pt1);
			pDC->LineTo(pt2);
		}

		pDC->SelectObject(pOldPen);
	}

	pDC->SetROP2(nOldRop);
}

void Plot2d::drawEvents(CDC* pDC)
{
	CFont* pOldFont = (CFont*)pDC->SelectObject(&m_xAxis.m_font);

	int nd = 4;
	CRect rLast;
	for (int i = 0; i < (int)m_events.size(); i++)
	{
		CEvent& ev = m_events[i];
		CPen pen(PS_SOLID, 3, ev.clr);
		CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);

		Point2d ptf0(ev.x, m_frectPlotArea.top());

		CPoint pt0 = scale(ptf0.x, ptf0.y, false);
		CPoint pt1 = pt0 + CSize(0, 5);

		CRect r(0, 0, 0, 0);
		pDC->DrawText(ev.szTitle, strlen(ev.szTitle), &r, DT_CALCRECT);

		int nWidth = r.Width();
		int nHeight = r.Height();
		r.left = pt0.x - nWidth / 2;
		r.right = pt0.x + nWidth / 2;
		r.top = pt1.y + nd;
		r.bottom = pt1.y + nd + nHeight;

		CRect rInter;

		if (i > 0 && rInter.IntersectRect(rLast, r))
		{
			r.top = rLast.bottom + nd;
			r.bottom = r.top + nHeight;
			pt1.y = r.top - nd;
		}

		rLast = r;

		pDC->MoveTo(pt0);
		pDC->LineTo(pt1);
		pDC->DrawText(ev.szTitle, strlen(ev.szTitle), &r, DT_CENTER);

		pDC->SelectObject(pOldPen);
	}

	pDC->SelectObject(pOldFont);
}

void Plot2d::drawAreas(CDC* pDC)
{
	COLORREF clrOld = pDC->GetBkColor();
	for (int i = 0; i < (int)m_areas.size(); i++)
	{
		CArea& a = m_areas[i];

		Point2d ptf0(a.x0, m_frectPlotArea.top());
		Point2d ptf1(a.x1, m_frectPlotArea.pt.y);

		CPoint pt0 = scale(ptf0.x, ptf0.y, false);
		CPoint pt1 = scale(ptf1.x, ptf1.y, false);

		CRect r(pt0, pt1);

		pDC->FillSolidRect(r, a.clr);

		{
			CFont* pOldFont = (CFont*)pDC->SelectObject(&m_xAxis.m_font);
			CRect rt(pt0.x, pt0.y+5, pt1.x, pt0.y+5);
			int nHeight = pDC->DrawText(a.szTitle, strlen(a.szTitle), rt, DT_CALCRECT);
			int nWidth = rt.Width();
			int nCenter = (pt0.x + pt1.x) / 2;
			rt.bottom += nHeight;
			rt.left = nCenter - nWidth/2;
			rt.right = nCenter + nWidth / 2;
			pDC->DrawText(a.szTitle, strlen(a.szTitle), rt, DT_CENTER);
			pDC->SelectObject(pOldFont);
		}
	}
	pDC->SetBkColor(clrOld);
}

void Plot2d::drawInscriptions(CDC* pDC)
{
	CFont* pOldFont = m_pLegendFont != nullptr ? (CFont *)pDC->SelectObject(m_pLegendFont) : nullptr;
	for (int i = 0; i < (int)m_inscs.size(); i++)
	{
		Inscription* pIns = m_inscs[i];
		pIns->draw(*pDC, this);
	}
	if (pOldFont != nullptr)
		pDC->SelectObject(pOldFont);
}


void Plot2d::drawTip(CDC* pDC)
{
	CRect r;
	GetClientRect(r);

	pDC->FillSolidRect(r, RGB(220, 230, 240)); //m_clrBack);

	CRect rectPlot(m_xAxis.m_r.left, m_yAxis.m_r.top, m_xAxis.m_r.right, m_yAxis.m_r.bottom);

	if (rectPlot.PtInRect(m_ptLastTip))
	{
		pDC->SetBkMode(TRANSPARENT);

		CFont* pOldFont = pDC->SelectObject(&m_xAxis.m_font);

		CPoint ptL(m_xAxis.m_r.left, m_ptLastTip.y);
		CPoint ptR(m_xAxis.m_r.right, m_ptLastTip.y);
		CPoint ptT(m_ptLastTip.x, m_yAxis.m_r.top);
		CPoint ptB(m_ptLastTip.x, m_yAxis.m_r.bottom);

		CPen pen(PS_DOT, 1, RGB(0, 0, 0));

		CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);

		pDC->MoveTo(ptL);
		pDC->LineTo(ptR);
		pDC->MoveTo(ptT);
		pDC->LineTo(ptB);

		CPen penBorder(PS_SOLID, 1, RGB(0, 0, 0));
		pDC->SelectObject(&penBorder);

		CString cs;
		Point2d ptf = reverseScale(m_ptLastTip.x, m_ptLastTip.y, FALSE);

		static CSize sc(6, 4);
		{
			cs.Format("%.3f", ptf.y);

			CRect r;
			int nHeight = pDC->DrawText(cs, r, DT_SINGLELINE | DT_CALCRECT) + sc.cy;
			int nWidth = r.Width() + sc.cx;

			r.top = m_ptLastTip.y - nHeight / 2;
			r.left = rectPlot.left - nWidth;
			r.bottom = r.top + nHeight;
			r.right = r.left + nWidth;

			pDC->FillSolidRect(&r, m_clrBack);
			pDC->Rectangle(r);
			pDC->DrawText(cs, r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}

		{
			if (m_xAxis.m_format == Axis2d::eaTime)
			{
				CTime t((long)ptf.x);
				cs = t.Format("%H:%M:%S");

			}
			else if (m_xAxis.m_format == Axis2d::eaDateTime || m_xAxis.m_format == Axis2d::eaDateHours)
			{
				CTime t((long)ptf.x);
				cs = t.Format("%d.%m.%y %H:%M:%S");

			}
			else if (m_xAxis.m_format == Axis2d::eaDateTimeGPS)
			{
				CTime t((long)ptf.x);
				double ms = (ptf.x - floor(ptf.x)) * 1000;
				if (m_xAxis.m_precision != 0)
				{
					ms = floor(ms / m_xAxis.m_precision + 0.5) * m_xAxis.m_precision;
					if (ms >= 1000)
					{
						ms = 0;
						t += CTimeSpan(0, 0, 0, 1);
					}
				}
				cs.Format("%s %.0f ms", (LPCTSTR)t.FormatGmt("%d.%m.%y %H:%M:%S"), ms);

			}
			else
			{
				cs.Format("%f", ptf.x);
			}

			int nArea = -1;
			for (int i = 0; i < (int)m_areas.size(); i++)
			{
				CArea& a = m_areas[i];
				if (a.x0 <= ptf.x && ptf.x <= a.x1)
				{
					nArea = i;
					break;
				}
			}

			int nFormats[2] = { DT_SINGLELINE | DT_CALCRECT, DT_SINGLELINE | DT_VCENTER | DT_CENTER };
			if (nArea >= 0)
			{
				CString csArea = m_areas[nArea].szTitle;
				cs += "\n" + csArea;
				nFormats[0] = DT_CALCRECT;
				nFormats[1] = DT_CENTER;
			}


			CRect r;
			int nHeight = pDC->DrawText(cs, r, nFormats[0]) + sc.cy;
			int nWidth = r.Width() + sc.cx;

			r.top = rectPlot.bottom + sc.cy;
			r.left = m_ptLastTip.x - nWidth / 2;
			r.bottom = r.top + nHeight;
			r.right = r.left + nWidth;

			pDC->FillSolidRect(&r, m_clrBack);
			pDC->Rectangle(r);
			pDC->DrawText(cs, r, nFormats[1]);

		}

		if (m_bEnableSecondaryAxis)
		{
			ptf = reverseScale(m_ptLastTip.x, m_ptLastTip.y, TRUE);

			cs.Format("%.2f", ptf.y);
			CRect r;
			int nHeight = pDC->DrawText(cs, r, DT_SINGLELINE | DT_CALCRECT) + sc.cy;
			int nWidth = r.Width() + sc.cx;

			r.top = m_ptLastTip.y - nHeight / 2;
			r.left = rectPlot.right + sc.cx;
			r.bottom = r.top + nHeight;
			r.right = r.left + nWidth;

			pDC->FillSolidRect(&r, m_clrBack);
			pDC->Rectangle(r);
			pDC->DrawText(cs, r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}

		pDC->SelectObject(pOldPen);

	}
}

void Plot2d::drawLegend(CDC* pDC, BOOL bPrimary)
{
	if (m_hgs.size() != 0)
	{
		if (bPrimary)
			drawHgLegend(pDC);
		return;
	}

	bool bLegend = bPrimary ? m_bLegend : m_bLegend2;
	CLegendAlign eLegendAlign = bPrimary ? m_eLegendAlign : m_eLegendAlign2;
	std::vector <Curve2d*>& curves = bPrimary ? m_curves : m_curves2;

	if (!bLegend)
		return;

	struct LegItem {
		int key;
		std::string name;
		COLORREF clr;
		bool draw;
	};
	std::vector <LegItem> legs;
	bool bHg = false;
	{
		for (int i = 0; i < (int)curves.size(); i++)
		{
			legs.push_back({ 0, curves[i]->m_name, curves[i]->m_rgb, curves[i]->m_bDrawCurve });
		}
	}

	if (legs.size() < 1)
		return;

	CFont* pOldFont = m_pLegendFont != nullptr ? (CFont*)pDC->SelectObject(m_pLegendFont) : nullptr;

	int nHeight = 0;
	int nWidth = 0;
	int nRowHeight;
	for (int i = 0; i < (int)legs.size(); i++)
	{
		//Curve2d* pCurve = curves[i];
		CRect r(0, 0, 0, 0);
		int n = nRowHeight = pDC->DrawText(legs[i].name.c_str(), r, DT_CALCRECT);
		nHeight += n + n / 2;
		if (i == 0)
			nHeight += n / 2;
		if (r.Width() > nWidth)
			nWidth = r.Width();
	}

	if (nHeight <= 0)
	{
		if (pOldFont != nullptr)
			pDC->SelectObject(pOldFont);
		return;
	}

	int nLineLen = 10;

	CRect rCheck(0, 0, nRowHeight, nRowHeight);

	CRect rLegend(0, 0, nWidth + nLineLen + 20 + rCheck.Width(), nHeight);
	CRect rectPlot;
	if (m_eMode == emDecart)
	{
		rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
		rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);
	}
	else if (m_eMode == emPolar)
	{
		rectPlot = m_rectPolar;
		rectPlot.InflateRect(CSize(50, 50));
	}
	switch (eLegendAlign)
	{
	default:
	case elaTopLeft:
		if (m_areas.size() > 0)
			rLegend.OffsetRect(rectPlot.left + 5, rectPlot.top + 40);
		else
			rLegend.OffsetRect(rectPlot.left + 5, rectPlot.top);
		break;
	case elaTopRight:
		if (m_areas.size() > 0)
			rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.top+40);
		else
			rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.top);
		break;
	case elaBottomRight:
		rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.bottom - rLegend.Height());
		break;
	case elaBottomLeft:
		rLegend.OffsetRect(rectPlot.left + 2, rectPlot.bottom - rLegend.Height());
		break;
	}

	COLORREF clr = pDC->GetTextColor();

	BOOL bDrawCheck = (bPrimary ? m_bCheckLegend : m_bCheckLegend2);
	CRect rRow(rLegend.left, rLegend.top + nRowHeight / 2, rLegend.right, rLegend.top + nRowHeight * 3 / 2);
	for (int i = 0; i < (int)legs.size(); i++)
	{
		//Curve2d* pCurve = curves[i];
		//CPen * pOldPen = (CPen *)pDC->SelectObject(&(pCurve->m_pen));
		//pDC->MoveTo(rRow.left + rCheck.Width() + 2, (rRow.top + rRow.bottom) / 2);
		//pDC->LineTo(rRow.left + rCheck.Width() + 18, (rRow.top + rRow.bottom) / 2);
		//pDC->SelectObject(pOldPen);
		pDC->SetTextColor(legs[i].clr);
		CRect r = rRow;

		CRect rc(rRow.left, rRow.top, rRow.left + rCheck.Width(), rRow.bottom);
		if (bDrawCheck && !bHg)
		{
			pDC->DrawFrameControl(rc, DFC_BUTTON, DFCS_BUTTONCHECK | (legs[i].draw ? DFCS_CHECKED : 0));
			curves[i]->m_rCheckBox = rc;
		}

		r.left += rc.Width() + 10;
		pDC->DrawText(legs[i].name.c_str(), r, DT_LEFT);
		rRow.OffsetRect(0, nRowHeight + nRowHeight / 2);
	}

	pDC->SetTextColor(clr);
	if (pOldFont != nullptr)
		pDC->SelectObject(pOldFont);
}

void Plot2d::drawHgLegend(CDC* pDC)
{
	CLegendAlign eLegendAlign = m_eLegendAlign;

	struct HeadItem
	{
		std::string name;
		CRect r;
	};

	struct LegItem {
		int key;
		std::string name;
		std::vector<COLORREF> clr;
		bool draw;
	};

	std::vector<HeadItem> heads;
	std::vector <LegItem> legs;

	CFont* pOldFont = m_pLegendFont != nullptr ? (CFont*)pDC->SelectObject(m_pLegendFont) : nullptr;

	static CPoint ptSpace(5, 2);
	CPoint pt0(0, 0);
	int nHeadHeight = 0;
	for (int i = 0; i < (int)m_hgs.size(); i++)
	{
		CRect r(0, 0, 0, 0);
		int n = nHeadHeight = pDC->DrawText(m_hgs[i].name.c_str(), r, DT_CALCRECT);
		r.OffsetRect(pt0);
		heads.push_back({ m_hgs[i].name, r });
		pt0.x += r.Width() + ptSpace.x;
	}
	int nHeadWidth = pt0.x;
	nHeadHeight = nHeadHeight;
	if (m_hgs.size() < 2)
		nHeadHeight = 0;

	for (std::map<int, HistGroup::Type>::iterator it = m_hgs[0].types.begin(); it != m_hgs[0].types.end(); it++)
	{
		std::vector<COLORREF> clrs;
		for (int i = 0; i < (int)m_hgs.size(); i++)
		{
			COLORREF clr = m_hgs[i].types[it->first].clr;
			clrs.push_back(clr);
		}
		legs.push_back({ it->first, it->second.name, clrs, true });
	}

	if (legs.size() < 1)
		return;


	int nHeight = nHeadHeight;
	int nWidth = 0;
	int nRowHeight;
	for (int i = 0; i < (int)legs.size(); i++)
	{
		//Curve2d* pCurve = curves[i];
		CRect r(0, 0, 0, 0);
		int n = nRowHeight = pDC->DrawText(legs[i].name.c_str(), r, DT_CALCRECT);
		nHeight += n + n / 4;
		//if (i == 0)
		//	nHeight += n / 2;
		if (r.Width() > nWidth)
			nWidth = r.Width();
	}

	if (nHeight <= 0)
	{
		if (pOldFont != nullptr)
			pDC->SelectObject(pOldFont);
		return;
	}

	int nLineLen = 10;

	CRect rCheck(0, 0, nRowHeight, nRowHeight);

	CRect rLegend(0, 0, nWidth + nLineLen + 20 + nHeadWidth, nHeight + nHeadHeight);
	CRect rectPlot;
	if (m_eMode == emDecart)
	{
		rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
		rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);
	}
	else if (m_eMode == emPolar)
	{
		rectPlot = m_rectPolar;
		rectPlot.InflateRect(CSize(50, 50));
	}
	switch (eLegendAlign)
	{
	default:
	case elaTopLeft:
		if (m_areas.size() > 0)
			rLegend.OffsetRect(rectPlot.left + 5, rectPlot.top + 40);
		else
			rLegend.OffsetRect(rectPlot.left + 5, rectPlot.top);
		break;
	case elaTopRight:
		if (m_areas.size() > 0)
			rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.top + 40);
		else
			rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.top);
		break;
	case elaBottomRight:
		rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.bottom - rLegend.Height());
		break;
	case elaBottomLeft:
		rLegend.OffsetRect(rectPlot.left + 2, rectPlot.bottom - rLegend.Height());
		break;
	}

	COLORREF clr = pDC->GetTextColor();

	if (heads.size() > 1)
	{
		for (int i = 0; i < (int)heads.size(); i++)
		{
			CRect r = heads[i].r;
			r.OffsetRect(rLegend.TopLeft());

			pDC->DrawText(heads[i].name.c_str(), heads[i].name.length(), r, DT_SINGLELINE);
		}
	}

	BOOL bDrawCheck = m_bCheckLegend;
	CRect rRow(rLegend.left, rLegend.top + nHeadHeight + nRowHeight / 8, rLegend.right, rLegend.top + nRowHeight * 3 / 2 + nHeadHeight);
	for (int i = 0; i < (int)legs.size(); i++)
	{
		//Curve2d* pCurve = curves[i];
		//CPen * pOldPen = (CPen *)pDC->SelectObject(&(pCurve->m_pen));
		//pDC->MoveTo(rRow.left + rCheck.Width() + 2, (rRow.top + rRow.bottom) / 2);
		//pDC->LineTo(rRow.left + rCheck.Width() + 18, (rRow.top + rRow.bottom) / 2);
		//pDC->SelectObject(pOldPen);
		for (int j = 0; j < (int)legs[i].clr.size(); j++)
		{
			COLORREF clr = pDC->GetBkColor();
			CSize s = heads[j].r.Size(); s.cx /= 4; s.cy /= 4;
			CRect r = heads[j].r;
			r.OffsetRect(rRow.TopLeft());
			r.left += s.cx;
			r.top += s.cy;
			r.right -= s.cx;
			r.bottom -= s.cy;
			pDC->FillSolidRect(r, legs[i].clr[j]);
			pDC->SetBkColor(clr);
		}
		CRect r = rRow;

		CRect rc(rRow.left, rRow.top, rRow.left + rCheck.Width(), rRow.bottom);
		{
			for (int j = 0; j < (int)m_hgs.size(); j++)
			{
				m_hgs[j].types[legs[i].key].rectLegend = rc;
			}
		}

		r.left += rRow.Width() - nWidth;
		pDC->DrawText(legs[i].name.c_str(), r, DT_LEFT);
		rRow.OffsetRect(0, nRowHeight + nRowHeight / 4);
	}

	pDC->SetTextColor(clr);
	if (pOldFont != nullptr)
		pDC->SelectObject(pOldFont);
}

void Plot2d::drawPolarLegend(CDC& dc)
{
	BOOL bLegend = m_bLegend;
	CLegendAlign eLegendAlign = m_eLegendAlign;
	std::vector <Curve2d*>& curves = m_curves;

	if (!bLegend)
		return;

	if (curves.size() < 1)
		return;

	CFont* pFont = GetParent()->GetFont();

	if (pFont == NULL)
		return;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	pFont->GetLogFont(&lf);

	CRect rectClient;
	GetClientRect(rectClient);
	lf.lfHeight = rectClient.Height() / 20; // 30;

	CFont font;
	font.CreateFontIndirect(&lf);

	CFont* pOldFont = (CFont*)dc.SelectObject(&font);

	int nHeight = 0;
	int nWidth = 0;
	int nRowHeight;
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];
		CRect r(0, 0, 0, 0);
		int n = nRowHeight = dc.DrawText(pCurve->m_name.c_str(), r, DT_CALCRECT);
		nHeight += n + n / 2;
		if (i == 0)
			nHeight += n / 2;
		if (r.Width() > nWidth)
			nWidth = r.Width();
	}

	if (nHeight <= 0)
		return;

	int nLineLen = 10;

	CRect rCheck(0, 0, nRowHeight, nRowHeight);

	CRect rLegend(0, 0, nWidth + nLineLen + 20 + rCheck.Width(), nHeight);
	CRect rectPlot;
	if (m_eMode == emDecart)
	{
		rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
		rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);
	}
	else if (m_eMode == emPolar)
	{
		rectPlot = m_rectPolar;
		rectPlot.InflateRect(CSize(50, 50));
	}
	switch (eLegendAlign)
	{
	default:
	case elaTopLeft:
		rLegend.OffsetRect(rectPlot.left + 2, rectPlot.top);
		break;
	case elaTopRight:
		rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.top);
		break;
	case elaBottomRight:
		rLegend.OffsetRect(rectPlot.right - rLegend.Width(), rectPlot.bottom - rLegend.Height());
		break;
	case elaBottomLeft:
		rLegend.OffsetRect(rectPlot.left + 2, rectPlot.bottom - rLegend.Height());
		break;
	}

	COLORREF clr = dc.GetTextColor();
	CRect rRow(rLegend.left, rLegend.top + nRowHeight / 2, rLegend.right, rLegend.top + nRowHeight * 3 / 2);
	for (int i = 0; i < (int)curves.size(); i++)
	{
		Curve2d* pCurve = curves[i];
		dc.SetTextColor(pCurve->m_rgb);
		CRect r = rRow;

		CRect rc(rRow.left, rRow.top, rRow.left + rCheck.Width(), rRow.bottom);
		pCurve->drawLegendLine(dc, rc);
		pCurve->m_rCheckBox = rc;

		r.left += rc.Width() + 10;
		dc.DrawText(pCurve->m_name.c_str(), r, DT_LEFT);
		rRow.OffsetRect(0, nRowHeight + nRowHeight / 2);
	}

	dc.SetTextColor(clr);
}

void Plot2d::drawPolarGrid(CDC& dc)
{
	double step = (m_yAxis.m_max - m_yAxis.m_min) / m_yAxis.m_divs;
	for (int i = 1; i <= m_yAxis.m_divs; i++)
	{
		double r = step * i;
		int n = (int)(r * m_fsizeScale.cy);
		dc.MoveTo(m_ptPolarCenter.x + n, m_ptPolarCenter.y);
		dc.AngleArc(m_ptPolarCenter.x, m_ptPolarCenter.y, n, 0, 360);
	}

	for (int i = 0; i < 12; i++)
	{
		double r = (m_yAxis.m_max - m_yAxis.m_min);
		double a = 30 * i;
		int nx = (int)(r * sin(a * PI / 180) * m_fsizeScale.cy);
		int ny = (int)(-r * cos(a * PI / 180) * m_fsizeScale.cy);

		CPoint pt = m_ptPolarCenter + CPoint(nx, ny);
		dc.MoveTo(m_ptPolarCenter);
		dc.LineTo(pt);

	}

}

void Plot2d::drawPolarGridLabels(CDC& dc)
{
	dc.BeginPath();
	drawPolarGridLabelsInt(dc);
	dc.EndPath();

	CPen pen(PS_SOLID, 9, RGB(255, 255, 255));
	int nNumPts = dc.GetPath(NULL, NULL, 0);
	if (nNumPts == 0)
	{
		return;
	}

	LPPOINT lpPoints = new POINT[nNumPts];
	if (lpPoints == NULL)
		return;
	LPBYTE lpTypes = new BYTE[nNumPts];
	if (lpTypes == NULL)
	{
		delete[] lpPoints;
		return;
	}

	nNumPts = dc.GetPath(lpPoints, lpTypes, nNumPts);

	CPen* pOldPen = (CPen*)dc.SelectObject(&pen);


	if (nNumPts != -1)
		dc.PolyDraw(lpPoints, lpTypes, nNumPts);

	dc.SelectObject(pOldPen);

	delete[] lpPoints;
	delete[] lpTypes;

	drawPolarGridLabelsInt(dc);
}

void Plot2d::drawPolarGridLabelsInt(CDC& dc)
{
	CFont* pFont = GetParent()->GetFont();

	if (pFont == NULL)
		return;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	pFont->GetLogFont(&lf);

	CRect rectClient;
	GetClientRect(rectClient);
	lf.lfHeight = rectClient.Height() / 20;
	strcpy_s(lf.lfFaceName, "Arial");

	CFont font;
	font.CreateFontIndirect(&lf);

	CFont* pOldFont = (CFont*)dc.SelectObject(&font);
	int nOldMode = dc.SetBkMode(TRANSPARENT);

	double step = (m_yAxis.m_max - m_yAxis.m_min) / m_yAxis.m_divs;
	for (int i = 1; i <= m_yAxis.m_divs; i++)
	{
		double r = step * i;
		int n = (int)(r * m_fsizeScale.cy);

		CString cs;
		cs.Format("%.0f dB", r + m_yAxis.m_min);
		CRect rect;
		int nHeight = dc.DrawText(cs, rect, DT_SINGLELINE | DT_CALCRECT);
		int nWidth = rect.Width();

		rect.OffsetRect(m_ptPolarCenter + CPoint(-nWidth / 2, -n - nHeight / 2));
		dc.DrawText(cs, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	for (int i = 0; i < 12; i++)
	{
		double r = (m_yAxis.m_max - m_yAxis.m_min);
		double a = 30 * i;
		int nx = (int)(r * sin(a * PI / 180) * m_fsizeScale.cy);
		int ny = (int)(-r * cos(a * PI / 180) * m_fsizeScale.cy);

		CPoint pt = m_ptPolarCenter + CPoint(nx, ny);

		r = sqrt(nx * nx + ny * ny) + 25;
		nx = (int)(r * sin(a * PI / 180));
		ny = (int)(-r * cos(a * PI / 180));

		CString cs;
		if (a <= 180)
			cs.Format("%.0f%c", a, 0xB0);
		else
			cs.Format("%.0f%c", a - 360.0, 0xB0);
		CRect rect;
		int nHeight = dc.DrawText(cs, rect, DT_SINGLELINE | DT_CALCRECT);
		int nWidth = rect.Width();

		rect.OffsetRect(m_ptPolarCenter + CPoint(nx - nWidth / 2, ny - nHeight / 2));
		dc.DrawText(cs, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	}

	dc.SelectObject(pOldFont);
	dc.SetBkMode(nOldMode);

}

void Plot2d::paintTip()
{
	Invalidate(FALSE);

	CClientDC dcPaint(this);

	{
		CDC dc;
		dc.CreateCompatibleDC(&dcPaint);
		CBitmap* pOldAllBmp = dc.SelectObject(&m_bmpAll);

		dcPaint.BitBlt(0, 0, m_sizeBmp.cx, m_sizeBmp.cy, &dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldAllBmp);
	}

	{
		CDC dc;
		dc.CreateCompatibleDC(&dcPaint);

		CBitmap* pOldBmp = dc.SelectObject(&m_bmpTip);

		dc.SelectObject(&m_brushBack);
		drawTip(&dc);

		COLORREF clrTransparent = RGB(220, 230, 240); //m_clrBack; //RGB (255, 255, 255);
		::TransparentBlt(dcPaint.m_hDC, 0, 0, m_sizeBmp.cx, m_sizeBmp.cy, dc.m_hDC, 0, 0, m_sizeBmp.cx, m_sizeBmp.cy, clrTransparent);

		dc.SelectObject(pOldBmp);
	}

	CRect rectClient;
	GetClientRect(rectClient);
	ValidateRect(rectClient);
}

void Plot2d::setAxis(bool bX, double dMin, double dMax, int nDivisions, bool bSecondary)
{
	if (bX)
	{
		if (!_isnan(dMin) && !_isnan(dMax) && dMin < dMax)
		{
			m_xAxis.m_auto = FALSE;
			m_xAxis.m_divs = nDivisions;
			if (!_isnan(dMin))
				m_xAxis.m_min = dMin;
			if (!_isnan(dMax))
				m_xAxis.m_max = dMax;
			if (!_isnan(dMin))
				m_frectPlotArea.pt.x = dMin;
			if (!_isnan(dMax))
				m_frectPlotArea.s.cx = dMax - dMin;
			if (!_isnan(dMin))
				m_frectPlotArea2.pt.x = dMin;
			if (!_isnan(dMax))
				m_frectPlotArea2.s.cx = dMax - dMin;
		}
		else
		{
			m_xAxis.m_auto = true;
		}
	}
	else
	{
		if (!bSecondary)
		{
			if (!_isnan(dMin) && !_isnan(dMax) && dMin < dMax)
			{
				m_yAxis.m_auto = FALSE;
				m_yAxis.m_divs = nDivisions;
				m_yAxis.m_min = dMin;
				m_yAxis.m_max = dMax;
				m_frectPlotArea.pt.y = dMin;
				m_frectPlotArea.s.cy = dMax - dMin;
			}
			else
			{
				m_yAxis.m_auto = true;
			}
		}
		else
		{
			if (!_isnan(dMin) && !_isnan(dMax) && dMin < dMax)
			{
				m_yAxis2.m_auto = FALSE;
				m_yAxis2.m_divs = nDivisions;
				m_yAxis2.m_min = dMin;
				m_yAxis2.m_max = dMax;
				m_frectPlotArea2.pt.y = dMin;
				m_frectPlotArea2.s.cy = dMax - dMin;
			}
			else
			{
				m_yAxis2.m_auto = true;
			}
		}
	}

	CRect rcClient;
	GetClientRect(rcClient);

	CClientDC dc(this);
	computeScale(&dc, rcClient);
}

void Plot2d::setAxisDivisions(bool bX, int nDivisions, bool bPrimary)
{
	if (bX)
		m_xAxis.m_divs = nDivisions;
	else
	{
		if (bPrimary)
			m_yAxis.m_divs = nDivisions;
		else
			m_yAxis2.m_divs = nDivisions;
	}
}

void Plot2d::setAxisPrecision(bool bX, int nPrec, bool bPrimary)
{
	if (bX)
		m_xAxis.m_precision = nPrec;
	else
	{
		if (bPrimary)
			m_yAxis.m_precision = nPrec;
		else
			m_yAxis2.m_precision = nPrec;
	}
}

void Plot2d::autoScale(bool bxAuto, bool byAuto, bool byAuto2)
{
	m_xAxis.m_auto = bxAuto;
	m_yAxis.m_auto = byAuto;

	if (m_hgs.size() != 0)
	{
		m_frectPlotArea.pt.x = 0;
		m_frectPlotArea.s.cx = 1;
		if (byAuto)
		{
			double yMin = DBL_MAX, yMax = -DBL_MAX;
			for (int i = 0; i < (int)m_hgs.size(); i++)
			{
				m_hgs[i].getMinMax(yMin, yMax);
			}

			m_frectPlotArea.pt.y = yMin;
			m_frectPlotArea.s.cy = yMax - yMin;

			m_yAxis.onDataChanged(yMin, yMax);

			m_frectPlotArea.pt.y = m_yAxis.m_min;
			m_frectPlotArea.s.cy = m_yAxis.m_max - m_yAxis.m_min;
		}
	}
	else
	{

		if (m_curves.size() <= 0)
		{
			if (bxAuto)
			{
				m_frectPlotArea.pt.x = 0;
				m_frectPlotArea.s.cx = 1;
			}

			if (byAuto)
			{
				m_frectPlotArea.pt.y = 0;
				m_frectPlotArea.s.cy = 1;
			}

		}
		else
		{

			Point2d ptMin(DBL_MAX, DBL_MAX);
			Point2d ptMax(-DBL_MAX, -DBL_MAX);

			for (int i = 0; i < (int)m_curves.size(); i++)
			{
				Curve2d* pCurve = m_curves[i];
				if (!pCurve->m_bDrawCurve)
					continue;

				if (ptMin.x > pCurve->m_ptMin.x)
					ptMin.x = pCurve->m_ptMin.x;

				if (ptMax.x < pCurve->m_ptMax.x)
					ptMax.x = pCurve->m_ptMax.x;

				if (ptMin.y > pCurve->m_ptMin.y)
					ptMin.y = pCurve->m_ptMin.y;

				if (ptMax.y < pCurve->m_ptMax.y)
					ptMax.y = pCurve->m_ptMax.y;
			}

			if (bxAuto)
			{
				m_frectPlotArea.pt.x = ptMin.x;
				m_frectPlotArea.s.cx = ptMax.x - ptMin.x;

				m_xAxis.onDataChanged(ptMin.x, ptMax.x);

				m_frectPlotArea.pt.x = m_xAxis.m_min;
				m_frectPlotArea.s.cx = m_xAxis.m_max - m_xAxis.m_min;
			}
			if (byAuto)
			{
				m_frectPlotArea.pt.y = ptMin.y;
				m_frectPlotArea.s.cy = ptMax.y - ptMin.y;

				m_yAxis.onDataChanged(ptMin.y, ptMax.y);

				m_frectPlotArea.pt.y = m_yAxis.m_min;
				m_frectPlotArea.s.cy = m_yAxis.m_max - m_yAxis.m_min;
			}
		}

		if (m_curves2.size() <= 0)
		{
			if (bxAuto)
			{
				m_frectPlotArea2.pt.x = 0;
				m_frectPlotArea2.s.cx = 1;
			}

			if (byAuto2)
			{
				m_frectPlotArea2.pt.y = 0;
				m_frectPlotArea2.s.cy = 1;
			}

			if (IsWindow(m_hWnd))
			{
				CRect rc;
				GetClientRect(rc);

				CClientDC dc(this);
				computeScale(&dc, rc);
			}
			return;
		}
		else
		{
			Point2d ptMin(DBL_MAX, DBL_MAX);
			Point2d ptMax(-DBL_MAX, -DBL_MAX);

			for (int i = 0; i < (int)m_curves2.size(); i++)
			{
				Curve2d* pCurve = m_curves2[i];

				if (ptMin.x > pCurve->m_ptMin.x)
					ptMin.x = pCurve->m_ptMin.x;

				if (ptMax.x < pCurve->m_ptMax.x)
					ptMax.x = pCurve->m_ptMax.x;

				if (ptMin.y > pCurve->m_ptMin.y)
					ptMin.y = pCurve->m_ptMin.y;

				if (ptMax.y < pCurve->m_ptMax.y)
					ptMax.y = pCurve->m_ptMax.y;
			}

			if (byAuto2)
			{
				m_frectPlotArea2.pt.y = ptMin.y;
				m_frectPlotArea2.s.cy = ptMax.y - ptMin.y;

				m_yAxis2.onDataChanged(ptMin.y, ptMax.y); //GetAutoValues (ptMin.y, ptMax.y, m_frectPlotArea.ptBase.y, m_frectPlotArea.ptTop.y, m_nyDiv);

				m_frectPlotArea2.pt.y = m_yAxis2.m_min;
				m_frectPlotArea2.s.cy = m_yAxis2.m_max - m_yAxis2.m_min;
			}

			m_frectPlotArea2.pt.x = m_frectPlotArea.pt.x;
			m_frectPlotArea2.s.cx = m_frectPlotArea.s.cx;
		}
	}

	if (IsWindow(m_hWnd))
	{
		CRect rc;
		GetClientRect(rc);

		CClientDC dc(this);
		computeScale(&dc, rc);
	}
}

bool Plot2d::getAutoValues(double dDataMin, double dDataMax, double& dAutoMin, double& dAutoMax, int& nDiv)
{
	double fD;

	ASSERT(dDataMin <= dDataMax);

	// 1. Choose Data Range in the first approximation
	fD = dDataMax - dDataMin;

	if (fD <= 0)
	{
		TRACE("Can't change axis parameters in GetXAxisMinMax\n");
		return false;
	}

	int nDeg;
	{
		nDeg = (int)log10(fD);
		fD = fD / pow(10.0, nDeg);
		if (fD > 3.0)
		{
			fD /= 10;
			nDeg++;
		}
		if (fD <= 0.3)
		{
			fD *= 10;
			nDeg--;
		}
	}

	// 2. Choose Label distance
	double dLabelDistance = 0;
	//if (m_bLabelDistanceAuto)
	{
		double tmp, tmpmin = DBL_MAX;
		int k, j;

		for (int i = 0; i < MAX_ROUNDS; i++)
		{
			tmp = fD / gl_dRounds[i];
			k = (int)ceil(tmp);
			tmp = fabs(tmp - floor(tmp)) * gl_dRounds[i];
			if ((tmpmin > tmp) && (k >= 2) && (k <= 6))
			{
				j = i;
				tmpmin = tmp;
			}
		}
		dLabelDistance = gl_dRounds[j] * (float)pow(10.0, (double)nDeg);
	}

	// 3. Choose MinAxis & MaxAxis
	dAutoMin = floor(dDataMin / dLabelDistance) * dLabelDistance;
	dAutoMax = ceil(dDataMax / dLabelDistance) * dLabelDistance;
	nDiv = (int)((dAutoMax - dAutoMin) / dLabelDistance);
	return true;
}

void Plot2d::computeScale(CDC* pDC, const CRect& rectClient)
{
	if (m_eMode == emDecart)
	{
		CRect rect = rectClient;
		//GetClientRect (&rect);

		rect.top += 20;
		rect.right -= 70;
		rect.bottom -= 30;
		rect.left += 30;

		if (!m_plotTitle.isEmpty())
		{
			m_plotTitle.layout(*pDC, rect);
			rect.top = m_plotTitle.m_rect.bottom + 20;
		}

		m_xAxis.layout(pDC, rect, this);

		m_yAxis.layout(pDC, rect, this);
		m_xAxis.m_r.left = m_yAxis.m_r.right;

		if (m_bEnableSecondaryAxis)
		{
			m_yAxis2.layout(pDC, rect, this);
			m_xAxis.m_r.right = m_yAxis2.m_r.left;
		}

		m_yAxis2.m_r.bottom = m_xAxis.m_r.top;
		m_yAxis.m_r.bottom = m_xAxis.m_r.top;

		m_ptPlotOrg.x = m_xAxis.m_r.left;
		m_ptPlotOrg.y = m_yAxis.m_r.bottom;

		int nPlotWidth = m_xAxis.m_r.Width();
		int nPlotHeight = m_yAxis.m_r.Height();

		m_fsizeScale.cx = (nPlotWidth / m_frectPlotArea.width()); // * 8 / 10);
		m_fsizeScale.cy = -(nPlotHeight / m_frectPlotArea.height()); // * 8 / 10);

		m_fsizeScale2.cx = (nPlotWidth / m_frectPlotArea2.width()); // * 8 / 10);
		m_fsizeScale2.cy = -(nPlotHeight / m_frectPlotArea2.height()); // * 8 / 10);

		if (!m_table.isEmpty())
			m_table.layout(pDC, rect, this);

		if (m_hgs.size() != 0)
		{
			CRect rplot(m_xAxis.m_r.left, m_yAxis.m_r.top, m_xAxis.m_r.right, m_yAxis.m_r.bottom);
			for (int i=0; i<(int)m_hgs.size(); i++)
				m_hgs[i].layout(pDC, rect, this);
		}
	}
	else if (m_eMode == emPolar)
	{
		m_rectPolar = rectClient;
		m_rectPolar.left += 60;
		m_rectPolar.right -= 60;
		m_rectPolar.top += 60;
		m_rectPolar.bottom -= 60;

		CPoint ptc(m_rectPolar.left + m_rectPolar.Width() / 2, m_rectPolar.top + m_rectPolar.Height() / 2);
		m_ptPolarCenter = ptc;
		int n = (std::min)(m_rectPolar.Width() / 2, m_rectPolar.Height() / 2);
		m_rectPolar.left = ptc.x - n;
		m_rectPolar.right = ptc.x + n;
		m_rectPolar.top = ptc.y - n;
		m_rectPolar.bottom = ptc.y + n;
		m_fsizeScale.cy = n / m_frectPlotArea.height();
	}
}


BEGIN_MESSAGE_MAP(Plot2d, CWnd)
	//{{AFX_MSG_MAP(Plot2d)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_PLOT_COPY_TO_CLIPBOARD, OnCopyToClipboard)
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Plot2d message handlers

bool Plot2d::isArgsEqual()
{
	std::vector <double> xs;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		Curve2d* pc = m_curves[i];
		if (i == 0)
		{
			for (int j = 0; j < pc->getCount(); j++)
			{
				Point2d pt = pc->GetPoint(j);
				xs.push_back(pt.x);
			}
		}
		else
		{
			if (pc->getCount() != xs.size())
				return false;

			for (int j = 0; j < pc->getCount(); j++)
			{
				Point2d pt = pc->GetPoint(j);
				if (pt.x != xs[j])
					return false;
			}
		}
	}

	return true;
}

bool Plot2d::hitTestLegend(bool bPrimary, CPoint pt) const
{
	if (m_hgs.size() == 0)
	{
		BOOL bLegend = (bPrimary ? m_bLegend : m_bLegend2);
		const std::vector <Curve2d*>& curves = bPrimary ? m_curves : m_curves2;

		if (!bLegend)
			return false;

		for (int i = 0; i < (int)curves.size(); i++)
		{
			if (curves[i]->m_rCheckBox.PtInRect(pt))
				return true;
		}
	}
	else
	{
		for (std::map<int, HistGroup::Type>::const_iterator it = m_hgs[0].types.begin(); it != m_hgs[0].types.end(); it++)
		{
			if (it->second.rectLegend.PtInRect(pt))
				return true;
		}
	}

	return false;
}

Curve2d* Plot2d::hitTestCurve(CPoint pt) const
{
	CRect rectPlot;
	rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		Curve2d* pCurve = m_curves[i];
		if (pCurve->hitTest(pt, rectPlot, m_frectPlotArea.pt, m_frectPlotArea.topRight()))
			return pCurve;
	}
	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		Curve2d* pCurve = m_curves2[i];
		if (pCurve->hitTest(pt, rectPlot, m_frectPlotArea2.pt, m_frectPlotArea2.topRight()))
			return pCurve;
	}

	return nullptr;
}

void Plot2d::clearSelCurve()
{
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		Curve2d* pCurve = m_curves[i];
		pCurve->m_bSel = false;
	}
	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		Curve2d* pCurve = m_curves2[i];
		pCurve->m_bSel = false;
	}
}

int Plot2d::getMaxCurveId() const
{
	int nId = 0;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		Curve2d* pCurve = m_curves[i];
		if (pCurve->m_id > nId)
			nId = pCurve->m_id;
	}
	for (int i = 0; i < (int)m_curves2.size(); i++)
	{
		Curve2d* pCurve = m_curves2[i];
		if (pCurve->m_id > nId)
			nId = pCurve->m_id;
	}
	return nId;
}

CRect Plot2d::getPlotRect() const
{
	CPoint ptTopLeft = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	CPoint ptBottomRight = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);
	return CRect(ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y);
}

void Plot2d::OnCopyToClipboard()
{
	BeginWaitCursor();

	CBitmap 	bitmap;
	CClientDC	dc(this);
	CDC 		memDC;
	CRect		rect;

	memDC.CreateCompatibleDC(&dc);

	GetWindowRect(rect);

	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());

	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);
	memDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	OpenClipboard();
	EmptyClipboard();
	bitmap.SetBitmapDimension(840, 840);
	SetClipboardData(CF_BITMAP, bitmap.GetSafeHandle());
	CloseClipboard();

	memDC.SelectObject(pOldBitmap);
	bitmap.Detach();

	EndWaitCursor();

}

void Plot2d::OnCopyData()
{
	if (false)
	{
		if (CountClipboardFormats() == 0)
			return; 
		OpenClipboard();

		UINT uFormat = EnumClipboardFormats(0);
		while (uFormat)
		{
			CHAR szFormatName[80];
			if (!GetClipboardFormatName(uFormat, szFormatName, sizeof(szFormatName)))
				strcpy_s(szFormatName, "unknown");

			TRACE("%lu: %s\n", uFormat, szFormatName);

			if (uFormat == 50363)
			{
				CStdioFile f;
				f.Open("TestCsv.txt", CFile::modeReadWrite | CFile::modeCreate);
				HGLOBAL hglb = GetClipboardData(uFormat);
				LPCTSTR lpstr = (LPCTSTR)GlobalLock(hglb);

				f.WriteString(lpstr);

				GlobalUnlock(hglb);
			}
			else if (uFormat == 49898)
			{
				CStdioFile f;
				f.Open("TestCsv.xml", CFile::modeReadWrite | CFile::modeCreate);
				HGLOBAL hglb = GetClipboardData(uFormat);
				LPCTSTR lpstr = (LPCTSTR)GlobalLock(hglb);

				f.WriteString(lpstr);

				GlobalUnlock(hglb);
			}

			uFormat = EnumClipboardFormats(uFormat);
		}
		CloseClipboard();
	}

	UINT uCsv = RegisterClipboardFormatA("Csv");
	std::string strData;
	if (m_curves.size() > 0)
	{
		bool bSucArg = true;
		for (int i = 0; i < (int)m_curves.size(); i++)
		{
			if (!m_curves[i]->isSuccessiveArguments())
			{
				bSucArg = false;
				break;
			}
		}

		if (bSucArg)
		{
			std::vector<double> xs;
			for (int i = 0; i < (int)m_curves.size(); i++)
			{
				Curve2d* pc = m_curves[i];
				for (int j = 0; j < pc->getCount(); j++)
				{
					double x = pc->GetPoint(j).x;
					if (std::find(xs.begin(), xs.end(), x) == xs.end())
						xs.push_back(x);
				}
			}
			std::sort(xs.begin(), xs.end());

			std::string strHead = ::stringFormat("\"%s\";", m_xAxis.m_title.c_str());
			
			for (int nc = 0; nc < (int)m_curves.size(); nc++)
			{
				std::string s = ::stringFormat("\"%s\";", m_curves[nc]->m_name.c_str());
				strHead += s;
			}
			strHead += "\r\n";
			strData += strHead;

			for (int nx = 0; nx < (int)xs.size(); nx++)
			{
				double x = xs[nx];
				std::string s = ::stringFormat("%f;", x);
				for (int nc = 0; nc < (int)m_curves.size(); nc++)
				{
					double y = m_curves[nc]->getY(x);
					s += _isnan(y) ? ";" : ::stringFormat("%f;", y);
				}
				strData += s + "\r\n";
			}
		}
		else
		{
			std::string strHead;

			int nMaxPoints = 0;
			for (int nc = 0; nc < (int)m_curves.size(); nc++)
			{
				std::string s = ::stringFormat("\"%s\";\"%s\";", m_xAxis.m_title.c_str(), m_curves[nc]->m_name.c_str());
				strHead += s;

				if (m_curves[nc]->getCount() > nMaxPoints)
					nMaxPoints = m_curves[nc]->getCount();
			}
			strHead += "\r\n";
			strData += strHead;
			for (int np = 0; np < nMaxPoints; np++)
			{
				std::string row;
				for (int nc = 0; nc < (int)m_curves.size(); nc++)
				{
					Curve2d* pc = m_curves[nc];
					if (np < pc->getCount())
					{
						Point2d pt = pc->GetPoint(np);
						row += ::stringFormat("%f;%f;", pt.x, pt.y);
					}
					else
					{
						row += ";;";
					}
				}
				strData += row + "\r\n";
			}

		}
	}
	else if (m_hgs.size() > 0)
	{
		int count = m_hgs.size();
		HistGroup& hg = m_hgs[0];

		std::string head = ";";
		for (int i = 0; i < (int)hg.types.size(); i++)
		{
			head += ::stringFormat("\"%s\";", hg.types[i].name.c_str());
		}

		strData += head + "\r\n";

		for (int nr = 0; nr < (int)hg.groups.size(); nr++)
		{
			HistGroup::Group& g = hg.groups[nr];
			std::string row = ::stringFormat("\"%s\";", g.name.c_str());
			for (int nc = 0; nc < (int)hg.types.size(); nc++)
			{
				double val = g.getVal(hg.types[nc].key);
				if (!_isnan(val))
					row += ::stringFormat("%f;", val);
				else
					row += ";";
			}
			strData += row + "\r\n";
		}
	}


	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (strData.length() + 1) * sizeof(CHAR));
	if (hglbCopy == NULL)
	{
		return;
	}

	// Lock the handle and copy the text to the buffer. 

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, strData.c_str(),	strData.length() * sizeof(CHAR));
	lptstrCopy[strData.length()] = (CHAR)0;    // null character 
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard. 
	OpenClipboard();
	SetClipboardData(uCsv, hglbCopy);
	CloseClipboard();
}

void Plot2d::OnPaint()
{
	CPaintDC dcPaint(this); // device context for painting

	CRect rectClient;
	GetClientRect(rectClient);

	if (m_bFirstPainting)
	{
		computeScale(&dcPaint, rectClient);
	}

	if (rectClient.Width() != m_sizeBmp.cx || rectClient.Height() != m_sizeBmp.cy)
	{
		m_sizeBmp = rectClient.Size();

		if (m_bmpAll.m_hObject != NULL)
			m_bmpAll.DeleteObject();
		BOOL bOk = m_bmpAll.CreateCompatibleBitmap(&dcPaint, m_sizeBmp.cx, m_sizeBmp.cy);

		if (m_bmpTip.m_hObject != NULL)
			m_bmpTip.DeleteObject();

		bOk = m_bmpTip.CreateCompatibleBitmap(&dcPaint, m_sizeBmp.cx, m_sizeBmp.cy);
	}

	{
		CDC dc;
		dc.CreateCompatibleDC(&dcPaint);
		CBitmap* pOldAllBmp = dc.SelectObject(&m_bmpAll);

		CBrush* pOldBrush = (CBrush*)dc.SelectObject(&m_brushBack);
		CPen* pOldPen = (CPen*)dc.SelectObject(&m_penGrid);

		drawBackground(&dc);
		if (m_eMode == emDecart)
		{
			m_plotTitle.draw(dc, this);

			drawAreas(&dc);

			drawAxis(&dc);

			if (m_hgs.size() == 0)
			{
				drawGrid(&dc);
				drawCurves(&dc, 0);
				drawEvents(&dc);
				drawMarkers(&dc, TRUE);
			}
			else
			{
				for (int i = 0; i < (int)m_hgs.size(); i++)
				{
					m_hgs[i].draw(&dc, this, m_hgs, i);
				}
			}

			drawInscriptions(&dc);
			if (m_curves.size() > 0 || m_hgs.size() != 0)
				drawLegend(&dc, TRUE);
			if (m_curves2.size() > 0)
				drawLegend(&dc, FALSE);

			if (!m_table.isEmpty())
				m_table.draw(&dc, this);

			CFont* pOldFont = (CFont*)dc.SelectObject(&m_xAxis.m_font);
			dc.SetBkColor(m_clrBack);
			drawCurves(&dc, 1);
			dc.SelectObject(pOldFont);
		}
		else if (m_eMode == emPolar)
		{
			drawPolarGrid(dc);
			drawCurves(&dc, 1);
			drawPolarGridLabels(dc);
			drawPolarLegend(dc);
		}

		dc.SelectObject(pOldBrush);
		dc.SelectObject(pOldPen);

		dcPaint.BitBlt(0, 0, m_sizeBmp.cx, m_sizeBmp.cy, &dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldAllBmp);
	}

	if (!m_bTipPainted)
	{
		CDC dc;
		dc.CreateCompatibleDC(&dcPaint);

		CBitmap* pOldBmp = dc.SelectObject(&m_bmpTip);

		dc.SelectObject(&m_brushBack);
		drawTip(&dc);

		dc.SelectObject(pOldBmp);

		COLORREF clrTransparent = RGB(220, 230, 240); //m_clrBack; //RGB (255, 255, 255);
		::TransparentBlt(dcPaint.m_hDC, 0, 0, m_sizeBmp.cx, m_sizeBmp.cy, dc.m_hDC, 0, 0, m_sizeBmp.cx, m_sizeBmp.cy, clrTransparent);
	}

}

BOOL Plot2d::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bCursorInPlot)
		SetCursor(::LoadCursor(NULL, IDC_CROSS)); //IDC_CROSSCUR1));	
	else
		SetCursor(::LoadCursor(NULL, IDC_ARROW));

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void Plot2d::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bLegend)
	{
		for (int i = 0; i < (int)m_curves.size(); i++)
		{
			Curve2d* pCurve = m_curves[i];
			if (pCurve->m_rCheckBox.PtInRect(point))
			{
				pCurve->m_bDrawCurve = !pCurve->m_bDrawCurve;
				if (m_yAxis.m_auto)
					autoScale(m_xAxis.m_auto, m_yAxis.m_auto, m_yAxis2.m_auto);
				Invalidate(TRUE);
				return;
			}
		}
	}

	if (m_bLegend2)
	{
		for (int i = 0; i < (int)m_curves2.size(); i++)
		{
			Curve2d* pCurve = m_curves2[i];
			if (pCurve->m_rCheckBox.PtInRect(point))
			{
				pCurve->m_bDrawCurve = !pCurve->m_bDrawCurve;
				Invalidate(TRUE);
				return;
			}
		}
	}

	CPoint pt[4];
	pt[0] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.pt.y, false);
	pt[1] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	pt[2] = scale(m_frectPlotArea.right(), m_frectPlotArea.top(), false);
	pt[3] = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	CRect rectPlot(pt[0].x, pt[1].y, pt[2].x, pt[3].y);
	if (!rectPlot.PtInRect(point))
		return;

	Curve2d* pSelCurve = hitTestCurve(point);

	if (pSelCurve != nullptr)
	{
		bool bSelBefore = pSelCurve->m_bSel;
		if (!(nFlags & MK_CONTROL))
			clearSelCurve();

		pSelCurve->m_bSel = !bSelBefore;

		Invalidate();
	}
	else
	{
		if (!(nFlags & MK_CONTROL))
			clearSelCurve();

		Invalidate();
	}

	int x = point.x;
	int y = point.y;

	m_ptLastHit = reverseScale(x, y, false);

	CWnd* pParent = GetParent();
	if (pParent != NULL)
	{
		int nControlID = GetDlgCtrlID();
		NMHDR nm;
		memset(&nm, 0, sizeof(NMHDR));
		nm.hwndFrom = m_hWnd;
		nm.idFrom = nControlID;
		nm.code = IVSPLOT_HITPOINT;
		pParent->SendMessage(WM_NOTIFY, nControlID, (LPARAM)&nm);
	}

	SetFocus();
}

void Plot2d::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint pt[4];
	pt[0] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.pt.y, false);
	pt[1] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	pt[2] = scale(m_frectPlotArea.right(), m_frectPlotArea.top(), false);
	pt[3] = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

	CRect rectPlot(pt[0].x, pt[1].y, pt[2].x, pt[3].y);
	if (!rectPlot.PtInRect(point))
		return;

	Curve2d* pSelCurve = hitTestCurve(point);
	if (pSelCurve != nullptr)
	{
		if (!pSelCurve->m_bSel)
		{
			pSelCurve->m_bSel = true;
			Invalidate();
		}
	}
}

void Plot2d::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (hitTestLegend(TRUE, point))
	{
		m_eLegendAlign = (CLegendAlign)((m_eLegendAlign + 1) % elaMax);
		//if (m_eLegendAlign == m_eLegendAlign2)
		//	m_eLegendAlign = (CLegendAlign)((m_eLegendAlign + 1) % elaMax);

		Invalidate(TRUE);
		return;
	}

	if (hitTestLegend(FALSE, point))
	{
		m_eLegendAlign2 = (CLegendAlign)((m_eLegendAlign2 + 1) % elaMax);
		if (m_eLegendAlign2 == m_eLegendAlign)
			m_eLegendAlign2 = (CLegendAlign)((m_eLegendAlign2 + 1) % elaMax);

		Invalidate(TRUE);
		return;
	}

	if (m_nPopupMenuResourceID <= 0)
		return;

	if (m_nPopupMenuIndex < 0)
		return;

	int nPopupMenuIndex = m_nPopupMenuIndex;

	if (m_nPopupMenuCurveIndex >= 0)
	{
		CPoint pt[4];
		pt[0] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.pt.y, false);
		pt[1] = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
		pt[2] = scale(m_frectPlotArea.right(), m_frectPlotArea.top(), false);
		pt[3] = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);

		CRect rectPlot(pt[0].x, pt[1].y, pt[2].x, pt[3].y);
		if (rectPlot.PtInRect(point))
		{
			m_pSelCurveToCopy = hitTestCurve(point);
			if (m_pSelCurveToCopy != nullptr && m_pSelCurveToCopy->m_bSel)
			{
				nPopupMenuIndex = m_nPopupMenuCurveIndex;
			}
		}
	}


	CWnd* pParent = AfxGetMainWnd();
	CMenu wndMenu;
	if (!wndMenu.LoadMenu(m_nPopupMenuResourceID))
		return;

	// Get the File popup menu from the top level menu.
	CMenu* pMenu = wndMenu.GetSubMenu(nPopupMenuIndex);


	// Convert the mouse location to screen coordinates before passing
	// it to the TrackPopupMenu() function.
	ClientToScreen(&point);

	// Display the File popup menu as a floating popup menu in the
	// client area of the main application window.
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,
		point.x,
		point.y,
		m_pPopupOwner);    // owner is the main application window

}

BOOL Plot2d::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int nTimes = -zDelta / WHEEL_DELTA;
	ScreenToClient(&pt);

	Point2d ptf = reverseScale(pt.x, pt.y, false);

	if (m_eZoomMode != ezmOnlyY)
	{
		if (m_xAxis.m_format == Axis2d::eaDateHours)
		{
			CTime t0((long)m_frectPlotArea.pt.x);
			CTime t1((long)m_frectPlotArea.right());

			t0 = CTime(t0.GetYear(), t0.GetMonth(), t0.GetDay(), t0.GetHour(), t0.GetMinute() / 30 * 30, 0);
			t1 = CTime(t1.GetYear(), t1.GetMonth(), t1.GetDay(), t1.GetHour(), t1.GetMinute() / 30 * 30, 0);

			double dx0 = ptf.x - m_frectPlotArea.pt.x;
			double dx1 = m_frectPlotArea.right() - ptf.x;


			if (dx0 < dx1)
			{
				if (nTimes > 0)
					t0 -= CTimeSpan(0, 0, 30, 0);
				else
					t0 += CTimeSpan(0, 0, 30, 0);
			}
			else
			{
				if (nTimes > 0)
					t1 += CTimeSpan(0, 0, 30, 0);
				else
					t1 -= CTimeSpan(0, 0, 30, 0);
			}

			if (t1 <= t0)
				return TRUE;

			double x0 = (double)t0.GetTime();
			double x1 = (double)t1.GetTime();

			if (x0 <= m_frectInitArea.pt.x)
				x0 = m_frectInitArea.pt.x;

			if (x1 >= m_frectInitArea.right())
				x1 = m_frectInitArea.right();


			setAxis(true, x0, x1, m_xAxis.m_divs, false);
			setAxis(true, x0, x1, m_xAxis.m_divs, true);
		}
		else
		{
			double x0 = m_frectPlotArea.pt.x;
			double x1 = m_frectPlotArea.right();

			double dx0 = ptf.x - x0;
			double dx1 = x1 - ptf.x;

			if (nTimes > 0)
			{
				dx0 *= 0.9;
				dx1 *= 0.9;

			}
			else
			{
				dx0 /= 0.9;
				dx1 /= 0.9;

			}

			x0 = ptf.x - dx0;
			x1 = ptf.x + dx1;

			if (x0 <= m_frectInitArea.pt.x)
				x0 = m_frectInitArea.pt.x;

			if (x1 >= m_frectInitArea.right())
				x1 = m_frectInitArea.right();


			setAxis(true, x0, x1, m_xAxis.m_divs, false);
			setAxis(true, x0, x1, m_xAxis.m_divs, true);
		}
	}

	if (m_eZoomMode != ezmOnlyX)
	{
		double y0 = m_frectPlotArea.pt.y;
		double y1 = m_frectPlotArea.top();

		double dy0 = ptf.y - y0;
		double dy1 = y1 - ptf.y;

		if (nTimes > 0)
		{
			dy0 *= 0.9;
			dy1 *= 0.9;

		}
		else
		{
			dy0 /= 0.9;
			dy1 /= 0.9;

		}

		y0 = ptf.y - dy0;
		y1 = ptf.y + dy1;

		if (y0 <= m_frectInitArea.pt.y)
			y0 = m_frectInitArea.pt.y;

		if (y1 >= m_frectInitArea.top())
			y1 = m_frectInitArea.top();


		setAxis(false, y0, y1, m_yAxis.m_divs, false);
		setAxis(false, y0, y1, m_yAxis.m_divs, true);
	}

	Invalidate(TRUE);
	UpdateWindow();

	return TRUE;
}

int Plot2d::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rectPlot;
	rectPlot.TopLeft() = scale(m_frectPlotArea.pt.x, m_frectPlotArea.top(), false);
	rectPlot.BottomRight() = scale(m_frectPlotArea.right(), m_frectPlotArea.pt.y, false);
	if (!rectPlot.PtInRect(point))
		return -1;

	{
		pTI->hwnd = m_hWnd;
		pTI->uId = (UINT)m_hWnd;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = CRect(0, 0, 30, 30);
		pTI->uFlags |= TTF_IDISHWND | TTF_ALWAYSTIP | TTF_TRANSPARENT | TTF_NOTBUTTON | TTF_TRACK;
		return pTI->uId;
	}

	return -1;
}

BOOL Plot2d::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	CString strTipText;

	const MSG* pMessage;
	CPoint pt;
	pMessage = GetCurrentMessage(); // get mouse pos 
	ASSERT(pMessage);
	pt = pMessage != nullptr ? pMessage->pt : CPoint(0, 0);
	ScreenToClient(&pt);

	if (m_xAxis.m_format == Axis2d::eaTime)
	{
		Point2d ptf = reverseScale(pt.x, pt.y, false);
		CTime t((long)ptf.x);
		strTipText.Format("x=%s y=%f", (LPCTSTR)t.Format("%H:%M:%S"), ptf.y);

	}
	else if (m_xAxis.m_format == Axis2d::eaDateTime || m_xAxis.m_format == Axis2d::eaDateHours)
	{
		Point2d ptf = reverseScale(pt.x, pt.y, false);
		CTime t((long)ptf.x);
		strTipText.Format("x=%s y=%f", (LPCTSTR)t.Format("%d.%m.%y %H:%M:%S"), ptf.y);
	}
	else if (m_xAxis.m_format == Axis2d::eaDateTimeGPS)
	{
		Point2d ptf = reverseScale(pt.x, pt.y, false);
		CTime t((long)ptf.x);
		double ms = (ptf.x - floor(ptf.x)) * 1000;
		if (m_xAxis.m_precision != 0)
		{
			ms = floor(ms / m_xAxis.m_precision + 0.5) * m_xAxis.m_precision;
			if (ms >= 1000)
			{
				ms = 0;
				t += CTimeSpan(0, 0, 0, 1);
			}
		}
		strTipText.Format("x=%s %.0f y=%f", (LPCTSTR)t.FormatGmt("%d.%m.%y %H:%M:%S"), ms, ptf.y);

	}
	else
	{
		Point2d ptf = reverseScale(pt.x, pt.y, FALSE);
		strTipText.Format("x=%.3f y=%.3f", ptf.x, ptf.y);
	}

	::SendMessage(pNMHDR->hwndFrom, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);
	::SendMessage(pNMHDR->hwndFrom, TTM_SETDELAYTIME, TTDT_INITIAL, 1);
	::SendMessage(pNMHDR->hwndFrom, TTM_SETDELAYTIME, TTDT_RESHOW, 1);

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, 80);
	else
		_mbstowcsz(pTTTW->szText, strTipText, 80);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, 80);
	else
		lstrcpyn(pTTTW->szText, strTipText, 80);
#endif

	* pResult = TRUE;
	return TRUE;
}

Plot2d::PlotRange Plot2d::getPlotRange() const
{
	PlotRange pr;
	pr.xMin = m_xAxis.m_min;
	pr.xMax = m_xAxis.m_max;
	pr.xDivs = m_xAxis.m_divs;
	pr.yMin = m_yAxis.m_min;
	pr.yMax = m_yAxis.m_max;
	pr.yDivs = m_yAxis.m_divs;
	for (int i = 0; i < (int)m_curves.size(); i++)
		pr.drawCurves.push_back(m_curves[i]->m_bDrawCurve);

	return pr;
}

void Plot2d::setPlotRange(const PlotRange& pr)
{
	setAxis(true, pr.xMin, pr.xMax, pr.xDivs);
	setAxis(true, pr.yMin, pr.yMax, pr.yDivs);
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		if (i < (int)pr.drawCurves.size())
			m_curves[i]->m_bDrawCurve = pr.drawCurves[i];
	}
}

void Plot2d::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLegend)
	{
		for (int i = 0; i < (int)m_curves.size(); i++)
		{
			if (m_curves[i]->m_rCheckBox.PtInRect(point))
			{
				m_bCursorInPlot = FALSE;
				return;
			}
		}
	}

	if (m_bLegend2)
	{
		for (int i = 0; i < (int)m_curves2.size(); i++)
		{
			if (m_curves2[i]->m_rCheckBox.PtInRect(point))
			{
				m_bCursorInPlot = FALSE;
				return;
			}
		}
	}

	CRect rectPlot(m_xAxis.m_r.left, m_yAxis.m_r.top, m_xAxis.m_r.right, m_yAxis.m_r.bottom);
	m_bCursorInPlot = rectPlot.PtInRect(point);

	if (m_ptLastTip != point)
	{
		m_ptLastTip = point;
		m_bTipPainted = FALSE;
		paintTip();
	}

	CWnd::OnMouseMove(nFlags, point);
}

BOOL Plot2d::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_PLOT_COPY_TO_CLIPBOARD)
	{
		OnCopyToClipboard();
		return CWnd::OnCommand(wParam, lParam);
	}
	else if (wParam == ID_PLOT_COPY_DATA)
	{
		OnCopyData();
		return CWnd::OnCommand(wParam, lParam);
	}

	CWnd * pParent = GetParent();
	if (pParent != nullptr)
	{
		return pParent->SendMessage(WM_COMMAND, wParam, lParam);
	}
	return FALSE;
}
// End of Plot2d implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////