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

#include "Archive.h"
#include "NzgException.h"

namespace nzg
{
	NEW_EXCEPTION_CLASS(GeometryException, nzg::Exception);

	///////////////////////////////////////////////////////////////////
	// Point2d interface
	class Point2d
	{
	public:
		Point2d() : x(0), y(0) {
		}
		Point2d(const Point2d& a) {
			*this = a;
		}
		Point2d(double x_, double y_) : x(x_), y(y_) {
		}

	public:
		double x;
		double y;
		
	// Operations:
	public:
		Point2d& operator=(const Point2d& a) {
			x = a.x;
			y = a.y;
			return *this;
		}

		friend Point2d operator+(const Point2d& a, const Point2d& b) {
			return Point2d(a.x + b.x, a.y + b.y);
		}
		friend Point2d operator-(const Point2d& a, const Point2d& b) {
			return Point2d(a.x - b.x, a.y - b.y);
		}
		friend Point2d operator*(const Point2d& a, double d) {
			return Point2d(a.x*d, a.y*d);
		}

		friend Archive& operator<<(Archive& ar, const Point2d& pt) {
			ar.write(&pt.x, sizeof(double));
			ar.write(&pt.y, sizeof(double));
			return ar;
		}
		friend Archive& operator>>(Archive& ar, Point2d& pt) {
			ar.read(&pt.x, sizeof(double));
			ar.read(&pt.y, sizeof(double));
			return ar;
		}

	};
	// End of Point2d interface
	///////////////////////////////////////////////////////////////////

	//////////////////////////////////////////
	// Size2d - a 2D floating point size.
	class Size2d // : public tagF2DSIZE
	{
		// Constructors:
	public:
		Size2d() : cx(0), cy(0) {}
		Size2d(double cx_, double cy_) : cx(cx_), cy(cy_) {}
		Size2d(const Point2d& pt) : cx(pt.x), cy(pt.y) {}
		Size2d(const CSize& s) : cx(s.cx), cy(s.cy) {}

		// Attributes:
	public:
		double cx;
		double cy;

		// Operations:
	public:
		bool operator==(const Size2d& s) const { return cx == s.cx && cy == s.cy; }
		bool operator!=(const Size2d& s) const { return cx != s.cx || cy != s.cy; }
		Size2d operator+=(const Size2d& s) { cx += s.cx, cy += s.cy; return *this; }
		Size2d operator-=(const Size2d& s) { cx -= s.cx, cy -= s.cy; return *this; }
		Size2d operator/=(double d) {
			cx /= d;
			cy /= d;
			return *this;
		}
		Size2d operator*=(double d) {
			cx *= d;
			cy *= d;
			return *this;
		}

		friend Size2d operator+(const Size2d& a, const Size2d& b) { return Size2d(a.cx + b.cx, a.cy + b.cy); }
		friend Size2d operator-(const Size2d& a, const Size2d& b) { return Size2d(a.cx - b.cx, a.cy - b.cy); }
		friend Point2d operator+(const Point2d& a, const Size2d& b) { return Point2d(a.x + b.cx, a.y + b.cy); }

		// Some geom. helpers:
		double area() const { return cx * cy; }
		double diag() const { return sqrt(cx * cx + cy * cy); }

		// Serialization:
	public:
		friend Archive& operator<<(Archive& ar, const Size2d& size) {
			ar.write(&size.cx, sizeof(double));
			ar.write(&size.cy, sizeof(double));
			return ar;
		}

		friend Archive& operator>>(Archive& ar, Size2d& size) {
			ar.read(&size.cx, sizeof(double));
			ar.read(&size.cy, sizeof(double));
			return ar;
		}

	};
	// End of Size2d interface
	//////////////////////////////////////////

	//////////////////////////////////////////
	// Rect2d
	class Rect2d
	{
		// Construction:
	public:
		Rect2d() {}
		Rect2d(const Point2d& pt_, const Size2d& s_) : pt(pt_), s(s_) {}
		Rect2d(const Point2d& pt0, const Point2d& pt1) : pt(pt0), s(pt1 - pt0) {}
		Rect2d(double x1, double y1, double x2, double y2) : pt(x1, y1), s(x2 - x1, y2 - y1) {}

		// Attributes:
	public:
		Size2d size() const { return s; }
		Point2d center() const { return Point2d(pt.x + s.cx / 2, pt.y + s.cy / 2); }
		double area() const { return s.area(); }
		bool contains(const Point2d& p) const {
			return pt.x <= p.x && p.x <= pt.x + s.cx
				&& pt.y <= p.y && p.y <= pt.y + s.cy;
		}

