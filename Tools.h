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

#include "Colors.h"

namespace nzg
{
	const double PI = 3.141592653589793238462643383280;
	/// Multiply degrees by DEG2RAD to get radians.
	const double DEG2RAD = PI / 180.0;
	/// Multiply radians by RAD2DEG to get degrees.
	const double RAD2DEG = 180.0 / PI;
	/// GPS value of PI*2
	const double TWO_PI = 6.283185307179586476925286766559;

	template<typename ... Args>
	std::string stringFormat(const std::string& format, Args ... args) {
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	extern const char* whiteSpace;

	// trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = whiteSpace)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = whiteSpace)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& trim(std::string& s, const char* t = whiteSpace)
	{
		return ltrim(rtrim(s, t), t);
	}

	// Get lowercase version of the string for standart ASCII characters
	inline std::string tolower(const char* source)
	{
		std::string r = source;
		for (auto & c : r)
		{
			c = ::tolower(c);
		}
		return r;
	}

	// Get Alpha (opacity) value from argb
	inline byte getAValue(DWORD dwArgb)
	{
		byte c = ((byte)((dwArgb) >> 24));
		return c;
	}

	// RGB to/from Humidity, Luma, Saturation conversion
	//void  rgb2hls(DWORD lRGBColor, unsigned short& H, unsigned short& L, unsigned short& S);
	//DWORD hls2rgb(unsigned short hue, unsigned short lum, unsigned short sat);

	CSize calcDialogSize(UINT nResourceId, HINSTANCE hInstance);

	struct ScreenParams
	{
		ScreenParams() {
			ppmm = NAN;
		}

		double ppmm; // Pixels in mm
		double ppinch; // Pixels in inch
		double pixInPoint; // Pixels in 1/72 of inch
	};

	inline CTime getDate(CTime t) {
		return CTime(t.GetYear(), t.GetMonth(), t.GetDay(), 0, 0, 0);
	}

	class CTimeEx : public CTime
	{
	// Construction:
	public:
		CTimeEx() {}
		CTimeEx(time_t t_) : CTime(t_) {}
		CTimeEx(const CTime& t_) : CTime(t_) {}
		CTimeEx(const COleDateTime& t_) : CTime(t_.GetYear(), t_.GetMonth(), t_.GetDay(), t_.GetHour(), t_.GetMinute(), t_.GetSecond()) {}

		enum TimeRegion
		{
			etrDef = 0,
			etrGmt = 1
		};

	// Operations:
	public:
		int getYearGmt() const;
		int getMonthGmt() const;
		int getDayGmt() const;
		int getHourGmt() const;

		inline operator COleDateTime() const;
		static CTimeEx fromAntexString(const char*);
		std::string getAntexString() const;
		CTimeEx& operator=(const COleDateTime& t);
	};

	inline CTimeEx::operator COleDateTime() const {
		return COleDateTime(GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond());
	}

	inline std::string getFileExt(const char* file) {
		const char* p = strchr(file, '.');
		if (p == nullptr)
			return "";

		return std::string(p);
	}

	/** This class simplifies the process of iterating over strongly
	 * typed enums.
	 * Example:
	 *   typedef EnumIterator<enum, enum::firstVal, enum::lastVal> Iterator;
	 *   for (enum x : Iterator()) { ... }
	 * Typically, the iterator will be defined in the include file
	 * that defines the enum.  The endVal value should be first
	 * value that will NOT be processed in a loop.  This means
	 * defining a final "Last" enumeration value in the enum that
	 * won't be iterated over.  This is done to facilitate adding
	 * additional enumeration values without having to change the
	 * code that defines the iterator.
	 *
	 * @warning Do not attempt to use this on enumerations that have
	 * assigned values resulting in gaps.  This will result in
	 * iterating over invalid enumeration values.
	 *
	 * @see CarrierBand.hpp
	 */
	template <typename C, C beginVal, C endVal>
	class EnumIterator
	{
		/// Value type as derived from the enum typename.
		typedef typename std::underlying_type<C>::type ValType;
		/// Current iterator value
		ValType val;
	public:
		/** Default iterator initializes to the beginVal specified in
		 * the template instantiation. */
		EnumIterator()
			: val(static_cast<ValType>(beginVal))
		{}
		/// Initialize the iterator to a specific value.
		EnumIterator(const C& f)
			: val(static_cast<ValType>(f))
		{}
		/** Increment the iterator to the next enum
		 * @note This assumes that there are no gaps between enum
		 *   values, otherwise it could not have a valid
		 *   enumeration.
		 * @note This is the prefix operator. */
		EnumIterator& operator++()
		{
			val++;
			return *this;
		}
		/// Dereference the iterator by returning the current enum value
		C operator*()
		{
			return static_cast<C>(val);
		}
		/** Iterator start value, which can be the value initialized
		 * from the value constructor. */
		EnumIterator begin()
		{
			return *this;
		}
		/// Iterator end value.
		EnumIterator end()
		{
			static const EnumIterator endIter = EnumIterator(endVal);
			return endIter;
		}
		/// Comparison to assist in iteration.
		bool operator!=(const EnumIterator& i)
		{
			return val != i.val;
		}
	};

	/// @ingroup NavFactory
	//@{

	/// Specify level of detail for dump output.
	enum class DumpDetail
	{
		Unknown, ///< Uninitialized or unknown value.
		OneLine, ///< Limit output to minimal information on a single line.
		Brief,   ///< Limit output to <= 5 lines of minimal information.
		Terse,   ///< Aptly named, multiple lines of output with no labels.
		Full,    ///< Include all detailed information.
		Last     ///< Used to create an iterator.
	};

	/** Define an iterator so C++11 can do things like
	 * for (DumpDetail i : DumpDetailIterator()) */
	typedef EnumIterator<DumpDetail, DumpDetail::Unknown, DumpDetail::Last> DumpDetailIterator;

	namespace StringUtils
	{
		/// Convert a DumpDetail to a whitespace-free string name.
		std::string asString(DumpDetail e) noexcept;
		/// Convert a string name to an DumpDetail
		DumpDetail asDumpDetail(const std::string& s) noexcept;
	}

	//@}

}


