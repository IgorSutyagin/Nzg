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
#include "pch.h"

#include "Sph.h"
#include "Tools.h"


/////////////////////////////////////////////////////////////
// Spherical Harmonics implementation

namespace nzg
{
	SphericalHarmonics gl_sph(-90); 
}


using namespace nzg;

double SphericalHarmonics::factorial(int n)
{
	static double dres[14] =
	{
		1.0, // 0
		1.0, // 1
		2.0, // 2
		6.0, // 3
		24.0, // 4
		120.0, // 5
		720.0, // 6
		5040.0, // 7
		40320.0, // 8
		362880.0, // 9
		3628800.0, // 10
		39916800.0, // 11
		479001600.0, // 12,
		6227020800.0, // 13
	};

	if (n <= 13)
		return dres[n];

	double result = dres[13];

	//for( int i=13; i<=n; ++i )
	//	result *= i;
	for (int i = 14; i <= n; ++i)
		result *= i;

	return result;
}

double SphericalHarmonics::factorial2(int n)
{
	static double dres[14] =
	{
		1.0, // 0
		1.0, // 1
		2.0, // 2
		3.0, // 3
		8.0, // 4
		15.0, // 5
		48.0, // 6
		105.0, // 7
		384.0, // 8
		945.0, // 9
		3840.0, // 10
		10395.0, // 11
		645120.0, // 12,
		135135.0, // 13
	};

	if (n <= 13)
		return dres[n];

	if (n % 2 == 0)
	{ // Even case
		double result = dres[12];

		for (int i = 14; i <= n; i += 2)
			result *= i;

		return result;
	}
	else
	{ // Odd case
		double result = dres[13];

		for (int i = 15; i <= n; i += 2)
			result *= i;

		return result;
	}
}

double SphericalHarmonics::factorialFact(int n, int m)
{
	double fact = 1;

	for (int i = n - m + 1; i <= n + m; i++)
	{
		fact *= i;
	}
	return fact;
}

double SphericalHarmonics::minPlus(int n)
{
	if (::abs(n) % 2 == 0)
		return 1.0;

	return -1.0;
}

