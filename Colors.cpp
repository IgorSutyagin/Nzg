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

#include "Colors.h"

namespace nzg
{

	constexpr auto HLSMAX = 240;
	constexpr auto RGBMAX = 255;
#define HUE_UNDEFINED (HLSMAX*2/3)

	void  rgb2hls(DWORD lRGBColor, short& H, unsigned short& L, unsigned short& S)
	{
		unsigned short R, G, B;
		BYTE cMax, cMin;
		unsigned short  Rdelta, Gdelta, Bdelta;

		R = GetRValue(lRGBColor);
		G = GetGValue(lRGBColor);
		B = GetBValue(lRGBColor);

		cMax = (BYTE)std::max(std::max(R, G), B);
		cMin = (BYTE)std::min(std::min(R, G), B);
		L = (((cMax + cMin) * HLSMAX) + RGBMAX) / (2 * RGBMAX);

		if (cMax == cMin)
		{
			S = 0;
			H = HUE_UNDEFINED;
		}
		else
		{
			if (L <= (HLSMAX / 2))
				S = (((cMax - cMin) * HLSMAX) + ((cMax + cMin) / 2)) / (cMax + cMin);
			else
				S = (((cMax - cMin) * HLSMAX) + ((2 * RGBMAX - cMax - cMin) / 2))
				/ (2 * RGBMAX - cMax - cMin);

			Rdelta = (((cMax - R) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
			Gdelta = (((cMax - G) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
			Bdelta = (((cMax - B) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);

			if (R == cMax)
				H = Bdelta - Gdelta;
			else if (G == cMax)
				H = (HLSMAX / 3) + Rdelta - Bdelta;
			else
				H = ((2 * HLSMAX) / 3) + Gdelta - Rdelta;

			if (H < 0)
				H += HLSMAX;
			if (H > HLSMAX)
				H -= HLSMAX;
		}
	}


	ColorRGB::operator ColorHLS() const
	{
		short h;
		unsigned short l, s;
		rgb2hls(rgb, h, l, s);
		return ColorHLS((byte)h, (byte)l, (byte)s);
	}

	static unsigned short hue2rgb(unsigned short n1, unsigned short n2, unsigned short hue)
	{
		if (hue < 0)
			hue += HLSMAX;

		if (hue > HLSMAX)
			hue -= HLSMAX;

		if (hue < (HLSMAX / 6))
			return (n1 + (((n2 - n1) * hue + (HLSMAX / 12)) / (HLSMAX / 6)));
		if (hue < (HLSMAX / 2))
			return (n2);
		if (hue < ((HLSMAX * 2) / 3))
			return (n1 + (((n2 - n1) * (((HLSMAX * 2) / 3) - hue) + (HLSMAX / 12)) / (HLSMAX / 6)));
		else
			return (n1);
	}


	DWORD hls2rgb(unsigned short hue, unsigned short lum, unsigned short sat)
	{
		unsigned short R, G, B;
		unsigned short  Magic1, Magic2;

		if (sat == 0)
		{
			R = G = B = (lum * RGBMAX) / HLSMAX;
			if (hue != HUE_UNDEFINED)
			{

			}
		}
		else
		{
			if (lum <= (HLSMAX / 2))
				Magic2 = (lum * (HLSMAX + sat) + (HLSMAX / 2)) / HLSMAX;
			else
				Magic2 = lum + sat - ((lum * sat) + (HLSMAX / 2)) / HLSMAX;
			Magic1 = 2 * lum - Magic2;

			R = (hue2rgb(Magic1, Magic2, hue + (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
			G = (hue2rgb(Magic1, Magic2, hue) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
			B = (hue2rgb(Magic1, Magic2, hue - (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX;
		}
		return(RGB(R, G, B));
	}

	ColorHLS::operator ColorRGB() const
	{
		return ColorRGB(hls2rgb(h, l, s));
	}

	DWORD rainbowColor(double dVal, double dMin, double dMax, BYTE cAlpha, BYTE lum, BYTE sat, double step)
	{
		double val = isnan(step) || step <= 0 ? dVal : (floor(dVal / step) + 0.5) * step;
		double v = (val - dMin) / (dMax - dMin);

		if (v < 0)
			v = 0;
		else if (v > 1)
			v = 1;

		DWORD dwColor = ColorHLS((byte)(160.0 * (1 - v)), lum, sat);
		BYTE r = GetRValue(dwColor);
		BYTE g = GetGValue(dwColor);
		BYTE b = GetBValue(dwColor);
		DWORD dwAlpha = cAlpha;
		return (dwAlpha << 24) | (0x00FFFFFF & dwColor);
	}

	COLORREF colorrefRainbowColor(double dVal, double dMin, double dMax)
	{
		double v = (dVal - dMin) / (dMax - dMin);

		if (v < 0)
			v = 0;
		else if (v > 1)
			v = 1;

		DWORD dwColor = ColorHLS((byte)(160.0 * (1 - v)), 120, 240);
		return dwColor;
	}

}