		// Operators:
	public:

		// Operations:
	public:
		Rect2d& normalize(void) {
			if (s.cx < 0)
			{
				pt.x += s.cx;
				s.cx = -s.cx;
			}
			if (s.cy < 0)
			{
				pt.y += s.cy;
				s.cy = -s.cy;
			}
			return *this;
		}
		Rect2d& stretch(double times) {
			s *= times;
		}
		double width(void) const { return s.cx; }
		double height(void) const { return s.cy; }
		double left() const { return pt.x; }
		double right() const { return pt.x + s.cx; }
		double top() const { return pt.y + s.cy; }
		double bottom() const {	return pt.y; }
		Point2d topRight() const {	return pt + s;	}
		Point2d corner(int i) const {
			return i == 0 ? Point2d(left(), bottom()) :
				i == 1 ? Point2d(left(), top()) :
				i == 2 ? Point2d(right(), top()) :
				i == 3 ? Point2d(right(), bottom()) :
				Point2d(NAN, NAN);
		}

		// Data members:
	public:
		Point2d pt;
		Size2d s;

		// Serialization:
	public:
		friend Archive& operator<<(Archive& ar, const Rect2d& r) {
			ar << r.pt;
			ar << r.pt + r.s;
		}
		friend Archive& operator>>(Archive& ar, Rect2d& r) {
			ar >> r.pt;
			Point2d pt;
			ar >> pt;
			r.s = pt - r.pt;
			return ar;
		}
	};
	// End of CF2DRect.
	//////////////////////////////////////////

	//////////////////////////////////////////
	// Line2d interface
	class Line2d
	{
		// Construction:
	public:
		Line2d() {}
		Line2d(const Point2d& ptStart_, const Point2d& ptEnd_)	{
			ptStart = ptStart_;
			ptEnd = ptEnd_;
			dir = ptEnd - ptStart;
		}

		// Attributes:
	public:
		Point2d ptStart;
		Point2d ptEnd;
		Size2d dir;

		// Operations:
	public:
		double getLen() const {
			return dir.diag();
		}

		// t - parameter. 
		// If t = 0.5 then returns the middle point, 
		//    t=0 returns ptStart, 
		//    t = 1 returns ptEnd, 
		//    t<0 returns points before ptStart, 
		//    t>1 returns points after ptEnd
		Point2d getPoint(double t) const {
			return Point2d(dir.cx * t, dir.cy * t) + ptStart;
		}

		// t - parameter which defines the point where the perpendicular crosses this line (see getPoint(t))
		// Returns line which ptStart is the cross point, ptEnd distance from ptStart is len. 
		// If len > 0 then 
		Line2d getPerp(double t, double len = 1) const {
			Point2d pt0 = getPoint(t);
			Size2d size(dir.cy, -dir.cx);
			size *= (len / getLen());
			Point2d pt1 = pt0 + size;
			return Line2d(pt0, pt1);
		}
	};
	// End of Line2d interface
	//////////////////////////////////////////