double SphericalHarmonics::P(int l, int m, double x)
{
	// Code taken from 'Robin Green - Spherical Harmonic Lighting'.
	double pmm = 1;
	if (m > 0)
	{
		double somx2 = ::sqrt((1 - x) * (1 + x));
		double fact = 1;
		for (int i = 1; i <= m; ++i)
		{
			pmm *= -fact * somx2;
			fact += 2;
		}
	}

	if (l == m)
		return pmm;

	double pmmp1 = x * (2 * m + 1) * pmm;
	if (l == m + 1)
		return pmmp1;

	double pll = 0.0;
	for (int ll = m + 2; ll <= l; ++ll)
	{
		pll = ((2 * ll - 1) * x * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}

	return pll;
}

double SphericalHarmonics::K(int l, int m) const
{
	return ::sqrt(((2 * l + 1) * factorial(l - m)) / (4 * PI * factorial(l + m)));
}

double SphericalHarmonics::Y(int l, int m, double theta, double phi) const
{
	const double sqrt2 = ::sqrt(2.0);
	if (m == 0)
		return K(l, 0) * P(l, m, m_dK * ::cos(theta) + m_dH);
	else if (m > 0)
		return sqrt2 * K(l, m) * ::cos(m * phi) * P(l, m, m_dK * ::cos(theta) + m_dH);
	else
		return sqrt2 * K(l, -m) * ::sin(-m * phi) * P(l, -m, m_dK * ::cos(theta) + m_dH);
}

void SphericalHarmonics::getLegendreDerivarives(int m, int lMax, double theta, std::vector<double>& ders, std::vector<int>& ls) const
{
	ders.resize(0);
	ls.resize(0);

	// For l = 0 P(0,0) = 1 so dP/dt = 0, so begin with l=max(m, 1)
	int lFirst = (std::max)(::abs(m), 1);
	double Plmin2 = 0;
	double Plmin1 = NAN;
	double sint = ::sin(theta);
	double cost = ::cos(theta);
	for (int n = lFirst; n <= lMax; n++)
	{
		if (n == lFirst)
		{
			if (n == ::abs(m))
			{
				Plmin1 = minPlus(n) * factorial2(2 * n - 1) * pow(sint, n);
				double dp = Plmin1 * n / sint * cost;
				if (m < 0)
				{
					double d = minPlus(n) / factorial(2 * n);
					dp *= d;
				}
				ders.push_back(dp);
				ls.push_back(n);
			}
			else // m == 0, n == 1, P(cos(theta)) = cos(theta)
			{
				ASSERT(m == 0 && n == 1);
				Plmin1 = cost;
				double dp = -sint;
				ders.push_back(dp);
				ls.push_back(n);
			}
			continue;
		}

		double Plm = ((2 * n - 1) * cost * Plmin1 - (n + ::abs(m) - 1) * Plmin2) / (n - ::abs(m));
		double dp = (n * cost * Plm - (n + ::abs(m)) * Plmin1) / sint;
		if (m < 0)
		{
			//double d = minPlus(m) * factorial (n - ::abs(m)) / factorial(n + ::abs(m));
			double d = minPlus(m) / factorialFact(n, ::abs(m));
			dp *= d;
		}

		Plmin2 = Plmin1;
		Plmin1 = Plm;
		ders.push_back(dp);
		ls.push_back(n);
	}
}

double SphericalHarmonics::integYsincos(int l, int m, double theta0) const
{
	if (m != 1)
		return 0.0;

	const double sqrt2 = ::sqrt(2.0);
	double dIntPhi = sqrt2 * K(l, 1) * PI;

	int nPoints = 100;
	double dt = theta0 / (nPoints - 1);
	double dIntTheta = 0;
	for (int i = 0; i < nPoints; i++)
	{
		double t = dt * i;
		double d = P(l, 1, m_dK * ::cos(t) + m_dH) * ::sin(t);
		if (i == 0 || i == nPoints - 1)
			dIntTheta += d / 2;
		else
			dIntTheta += d;
	}

	dIntTheta *= theta0 / (nPoints - 1);
	return dIntTheta * dIntPhi;
}

double SphericalHarmonics::integYsinsin(int l, int m, double theta0) const
{
	if (m != -1)
		return 0.0;

	const double sqrt2 = ::sqrt(2.0);
	double dIntPhi = sqrt2 * K(l, 1) * PI;

	int nPoints = 100;
	double dt = theta0 / (nPoints - 1);
	double dIntTheta = 0;
	for (int i = 0; i < nPoints; i++)
	{
		double t = dt * i;
		double d = P(l, 1, m_dK * ::cos(t) + m_dH) * ::sin(t);
		if (i == 0 || i == nPoints - 1)
			dIntTheta += d / 2;
		else
			dIntTheta += d;
	}

	dIntTheta *= theta0 / (nPoints - 1);
	return dIntTheta * dIntPhi;
}

double SphericalHarmonics::integYcos(int l, int m, double theta0) const
{
	if (m != 0)
		return 0.0;

	double dIntPhi = K(l, 0) * 2 * PI;

	int nPoints = 100;
	double dt = theta0 / (nPoints - 1);
	double dIntTheta = 0;
	for (int i = 0; i < nPoints; i++)
	{
		double t = dt * i;
		double d = P(l, 0, m_dK * ::cos(t) + m_dH) * ::cos(t);
		if (i == 0 || i == nPoints - 1)
			dIntTheta += d / 2;
		else
			dIntTheta += d;
	}

	dIntTheta *= theta0 / (nPoints - 1);
	return dIntTheta * dIntPhi;
}

double SphericalHarmonics::integY(int l, int m, double theta0) const
{
	if (m != 0)
		return 0.0;

	double dIntPhi = K(l, 0) * 2 * PI;

	int nPoints = 100;
	double dt = theta0 / (nPoints - 1);
	double dIntTheta = 0;
	for (int i = 0; i < nPoints; i++)
	{
		double t = dt * i;
		double d = P(l, 0, m_dK * ::cos(t) + m_dH);
		if (i == 0 || i == nPoints - 1)
			dIntTheta += d / 2;
		else
			dIntTheta += d;
	}

	dIntTheta *= theta0 / (nPoints - 1);
	return dIntTheta * dIntPhi;
}

SphericalHarmonics& SphericalHarmonics::operator=(const SphericalHarmonics& a)
{
	m_dK = a.m_dK;
	m_dH = a.m_dH;
	m_dEleMask = a.m_dEleMask;
	return *this;
}

bool SphericalHarmonics::operator==(const SphericalHarmonics& a) const
{
	return m_dK == a.m_dK && m_dH == a.m_dH;
}

bool SphericalHarmonics::operator!=(const SphericalHarmonics& a) const
{
	return !(m_dK == a.m_dK && m_dH == a.m_dH);
}

void SphericalHarmonics::setShifting(double eleMaskDeg)
{
	ASSERT(eleMaskDeg < 90.0 && eleMaskDeg >= -90.0);
	double t0 = (90.0 - eleMaskDeg) * PI / 180.0;
	m_dK = 2.0 / (1 - ::cos(t0));
	m_dH = (::cos(t0) + 1) / (::cos(t0) - 1);
	m_dEleMask = eleMaskDeg;
}

/*
double SphericalHarmonics::getEleMaskDeg() const
{
	double t0 = acos(1 - (2.0 / m_dK));
	return 90.0 - t0 * 180 / PI;
}
*/

void SphericalHarmonics::serialize(Archive& ar)
{
	if (ar.isStoring())
	{
		DWORD dwVer = 2;
		ar << dwVer;
		ar << m_dK;
		ar << m_dH;
		ar << m_dEleMask;
	}
	else
	{
		DWORD dwVer;
		ar >> dwVer;
		ar >> m_dK;
		ar >> m_dH;
		if (dwVer >= 2)
		{
			ar >> m_dEleMask;
		}
		else
		{
			double t0 = acos(1 - (2.0 / m_dK));
			m_dEleMask = 90.0 - t0 * 180 / PI;
		}
	}
}
// End of Spherical Harmonics implementation
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// ShProjection implementation

////////////////////
// Construction:
ShProjection::ShProjection()
{
	m_nBands = 0;
	m_mBands = 0;
	m_sigma = 0;
}

ShProjection::~ShProjection()
{
}

void ShProjection::reset()
{
	setBands(0, 0, gl_sph); // -90);
}

///////////////////////
// Attributes:
void ShProjection::setBands(int nBands, int mBands, double dEleMaskDeg)
{
	//ASSERT (nBands >= 0);
	m_nBands = nBands;
	m_mBands = mBands;
	m_sph.setShifting(dEleMaskDeg);

	// int nCoefs = m_nBands * m_nBands;
	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	m_coefs.resize(nCoefs); // m_nBands * m_nBands);
	for (int i = 0; i < nCoefs; i++)
	{
		m_coefs[i] = 0;
	}
	m_q.resize(nCoefs, nCoefs);
	m_q.init(0);
	m_sigma = 0.0;
}

void ShProjection::setBands(int nBands, int mBands, const SphericalHarmonics& sp)
{
	//ASSERT (nBands >= 0);
	m_nBands = nBands;
	m_mBands = mBands;
	m_sph = sp;

	// int nCoefs = m_nBands * m_nBands;
	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	m_coefs.resize(nCoefs); // m_nBands * m_nBands);
	for (int i = 0; i < nCoefs; i++)
	{
		m_coefs[i] = 0;
	}
	m_q.resize(nCoefs, nCoefs);
	m_q.init(0);
	m_sigma = 0.0;
}

bool ShProjection::canEvaluate() const
{
	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	if (m_coefs.size() != nCoefs) //m_nBands * m_nBands)
		return false;

	for (int i = 0; i < (int)m_coefs.size(); i++)
	{
		if (m_coefs[i] != 0.0)
			return true;
	}

	return false;
}

void ShProjection::setCoef(int l, int m, double c)
{
	ASSERT(0 <= l && l < m_nBands);
	ASSERT(-m_mBands < m && m < m_mBands);

	int nIndex = getIndex(l, m);
	m_coefs[nIndex] = c;
}

double ShProjection::getCoef(int l, int m) const
{
	ASSERT(0 <= l && l < m_nBands);
	//ASSERT(-m_mBands < m && m < m_mBands);
	if (m <= -m_mBands || m >= m_mBands)
		return 0;

	int nIndex = getIndex(l, m);
	return m_coefs[nIndex];
}

void ShProjection::getYmn(Vector& Y, double theta, double phi) const
{
	Y.resize(getSize());

	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			int nIndex = getIndex(l, m);

			double d = m_sph.Y(l, m, theta, phi);
			Y[nIndex] = d;
		}
	}
}

void ShProjection::getY0n(Vector& Y, double theta) const
{
	Y.resize(m_nBands);

	for (int l = 0; l < m_nBands; l++)
	{
		//for (int m = -min(l, m_mBands - 1); m <= min(l, m_mBands - 1); m++)
		int m = 0;
		{
			//int nIndex = getIndex(l, m);

			double d = m_sph.Y(l, m, theta, 0);
			Y[l] = d;
		}
	}
}

//CF3DPoint ShProjection::getPhaseCenter() const
//{
//	double c = ::sqrt(3.0 / (4 * PI));
//	return CF3DPoint(-getCoef(1, 1)*c, -getCoef(1, -1)*c, getCoef(1, 0)*c);
//}

class IdealPatFunction : public ShFunction
{
public:
	IdealPatFunction(const SphericalHarmonics& sph_, const Point3d& ptPco_, double wl_, int nEles_, int nAzs_) {
		sph = sph_;
		ptPcoWl = ptPco_ / wl_;
		nEles = nEles_;
		nAzs = nAzs_;
	}

	// Attributes:
public:
	SphericalHarmonics sph;
	Point3d ptPcoWl;
	int nEles;
	int nAzs;

	// Overrides:
public:
	virtual int getMeasCount() {
		return nEles * nAzs;
	}

	virtual double getMeas(int i, double* ptheta, double* pfi) {
		int nEle = i / nAzs;
		int nAz = i % nAzs;
		double dt = sph.getThetaMask() / (nEles - 1);
		double da = 360.0 / nAzs;
		*ptheta = dt * nEle;
		*pfi = da * nAz;
		double ct = cos(*ptheta * PI / 180);
		double st = sin(*ptheta * PI / 180);
		double cf = cos(*pfi * PI / 180);
		double sf = sin(*pfi * PI / 180);
		double ph = st * cf * ptPcoWl.x + st * sf * ptPcoWl.y + ct * ptPcoWl.z;
		return ph;
	}
};

