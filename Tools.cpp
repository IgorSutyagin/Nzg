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

namespace nzg
{
	const char* whiteSpace = " \t\n\r\f\v";


	CSize calcDialogSize(UINT nResourceId, HINSTANCE hInstance)
	{
		CSize size;
		HRSRC hRsrc = ::FindResource(hInstance, MAKEINTRESOURCE(nResourceId), RT_DIALOG);
		ASSERT(hRsrc != NULL);
		if (hRsrc == NULL)
			return CSize(0, 0);

		HGLOBAL hTemplate = ::LoadResource(hInstance, hRsrc);
		ASSERT(hTemplate != NULL);
		if (hTemplate == nullptr)
			return CSize(0, 0);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)::LockResource(hTemplate);
		CDialogTemplate dlgtemplate(pTemplate);
		dlgtemplate.GetSizeInPixels(&size);
		::UnlockResource(hTemplate);

		size.cy += GetSystemMetrics(SM_CYCAPTION);

		return size;
	}

	int CTimeEx::getYearGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? (ptm->tm_year) + 1900 : 0;
	}

	int CTimeEx::getMonthGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_mon + 1 : 0;
	}

	int CTimeEx::getDayGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_mday : 0;
	}

	int CTimeEx::getHourGmt() const
	{
		struct tm ttm;
		struct tm* ptm;

		ptm = GetGmtTm(&ttm);
		return ptm ? ptm->tm_hour : -1;
	}

	CTimeEx CTimeEx::fromAntexString(const char* sz)
	{
		// 16-Apr-24

		static char* seps = " \t\r\n";
		static char* szms[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		const char* pDay = sz + strspn(sz, seps);
		const char* pMonth = pDay + strcspn(pDay, "-") + 1;
		const char* pYear = pMonth + strcspn(pMonth, "-") + 1;
		CString strDay(pDay, 2);
		CString strMonth(pMonth, 3);
		CString strYear(pYear, 2);

		int nDay = atoi(strDay);
		if (nDay <= 0 || 31 <= nDay)
			return CTimeEx();
		int nYear = atoi(strYear) + 2000;
		if (nYear < 2000)
			return CTimeEx();
		int nMonth = 0;
		for (int i = 0; i < 12; i++)
		{
			if (strYear.CompareNoCase(szms[i]) == 0)
			{
				nMonth = i + 1;
				break;
			}
		}

		if (nMonth <= 0)
			return CTimeEx();

		CTime t(nYear, nMonth, nDay, 0, 0, 0);
		return t;
	}

	std::string CTimeEx::getAntexString() const
	{
		static char* szms[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		return stringFormat("%02i-%s-%02i", GetDay(), szms[GetMonth() - 1], GetYear() - 2000);
	}

	CTimeEx& CTimeEx::operator=(const COleDateTime& t)
	{
		*this = CTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
		return *this;
	}

	namespace StringUtils
	{
		std::string asString(DumpDetail e) noexcept
		{
			switch (e)
			{
			case DumpDetail::Unknown: return "Unknown";
			case DumpDetail::OneLine: return "OneLine";
			case DumpDetail::Brief:   return "Brief";
			case DumpDetail::Terse:   return "Terse";
			case DumpDetail::Full:    return "Full";
			default:                  return "???";
			} // switch (e)
		} // asString(DumpDetail)


		DumpDetail asDumpDetail(const std::string& s) noexcept
		{
			if (s == "Unknown")
				return DumpDetail::Unknown;
			if (s == "OneLine")
				return DumpDetail::OneLine;
			if (s == "Brief")
				return DumpDetail::Brief;
			if (s == "Terse")
				return DumpDetail::Terse;
			if (s == "Full")
				return DumpDetail::Full;
			return DumpDetail::Unknown;
		} // asDumpDetail(string)
	} // namespace StringUtils

}