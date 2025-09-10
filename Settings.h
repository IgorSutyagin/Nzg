////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  The CAN2 software is distributed under the following BSD 2-clause license and 
//  additional exclusive clauses. Users are permitted to develop, produce or sell their 
//  own non-commercial or commercial products utilizing, linking or including CAN2 as 
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

#include "Plot3d.h"
#include "Archive.h"

namespace nzg
{

	////////////////////////////////////////////////////////////////////////////
	// Collection of Can2 application settings
	class Settings
	{
	// Construction:
	public:
		Settings();
		~Settings();

	// Attributes:
	public:
		enum Plot3dType
		{
			ep3Pcv = 0,
			ep3DeltaPcv = 1,
			ep3Rms = 2,
			ep3Max
		};
		struct Plot3dSettings
		{
			std::shared_ptr<Plot3dCamera> camera[Plot3dCamera::ecMax];
			std::shared_ptr<Plot3dParams> params;
		};
		Plot3dSettings plot3d[ep3Max];

		enum Plot2dType
		{
			ep2PcoEast = 0,
			ep2PcoNorth = 1,
			ep2PcoUp = 2,
			ep2PcoHor = 3,
			ep2Max
		};

		struct Plot2dSettings
		{
			std::string title;
			void serialize(Archive& ar);
		};
		Plot2dSettings plot2d[ep2Max];
		std::map<std::string, COLORREF> plot2dColors;

		void savePlot3d(const Node* node, const Plot3d * plot);
		void loadPlot3d(const Node* node, Plot3d* plot) const;

	// Serialization:
	public:
		void load(const char * pathName);
		void save();
		void serialize(Archive& ar);

	// Implementation:
	protected:
		std::string pathName;

	};

	extern Settings gl_settings;
	// End of Settings interface
	/////////////////////////////////////////////////////////////////////////////
}