void ShProjection::setPhaseCenter(const Point3d& ptPco, double wl)
{
	/*
	ASSERT(m_nBands >= 2 && m_mBands >= 2);
	CF3DPoint pt = ptCnt / wl;
	double c = ::sqrt(3.0 / (4 * PI));
	setCoef(1, 1, -pt.x / c);
	setCoef(1, -1, -pt.y / c);
	setCoef(1, 0, pt.z / c);
	*/
	IdealPatFunction f(m_sph, ptPco, wl, 90, 120);

	project(f, m_nBands, m_mBands);
}


///////////////////////
// Operations:
ShProjection& ShProjection::operator=(const ShProjection& a)
{
	m_nBands = a.m_nBands;
	m_mBands = a.m_mBands;
	m_coefs.resize(a.m_coefs.size());
	m_sph = a.m_sph;
	for (int i = 0; i < (int)m_coefs.size(); i++)
	{
		m_coefs[i] = a.m_coefs[i];
	}
	m_q = a.m_q;
	m_sigma = a.m_sigma;

	return *this;
}

void ShProjection::averageWeight(const ShProjection& a, double dWeight)
{
	if (m_nBands != a.m_nBands)
	{
		setBands(a.m_nBands, a.m_mBands, a.m_sph);
	}

	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	for (int i = 0; i < nCoefs; i++) // m_nBands*m_nBands
	{
		m_coefs[i] += dWeight * a.m_coefs[i];
	}
}

void ShProjection::mix(const ShProjection& a, double alfa)
{
	ASSERT(m_nBands == a.m_nBands && m_mBands == a.m_mBands && m_sph == a.m_sph);

	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	for (int i = 0; i < nCoefs; i++) // m_nBands*m_nBands
	{
		m_coefs[i] = (1 - alfa) * m_coefs[i] + alfa * a.m_coefs[i];
	}
}

void ShProjection::project(ShFunction& shf, int nBands, int mBands)
{
	m_nBands = nBands;
	m_mBands = mBands;

	//int nCoefs = m_nBands * m_nBands;
	int nCoefs = getSize(); // m_mBands*m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	m_coefs.resize(nCoefs);

	Matrix A(nCoefs, nCoefs);
	A.init(0);
	Vector B(nCoefs);
	B.init(0);
	Vector X(nCoefs);

	for (int j = 0; j < shf.getMeasCount(); j++)
	{
		double theta = 0;
		double fi = 0;
		double dMeas = shf.getMeas(j, &theta, &fi);
		if (_isnan(dMeas))
			continue;

		for (int l = 0; l < m_nBands; l++)
		{
			for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
			{
				//int nIndex1 = l < m_mBands ? l * (l+1) + m : 
				//	m_mBands * m_mBands + (l - m_mBands) * (2*m_mBands - 1) + (m + m_mBands - 1);
				int nIndex1 = getIndex(l, m);

				double f1 = m_sph.Y(l, m, theta, fi);
				B[nIndex1] += dMeas * f1;
				for (int p = 0; p < m_nBands; p++)
				{
					for (int q = -std::min(p, m_mBands - 1); q <= std::min(p, m_mBands - 1); q++)
					{
						//int nIndex2 = p < m_mBands ? p * (p+1) + q :
						//	m_mBands * m_mBands + (p - m_mBands) * (2 * m_mBands - 1) + (q + m_mBands - 1);
						int nIndex2 = getIndex(p, q);

						double f2 = m_sph.Y(p, q, theta, fi);

						A(nIndex1, nIndex2) += f1 * f2;
						//A(nIndex2, nIndex1) = A(nIndex1, nIndex2);
					}
				}
			}
		}
	}

	//A.CopyUpperTriangle();
	//A.fileDump("test.txt", B, "shp", TRUE);
	X = A.solve(B);

	for (int i = 0; i < nCoefs; i++)
	{
		m_coefs[i] = X[i];
	}

	m_q = A;
	m_q.inverse();

	double dSum = 0;
	for (int j = 0; j < shf.getMeasCount(); j++)
	{
		double theta = 0;
		double fi = 0;
		double dMeas = shf.getMeas(j, &theta, &fi);
		double proj = pcc(ThetaPhi(theta, fi));
		dSum += (dMeas - proj) * (dMeas - proj);
	}

	m_sigma = dSum / (shf.getMeasCount() - nCoefs);
}

class ShProjectionFunction : public ShFunction
{
public:
	ShProjectionFunction(const ShProjection* p, double eleMask) {
		m_psh = p;
		m_eleMask = eleMask;
	}
	~ShProjectionFunction() {}

	const ShProjection* m_psh;
	double m_eleMask;

	// Overrides:
public:
	int getMeasCount() {
		return 180 * 181;
	}
	double getMeas(int i, double* ptheta, double* pfi)
	{
		int nAz = 2 * (i % 180); // i % 181;
		int nT = (i / 181);
		*ptheta = nT * PI / 180;
		*ptheta *= (90 - m_eleMask) / 180.0;
		*pfi = nAz * PI / 180;
		return m_psh->pcc(ThetaPhi(*ptheta, *pfi));
	}
};

ShProjection ShProjection::project(int nBands, int mBands, const SphericalHarmonics& sph, double eleMask) const
{
	ShProjectionFunction fsh(this, eleMask);
	ShProjection shp;
	shp.m_sph = sph;
	shp.project(fsh, nBands, mBands); // m_nBands, m_mBands
	return shp;
}

void ShProjection::appendEqs(double ele, double az, double dRotAngle, double dPh, double weight, Matrix& N, Vector& U) const
{
	double theta = (90 - ele) * PI / 180;
	double fi = (90 - az) * PI / 180;

	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			//int nIndex1 = l * (l+1) + m;
			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			double f1 = m_sph.Y(l, m, theta, fi);
			U[nIndex1] += weight * dPh * f1;
			for (int p = 0; p < m_nBands; p++)
			{
				for (int q = -std::min(p, m_mBands - 1); q <= std::min(p, m_mBands - 1); q++)
				{
					//int nIndex2 = p * (p+1) + q;
					//int nIndex2 = p < m_mBands ? p * (p + 1) + q :
					//	m_mBands * m_mBands + (p - m_mBands) * (2 * m_mBands - 1) + (q + m_mBands - 1);
					int nIndex2 = getIndex(p, q);

					if (nIndex2 > nIndex1)
						continue;

					double f2 = m_sph.Y(p, q, theta, fi);

					N(nIndex2, nIndex1) += weight * f1 * f2;
					//N(nIndex1, nIndex2) = N(nIndex2, nIndex1);
				}
			}
		}
	}
}

void ShProjection::appendEqs(double ele, double az, double dPh, double weight, Matrix& N, Vector& U) const
{
	appendEqs(ele, az, 0, dPh, weight, N, U);
}

void ShProjection::appendEqs2(double ele, double az, double dPh, double weight, Matrix& N, Vector& U)
{
	double theta = (90 - ele) * PI / 180;
	double phid = 90 - az;
	double fi = phid * PI / 180;

	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			int nIndex1 = getIndex(l, m);

			double f1 = m_sph.Y(l, m, theta, fi);
			U[nIndex1] += weight * dPh * f1;
			for (int p = 0; p < m_nBands; p++)
			{
				for (int q = -std::min(p, m_mBands - 1); q <= std::min(p, m_mBands - 1); q++)
				{
					int nIndex2 = getIndex(p, q);

					if (nIndex2 > nIndex1)
						continue;

					double f2 = m_sph.Y(p, q, theta, fi);

					N(nIndex2, nIndex1) += weight * f1 * f2;
				}
			}
		}
	}
}

