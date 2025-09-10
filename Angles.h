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

#include "Points.h"
#include "Archive.h"
#include "Tools.h"

namespace nzg
{
	//constexpr double PI = 3.141592653589793238462643383280;
	//const double DEG2RAD = (PI / 180.0);
	//const double RAD2DEG = (180.0 / PI);


	///////////////////////////////////////////////////////////////////////////////
	// AngleType interface
	/// @ingroup geodeticgroup
	//@{

	/** Because the angle can be initialized via a variety of
	 * different values that are the same type, we use this enum
	 * to indicate in the constructor what type is being
	 * passed. */
	enum class AngleType
	{
		Unknown,    ///< Uninitialized value.
		Rad,        ///< Value is in radians.
		Deg,        ///< Value is in degrees.
		SemiCircle, ///< Value is in semi-circles (aka half-cycles).
		Sin,        ///< Value is the sine of the angle.
		Cos,        ///< Value is the cosine of the angle.
		Last        ///< Used to create an iterator.
	};

	/** Define an iterator so C++11 can do things like
	 * for (AngleType i : AngleTypeIterator()) */
	typedef EnumIterator<AngleType, AngleType::Unknown, AngleType::Last> AngleTypeIterator;

	namespace StringUtils
	{
		/// Convert a AngleType to a whitespace-free string name.
		std::string asString(AngleType e) noexcept;
		/// Convert a string name to an AngleType
		AngleType asAngleType(const std::string& s) noexcept;
	}

	//@}
	// End of AngleType interface
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// AngleReduced interface
	  /// @ingroup geodeticgroup
	  //@{

	  /** Wrap data for just the sine and cosine of an angle.
	   * Intended to be used in geometry where the actual angle isn't
	   * used as a term itself, but only sine and/or cosine are.  This
	   * saves a bit of computing time by storing the sin and cos of
	   * an angle when such values are used repeatedly in geometric
	   * equations, rather than recomputing the value multiple times
	   * over the course of a method/function. */
	class AngleReduced
	{
	public:
		/// Initialize all data to NaN.
		AngleReduced();

		/** Initialize using an angular value.  The sine and cosine
		 * will be derived from each other using the Pythagorean
		 * identity sin**2+cos**2=1.
		 * @param[in] v The value to set.
		 * @param[in] t The type of datum contained in v.
		 * @post sin and cos are set. */
		AngleReduced(double v, AngleType t)
		{
			setValue(v, t);
		}

		/** Initialize using the sine and cosine values.
		 * @param[in] s The sine value of the angle being represented.
		 * @param[in] c The cosine value of the angle being represented.
		 * @post sin and cos are set. */
		AngleReduced(double s, double c)
			: sine(s), cosine(c)
		{}

		/// Standard equality operator.
		inline bool operator==(const AngleReduced& right) const
		{
			return ((sine == right.sine) && (cosine == right.cosine));
		}

		/** Set all values from a single angle datum.
		 * @param[in] v The value to set.
		 * @param[in] t The type of datum contained in v.
		 * @post sin and cos are set. */
		void setValue(double v, AngleType t);

		/// Get the sine of this angle.
		inline double sin() const
		{
			return sine;
		}

		/// Get the cosine of this angle.
		inline double cos() const
		{
			return cosine;
		}

		/// Return a string containing the data separated by commas (sin,cos).
		inline std::string asString() const;

	protected:
		double sine;   ///< The sine of the angle.
		double cosine; ///< The cosine of the angle.
	}; // class AngleReduced

	inline std::ostream& operator<<(std::ostream& s, const AngleReduced& a)
	{
		s << std::setprecision(20) << "sin:" << a.sin() << ",cos:" << a.cos();
		return s;
	}
	// End of AngleReduced interface
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Angle interface
	  /// @ingroup geodeticgroup
	  //@{

	  /** Wrap data for an angle, including the angle in degrees,
	   * radians, and the sine and cosine of the angle.  This is done
	   * for convenience when doing geometric computations that
	   * require the use of a lot of trigonometry. */
	class Angle : public AngleReduced
	{
	public:
		/// Initialize all data to NaN.
		Angle();

		/** Initialize from a pair of sin/cos values, filling out the rest.
		 * @param[in] s The sine of the angle.
		 * @param[in] c The cosine of the angle.
		 * @post rad, deg, sin and cos are all set. */
		explicit Angle(double s, double c);

