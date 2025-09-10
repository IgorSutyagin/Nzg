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

#include "NzgException.h"
#include "Angles.h"
#include "Points.h"

namespace nzg
{
	class Vector
	{
		// Construction:
	public:
		Vector() {}
		Vector(const Vector& a) : data(a.data) {}
		Vector(Vector&& a) noexcept : data(a.data) {}
		Vector(int len) : data(len, 0) {}
		virtual ~Vector() {}
		void clear() {
			data.clear();
		}
		void resize(int n) {
			data.resize(n);
		}
		void init(double v) {
			for (size_t i = 0; i < data.size(); i++)
				data[i] = v;
		}

	// Operations:
	public:
		int len() const {
			return data.size();
		}
		int size() const {
			return data.size();
		}
		Vector& operator=(const Vector& a) {
			data = a.data;
			return *this;
		}
		Vector& operator=(Vector&& a) noexcept {
			data = a.data;
			return *this;
		}
		void getSubVector(Vector& v, int n0, int n1) const;
		double operator()(int i) const { return data[i]; }
		double& operator()(int i) {	return data[i];	}
		double operator[](int i) const { return data[i]; }
		double& operator[](int i) { return data[i]; }

	// Implementation:
	protected:
		std::vector<double> data;
	};

	class Matrix
	{
	// Construction:
	public:
		Matrix() : rs(0), cs(0) {}
		Matrix(const Matrix& a) : rs(a.rs), cs(a.cs), mat(a.mat) {}
		Matrix(Matrix&& a) noexcept : rs(a.rs), cs(a.cs), mat(a.mat) {}
		Matrix(int rows_, int cols_) : rs(rows_), cs(cols_), mat(rows_* cols_, 0) {}
		~Matrix() {}
		void clear() {
			mat.clear();
			cs = rs = 0;
		}
		void resize(int rs_, int cs_) {
			rs = rs_;
			cs = cs_;
			mat.resize(rs * cs);
		}
		void init(double v) {
			for (size_t i = 0; i < mat.size(); i++)
				mat[i] = v;
		}
		void identity() {
			for (int i = 0; i < rows(); i++)
				for (int j=0; j<cols(); j++)
					(*this)(i, j) = (i == j ? 1.0 : 0);
		}
		void transpose() {
			Matrix tmp(cols(), rows());
			for (int i = 0; i < rows(); i++)
			{
				for (int j = 0; j < cols(); j++)
				{
					tmp(j, i) = (*this)(i, j);
				}
			}
			*this = tmp;
		}

	// Operations:
	public:
		int cols() const {
			return cs;
		}
		int rows() const {
			return rs;
		}
		Matrix& operator=(const Matrix& a) {
			cs = a.cs; rs = a.rs; mat = a.mat;
			return *this;
		}
		Matrix& operator=(Matrix&& a) noexcept {
			cs = a.cs; a.cs = 0;
			rs = a.rs; a.rs = 0;
			mat = a.mat;
			return *this;
		}
		double& operator()(int row, int col) {	return mat[cs* row + col];	}
		double operator()(int row, int col) const { return mat[cs * row + col]; }
		friend Vector operator*(const Matrix& m, const Vector& v) {
			assert(m.cols() == v.len());
			Vector r(m.rows());
			for (int i = 0; i < r.len(); i++) {
				double& d = r(i);
				for (int j = 0; j < m.cols(); j++) {
					d += m(i, j) * v(j);
				}
			}
			return r;
		}
		friend Matrix operator*(const Matrix& a, const Matrix& b) {
			assert(a.cols() == b.rows());
			Matrix r(a.rows(), b.cols());
			for (int m = 0; m < r.rows(); m++) {
				for (int n = 0; n < r.cols(); n++) {
					double& d = r(m, n);
					for (int k = 0; k < a.cols(); k++) {
						d += a(m, k) * b(k, n);
					}
				}
			}
			return r;
		}

		Matrix& operator*=(const Matrix& a) {
			*this = *this * a;
			return *this;
		}

		// b = A*x
		Vector solve(const Vector& b) const;

		void inverse();
		void copyUpperTriangle();

		Point3d transPoint(const Point3d& pt) const;

