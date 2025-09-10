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

#include "VecMat.h"
#include "Tools.h"
//#include "EllipsoidModel.h"


namespace nzg
{

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Vector implementation 

	void Vector::getSubVector(Vector& v, int n0, int n1) const
	{
		v.resize(n1 - n0);

		for (int i = n0; i < n1; i++)
		{
			v[i - n0] = (*this)[i];
		}
	}

	// End of Vector implementation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Matrix implementation

	Vector Matrix::solve(const Vector& b) const
	{
		assert(rows() == cols());
		assert(b.len() == rows());

		Matrix t(*this);
		Vector v(b);
		Vector x(cols());

		// Excluding Method
		for (int k = 0; k < cols() - 1; k++)
		{
			if (fabs(t(k, k)) == 0.0)
			{
				throw MatrixException("Matrix::solve: Matrix is singular");
			}
			for (int i = k + 1; i < rows(); i++)
			{
				double d = t(i, k) / t(k, k);
				t(i, k) = 0.0;
				for (int j = k + 1; j < cols(); j++)
				{
					t(i, j) = t(i, j) - d * t(k, j);
				}
				v(i) = v(i) - d * v(k);
			}
		}

		// Back ...
		x(rows() - 1) = v(rows() - 1) / t(rows() - 1, cols() - 1);

		for (int i = rows() - 2; i >= 0; i--)
		{
			double s = 0.0;
			for (int j = i + 1; j < rows(); j++)
			{
				s += t(i, j) * x(j);
			}
			x(i) = (v(i) - s) / t(i, i);
		}

		return x;
	}

	void Matrix::inverse()
	{
		if (rs != cs || rs == 0)
			throw MatrixException("Matrix::inverse: Matrix is not square or matrix is empty");

		int M = rs;
		int i, j;

		Matrix E(M, M);

		// E is a E-matrix at the beginning
		for (i = 0; i < M; i++)
			for (j = 0; j < M; j++)
				if (i == j)
					E(i, j) = 1.0;
				else
					E(i, j) = 0.0;

		int* nOrgCol = NULL;
		nOrgCol = new int[M];
		if (nOrgCol == NULL)
			// Never get here because of
			// throwing MemoryException
			return;

		// i-th item contains original column number
		for (i = 0; i < M; i++)
			nOrgCol[i] = i;

		double xNull = 0.0;
		double xDiag = 0.0;
		// Convert matrix to a E-matrix
		// and get inverse matrix in Tmp.
		double* pMatrix = mat.data();
		for (i = 0; i < M; i++)
		{
			double* pRow = pMatrix + M * i;
			// Get diagonal item in current row:
			double* pDiag = pMatrix + M * i + i;
			if (*pDiag == xNull)
			{// Exchange columns:

				// Find appropriate column
				for (j = i + 1; j < M; j++)
					if (*(pRow + j) != xNull)
						break;

				if (j == M) { // Nothing could be done.
					delete[] nOrgCol;
					throw MatrixException("Matrix::inverse: Matrix is singular");
				}

				exchCol(i, j);
				E.exchCol(i, j);
				int nTmp = nOrgCol[i];
				nOrgCol[i] = nOrgCol[j];
				nOrgCol[j] = nTmp;

			}

			// Normalize i-th row
			xDiag = 1.0 / *pDiag;
			multRowByVal(i, xDiag);
			E.multRowByVal(i, xDiag);

			// Process all rows bellow this one
			// to make them like E-matrix rows:
			for (j = i + 1; j < M; j++)
			{
				double* px = pMatrix + M * j + i;
				double xMult = -*px;
				addRows(j, i, xMult);
				E.addRows(j, i, xMult);
			}

			// The same action above this row:
			for (j = i - 1; j >= 0; j--)
			{
				double* px = pMatrix + M * j + i;
				double xMult = -*px;
				addRows(j, i, xMult);
				E.addRows(j, i, xMult);
			}
		}

		for (i = 0; i < M; i++)
		{
			if (nOrgCol[i] == i)
				continue;
			for (j = i + 1; j < M; j++)
			{
				if (i == nOrgCol[j])
				{
					E.exchCol(i, j);
					nOrgCol[j] = nOrgCol[i];
					nOrgCol[i] = i;
					break;
				}
			}
			ASSERT(j < M);
		}

		*this = E;
		delete[] nOrgCol;
	}