void ShProjection::appendEqs(const ZenAz& za, double dPh, double weight, Matrix& N, Vector& U) const
{
	double theta = za.zen * PI / 180;
	double phid = 90 - za.az;
	double fi = phid * PI / 180;

	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			int nIndex1 = getIndex(l, m);
			if (nIndex1 < 0)
				continue;

			double f1 = m_sph.Y(l, m, theta, fi);
			U[nIndex1] += weight * dPh * f1;
			for (int p = 0; p < m_nBands; p++)
			{
				for (int q = -std::min(p, m_mBands - 1); q <= std::min(p, m_mBands - 1); q++)
				{
					int nIndex2 = getIndex(p, q);
					if (nIndex2 < 0)
						continue;

					if (nIndex2 > nIndex1)
						continue;

					double f2 = m_sph.Y(p, q, theta, fi);

					N(nIndex2, nIndex1) += weight * f1 * f2;
					N(nIndex1, nIndex2) = N(nIndex2, nIndex1);
				}
			}
		}
	}
}

void ShProjection::appendEqsSemi(const ZenAz& za, double dPh, double weight, Matrix& N, Vector& U) const
{
	double theta = za.zen * PI / 180;
	double phid = 90 - za.az;
	double fi = phid * PI / 180;

	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			int nIndex1 = getSemiIndex(l, m);
			if (nIndex1 < 0)
				continue;

			double f1 = m_sph.Y(l, m, theta, fi);
			U[nIndex1] += weight * dPh * f1;
			for (int p = 0; p < m_nBands; p++)
			{
				for (int q = -std::min(p, m_mBands - 1); q <= std::min(p, m_mBands - 1); q++)
				{
					int nIndex2 = getSemiIndex(p, q);
					if (nIndex2 < 0)
						continue;

					if (nIndex2 > nIndex1)
						continue;

					double f2 = m_sph.Y(p, q, theta, fi);

					N(nIndex2, nIndex1) += weight * f1 * f2;
					N(nIndex1, nIndex2) = N(nIndex2, nIndex1);
				}
			}
		}
	}
}

void ShProjection::setCoefs(Vector& X)
{
	ASSERT(m_coefs.size() == X.size());
	m_coefs.resize(X.size());
	for (int i = 0; i < (int)m_coefs.size(); i++)
	{
		m_coefs[i] = X[i];
	}
}

void ShProjection::setSemiCoefs(Vector& X)
{
	m_coefs.resize(getSize());
	for (int l = 0; l < m_nBands; l++)
	{
		for (int m = -l; m <= l; m++)
		{
			int nIndex = getIndex(l, m);
			if ((m + l) % 2 != 0)
			{
				m_coefs[nIndex] = 0;
				continue;
			}

			int nSemiIndex = getSemiIndex(l, m);
			m_coefs[nIndex] = X[nSemiIndex];
		}
	}
}

double ShProjection::pcc(const ThetaPhi& tp, const ShProjection::UseHarms* pUseHarms) const
{
	double result = 0.0;

	for (int l = 0; l < m_nBands; l++)
	{
		double dBand = 0;
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			if (pUseHarms != nullptr && !pUseHarms->get(l, m))
				continue;
			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			double dY = m_sph.Y(l, m, tp.theta, tp.phi);
			double dC = m_coefs[nIndex1]; // l*(l + 1) + m];
			dBand += dC * dY;
		}

		result += dBand;
	}

	return result;
}

double ShProjection::pcv(const ThetaPhi& tp, const Point3d& pco) const
{
	double result = pcc(tp);

	double pcoPcc = ZenAz(tp).dirCos() * pco;

	return result - pcoPcc;
}
/*
double ShProjection::evaluateEleAz(const CEleAz& ea) const
{
double theta = (90 - ele) * PI / 180;
double fi = (90 - az) * PI / 180;
return evaluate(theta, fi);
}
*/

double ShProjection::evaluateWo0(const ThetaPhi& tp) const
{
	double result = 0.0;

	for (int l = 0; l < m_nBands; l++)
	{
		//if (l == 0)
		//	continue;

		double dBand = 0;
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			if (l == 0 && m == 0)
				continue;

			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			double dY = m_sph.Y(l, m, tp.theta, tp.phi);
			double dC = m_coefs[nIndex1]; // l*(l + 1) + m];
			dBand += dC * dY;
		}

		result += dBand;
	}

	return result;
}

double ShProjection::evaluate(double theta) const
{
	double result = 0.0;

	for (int l = 0; l < m_nBands; l++)
	{
		double dBand = 0;
		//for (int m = -l; m <= l; m++)
		int m = 0;
		{
			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			double dY = m_sph.Y(l, m, theta, 0);
			double dC = m_coefs[nIndex1]; // l*(l + 1) + m];
			dBand += dC * dY;
		}

		result += dBand;
	}

	return result;
}

double ShProjection::evaluate(const ThetaPhi& tp) const
{
	double result = 0.0;

	for (int l = 0; l < m_nBands; l++)
	{
		double dBand = 0;
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			double dY = m_sph.Y(l, m, tp.theta, tp.phi);
			double dC = m_coefs[nIndex1]; // l*(l + 1) + m];
			dBand += dC * dY;
		}

		result += dBand;
	}

	return result;
}

double ShProjection::evaluatePcvOnly(double theta, Point3d* pptCnt) const
{
	double result = 0.0;

	if (pptCnt == NULL)
	{

		for (int l = 2; l < m_nBands; l++)
		{
			double dBand = 0;
			//for (int m = -l; m <= l; m++)
			int m = 0;
			{
				//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
				//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
				int nIndex1 = getIndex(l, m);

				double dY = m_sph.Y(l, m, theta, 0);
				double dC = m_coefs[nIndex1]; // l*(l + 1) + m];
				dBand += dC * dY;
			}

			result += dBand;
		}
	}
	else
	{
		double cost = ::cos(theta);
		double sint = ::sin(theta);
		double daz = 2.0;
		double dphi = daz * DEG2RAD;
		double dSum = 0;
		int nCount = 0;
		//for (double az = 0; az < 360.0; az += daz)
		for (double phi = 0; phi < 2 * PI; phi += dphi)
		{
			double cosa = ::cos(phi * PI / 180); // ::cos(az * PI / 180);
			double sina = ::sin(phi * PI / 180); // ::sin(az * PI / 180);
			double phCnt = sint * cosa * pptCnt->x + sint * sina * pptCnt->y + cost * pptCnt->z;
			double ph = pcc(ThetaPhi(theta, phi)); // az));
			dSum += (ph - phCnt);
			nCount++;
		}

		result = dSum / nCount;
	}

	return result;
}