		// Get submatrix for (nRow=nRow0; nRow<nRow1; nRow++) and for (nCol=nCol0; nCol<nCol1; nCol++)
		Matrix subMatrix(int nRow0, int nRow1, int nCol0, int nCol1) const;

		// Set submatrix for(nRow=nRow0; nRow<sm.getRows(); nRow++) && for(nCol=nCol0; nCol<sm.getCols(); nCol++)
		void setSubMatrix(const Matrix& sm, int nRow0, int nCol0);

	// Implementation:
	protected:
		std::vector<double> mat;
		int rs;
		int cs;

		void exchRow(int row1, int row2);
		void exchCol(int col1, int col2);
		void multRowByVal(int row, double val);
		void addRows(int rowDst, int rowSrc, double mult);
	};
	// End of Matrix implementation
	////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////
	// HomoTransform interface
	class HomoTransform
	{
		// Construction:
	public:
		HomoTransform();
		HomoTransform(const HomoTransform& a);
		~HomoTransform();
		void Reset(); // Reset to unity matrix

		// Attributes:
	public:
		// Transformation matrix
		Matrix m_t;

		// Temporary vectors
		Vector m_s;
		Vector m_d;

		// Operations:
	public:
		HomoTransform& operator=(const HomoTransform& a);
		HomoTransform& operator*=(const HomoTransform& a);
		Point3d transform(const Point3d& pt);
		Point3d transform(const Point3d& pt) const;
		Size3d transform(const Size3d& pt);
		Point3d transVector(const Point3d& ptVec) const;
		Matrix propagateCovariance(const Matrix& Q) const;
		void inverse();

		// Angle is in radians
		void rotate(Axis3d& axis, double dAngle);

		// Translate
		void translate(double x, double y, double z);

		friend HomoTransform operator*(const HomoTransform& a, const HomoTransform& b);

		// Get ENU to ECEF transformation at point ptEcefBase
		// (point in ENU transformed to ECEF)
		//static HomoTransform enu2ecef(const Point3d& ptEcefBase);

		// Get ECEF to ENU transformation at point ptEcefBase
		// (point in ECEF transformed to ENU)
		//static HomoTransform ecef2enu(const Point3d& ptEcefBase);

	private:
		// Compute BLH coords of the given point
		//static Point3d ecef2blh(const Point3d& ptEcef);

	};

	inline HomoTransform operator*(const HomoTransform& a, const HomoTransform& b)
	{
		HomoTransform c;
		c.m_t = a.m_t;
		c.m_t *= b.m_t;
		return c;
	}
	// End of HomoTransform interface
	///////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	// CEuler313 interface
	class Euler313
	{
		// Construction:
	public:
		Euler313();
		Euler313(double a_, double b_, double g_);
		~Euler313();
		void setMatrix();


		// Attributes:
	public:
		// All rotations are clockwise
		double alfa; // Precession, rotation around Z, (deg) 
		double beta; // Nutation, rotation around rotated X, (deg)
		double gamma; // Own rotation around rotated Z, (deg)
		Matrix rd; // X (coord of rotated vector in original frame) = rd * Xd (coord of vector before the rotation, or coord in rotated frame)
		Matrix rb; // Xd (coord of vector in rotated frame) = rb * X (coord of vector in original frame)

		void setAngles(double alfa_, double beta_, double gamma_) {
			alfa = alfa_;
			beta = beta_;
			gamma = gamma_;
			setMatrix();
		}

		// Operations:
	public:
		// Get Theta,Phi in rotated cord given Theta,Phi in fixed coord
		ThetaPhi f2r(const ThetaPhi& tp) const;

		// Get Vector in spherical coord of fixed frame  (Theta, Phi, r) at point (theta, phi)
		// given the Vecord in spherical coord of rotated frame
		// vrSph - Vector in spherical coord of rotated frame (theta, phi, r)
		// tpr - Point in rotated frame
		// tpf - Point in fixed frame
		Point3d vec2f(const Point3d& vrSph, const ThetaPhi& tpr, const ThetaPhi& tpf) const;

		// Get homogenious transformation from rd
		HomoTransform getHomoDirect() const;

		// Get homogenious transformation from rb
		HomoTransform getHomoInverse() const;
	};
	// End of CEuler313 interface
	////////////////////////////////////////////////////////////////////////////////


}