	void Matrix::exchRow(int nRow1, int nRow2)
	{
		if (nRow1 == nRow2)
			return;

		for (int i = 0; i < cs; i++)
		{
			std::swap((*this)(nRow1, i), (*this)(nRow2, i));
		}
	}

	void Matrix::exchCol(int nCol1, int nCol2)
	{
		if (nCol1 == nCol2)
			return;

		for (int i = 0; i < rs; i++)
		{
			double t = (*this)(i, nCol1);
			(*this)(i, nCol1) = (*this)(i, nCol2);
			(*this)(i, nCol2) = t;
		}
	}

	void Matrix::multRowByVal(int nDst, double xMult)
	{
		for (int i = 0; i < cs; i++)
			(*this)(nDst, i) *= xMult;
	}

	void Matrix::addRows(int nDst, int nSrc, double mult)
	{
		double* pMatrix = mat.data();
		double* pSrcRow = pMatrix + cs * nSrc;
		double* pDstRow = pMatrix + cs * nDst;

		for (int i = 0; i < cs; i++)
			*(pDstRow + i) += *(pSrcRow + i) * mult;
	}

	void Matrix::copyUpperTriangle()
	{
		for (int i = 0; i < rows(); i++)
		{
			for (int j = 0; j < i; j++)
			{
				(*this)(i, j) = (*this)(j, i);
			}
		}
	}

	Point3d Matrix::transPoint(const Point3d& pt) const
	{
		Point3d ptRes(NAN, NAN, NAN);
		if (cols() == 4 && rows() == 4)
		{
			Vector r1(4); r1[0] = pt.x; r1[1] = pt.y; r1[2] = pt.z; r1[3] = 1;
			Vector r2(4);
			r2 = *this * r1;
			ptRes.x = r2[0];
			ptRes.y = r2[1];
			ptRes.z = r2[2];
		}
		else if (cols() == 3 && rows() == 3)
		{
			Vector r1(3); r1[0] = pt.x; r1[1] = pt.y; r1[2] = pt.z;
			Vector r2(3);
			r2 = *this * r1;
			ptRes.x = r2[0];
			ptRes.y = r2[1];
			ptRes.z = r2[2];
		}
		else
		{
			ASSERT(FALSE);
		}
		return ptRes;
	}

	Matrix Matrix::subMatrix(int nRow0, int nRow1, int nCol0, int nCol1) const
	{
		//ASSERT (sm.GetRowMax () == nRow1 - nRow0);
		//ASSERT (sm.GetColMax () == nCol1 - nCol0);
		Matrix sm;
		sm.resize(nRow1 - nRow0, nCol1 - nCol0);
		for (int i = nRow0; i < nRow1; i++)
		{
			for (int j = nCol0; j < nCol1; j++)
			{
				sm(i - nRow0, j - nCol0) = (*this)(i, j);
			}
		}
		return sm;
	}

	void Matrix::setSubMatrix(const Matrix& sm, int nRow0, int nCol0)
	{
		for (int i = nRow0; i < nRow0 + sm.rows(); i++)
		{
			for (int j = nCol0; j < nCol0 + sm.cols(); j++)
			{
				(*this)(i, j) = sm(i - nRow0, j - nCol0);
			}
		}
	}

	// End of Matrix implementation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HomoTransform implementation

	///////////////////////////
	// Construction:
	HomoTransform::HomoTransform()
	{
		m_t.resize(4, 4);
		m_s.resize(4);
		m_d.resize(4);
		Reset();
	}