Point3d ShProjection::calcOffset(double dEleMask, ZenAz zaStep) const
{
	CWaitCursor wc;

	Matrix a(4, 4); // N, N);
	a.init(0);
	Vector b(4); // N);
	b.init(0);
	int nCurEq = 0;

	double dAzStep = zaStep.az;
	double dZenStep = zaStep.zen;
	int nStep = 0;

	for (double az = 0; az < 360; az += dAzStep)
	{
		double phi = (90 - az) * PI / 180;
		double cphi = ::cos(phi);
		double sphi = ::sin(phi);

		for (double zen = 0; zen<=90.0 - dEleMask; zen += dZenStep)
		{
			double theta = zen * PI / 180;
			double ct = ::cos(theta);
			double st = ::sin(theta);

			double pcv = pcc(ThetaPhi(theta, phi)); // evaluate(PI / 2 - theta, phi);
			double ph = pcv;

			double w = 1;
			Point3d pt(cphi * st, sphi * st, ct);

			a(0, 0) += w * pt.x * pt.x; a(0, 1) += w * pt.x * pt.y; a(0, 2) += w * pt.x * pt.z;
			a(1, 0) += w * pt.y * pt.x; a(1, 1) += w * pt.y * pt.y; a(1, 2) += w * pt.y * pt.z;
			a(2, 0) += w * pt.z * pt.x; a(2, 1) += w * pt.z * pt.y; a(2, 2) += w * pt.z * pt.z;

			a(0, 3) += w * pt.x; //  + nCurEq
			a(1, 3) += w * pt.y; //  + nCurEq
			a(2, 3) += w * pt.z; //  + nCurEq

			a(3, 0) += w * pt.x; //  + nCurEq
			a(3, 1) += w * pt.y; //  + nCurEq
			a(3, 2) += w * pt.z; //  + nCurEq

			a(3, 3) += 1 * w;

			b[0] += w * pt.x * ph;
			b[1] += w * pt.y * ph;
			b[2] += w * pt.z * ph;
			b[3] += w * ph;

		}
	}

	Vector x = a.solve(b);

	return Point3d(x[0], x[1], x[2]);
}


void ShProjection::dump() const
{
	for (int l = 0; l < m_nBands; l++)
	{
		CString csRow;
		//for (int m=0; m<=l; m++)
		for (int m = -std::min(l, m_mBands - 1); m <= std::min(l, m_mBands - 1); m++)
		{
			//int nIndex1 = l < m_mBands ? l * (l + 1) + m :
			//	m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
			int nIndex1 = getIndex(l, m);

			if (m == 0)
			{
				double d = m_coefs[nIndex1]; // l*(l + 1) + m];
				CString cs;
				cs.Format("%f\t", d);
				csRow += cs;
			}
			else
			{
				double d1 = m_coefs[nIndex1]; // l*(l + 1) + m];
				//double d2 = m_coefs[nIndex1]; // l*(l + 1) - m];
				CString cs;
				cs.Format("%f\t", d1);
				csRow += cs;
			}
		}

		csRow += "\n";
		TRACE(csRow);
	}

}

///////////////////////////
// Serialization:
void ShProjection::serialize(Archive& ar)
{
	static DWORD dwVersion = 3;
	if (ar.isStoring())
	{
		ar << dwVersion;
		ar << m_nBands;
		int nSize = m_coefs.size();
		ar << nSize;
		for (int i = 0; i < nSize; i++)
		{
			ar << m_coefs[i];
		}
		ar << m_mBands;

		m_sph.serialize(ar);
	}
	else
	{
		DWORD dwVer = 0;
		ar >> dwVer;
		ar >> m_nBands;
		int nSize = 0;
		ar >> nSize;
		m_coefs.resize(nSize);
		for (int i = 0; i < nSize; i++)
		{
			ar >> m_coefs[i];
		}
		if (dwVersion >= 2)
		{
			ar >> m_mBands;
		}
		else
		{
			m_mBands = m_nBands;
		}

		if (dwVer >= 3)
			m_sph.serialize(ar);
		else
			m_sph.setShifting(-90);
	}
}
// End of ShProjection implementation
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// ShFreqProjection implementation

//////////////////////////////////////
// Construction:
ShFreqProjection::ShFreqProjection()
{
}

ShFreqProjection::~ShFreqProjection()
{
}

void ShFreqProjection::reset()
{
	setBands(0, 0, 0, gl_sph.getEleMaskDeg()); // -90);
}

///////////////////////////////////////
// Attributes:
void ShFreqProjection::setBands(int nBands, int mBands, int M, double dEleMaskDeg)
{
	m_sph.setShifting(dEleMaskDeg);
	m_shps.resize(M);
	for (int i = 0; i < M; i++)
	{
		m_shps[i].setBands(nBands, mBands, dEleMaskDeg);
	}
}

bool ShFreqProjection::canEvaluate() const
{
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		return m_shps[i].canEvaluate();
	}

	return false;
}

///////////////////////////
// Operations:
ShFreqProjection& ShFreqProjection::operator=(const ShFreqProjection& a)
{
	m_shps.resize(a.m_shps.size());
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		m_shps[i] = a.m_shps[i];
	}
	m_sph = a.m_sph;

	return *this;
}

void ShFreqProjection::averageWeight(const ShFreqProjection& a, double dWeight)
{
	if (m_shps.size() != a.m_shps.size())
	{
		m_shps.resize(a.m_shps.size());
	}

	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		m_shps[i].averageWeight(a.m_shps[i], dWeight);
	}
}

void ShFreqProjection::appendEqs(const ZenAz& za, double f, double dPh, double weight, Matrix& N, Vector& U)
{
	int nBands = m_shps[0].m_nBands;
	int mBands = m_shps[0].m_mBands;
	//double theta = (90 - ele) * PI / 180;
	//double fi = (90-az) * PI / 180;
	ThetaPhi tp(za);

	for (int k0 = 0; k0 < (int)m_shps.size(); k0++)
	{
		for (int l0 = 0; l0 < nBands; l0++)
		{
			for (int m0 = -std::min(l0, mBands - 1); m0 <= std::min(l0, mBands - 1); m0++)
			{
				//int nIndex1 = l * (l+1) + m;
				int nIndex0 = l0 < mBands ? l0 * (l0 + 1) + m0 :
					mBands * mBands + (l0 - mBands) * (2 * mBands - 1) + (m0 + mBands - 1);
				if (k0 > 0)
					nIndex0 += N.rows() / 2;

				double coef0 = m_sph.Y(l0, m0, tp.theta, tp.phi) * pow(f, k0);
				U[nIndex0] += weight * dPh * coef0;
				for (int k1 = 0; k1 < (int)m_shps.size(); k1++)
				{
					for (int l1 = 0; l1 < nBands; l1++)
					{
						for (int m1 = -std::min(l1, mBands - 1); m1 <= std::min(l1, mBands - 1); m1++)
						{
							//int nIndex2 = p * (p+1) + q;
							int nIndex1 = l1 < mBands ? l1 * (l1 + 1) + m1 :
								mBands * mBands + (l1 - mBands) * (2 * mBands - 1) + (m1 + mBands - 1);
							if (k1 > 0)
								nIndex1 += N.cols() / 2;

							if (nIndex1 > nIndex0)
								continue;

							double coef1 = m_sph.Y(l1, m1, tp.theta, tp.phi) * pow(f, k1);

							N(nIndex1, nIndex0) += weight * coef0 * coef1;
							//N(nIndex1, nIndex2) = N(nIndex2, nIndex1);
						}
					}
				}
			}
		}
	}
}

