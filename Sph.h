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

#include "Archive.h"
#include "Angles.h"
#include "VecMat.h"

namespace nzg
{
	/////////////////////////////////////////////
	// Spherical harmonics
	class SphericalHarmonics
	{
	public:
		SphericalHarmonics() {
			// setShifting (-90)
			m_dK = 1.0;
			m_dH = 0;
			m_dEleMask = -90;
		}
		SphericalHarmonics(double eleMask) {
			setShifting(eleMask);
		}

		static double factorial(int n); // factorial n!
		static double factorial2(int n); // double factorial n!!
		static double factorialFact(int n, int m); // (n+m)! / (n-m)!
		static double minPlus(int n); // pow (-1, n)

		static double P(int l, int m, double x);
		double K(int l, int m) const;
		double Y(int l, int m, double theta, double phi) const;


		// Returns derivatives dP(cos(theta))/dtheta for given theta [radians]
		// given m and for different |l| <= lMax. 
		// Corresponding l and derivatives fill up arrays ders and ls
		void getLegendreDerivarives(int m, int lMax, double theta, std::vector<double>& ders, std::vector<int>& ls) const;

		// Integrals from 0 to theta0 by theta and from 0 to 2 * PI
		// of the following types: 
		//		Int (Ylm(theta, fi) * sin(theta)*cos(fi) dfi)
		double integYsincos(int l, int m, double theta0) const;
		//		Int (Ylm(theta, fi) * sin(theta)*sin(fi) dfi)
		double integYsinsin(int l, int m, double theta0) const;
		//		Int (Ylm(theta, fi) * cos(theta) dfi)
		double integYcos(int l, int m, double theta0) const;
		//		Int (Ylm(theta, fi) dfi)
		double integY(int l, int m, double theta0) const;

		SphericalHarmonics& operator=(const SphericalHarmonics& a);
		bool operator==(const SphericalHarmonics& a) const;
		bool operator!=(const SphericalHarmonics& a) const;

		// Shifting properties x = c_dK*xdash + c_dH
		double m_dK;
		double m_dH;
		double m_dEleMask;
		void setShifting(double eleMaskDeg);
		double getEleMaskDeg() const { return m_dEleMask; }
		double getThetaMask() const { return (90 - m_dEleMask) * PI / 180.0; }

		void serialize(Archive& ar);
	};

#ifdef _CCOMPLEX_CPP_
	//SphericalHarmonics gl_sph;
#else
	//extern SphericalHarmonics gl_sph;
#endif

	extern SphericalHarmonics gl_sph;

	// End of Spherical harmonics
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// ShFunction interface
	class ShFunction
	{
		// Construction:
	public:
		ShFunction() {}
		~ShFunction() {}

		// Overrides:
	public:
		virtual int getMeasCount() = 0;
		virtual double getMeas(int i, double* ptheta, double* pfi) = 0;
	};
	// End of ShFunction interface
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// ShProjection - a projection of measured data
	// onto spherical harmonics basic functions
	class ShProjection
	{
		// Construction:
	public:
		ShProjection();
		ShProjection(const ShProjection& a)
		{
			*this = a;
		}
		~ShProjection();
		void reset();

		struct UseHarms
		{
			UseHarms() {
				nBands = mBands = 0;
			}
			UseHarms(const UseHarms& a) {
				*this = a;
			}
			UseHarms(int nBands_, int mBands_, bool bUse_) {
				setBands(nBands_, mBands_, bUse_);
			}

			int mBands;
			int nBands;
			std::vector<bool> bUse;

			bool isEmpty() const {
				return getSize() == 0;
			}
			int getSize() const {
				return ShProjection::getSize(nBands, mBands);
			}
			void setBands(int nBands_, int mBands_, bool bUse_) {
				nBands = nBands_;
				mBands = mBands_;
				bUse.resize(getSize(), bUse_);
			}
			bool get(int l, int m) const {
				int index = ShProjection::getIndex(nBands, mBands, l, m);
				return bUse[index];
			}
			void set(int l, int m, bool use) {
				int index = ShProjection::getIndex(nBands, mBands, l, m);
				bUse[index] = use;
			}

			UseHarms& operator=(const UseHarms& a) {
				nBands = a.nBands;
				mBands = a.mBands;
				bUse = a.bUse;
				return *this;
			}
		};

		// Attributes:
	public:
		int m_nBands;
		int m_mBands;
		std::vector <double> m_coefs;
		Matrix m_q; // Cofactor matrix (Invert(N))
		double m_sigma; // Estimated variance of unit weight
		SphericalHarmonics m_sph;
		void setBands(int nBands, int mBands, double dEleMaskDeg);
		void setBands(int nBands, int mBands, const SphericalHarmonics& sph);
		bool canEvaluate() const;

		// Get the position of coeficient (l, m) in m_dCoefs
		int getIndex(int l, int m) const {
			return getIndex(m_nBands, m_mBands, l, m);
		}