	HomoTransform::HomoTransform(const HomoTransform& a)
	{
		*this = a;
	}

	void HomoTransform::Reset()
	{
		m_t.identity();
		m_s.init(0.0);
		m_d.init(0.0);
	}

	HomoTransform::~HomoTransform()
	{
	}

	//////////////////////////////
	// Operations:
	HomoTransform& HomoTransform::operator=(const HomoTransform& a)
	{
		m_t = a.m_t;
		m_s = a.m_s;
		m_d = a.m_d;

		return *this;
	}

	HomoTransform& HomoTransform::operator*=(const HomoTransform& a)
	{
		m_t *= a.m_t;

		return *this;
	}

	Point3d HomoTransform::transform(const Point3d& pt)
	{
		m_s[0] = pt.x;
		m_s[1] = pt.y;
		m_s[2] = pt.z;
		m_s[3] = 1;

		//m_t.VMultRight (m_d, m_s);
		m_d = m_t * m_s;

		Point3d ptDst(m_d[0], m_d[1], m_d[2]);
		return ptDst;
	}

	Point3d HomoTransform::transform(const Point3d& pt) const
	{
		Vector v(4);

		v[0] = pt.x;
		v[1] = pt.y;
		v[2] = pt.z;
		v[3] = 1;

		//m_t.VMultRight (m_d, m_s);
		Vector d(4);
		d = m_t * v;

		Point3d ptDst(d[0], d[1], d[2]);
		return ptDst;
	}

	Size3d HomoTransform::transform(const Size3d& size)
	{
		m_s[0] = size.cx;
		m_s[1] = size.cy;
		m_s[2] = size.cz;
		m_s[3] = 1;

		//m_t.VMultRight (m_d, m_s);
		m_d = m_t * m_s;

		Size3d sizeDst(m_d[0], m_d[1], m_d[2]);
		return sizeDst;
	}

	Point3d HomoTransform::transVector(const Point3d& ptVec) const
	{
		Vector s(4);
		s[0] = ptVec.x;
		s[1] = ptVec.y;
		s[2] = ptVec.z;
		s[3] = 0; // For vectors it seems to be 0 not 1

		//Vector d(4);

		//m_t.VMultRight (m_d, m_s);
		Vector d = m_t * s;

		Point3d ptDst(d[0], d[1], d[2]);
		return ptDst;
	}

	Matrix HomoTransform::propagateCovariance(const Matrix& Q) const
	{
		Matrix A = m_t.subMatrix(0, 3, 0, 3);

		Matrix tmp = A * Q;
		A.transpose();

		Matrix R = tmp * A;
		return R;
	}

	void HomoTransform::rotate(Axis3d& axis, double dAngle)
	{
		double dSinA = sin(dAngle);
		double dCosA = cos(dAngle);

		Matrix t(4, 4);

		Size3d& v = axis.m_sVec;

		t(0, 0) = v.cx * v.cx * (1 - dCosA) + dCosA;
		t(0, 1) = v.cx * v.cy * (1 - dCosA) + v.cz * dSinA;
		t(0, 2) = v.cx * v.cz * (1 - dCosA) - v.cy * dSinA;
		t(0, 3) = 0;

		t(1, 0) = v.cx * v.cy * (1 - dCosA) - v.cz * dSinA;
		t(1, 1) = v.cy * v.cy * (1 - dCosA) + dCosA;
		t(1, 2) = v.cy * v.cz * (1 - dCosA) + v.cx * dSinA;
		t(1, 3) = 0;

		t(2, 0) = v.cx * v.cz * (1 - dCosA) + v.cy * dSinA;
		t(2, 1) = v.cy * v.cz * (1 - dCosA) - v.cx * dSinA;
		t(2, 2) = v.cz * v.cz * (1 - dCosA) + dCosA;
		t(2, 3) = 0;

		t(3, 0) = 0;
		t(3, 1) = 0;
		t(3, 2) = 0;
		t(3, 3) = 1;

		Matrix tOff1(4, 4);
		tOff1.identity();

		tOff1(3, 0) = -axis.m_ptOrg.x;
		tOff1(3, 1) = -axis.m_ptOrg.y;
		tOff1(3, 2) = -axis.m_ptOrg.z;

		Matrix tOff2(4, 4);
		tOff2.identity();

		tOff2(3, 0) = axis.m_ptOrg.x;
		tOff2(3, 1) = axis.m_ptOrg.y;
		tOff2(3, 2) = axis.m_ptOrg.z;

		m_t *= tOff1;
		m_t *= t;
		m_t *= tOff2;
	}