void ShFreqProjection::appendEqsSemi(const ZenAz& za, double f, double dPh, double weight, Matrix& N, Vector& U)
{
	int nBands = m_shps[0].m_nBands;
	int mBands = m_shps[0].m_mBands;
	//double theta = (90 - ele) * PI / 180;
	//double fi = (90-az) * PI / 180;
	ThetaPhi tp(za);

	for (int k0 = 0; k0 < (int)m_shps.size(); k0++)
	{
		for (int l0 = 0; l0 < nBands; l0++)
		{
			for (int m0 = -std::min(l0, mBands - 1); m0 <= std::min(l0, mBands - 1); m0++)
			{
				//int nIndex1 = l * (l+1) + m;
				//int nIndex0 = l0 < mBands ? l0 * (l0 + 1) + m0 :
				//	mBands * mBands + (l0 - mBands) * (2 * mBands - 1) + (m0 + mBands - 1);
				int nIndex0 = m_shps[0].getSemiIndex(l0, m0);
				if (nIndex0 < 0)
					continue;
				if (k0 > 0)
					nIndex0 += m_shps[0].getSemiSize()*k0; // N.rows() / 2;

				double coef0 = m_sph.Y(l0, m0, tp.theta, tp.phi) * pow(f, k0);
				U[nIndex0] += weight * dPh * coef0;
				for (int k1 = 0; k1 < (int)m_shps.size(); k1++)
				{
					for (int l1 = 0; l1 < nBands; l1++)
					{
						for (int m1 = -std::min(l1, mBands - 1); m1 <= std::min(l1, mBands - 1); m1++)
						{
							//int nIndex2 = p * (p+1) + q;
							//int nIndex1 = l1 < mBands ? l1 * (l1 + 1) + m1 :
							//	mBands * mBands + (l1 - mBands) * (2 * mBands - 1) + (m1 + mBands - 1);
							int nIndex1 = m_shps[0].getSemiIndex(l1, m1);
							if (nIndex1 < 0)
								continue;
							if (k1 > 0)
								nIndex1 += m_shps[0].getSemiSize() * k1; // N.cols() / 2;

							if (nIndex1 > nIndex0)
								continue;

							double coef1 = m_sph.Y(l1, m1, tp.theta, tp.phi) * pow(f, k1);

							N(nIndex1, nIndex0) += weight * coef0 * coef1;
							//N(nIndex1, nIndex2) = N(nIndex2, nIndex1);
						}
					}
				}
			}
		}
	}
}

void ShFreqProjection::setCoefs(Vector& X)
{
	for (int k = 0; k < (int)m_shps.size(); k++)
	{
		ShProjection& shp = m_shps[k];
		int nSubSize = shp.m_coefs.size();
		Vector x(nSubSize);
		X.getSubVector(x, k * nSubSize, (k + 1) * nSubSize);
		shp.setCoefs(x);
	}
}

void ShFreqProjection::setSemiCoefs(Vector& X)
{
	for (int k = 0; k < (int)m_shps.size(); k++)
	{
		ShProjection& shp = m_shps[k];
		int nSubSize = shp.getSemiSize(); // m_coefs.size();
		Vector x(nSubSize);
		X.getSubVector(x, k * nSubSize, (k + 1) * nSubSize);
		shp.setSemiCoefs(x);
	}
}

double ShFreqProjection::evaluate(const ThetaPhi& tp, double f) const
{
	double result = 0;
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		const ShProjection& shp = m_shps[i];
		result += shp.pcc(tp) * pow(f, i);
	}

	return result;
}

ShProjection ShFreqProjection::getProjection(double f) const
{
	ShProjection shp;
	shp.setBands(m_shps[0].m_nBands, m_shps[0].m_mBands, m_sph);
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		for (int j = 0; j < (int)shp.m_coefs.size(); j++)
		{
			shp.m_coefs[j] += m_shps[i].m_coefs[j] * pow(f, i);
		}
	}

	return shp;
}
// End of ShFreqProjection implementation
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// CShFreqFunction 

ShFreqFunction::ShFreqFunction(const SphericalHarmonics& sph_, double x_, double y_, double z_, int nts_, int nps_, const double* pfs_, const double* pfNorms_, int nfs_) {
	sph = sph_;
	x = x_;
	y = y_;
	z = z_;
	nts = nts_;
	nps = nps_;
	nfs = nfs_;
	dt = sph.getThetaMask() / nts;
	dp = 2 * PI / nps;
	pfs = pfs_;
	pfnorms = pfNorms_;
}


double ShFreqFunction::getMeas(int i, double& fNorm, ThetaPhi& tp)
{
	// nts = nps = nfs = 2
	// i   nt   np   nf
	// 0   0    0    0
	// 1   0    0    1
	// 2   0    1    0
	// 3   0    1    1
	// 4   1    0    0
	// 5   1    0    1
	// 6   1    1    0
	// 7   1    1    1
	int nt = i / (nps * nfs);
	i -= nt * (nps * nfs);
	int np = i / nfs;
	i -= np * nfs;
	int nf = i % nfs;

	tp = ThetaPhi(nt * dt, np * dp);
	fNorm = pfnorms[nf];
	double f = pfs[nf];

	const double c_WGS84 = 299792458.;

	double ct = ::cos(tp.theta);
	double st = ::sin(tp.theta);
	double cp = ::cos(tp.phi);
	double sp = ::sin(tp.phi);
	double xwl = x / (c_WGS84 / f);
	double ywl = y / (c_WGS84 / f);
	double zwl = z / (c_WGS84 / f);
	double phase = st * cp * xwl + st * sp * ywl + ct * zwl;
	return phase;
}
// End of ShFreqFunction
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// ShFreqProjection2 implementation

/////////////////////////////
// Construction:
ShFreqProjection2::ShFreqProjection2()
{
	m_nBands = 0;
	m_mBands = 0;
	m_K = 0;
	m_Q = 0;
}

ShFreqProjection2::~ShFreqProjection2()
{
}

void ShFreqProjection2::reset()
{
	setBands(0, 0, 0, 0, gl_sph); // -90);
}

///////////////////////////////////////
// Attributes:
void ShFreqProjection2::setBands(int nBands, int mBands, int K, int Q, double dEleMaskDeg)
{
	m_nBands = nBands;
	m_mBands = mBands;
	m_K = K;
	m_Q = Q;

	m_sph.setShifting(dEleMaskDeg);
	//m_sph.setShifting(-60);

	m_shps.resize(K);
	for (int i = 0; i < K; i++)
	{
		m_shps[i].setBands(nBands, mBands, dEleMaskDeg);
		//m_shps[i].setBands(nBands, mBands, -60);
	}
	m_0s.SetSize(Q);
	for (int i = 0; i < Q; i++)
		m_0s[i] = 0.0;
}

void ShFreqProjection2::setBands(int nBands, int mBands, int K, int Q, const SphericalHarmonics& sph)
{
	m_nBands = nBands;
	m_mBands = mBands;
	m_K = K;
	m_Q = Q;

	m_sph = sph; // .setShifting(dEleMaskDeg);

	m_shps.resize(K);
	for (int i = 0; i < K; i++)
	{
		m_shps[i].setBands(nBands, mBands, sph);
	}
	m_0s.SetSize(Q);
	for (int i = 0; i < Q; i++)
		m_0s[i] = 0.0;
}

void ShFreqProjection2::setPhaseCenter(const Point3d& pco, const double* pfs, const double* pfNorms, int nfs, int nTetas, int nPhis)
{
	ShFreqFunction f(m_sph, pco.x, pco.y, pco.z, nTetas, nPhis, pfs, pfNorms, nfs);

	project(f, m_nBands, m_mBands);
}

bool ShFreqProjection2::canEvaluate() const
{
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		return m_shps[i].canEvaluate();
	}

	return false;
}