		static int getIndex(int nBands, int mBands, int l, int m) {
			ASSERT(0 <= l && l < nBands);
			ASSERT(::abs(m) <= l);
			ASSERT(::abs(m) < mBands);

			int nIndex = l < mBands ? l * (l + 1) + m :
				mBands * mBands + (l - mBands) * (2 * mBands - 1) + (m + mBands - 1);

			return nIndex;
		}

		int getSemiIndex(int l, int m) const {
			return getSemiIndex(m_nBands, l, m);
		}

		static int getSemiIndex(int nBands, int l, int m) {
			ASSERT(0 <= l && l < nBands);
			ASSERT(::abs(m) <= l);
			if ((l + m) % 2 != 0)
				return -1;

			int nIndex = getSemiSize(l) + (l + m) / 2;

			return nIndex;
		}

		// Compute number of coefficient basing on m_nBands and m_mBands (equal to m_dCoefs.GetSize())
		int getSize() const {
			return getSize(m_nBands, m_mBands);
		}
		static int getSize(int nBands, int mBands) {
			int nCoefs = mBands * mBands + (nBands - mBands) * (2 * mBands - 1);
			return nCoefs;
		}
		int getSemiSize() const {
			return getSemiSize(m_nBands);
		}
		static int getSemiSize(int nBands) {
			return (nBands + 1)* (nBands) / 2;
		}

		void setCoef(int l, int m, double c);
		double getCoef(int l, int m) const;

		void getYmn(Vector& Y, double theta, double phi) const; // Fill vector Y with Ymn(theta, phi)
		void getY0n(Vector& Y, double theta) const; // Fill vector Y with Y0n(theta)

		// Project ideal pattern into us. ptCnt is in meters
		void setPhaseCenter(const Point3d& ptCnt, double wl);

		// Operations:
	public:
		ShProjection& operator=(const ShProjection& a);
		void averageWeight(const ShProjection& a, double dWeight);
		void mix(const ShProjection& a, double alfa); // this = (1-alfa)*this + alfa*a
		void project(ShFunction& shf, int nBands, int mBands);
		ShProjection project(int nBands, int mBands, const SphericalHarmonics& sph, double eleMask) const; // rebuild this projection into (nBands, mBands)
		void appendEqs(double ele, double az, double dRotAngle, double dPh, double weight, Matrix& N, Vector& U) const;
		void appendEqs(double ele, double az, double dPh, double weight, Matrix& N, Vector& U) const;
		void appendEqs2(double ele, double az, double dPh, double weight, Matrix& N, Vector& U);
		void appendEqs(const ZenAz& za, double dPh, double weight, Matrix& N, Vector& U) const;
		void appendEqsSemi(const ZenAz& za, double dPh, double weight, Matrix& N, Vector& U) const;
		void setCoefs(Vector& X);
		void setSemiCoefs(Vector& X);

		double pcc(const ThetaPhi& tp, const ShProjection::UseHarms* pUseHarms = nullptr) const;
		double pcv(const ThetaPhi& tp, const Point3d& pco) const;
		//double evaluate(double theta, double phi) const;
		double evaluateWo0(const ThetaPhi& tp) const;
		double evaluate(double theta) const;
		double evaluate(const ThetaPhi& tp) const;
		//double evaluateEleAz(const CEleAz& ea) const;
		double evaluatePcvOnly(double theta, Point3d* pptCnt = NULL) const;
		friend ShProjection operator-(const ShProjection& a, const ShProjection& b);
		Point3d calcOffset(double dEleMask, ZenAz zaStep) const;

		void dump() const;

		// Serialization:
	public:
		void serialize(Archive& ar);
	};

	// End of ShProjection interface
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// ShFreqProjection interface
	// Shperical harmonics coefficients depends on frequency as polynoms
	class ShFreqProjection
	{
		// Construction:
	public:
		ShFreqProjection();
		ShFreqProjection(const ShFreqProjection& a)
		{
			*this = a;
		}
		~ShFreqProjection();
		void reset();

		// Attributes:
	public:
		std::vector <ShProjection> m_shps;
		SphericalHarmonics m_sph;
		void setBands(int nBands, int mBands, int M, double dEleMaskDeg);
		bool canEvaluate() const;
		int getMatrixSize() const {
			if (m_shps.size() == 0)
				return 0;
			return m_shps[0].getSize() * m_shps.size();
		}
		int getSemiMatrixSize() const {
			if (m_shps.size() == 0)
				return 0;
			return m_shps[0].getSemiSize() * m_shps.size();
		}

