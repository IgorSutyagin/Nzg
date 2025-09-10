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

namespace nzg
{
	struct ColorHLS;

	struct ColorRGB
	{
		ColorRGB() : rgb(0) {}
		ColorRGB(byte r, byte g, byte b) {
			rgb = RGB(r, g, b);
		}
		ColorRGB(unsigned long rgb_) { rgb = rgb_; }
		unsigned long rgb;

		byte getA() const {
			return ((byte)((rgb) >> 24));
		}

		operator ColorHLS () const;
		operator DWORD () const {
			return rgb;
		}
	};

	struct ColorHLS
	{
		ColorHLS() : h(0), l(0), s(0) {}
		ColorHLS(byte h_, byte l_, byte s_) : h(h_), l(l_), s(s_) {
		}

		byte h;
		byte l;
		byte s;

		operator ColorRGB () const;
		operator DWORD () const {
			const ColorRGB a = *this;
			return a.rgb;
		}
	};

	// Get RGB color from color range given dMin and dMax as a source range if luma and saturation are given
	DWORD rainbowColor(double dVal, double dMin, double dMax, BYTE cAlpha, BYTE lum = 120, BYTE sat = 240, double step = NAN);

	// Get a color from rainbow given dMin and dMax. Luma and saturation are 120 and 240
	COLORREF colorrefRainbowColor(double dVal, double dMin, double dMax);

}