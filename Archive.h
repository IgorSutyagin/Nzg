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

#include <ios>
#include <ostream>
#include <istream>
#include <sstream>

namespace nzg
{

	class Archive;

	class IvsMemFile
	{
		// Construction:
	public:
		IvsMemFile() {}
		IvsMemFile(std::string& s) : m_ss(s) {}
		~IvsMemFile() {}

		// Attributes:
	public:
		std::stringstream m_ss;

		// Operations:
	public:
		std::streamsize read(void* pData, std::streamsize len) {
			char* p = (char *)pData;
			std::streamsize pos0 = m_ss.gcount();
			m_ss.read((char*)pData, len);
			return m_ss.gcount() - pos0;
		}
		void write(const void* pData, size_t len) {
			m_ss.write((const char*)pData, len);
		}
		void seek(long lPos) {
			seekg(lPos);
			seekp(lPos);
		}
		void seekg(long lPos) {
			m_ss.seekg(lPos);
		}
		void seekp(long lPos) {
			m_ss.seekp(lPos);
		}

		// Serialization:
	public:
		void serialize(Archive& ar);
	};

	class Archive
	{
		// Construction:
	public:
		enum Mode
		{
			load = 0,
			store = 1
		};
		Archive(std::basic_ios<char>& f, Mode em) : m_f(f), m_em(em) {}
		Archive(IvsMemFile& fMem, Mode em) : m_f(fMem.m_ss), m_em(em) {}
		virtual ~Archive() {}

		typedef std::basic_ostream<char> Out;
		typedef std::basic_istream<char> In;

		// Attributes:
	public:
		Mode m_em;
		std::basic_ios<char>& m_f;

		bool isLoading() const { return m_em == load; }
		bool isStoring() const { return m_em == store; }

		size_t read(void* p, size_t size) {
			size_t r = (size_t)dynamic_cast<In&>(m_f).read(reinterpret_cast<char*>(p), size).gcount();
			if (!dynamic_cast<In&>(m_f) || r != size)
			{
				throw std::runtime_error("EOF reading archive");
			}
			return r;
		}
		void write(const void* p, size_t size) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<const char*>(p), size);
		}

		Archive& operator<<(long n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(long& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(int n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(int& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(unsigned int n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(unsigned int& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(unsigned long n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(unsigned long& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(short n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(short& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(unsigned short n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(unsigned short& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(char n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(char& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(bool n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(char));
			return *this;
		}
		Archive& operator>>(bool& n) {
			read(reinterpret_cast<char*>(&n), sizeof(char));
			return *this;
		}

		Archive& operator<<(unsigned char n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(unsigned char& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(double n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(double& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		Archive& operator<<(float n) {
			dynamic_cast<Out&>(m_f).write(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}
		Archive& operator>>(float& n) {
			read(reinterpret_cast<char*>(&n), sizeof(n));
			return *this;
		}

		static size_t getStringArchLen(const std::string& str) {
			size_t nLen = str.length();
			int nSize = 0;
			if (nLen < 255)
			{
				nSize = sizeof(char);
			}
			else if (nLen < 0xFFFE)
			{
				nSize = sizeof(char) + sizeof(short);
			}
			else
			{
				nSize = sizeof(char) + sizeof(short) + sizeof(int);
			}

			nSize += nLen * sizeof(char);

			return nSize;
		}
		Archive& operator<<(const std::string& str) {
			size_t size = str.length();
			if (size < 0xFF)
				(*this) << (unsigned char)size;
			else
			{
				(*this) << (unsigned char)0xFF;
				if (size < 0xFFFE)
				{
					(*this) << (unsigned short)size;
				}
				else
				{
					(*this) << (unsigned short)0xFFFE;
					(*this) << (unsigned int)size;
				}
			}

			dynamic_cast<Out&>(m_f).write(str.c_str(), (unsigned int)size);
			return *this;
		}
		Archive& operator>>(std::string& str) {
			size_t size = 0;
			unsigned char cl = 0;
			(*this) >> cl;
			if (cl != 0xFF)
				size = cl;
			else
			{
				unsigned short nl;
				(*this) >> nl;
				if (nl < 0xFFFE)
					size = nl;
				else
				{
					unsigned int n;
					(*this) >> n;
					size = n;
				}
			}
			str.resize(size);
			if (size > 0)
				read(&str[0], size);
			return *this;
		}

		template <typename T>
		Archive& operator<<(const std::vector<T>& v) {
			size_t s = v.size();
			write(&s, sizeof(s));
			for (size_t i = 0; i < s; i++)
				write(&v[i], sizeof(T));
			return *this;
		}

		template <typename T>
		Archive& operator>>(std::vector<T>& v) {
			size_t s;
			read(&s, sizeof(s));
			v.resize(s);
			for (size_t i = 0; i < s; i++)
				read(&v[i], sizeof(T));
			return *this;
		}
	};

}