	//////////////////////////////////////////
	// Point3d interface
	class Point3d
	{
	public:
		Point3d() : x(0), y(0), z(0) {
		}
		Point3d(const Point3d& a) {
			*this = a;
		}
		Point3d(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {
		}
		void reset() {
			(*this) = Point3d(NAN, NAN, NAN);
		}

	public:
		union
		{
			struct
			{
				double x;
				double y;
				double z;
			};

			struct
			{
				double b;
				double l;
				double h;
			};

			struct
			{
				double e;
				double n;
				double u;
			};
		};

		// Operations:
	public:
		Point3d& operator=(const Point3d& a) {
			x = a.x;
			y = a.y;
			z = a.z;
			return *this;
		}
		double& operator[](int i) {	return i == 0 ? x : i == 1 ? y : z;	}
		double operator[](int i) const { return i == 0 ? x : i == 1 ? y : z; }
		Point3d operator-() const {	return (*this) * -1; }
		double rad() const {
			return sqrt(x * x + y * y + z * z);
		}
		bool operator==(const Point3d& a) const {
			return x == a.x && y == a.y && z == a.z;
		}

		bool isValid() const {
			return !isnan(x) && !isnan(y) && !isnan(z);
		}
		bool isNull() const {
			return x == 0 && y == 0 && z == 0;
		}

		friend Point3d operator+(const Point3d& a, const Point3d& b) {
			return Point3d(a.x + b.x, a.y + b.y, a.z+b.z);
		}
		friend Point3d operator-(const Point3d& a, const Point3d& b) {
			return Point3d(a.x - b.x, a.y - b.y, a.z - b.z);
		}
		friend Point3d operator*(const Point3d& a, double d) {
			return Point3d(a.x * d, a.y * d, a.z * d);
		}
		friend Point3d operator*(double d, const Point3d& a) {
			return Point3d(a.x * d, a.y * d, a.z * d);
		}
		friend Point3d operator/(const Point3d& a, double d) {
			return Point3d(a.x / d, a.y / d, a.z / d);
		}
		friend double operator*(const Point3d& a, const Point3d& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
		friend Point3d vecProd(const Point3d& a, const Point3d& b) {
			return Point3d(b.y * a.z - b.z * a.y, b.z * a.x - b.x * a.z, b.x * a.y - b.y * a.x);
		}
		Point3d& operator*=(double d) {
			x *= d; y *= d; z *= d;
			return *this;
		}
		Point3d& operator+=(const Point3d& a) {
			x += a.x;
			y += a.y;
			z += a.z;
			return *this;
		}

		Point3d& operator/=(double d) {
			x /= d;
			y /= d;
			z /= d;
			return *this;
		}

		size_t size() const {
			return 3;
		}

		/**
		 * Computes the Dot Product of two vectors
		 * @param right vector to compute dot product with.
		 * @return The dot product of \c this and \c right
		 */
		double dot(const Point3d& right) const
			noexcept;

		Point3d& normalize() {
			double n = rad();
			*this = *this / n;
			return *this;
		}

		/**
		 * Computes the Cross Product of two vectors
		 * @param right vector to compute cross product with
		 * @return The cross product of \c v1 and \c v2
		 */
		Point3d cross(const Point3d& right) const
			noexcept;

		static Point3d cross(const Point3d& left, const Point3d& right)	noexcept;

		/**
		 * Computes the Magnigude of this vector
		 */
		double mag() const noexcept {
			return rad();
		}

		Point3d perp() const {
			double min = fabs(x);
			Point3d cardinalAxis(1, 0, 0);

			if (fabs(y) < min) {
				min = fabs(y);
				cardinalAxis = Point3d(0, 1, 0);

			}

			if (fabs(z) < min) {
				cardinalAxis = Point3d(0, 0, 1);
			}

			Point3d p = *this;
			p = p.cross(cardinalAxis);
			return p;
		}

		/**
		 * Computes the Cosine of the Angle Between this vector and another.
		 * @param right the other vector
		 * @return The cosine of the angle between \c this and \c right
		 * @throw GeometryException
		 */
		double cosVector(const Point3d& right) const;

		// Computes the slant range between two vectors
		double slantRange(const Point3d& right) const noexcept {
			return (right - *this).rad();
		}

		/**
		 * Computes the elevation of a point with respect to this
		 * point.
		 * @param right The second point
		 * @return The elevation of \c right relative to \c this
		 * @throw GeometryException
		 */
		double elvAngle(const Point3d& right) const;

		/**
		 * Computes an azimuth from this point.
		 * @param right The position to determine azimuth of.
		 * @return The azimuth of \c right relative to \c this
		 * @throw GeometryException
		 */
		double azAngle(const Point3d& right) const;

		/** Computes rotation about axis X.
		 * @param angle    Angle to rotate, in degrees
		 * @return A triple which is the original triple rotated angle about X
		 */
		Point3d R1(const double& angle) const
			noexcept;


		/** Computes rotation about axis Y.
		 * @param angle    Angle to rotate, in degrees
		 * @return A triple which is the original triple rotated angle about Y
		 */
		Point3d R2(const double& angle) const
			noexcept;


		/** Computes rotation about axis Z.
		 * @param angle    Angle to rotate, in degrees
		 * @return A triple which is the original triple rotated angle about Z
		 */
		Point3d R3(const double& angle) const
			noexcept;

		friend Archive& operator<<(Archive& ar, const Point3d& pt) {
			ar.write(&pt.x, sizeof(double));
			ar.write(&pt.y, sizeof(double));
			ar.write(&pt.z, sizeof(double));
			return ar;
		}
		friend Archive& operator>>(Archive& ar, Point3d& pt) {
			ar.read(&pt.x, sizeof(double));
			ar.read(&pt.y, sizeof(double));
			ar.read(&pt.z, sizeof(double));
			return ar;
		}

		friend std::ostream& operator<<(std::ostream& os, const Point3d& p) noexcept;

		std::string asString() const {
			return stringFormat("(%.4f, %.4f, %.4f)", x, y, z);
		}
	};

	// End of Point3d interface
	//////////////////////////////////////////////////////

	//////////////////////////////////////////////////////
	// Size3d - a 3D floating point size.
	class Size3d
	{
		// Constructors:
	public:
		Size3d() : cx(0), cy(0), cz(0) {}
		Size3d(double cx_, double cy_, double cz_) : cx(cx_), cy(cy_), cz(cz_) {}
		Size3d(const Point3d& pt) : cx(pt.x), cy(pt.y), cz(pt.z) {}
		Size3d(const Size3d& s) : cx(s.cx), cy(s.cy), cz(s.cz) {}

		// Attributes:
	public:
		double cx;
		double cy;
		double cz;

		// Operations:
	public:
		bool operator==(Size3d s) const { return s.cx == cx && s.cy == cy && s.cz == cz; }
		bool operator!=(Size3d s) const { return s.cx != cx || s.cy != cy || s.cz != cz; }
		Size3d operator+=(Size3d s) { cx += s.cx, cy += s.cy, cz += s.cz; return *this; }
		Size3d operator-=(Size3d s) { cx -= s.cx, cy -= s.cy, cz -= s.cz; return *this; }
		Size3d& operator/=(double d) { cx /= d, cy /= 2, cz /= d; return *this; }

		// Operators returning CF3DPoint values
		friend Size3d operator+(const Size3d& a, const Size3d& b) { return Size3d(a.cx + b.cx, a.cy + b.cy, a.cz + b.cz); }
		friend Size3d operator-(const Size3d& a, const Size3d& b) { return Size3d(a.cx - b.cx, a.cy - b.cy, a.cz - b.cz); }
		Size3d operator-() const { return Size3d(-cx, -cy, -cz); }
		operator Point3d () const {
			return Point3d(cx, cy, cz);
		}

		// Some geom. helpers:
		double volume(void) const { return cx * cy * cz; }
		double diag(void) const { return sqrt(cx * cx + cy * cy + cz * cz); }

		// Serialization:
	public:
		friend Archive& operator<<(Archive& ar, const Size3d& s) {
			ar.write(&s.cx, sizeof(double));
			ar.write(&s.cy, sizeof(double));
			ar.write(&s.cz, sizeof(double));
			return ar;
		}
		friend Archive& operator>>(Archive& ar, Size3d& s) {
			ar.read(&s.cx, sizeof(double));
			ar.read(&s.cy, sizeof(double));
			ar.read(&s.cz, sizeof(double));
			return ar;
		}
	};
	// End of Size3d interface
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Cube3d interface
	class Cube3d
	{
	// Construction:
	public:
		Cube3d() {}
		Cube3d(const Point3d& pt0_, const Point3d& pt1_) : pt0(pt0_), pt1(pt1_) {}

	// Attributes:
	public:
		Point3d pt0;
		Point3d pt1;

	// Operations:
	public:
		void onMinMax(const Point3d& p) {
			if (pt0.x > p.x)
				pt0.x = p.x;
			if (pt0.y > p.y)
				pt0.y = p.y;
			if (p.x > pt1.x)
				pt1.x = p.x;
			if (p.y > pt1.y)
				pt1.y = p.y;
			if (p.z < pt0.z)
				pt0.z = p.z;
			if (p.z > pt1.z)
				pt1.z = p.z;
		}
		Size3d size() const {
			return pt1 - pt0;
		}
		Point3d center() const {
			return (pt0 + pt1) * 0.5;
		}

	};
	// End of Cube3d interface
	//////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// Axis3d interface
	class Axis3d
	{
		// Construction:
	public:
		Axis3d();
		Axis3d(double cx, double cy, double cz) { m_sVec.cx = cx; m_sVec.cy = cy; m_sVec.cz = cz; }
		~Axis3d();

		// Attributes:
	public:
		Size3d m_sVec; // Axis direction (normalized vector)
		Point3d m_ptOrg; // A point on the axis

		// Operations:
	public:
		void getSinCos(double* pdCosFi, double* pdSinFi, double* pdCosT, double* pdSinT) const;
	};
	// End of Axis3d interface
	///////////////////////////////////////////////////////////////////////////////

}