		/** Initialize from a single value, filling out the rest.
		 * @param[in] v The value to set.
		 * @param[in] t The type of datum contained in v.
		 * @post rad, deg, sin and cos are all set. */
		explicit Angle(double v, AngleType t)
		{
			setValue(v, t);
		}

		/** Set all values from a single angle datum.
		 * @param[in] v The value to set.
		 * @param[in] t The type of datum contained in v.
		 * @post rad, deg, sin and cos are all set. */
		void setValue(double v, AngleType t);

		/** Basic difference.
		 * @param[in] right The angle to subtract from this one.
		 * @return The resulting angle when differenced with this one. */
		Angle operator-(const Angle& right) const;

		/** Numeric negation of angle. Changes radians to -radians
		 * then recomputes the rest. */
		Angle operator-() const
		{
			return Angle(-radians, AngleType::Rad);
		}

		/** Basic addition.
		 * @param[in] right The angle to add to this one.
		 * @return The resulting angle when added to this one. */
		Angle operator+(const Angle& right) const;

		/// Get the angle in radians.
		inline double rad() const
		{
			return radians;
		}

		/// Get the angle in degrees.
		inline double deg() const
		{
			return degrees;
		}

		/// Get the angle in semi-circles (aka half-cycles).
		inline double semicirc() const
		{
			return semicircles;
		}

		/// Get the tangent of this angle.
		inline double tan() const
		{
			return tangent;
		}

		/** Return a string containing the data separated by commas
		 * (rad,deg,sin,cos). */
		std::string asString() const;

	protected:
		double radians; ///< The angle in radians.
		double degrees; ///< The angle in degrees.
		double tangent; ///< The tangent of the angle.
		double semicircles; ///< The angle in semi-circles (aka half-cycles).

	private:
		// Disable a bunch of constructors so unexpected behavior is
		// avoided in C++.  In python, all bets are off,
		// unfortunately.
		explicit Angle(int, int);
		explicit Angle(long, long);
		explicit Angle(unsigned, unsigned);
		explicit Angle(unsigned long, unsigned long);
	}; // class Angle


	inline std::ostream& operator<<(std::ostream& s, const Angle& a)
	{
		s << std::setprecision(20) << "rad:" << a.rad() << ",deg:" << a.deg()
			<< ",semi-circles:" << a.semicirc() << ",sin:" << a.sin() << ",cos:"
			<< a.cos() << ",tan:" << a.tan();
		return s;
	}

	//@}
	// End of Angle interface
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// ZenAz and ThetaPhi implementation
	class ThetaPhi;
	class EleAz;

	//
	// ZenAz - zenith and azimuth angles in spherical coord system
	// angles are in degtrees
	// Zenith angle is in [0, 180], counting from zenith direction
	// Azimuth angle is in [0, 360], counting clockwise from Y axis of the corresponding Decart coords
	//
	class ZenAz
	{
	public:
		ZenAz() : zen(0), az(0) {}
		ZenAz(const ZenAz& a) {
			*this = a;
		}
		ZenAz(double zen_, double az_) : zen(zen_), az(az_) {}

	public:
		double zen; // Zenith angle (deg)
		double az;	// Azimuth angle (deg)

	public:
		ZenAz& operator=(const ZenAz& a) {
			zen = a.zen;
			az = a.az;
			return *this;
		}

		// Direction cosines
		Point3d dirCos() const {
			double sinz = ::sin(zen * DEG2RAD);
			return Point3d(sinz * ::sin(az * DEG2RAD), sinz * ::cos(az * DEG2RAD), ::cos(zen * DEG2RAD));
		}
		double getTheta() const {
			return DEG2RAD * zen;
		}
		double getPhi() const {
			double phiDeg = 90 - az;
			return DEG2RAD * phiDeg;
		}
		inline operator ThetaPhi () const;
		inline operator EleAz() const;

		bool operator== (const ZenAz& za) const {
			return az == za.az && zen == za.zen;
		}

		friend Archive& operator<<(Archive& ar, const ZenAz& za) {
			ar << za.zen;
			ar << za.az;
			return ar;
		}
		friend Archive& operator>>(Archive& ar, ZenAz& za) {
			ar >> za.zen;
			ar >> za.az;
			return ar;
		}
	};