		// Operations:
	public:
		ShFreqProjection& operator=(const ShFreqProjection& a);
		void averageWeight(const ShFreqProjection& a, double dWeight);
		void appendEqs(const ZenAz& ea, double f, double dPh, double weight, Matrix& N, Vector& U);
		void appendEqsSemi(const ZenAz& ea, double f, double dPh, double weight, Matrix& N, Vector& U);
		void setCoefs(Vector& X);
		void setSemiCoefs(Vector& X);
		double evaluate(const ThetaPhi& tp, double f) const;
		ShProjection getProjection(double f) const;
	};
	// End of ShFreqProjection interface
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// ShFreqFunction
	class ShFreqFunction
	{
		// Construction:
	public:
		ShFreqFunction(const SphericalHarmonics& sph_, double x_, double y_, double z_, int nts_, int nps_, const double* pfs_, const double* pfnorms_, int nfs_);
		~ShFreqFunction() {}

		// Attributes for ideal phase pattern
	public:
		SphericalHarmonics sph;
		double x, y, z; // in meters
		int nts; // theta steps
		int nps; // phi steps
		int nfs; // number of frequencies values 
		double dt; // Theta step
		double dp; // Phi step
		const double* pfs; // Pointer to frequncies array [Hz] used to calc wavelength
		const double* pfnorms; // Pointer to normalized frequencies for projection

		// Overrides:
	public:
		virtual int getMeasCount() {
			return nts * nps * nfs;
		}
		// Frequncy in Hertz
		virtual double getMeas(int i, double& fNorm, ThetaPhi& tp);
	};
	// End of ShFreqFunction interface
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// CShFreqProjection2 interface
	// Shperical harmonics coefficients depends on frequency as polynoms of degree M,
	// and in addition zero coefficient depends on frequecny as polynoms of degree Q,
	// that can be higher than M. It helps describing GLONASS interchannel biases.
	class ShFreqProjection2
	{
		// Construction:
	public:
		ShFreqProjection2();
		ShFreqProjection2(const ShFreqProjection2& a)
		{
			*this = a;
		}
		~ShFreqProjection2();
		void reset();

		struct HarmIndex
		{
			HarmIndex() {
				q = k = l = m = 0;
			}
			HarmIndex(int q_, int k_, int l_, int m_) {
				q = q_;
				k = k_;
				l = l_;
				m = m_;
			}
			int q; // Zero frequency harmonics index
			int k; // Frequency index for non zero harmonics
			int l; // Zenith angle index
			int m; // Azimuth index
		};

		// Attributes:
	public:
		int m_nBands;
		int m_mBands;
		int m_K; // Polynom power for harmonics. (Frequency coefficients for harmonics except of 0)
		int m_Q; // Frequency coefficients for harmonics 0
		SphericalHarmonics m_sph;
		std::vector <ShProjection> m_shps;
		CArray <double, double> m_0s;
		void setBands(int nBands, int mBands, int K, int Q, double dEleMaskDeg);
		void setBands(int nBands, int mBands, int K, int Q, const SphericalHarmonics& sph);

		// ptPco is in meters, pfs - frequencies for which to evaluate, nfs - number of freqs
		void setPhaseCenter(const Point3d& ptPco, const double* pfs, const double* pfNorms, int nfs, int nTetas, int nPhis);
		bool canEvaluate() const;
		int getMatrixSize() const;

		// Operations:
	public:
		ShFreqProjection2& operator=(const ShFreqProjection2& a);
		void appendEqs(const ZenAz& ea, double f, double dPh, double weight, Matrix& N, Vector& U);
		void appendEqs(const ZenAz& ea, double f, double weight, Matrix& N);

		// Index operations:
		int getIndex(const HarmIndex& hi) const;
		HarmIndex getHarmIndex(int i) const;
		int getIndex(int q, int k, int l, int m) const;
		bool getNextIndex(HarmIndex& hi) const;
		bool getNextIndex(int& q, int& k, int& l, int& m) const;
		HarmIndex getFirstHarmIndex() const;

		// Get Y(l, m, theta, fi) * pow(f, k)
		double getMoment(double f, const ThetaPhi& tp, const HarmIndex& hi) const;
		double getMoment(double f, const ThetaPhi& tp, int q, int k, int l, int m) const;

		void getCoefs(Vector& X) const;
		void setCoefs(Vector& X);
		double evaluate(const ThetaPhi& tp, double f) const;
		double evaluate(double theta, double f) const;

		// Evaluate frequency derivative
		double evaluateDot(const ThetaPhi& tp, double f) const;
		ShProjection getProjection(double fNorm) const;
		void propagateCovariance(double fNorm, Matrix& cDst, const Matrix& cSrc) const;
		void mix(const ShFreqProjection2& shp, double alfa); // Mix this widh shp as '(1-alfa) * this + alfa * shp'

		bool project(ShFreqFunction& shf, int nBands, int mBands, Matrix* pN = nullptr, Matrix* pC = nullptr);

		// Serialization:
	public:
		void serialize(Archive& ar);
	};
	// End of ShFreqProjection2 interface
	/////////////////////////////////////////////


}