	void HomoTransform::translate(double x, double y, double z)
	{
		Matrix tOff1(4, 4);
		tOff1.identity();

		//tOff1(3, 0) = + x;
		//tOff1(3, 1) = + y;
		//tOff1(3, 2) = + z;
		tOff1(0, 3) = +x;
		tOff1(1, 3) = +y;
		tOff1(2, 3) = +z;

		m_t *= tOff1;
	}

	void HomoTransform::inverse()
	{
		Matrix a(3, 3);
		Vector v(3);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				a(i, j) = m_t(j, i);
			}

			v[i] = m_t(i, 3);
		}

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				m_t(i, j) = a(i, j);
			}
		}

		m_t(0, 3) = -m_t(0, 0) * v[0] - m_t(0, 1) * v[1] - m_t(0, 2) * v[2];
		m_t(1, 3) = -m_t(1, 0) * v[0] - m_t(1, 1) * v[1] - m_t(1, 2) * v[2];
		m_t(2, 3) = -m_t(2, 0) * v[0] - m_t(2, 1) * v[1] - m_t(2, 2) * v[2];
	}

	/*
	Point3d HomoTransform::ecef2blh(const Point3d& ptEcef)
	{
		GPSEllipsoid ell;
		
		Point3d blh;
		double R = sqrt(ptEcef.x * ptEcef.x + ptEcef.y * ptEcef.y);

		double inv_alfa = 1 / ell.flattening();
		double e2 = (inv_alfa + inv_alfa - 1) / (inv_alfa * inv_alfa);
		double tmp = inv_alfa / (inv_alfa - 1.);
		double e12 = tmp * tmp;
		double c = ell.a() * tmp;

		if (R < 1.0e-10)
		{
			blh.l = 0;
			if (ptEcef.z == 0)
			{
				blh.b = 0.;
				blh.h = -ell.a(); // m_a;
			}
			else
			{
				blh.h = fabs(ptEcef.z) - ell.a() * sqrt(1. - e2);
				blh.b = (ptEcef.z < 0) ? -PI / 2 : PI / 2;
			}
		}
		else
		{
			double zr;
			double p = c * e2 / R;
			double tanb0;

			double L = atan2(ptEcef.y, ptEcef.x);
			if (L < 0.)
				L += 2 * PI;

			double tanb = zr = ptEcef.z / R;
			double tanb2 = tanb * tanb;
			blh.l = L;

			do
			{
				tanb0 = tanb;
				tanb = zr + p * tanb / sqrt(e12 + tanb2);
				tanb2 = tanb * tanb;
			} while (fabs(tanb - tanb0) > 1.0e-12);

			blh.h = (R - c / sqrt(e12 + tanb2)) * sqrt(1 + tanb2);
			blh.b = atan(tanb);
		}
		return blh;
	}
	*/

	/*

	HomoTransform HomoTransform::enu2ecef(const Point3d& ptEcefBase)
	{
		Point3d ptBlh = ecef2blh(ptEcefBase);

		double sinb = sin(ptBlh.b);
		double cosb = cos(ptBlh.b);

		double sinl = sin(ptBlh.l);
		double cosl = cos(ptBlh.l);

		HomoTransform t;

		t.m_t(0, 0) = -sinl; t.m_t(0, 1) = -sinb * cosl; t.m_t(0, 2) = cosb * cosl; t.m_t(0, 3) = ptEcefBase.x;
		t.m_t(1, 0) = cosl; t.m_t(1, 1) = -sinb * sinl; t.m_t(1, 2) = cosb * sinl; t.m_t(1, 3) = ptEcefBase.y;
		t.m_t(2, 0) = 0; t.m_t(2, 1) = cosb; t.m_t(2, 2) = sinb; t.m_t(2, 3) = ptEcefBase.z;
		t.m_t(3, 0) = 0; t.m_t(3, 1) = 0; t.m_t(3, 2) = 0; t.m_t(3, 3) = 1;

		return t;
	}
	*/

	/*
	HomoTransform HomoTransform::ecef2enu(const Point3d& ptEcefBase)
	{
		Point3d ptBlh = ecef2blh(ptEcefBase);

		double sinb = sin(ptBlh.b);
		double cosb = cos(ptBlh.b);

		double sinl = sin(ptBlh.l);
		double cosl = cos(ptBlh.l);

		HomoTransform t;

		//     Penu = A * [Pece-Pecefbase] = A * Pecef - A * Pecefbase =>
		// =>  T(1,*) = A(1,1)   A(1,2)  A(1,3)  (-A(1,1)*Pecefbasex - A(1,2)*Pecefbasey - A(1,3)*Pecefbasez)

		t.m_t(0, 0) = -sinl; t.m_t(0, 1) = cosl; t.m_t(0, 2) = 0;
		t.m_t(1, 0) = -sinb * cosl; t.m_t(1, 1) = -sinb * sinl; t.m_t(1, 2) = cosb;
		t.m_t(2, 0) = cosb * cosl; t.m_t(2, 1) = cosb * sinl; t.m_t(2, 2) = sinb;
		t.m_t(3, 0) = 0; t.m_t(3, 1) = 0; t.m_t(3, 2) = 0; t.m_t(3, 3) = 1;

		t.m_t(0, 3) = -t.m_t(0, 0) * ptEcefBase.x - t.m_t(0, 1) * ptEcefBase.y - t.m_t(0, 2) * ptEcefBase.z;
		t.m_t(1, 3) = -t.m_t(1, 0) * ptEcefBase.x - t.m_t(1, 1) * ptEcefBase.y - t.m_t(1, 2) * ptEcefBase.z;
		t.m_t(2, 3) = -t.m_t(2, 0) * ptEcefBase.x - t.m_t(2, 1) * ptEcefBase.y - t.m_t(2, 2) * ptEcefBase.z;

		//t.m_t.fileDump ("m_t.txt", "m_t", TRUE);

		Point3d ptTest = t.transform(ptEcefBase);

		return t;
	}
	*/
	// End of HomoTransform implementation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Euler313 implementation
	Euler313::Euler313() : alfa(0), beta(0), gamma(0)
	{
		rd.resize(3, 3);
		rd.identity();
		rb.resize(3, 3);
		rb.identity();
	}

	Euler313::Euler313(double a_, double b_, double g_) : alfa(a_), beta(b_), gamma(g_)
	{
		setMatrix();
	}

	Euler313::~Euler313()
	{
	}

	void Euler313::setMatrix()
	{
		rd.resize(3, 3);
		rb.resize(3, 3);

		double ca = cos(alfa * PI / 180);
		double sa = sin(alfa * PI / 180);
		double cb = cos(beta * PI / 180);
		double sb = sin(beta * PI / 180);
		double cg = cos(gamma * PI / 180);
		double sg = sin(gamma * PI / 180);

		rd(0, 0) = ca * cg - sa * cb * sg;
		rd(0, 1) = -ca * sg - sa * cb * cg;
		rd(0, 2) = sa * sb;

		rd(1, 0) = sa * cg + ca * cb * sg;
		rd(1, 1) = -sa * sg + ca * cb * cg;
		rd(1, 2) = -ca * sb;

		rd(2, 0) = sb * sg;
		rd(2, 1) = sb * cg;
		rd(2, 2) = cb;

		rb = rd;
		rb.transpose();
	}

	////////////////////////////
	// Operations:
	ThetaPhi Euler313::f2r(const ThetaPhi& tp) const
	{
		Point3d f = tp.unitPoint();

		// Coord of unit vector in rotated frame
		// r = (sin(td)*cos(pd), sin(td)*sin(pd), cos(td))
		Point3d r = rb.transPoint(f);

		double phi = atan2(r.y, r.x);
		double cosPhi = cos(phi);
		double sinPhi = sin(phi);
		// Avoid cos(phi) = 0 and sin(phi) = 0 case
		double sinTheta = fabs(cosPhi) > fabs(sinPhi) ? r.x / cosPhi : r.y / sinPhi;

		double theta = atan2(sinTheta, r.z);
		return ThetaPhi(theta, phi);
	}

	Point3d Euler313::vec2f(const Point3d& vrSph, const ThetaPhi& tpr, const ThetaPhi& tpf) const
	{
		// Vector transformation matrix from sph to decart in rotated frame
		Matrix rr(3, 3);

		{
			double ce = cos(PI / 2 - tpr.theta);
			double se = sin(PI / 2 - tpr.theta);
			double cp = cos(tpr.phi);
			double sp = sin(tpr.phi);

			// From https://www.mathworks.com/help/phased/ref/cart2sphvec.html 
			// Columns 0 and 1 are rearranged because in mathworks vector is (Phi, Theta, r) in cotrast to (Theta, Phi, r) here
			// Sign of Theta component is changed because in mathworks it is Elevation, not Theta
			rr(0, 1) = -sp; rr(0, 0) = -(-se * cp); rr(0, 2) = ce * cp;
			rr(1, 1) = cp; rr(1, 0) = (-se * sp); rr(1, 2) = ce * sp;
			rr(2, 1) = 0; rr(2, 0) = -(ce); rr(2, 2) = se;
		}

		// Decart vector in rotated frame
		Point3d vr = rr.transPoint(vrSph);

		// Decart vector in fixed frame
		Point3d vf = rd.transPoint(vr);

		// Vector transformation matrix from decart to sph in original fixed frame
		Matrix rf(3, 3);
		{
			double ce = cos(PI / 2 - tpf.theta);
			double se = sin(PI / 2 - tpf.theta);
			double cp = cos(tpf.phi);
			double sp = sin(tpf.phi);

			// From https://www.mathworks.com/help/phased/ref/cart2sphvec.html 
			// Columns 0 and 1 are rearranged because in mathworks vector is (Phi, Theta, r) in cotrast to (Theta, Phi, r) here
			// After the transpose bellow it is equivalent to rearrange rows 0 and 1
			rf(0, 1) = -sp; rf(0, 0) = -(-se * cp); rf(0, 2) = ce * cp;
			rf(1, 1) = cp; rf(1, 0) = -(-se * sp); rf(1, 2) = ce * sp;
			rf(2, 1) = 0; rf(2, 0) = -(ce); rf(2, 2) = se;
			rf.transpose();
		}

		Point3d vfSph = rf.transPoint(vf);
		return vfSph;
	}

	HomoTransform Euler313::getHomoDirect() const
	{
		HomoTransform t;
		t.m_t.setSubMatrix(rd, 0, 0);
		return t;
	}

	HomoTransform Euler313::getHomoInverse() const
	{
		HomoTransform t;
		t.m_t.setSubMatrix(rb, 0, 0);
		return t;
	}
	// End of Euler313 implementation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}