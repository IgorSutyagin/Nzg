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

#include "Points.h"
#include "Archive.h"

namespace nzg
{
	// Node - Abstract base class for calibrations and other data
	class Node
	{
	public:
		Node() : m_ent(entUnk), m_parent(nullptr) {
		}
		Node(const Node& a) {
			*this = a;
		}
		virtual ~Node() {
		}

		Node& operator=(const Node& a)	{
			m_ent = a.m_ent;
			m_parent = a.m_parent;
			return *this;
		}

		enum NodeType
		{
			entUnk = 0,
			entNzg = 1
		};

		// Attributes:
	public:
		NodeType m_ent;
		Node* m_parent;

		bool isNzg() const { return m_ent == entNzg; }

		// Overrides:
	public:
		virtual std::string getName() const { return std::string(); }
		virtual Node* subtract(const Node* pMinus) const { return nullptr; }
		virtual int childs() const { return 0; }
		virtual Node* getChild(int index) const { assert(false); return nullptr; }

		virtual void serialize(Archive& ar) {
			if (ar.isStoring())
			{
				ar.write(&m_ent, sizeof(m_ent));
			}
			else
			{
				ar.read(&m_ent, sizeof(m_ent));
			}
		}
	};
}