int ShFreqProjection2::getMatrixSize() const
{
	int nHarmCoefs = m_mBands * m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1) - 1;
	return nHarmCoefs * m_K + m_Q;
}

///////////////////////////
// Operations:
ShFreqProjection2& ShFreqProjection2::operator=(const ShFreqProjection2& a)
{
	m_nBands = a.m_nBands;
	m_mBands = a.m_mBands;
	m_K = a.m_K;
	m_Q = a.m_Q;

	m_shps.resize(a.m_shps.size());
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		m_shps[i] = a.m_shps[i];
	}

	m_0s.SetSize(a.m_0s.GetSize());
	for (int i = 0; i < m_0s.GetSize(); i++)
	{
		m_0s[i] = a.m_0s[i];
	}

	m_sph = a.m_sph;

	return *this;
}

void ShFreqProjection2::appendEqs(const ZenAz& ea, double f, double dPh, double weight, Matrix& N, Vector& U)
{
	int nBands = m_shps[0].m_nBands;
	int mBands = m_shps[0].m_mBands;
	ThetaPhi tp(ea);

	HarmIndex hi0 = getFirstHarmIndex();

	do
	{
		int nIndex0 = getIndex(hi0); // q0, k0, l0, m0);

		double y0 = getMoment(f, tp, hi0); // q0, k0, l0, m0);

		N(nIndex0, nIndex0) += y0 * y0 * weight;
		U[nIndex0] += weight * dPh * y0;


		HarmIndex hi1 = hi0;
		while (getNextIndex(hi1)) //q1, k1, l1, m1))
		{
			int nIndex1 = getIndex(hi1); // q1, k1, l1, m1);
			double y1 = getMoment(f, tp, hi1); // q1, k1, l1, m1);

			N(nIndex0, nIndex1) += y0 * y1 * weight;
		}

	} while (getNextIndex(hi0)); // q0, k0, l0, m0));

}

void ShFreqProjection2::appendEqs(const ZenAz& ea, double f, double weight, Matrix& N)
{
	//ASSERT(m_shps.GetSize() > 0);
	//int nBands = m_shps[0].m_nBands;
	//int mBands = m_shps[0].m_mBands;
	//double theta = (90 - ele) * PI / 180;
	//double fi = (90 - az) * PI / 180;
	ThetaPhi tp(ea);

	int q0 = 0;
	int k0 = 0;
	int l0 = 0;
	int m0 = 0;
	do
	{
		int nIndex0 = getIndex(q0, k0, l0, m0);

		double y0 = getMoment(f, tp, q0, k0, l0, m0);

		N(nIndex0, nIndex0) += y0 * y0 * weight;

		int q1 = q0;
		int k1 = k0;
		int l1 = l0;
		int m1 = m0;
		while (getNextIndex(q1, k1, l1, m1))
		{
			int nIndex1 = getIndex(q1, k1, l1, m1);
			double y1 = getMoment(f, tp, q1, k1, l1, m1);

			N(nIndex0, nIndex1) += y0 * y1 * weight;
		}

	} while (getNextIndex(q0, k0, l0, m0));
}

int ShFreqProjection2::getIndex(const HarmIndex& hi) const
{
	return getIndex(hi.q, hi.k, hi.l, hi.m);
}

int ShFreqProjection2::getIndex(int q, int k, int l, int m) const
{
	if (l == 0)
	{
		ASSERT(m == 0);
		return q;
	}

	int nHarmCoefs = m_mBands * m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1) - 1;
	int nHarmIndex = l < m_mBands ? l * (l + 1) + m :
		m_mBands * m_mBands + (l - m_mBands) * (2 * m_mBands - 1) + (m + m_mBands - 1);
	nHarmIndex -= 1; // Zero coefficients are replaces by interhcannel biases, so decrease by 1

	int nIndex = m_Q + k * nHarmCoefs + nHarmIndex;

	return nIndex;
}

ShFreqProjection2::HarmIndex ShFreqProjection2::getHarmIndex(int i) const
{
	HarmIndex hi;

	if (i < m_Q)
	{
		hi.q = i;
		return hi;
	}
	hi.q = m_Q;
	int nLastIndex = m_Q;
	int nHarmCoefs = m_mBands * m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1) - 1;
	for (int k = 0; k < m_K; k++)
	{
		if (i >= nLastIndex + nHarmCoefs)
		{
			nLastIndex += nHarmCoefs;
			continue;
		}

		for (int l = 1; l < m_nBands; l++)
		{
			int nRowSize = std::min(l, m_mBands - 1) * 2 + 1; // Number of harmonics in current row of triangle
			if (i < nLastIndex + nRowSize)
			{
				hi.k = k;
				hi.l = l;
				hi.m = -std::min(l, m_mBands - 1) + (i - nLastIndex);
				return hi;
			}
			nLastIndex += nRowSize;
		}

		ASSERT(FALSE);
	}

	ASSERT(FALSE);
	throw new MatrixException("Index is out of range");
}

bool ShFreqProjection2::getNextIndex(HarmIndex& hi) const
{
	return getNextIndex(hi.q, hi.k, hi.l, hi.m);
}

bool ShFreqProjection2::getNextIndex(int& q, int& k, int& l, int& m) const
{
	if (q < m_Q)
		q++;

	if (q < m_Q)
		return true;

	if (m <= std::min(l, m_mBands - 1)) //m_mBands)
		m++;

	if (m <= std::min(l, m_mBands - 1)) //m_mBands)
		return true;

	if (l < m_nBands)
		l++;

	if (l < m_nBands)
	{
		m = -std::min(l, m_mBands - 1);
		return true;
	}

	l = 1;
	m = -l;

	if (k < m_K)
		k++;

	if (k < m_K)
		return true;

	return false;
}

ShFreqProjection2::HarmIndex ShFreqProjection2::getFirstHarmIndex() const
{
	HarmIndex hi;
	if (m_Q == 0)
	{
		hi.l = 1;
		hi.m = -1;
	}
	return hi;
}

double ShFreqProjection2::getMoment(double f, const ThetaPhi& tp, const HarmIndex& hi) const
{
	return getMoment(f, tp, hi.q, hi.k, hi.l, hi.m);
}

double ShFreqProjection2::getMoment(double f, const ThetaPhi& tp, int q, int k, int l, int m) const
{
	// Interchannel biases polynom goes first
	if (l == 0)
	{
		return pow(f, q);
	}

	double d = m_sph.Y(l, m, tp.theta, tp.phi) * pow(f, k);
	return d;
}

void ShFreqProjection2::getCoefs(Vector& X) const
{
	X.resize(getMatrixSize());
	for (int i = 0; i < m_Q; i++)
	{
		X[i] = m_0s[i];
	}

	int n = m_0s.GetSize();
	for (int k = 0; k < m_K; k++)
	{
		const ShProjection& s = m_shps[k];
		for (int i = 1; i < (int)s.m_coefs.size(); i++)
		{
			X[n++] = s.m_coefs[i];
		}
	}
}

void ShFreqProjection2::setCoefs(Vector& X)
{
	for (int i = 0; i < m_Q; i++)
	{
		m_0s[i] = X[i];
	}

	int nHarmSize = m_mBands * m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1);
	Vector xHarm(nHarmSize);

	int n = m_0s.GetSize();
	for (int k = 0; k < m_K; k++)
	{
		xHarm[0] = 0.0;
		for (int i = 1; i < nHarmSize; i++)
		{
			xHarm[i] = X[n + i - 1];
		}
		m_shps[k].setCoefs(xHarm);
		n += nHarmSize - 1;
	}
}