	/////////////////////////////////////////////
	// EleAz
	class EleAz
	{
	public:
		EleAz() : ele(0), az(0) {}
		EleAz(const EleAz& a) {
			*this = a;
		}
		EleAz(double ele_, double az_) : ele(ele_), az(az_) {}
		EleAz(const Point3d& ptEnu) {
			az = atan2(ptEnu.e, ptEnu.n) * RAD2DEG;
			double r = ::sqrt(ptEnu.n * ptEnu.n + ptEnu.e * ptEnu.e);
			ele = atan2(ptEnu.u, r) * RAD2DEG;
		}

	public:
		double ele; // Zenith angle (deg)
		double az;	// Azimuth angle (deg)

	public:
		EleAz& operator=(const EleAz& a) {
			ele = a.ele;
			az = a.az;
			return *this;
		}

		// Direction cosines
		Point3d dirCos() const {
			double cose = ::cos(ele * DEG2RAD);
			return Point3d(cose * ::sin(az * DEG2RAD), cose * ::cos(az * DEG2RAD), ::sin(ele * DEG2RAD));
		}
		double getTheta() const {
			return DEG2RAD * (90 - ele);
		}
		double getPhi() const {
			double phiDeg = 90 - az;
			return DEG2RAD * phiDeg;
		}
		double getZen() const {
			return 90 - ele;
		}

		inline operator ThetaPhi () const;
		operator ZenAz() const {
			return ZenAz(90 - ele, az);
		}

		bool operator== (const EleAz& za) const {
			return az == za.az && ele == za.ele;
		}

		friend Archive& operator<<(Archive& ar, const EleAz& ea) {
			ar << ea.ele;
			ar << ea.az;
			return ar;
		}
		friend Archive& operator>>(Archive& ar, EleAz& ea) {
			ar >> ea.ele;
			ar >> ea.az;
			return ar;
		}
	};

	/////////////////////////////////////////////
	// ThetaPhi - zenith angle theta and phi = atan2(y/x) 
	// always in radians
	class ThetaPhi
	{
		// Construction:
	public:
		ThetaPhi() { theta = phi = 0; }
		ThetaPhi(double t, double p) { theta = t; phi = p; }
		ThetaPhi(const Point3d& ptEnu) {
			double r = ::sqrt(ptEnu.x * ptEnu.x + ptEnu.y * ptEnu.y);
			theta = atan2(r, ptEnu.z);
			phi = atan2(ptEnu.y, ptEnu.x);
		}
		ThetaPhi(const ZenAz& za) : theta (za.zen* DEG2RAD), phi ((90 - za.az)* DEG2RAD) {
		}

		~ThetaPhi() {}

		// Attributes:
	public:
		double theta; // zenith angle in radians 0 - zenith, PI - nadir
		double phi; // phi angle in radians = atan(y/x)

		double getZen() const {
			return RAD2DEG * theta;
		}
		double getEle() const {
			return RAD2DEG * (PI / 2 - theta);
		}
		double getAz() const {
			return RAD2DEG * (PI / 2 - phi);
		}
		Point3d unitPoint() const;

		inline operator ZenAz () const;
		inline operator EleAz() const;
	};

	inline ThetaPhi::operator ZenAz() const {
		return ZenAz(RAD2DEG * theta, RAD2DEG*(PI/2 - phi));
	}

	inline ZenAz::operator ThetaPhi() const {
		return ThetaPhi(getTheta(), getPhi());
	}

	inline ThetaPhi::operator EleAz() const {
		return EleAz(RAD2DEG * (PI/2 - theta), RAD2DEG * (PI / 2 - phi));
	}

	inline EleAz::operator nzg::ThetaPhi() const {
		return ThetaPhi(getTheta(), getPhi());
	}

	// End of ZenAz and ThetaPhi implementation
	///////////////////////////////////////////////////////////////////////////////


} // End of nzg namespace


namespace std
{
	inline double sin(nzg::AngleReduced x)
	{
		return x.sin();
	}
	inline double cos(nzg::AngleReduced x)
	{
		return x.cos();
	}
	inline double sin(nzg::Angle x)
	{
		return x.sin();
	}
	inline double cos(nzg::Angle x)
	{
		return x.cos();
	}
	inline double tan(nzg::Angle x)
	{
		return x.tan();
	}
}