double ShFreqProjection2::evaluate(const ThetaPhi& tp, double f) const
{
	double result = 0;
	for (int i = 0; i < m_0s.GetSize(); i++)
	{
		result += m_0s[i] * pow(f, i);
	}

	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		const ShProjection& shp = m_shps[i];
		double p = pow(f, i);
		result += shp.pcc(tp) * p;
	}

	return result;
}

double ShFreqProjection2::evaluate(double theta, double fNorm) const
{
	double result = 0;
	for (int i = 0; i < m_0s.GetSize(); i++)
	{
		result += m_0s[i] * pow(fNorm, i);
	}

	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		const ShProjection& shp = m_shps[i];
		result += shp.evaluate(theta) * pow(fNorm, i);
	}

	return result;
}

double ShFreqProjection2::evaluateDot(const ThetaPhi& tp, double f) const
{
	double result = 0;
	for (int i = 1; i < m_0s.GetSize(); i++)
	{
		result += i * m_0s[i] * pow(f, i - 1);
	}

	for (int i = 1; i < (int)m_shps.size(); i++)
	{
		const ShProjection& shp = m_shps[i];
		double p = i * pow(f, i - 1);
		result += shp.pcc(tp) * p;
	}

	return result;
}

ShProjection ShFreqProjection2::getProjection(double f) const
{
	//ASSERT(m_shps.size() > 0);
	double d0 = 0;
	for (int i = 0; i < m_0s.GetSize(); i++)
	{
		d0 += m_0s[i] * pow(f, i);
	}

	ShProjection shp;
	shp.setBands(m_shps[0].m_nBands, m_shps[0].m_mBands, m_shps[0].m_sph);
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		for (int j = 0; j < (int)shp.m_coefs.size(); j++)
		{
			shp.m_coefs[j] += m_shps[i].m_coefs[j] * pow(f, i);
		}
	}

	shp.m_coefs[0] = d0;

	return shp;
}

void ShFreqProjection2::propagateCovariance(double fNorm, Matrix& cDst, const Matrix& cSrc) const
{
	if (m_shps.size() == 0)
		return;
	int nDstSize = m_shps[0].getSize();
	cDst.resize(nDstSize, nDstSize);
	cDst.init(0);

	int nSrcSize = getMatrixSize();

	CArray <double, double> pows;
	pows.SetSize(std::max(m_Q, m_K));
	for (int i = 0; i < pows.GetSize(); i++)
	{
		pows[i] = pow(fNorm, i);
	}

	Matrix tmp(nDstSize, nSrcSize);
	tmp.init(0);

	int nHarmSize = m_mBands * m_mBands + (m_nBands - m_mBands) * (2 * m_mBands - 1) - 1;

	// tmp = A * Covariance
	for (int i = 0; i < nDstSize; i++)
	{
		for (int j = 0; j < nSrcSize; j++)
		{
			if (i == 0)
			{
				for (int q = 0; q < m_Q; q++)
				{
					tmp(i, j) += pows[q] * cSrc(q, j);
				}
			}
			else
			{
				for (int k = 0; k < m_K; k++)
				{
					int nIndex = m_Q + k * nHarmSize + i - 1;
					tmp(i, j) += pows[k] * cSrc(nIndex, j);
				}
			}
		}
	}

	// cDst = A * Covariance * At = tmp * At
	for (int i = 0; i < nDstSize; i++)
	{
		for (int j = 0; j < nDstSize; j++)
		{
			if (j == 0)
			{
				for (int q = 0; q < m_Q; q++)
				{
					cDst(i, j) += tmp(i, q) * pows[q];
				}
			}
			else
			{
				for (int k = 0; k < m_K; k++)
				{
					int nIndex = m_Q + k * nHarmSize + j - 1;
					cDst(i, j) += tmp(i, nIndex) * pows[k];
				}
			}
		}
	}

}

void ShFreqProjection2::mix(const ShFreqProjection2& shp, double alfa)
{
	ASSERT(m_0s.GetSize() == shp.m_0s.GetSize());

	for (int i = 0; i < m_0s.GetSize(); i++)
	{
		m_0s[i] = (1.0 - alfa) * m_0s[i] + alfa * shp.m_0s[i];
	}

	//ASSERT(m_shps.GetSize() == shp.m_shps.GetSize());
	for (int i = 0; i < (int)m_shps.size(); i++)
	{
		m_shps[i].mix(shp.m_shps[i], alfa);
	}
}

bool ShFreqProjection2::project(ShFreqFunction& shf, int nBands, int mBands, Matrix* pN, Matrix* pC)
{
	setBands(nBands, mBands, m_K, m_Q, m_sph.getEleMaskDeg());

	int nMatrixSize = getMatrixSize();

	Matrix A(nMatrixSize, nMatrixSize);
	Vector Y(nMatrixSize);
	A.init(0);
	Y.init(0);
	for (int nObs = 0; nObs < shf.getMeasCount(); nObs++)
	{
		ThetaPhi tp;
		double f;
		double meas = shf.getMeas(nObs, f, tp);
		for (int nRow = 0; nRow < nMatrixSize; nRow++)
		{
			HarmIndex row = getHarmIndex(nRow);
			double rowMoment = getMoment(f, tp, row);
			Y[nRow] += meas * rowMoment;
			for (int nCol = 0; nCol < nMatrixSize; nCol++)
			{
				HarmIndex col = getHarmIndex(nCol);
				double colMoment = getMoment(f, tp, col);

				A(nRow, nCol) += rowMoment * colMoment;
			}
		}
	}

	Vector X(nMatrixSize);
	X = A.solve(Y);

	if (pN != nullptr)
	{
		*pN = A;
	}
	if (pC != nullptr)
	{
		*pC = A;
		pC->inverse();
	}

	setCoefs(X);
	return true;
}



////////////////////////////////
// Serialization:
void ShFreqProjection2::serialize(Archive& ar)
{
	DWORD dwVer = 2;
	if (ar.isStoring())
	{
		ar << dwVer;
		ar << m_nBands;
		ar << m_mBands;
		ar << m_K;
		ar << m_Q;
		int nSize = m_shps.size();
		ar << nSize;
		for (int i = 0; i < nSize; i++)
		{
			m_shps[i].serialize(ar);
		}
		nSize = m_0s.GetSize();
		ar << nSize;
		for (int i = 0; i < nSize; i++)
		{
			ar << m_0s[i];
		}

		m_sph.serialize(ar);
	}
	else
	{
		ar >> dwVer;
		ar >> m_nBands;
		ar >> m_mBands;
		ar >> m_K;
		ar >> m_Q;
		int nSize = 0;
		ar >> nSize;
		m_shps.resize(nSize);
		for (int i = 0; i < nSize; i++)
		{
			m_shps[i].serialize(ar);
		}
		nSize = 0;
		ar >> nSize;
		m_0s.SetSize(nSize);
		for (int i = 0; i < nSize; i++)
		{
			ar >> m_0s[i];
		}

		if (dwVer >= 2)
			m_sph.serialize(ar);
		else
			m_sph.setShifting(-90);
	}
}
// End of ShFreqProjection2 implementation
/////////////////////////////////////////////////////////////

