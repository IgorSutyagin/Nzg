#include "pch.h"

//#include "GnssConstants.h"
#include "NzgException.h"
#include "Points.h"
#include "Tools.h"

namespace nzg
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Points3d implementation

      // returns the dot product of the two vectors
    double Point3d::dot(const Point3d& right) const
        noexcept
    {
        //Point3d z (x*right.x, y*right.y, z*right.z);
        //z = (*this) * right;
        //double a = x * right.x + y* right.y + z* right.z; // theArray.sum();
        return (*this) * right;
    }


    // retuns v1 x v2 , vector cross product
    Point3d Point3d::cross(const Point3d& right) const
        noexcept
    {
        return vecProd(*this, right);
        //Point3d cp;
        //cp[0] = (*this)[1] * right[2] - (*this)[2] * right[1];
        //cp[1] = (*this)[2] * right[0] - (*this)[0] * right[2];
        //cp[2] = (*this)[0] * right[1] - (*this)[1] * right[0];
        //return cp;
    }

    Point3d Point3d::cross(const Point3d& left, const Point3d& right)	noexcept
    {
        return vecProd(left, right);
    }
    
    // function that returns the cosine of angle between this and right
    double Point3d::cosVector(const Point3d& right) const
    {
        double rx, ry, cosvects;

        rx = dot(*this);
        ry = right.dot(right);

        if (rx <= 1e-14 || ry <= 1e-14)
        {
            NZG_THROW(GeometryException("Divide by Zero Error"));
        }
        cosvects = dot(right) / ::sqrt(rx * ry);

        /* this if checks for and corrects round off error */
        if (fabs(cosvects) > 1.0e0)
        {
            cosvects = fabs(cosvects) / cosvects;
        }

        return cosvects;
    }

    // Finds the elevation angle of the second point with respect to
    // the first point
    double Point3d::elvAngle(const Point3d& right) const
    {
        Point3d z;
        z = right - *this;
        double c = z.cosVector(*this);
        return 90.0 - ::acos(c) * RAD2DEG;
    }


    //  Calculates a satellites azimuth from a station
    double Point3d::azAngle(const Point3d& right) const
    {
        double xy, xyz, cosl, sinl, sint, xn1, xn2, xn3, xe1, xe2;
        double z1, z2, z3, p1, p2, test, alpha;

        xy = (*this)[0] * (*this)[0] + (*this)[1] * (*this)[1];
        xyz = xy + (*this)[2] * (*this)[2];
        xy = ::sqrt(xy);
        xyz = ::sqrt(xyz);

        if (xy <= 1e-14 || xyz <= 1e-14)
            NZG_THROW(GeometryException("Divide by Zero Error"))

            cosl = (*this)[0] / xy;
        sinl = (*this)[1] / xy;
        sint = (*this)[2] / xyz;

        xn1 = -sint * cosl;
        xn2 = -sint * sinl;
        xn3 = xy / xyz;

        xe1 = -sinl;
        xe2 = cosl;

        z1 = right[0] - (*this)[0];
        z2 = right[1] - (*this)[1];
        z3 = right[2] - (*this)[2];

        p1 = (xn1 * z1) + (xn2 * z2) + (xn3 * z3);
        p2 = (xe1 * z1) + (xe2 * z2);

        test = fabs(p1) + fabs(p2);

        if (test < 1.0e-14)
        {
            NZG_THROW(GeometryException("azAngle(), failed p1+p2 test."));
        }

        alpha = 90 - ::atan2(p1, p2) * RAD2DEG;
        if (alpha < 0)
        {
            return alpha + 360;
        }
        else
        {
            return alpha;
        }
    }


    /* Computes rotation about axis X.
     * @param angle    Angle to rotate, in degrees
     * @return A triple which is the original triple rotated angle about X
     */
    Point3d Point3d::R1(const double& angle) const
        noexcept
    {
        double ang(angle * DEG2RAD);
        double sinangle(std::sin(ang));
        double cosangle(std::cos(ang));
        Point3d rot;
        rot[0] = (*this)[0];
        rot[1] = cosangle * (*this)[1] + sinangle * (*this)[2];
        rot[2] = -sinangle * (*this)[1] + cosangle * (*this)[2];
        return rot;
    }


    /* Computes rotation about axis Y.
     * @param angle    Angle to rotate, in degrees
     * @return A triple which is the original triple rotated angle about Y
     */
    Point3d Point3d::R2(const double& angle) const
        noexcept
    {
        double ang(angle * DEG2RAD);
        double sinangle(std::sin(ang));
        double cosangle(std::cos(ang));
        Point3d rot;
        rot[0] = cosangle * (*this)[0] - sinangle * (*this)[2];
        rot[1] = (*this)[1];
        rot[2] = sinangle * (*this)[0] + cosangle * (*this)[2];
        return rot;
    }


    /* Computes rotation about axis Z.
     * @param angle    Angle to rotate, in degrees
     * @return A triple which is the original triple rotated angle about Z
     */
    Point3d Point3d::R3(const double& angle) const
        noexcept
    {
        double ang(angle * DEG2RAD);
        double sinangle(std::sin(ang));
        double cosangle(std::cos(ang));
        Point3d rot;
        rot[0] = cosangle * (*this)[0] + sinangle * (*this)[1];
        rot[1] = -sinangle * (*this)[0] + cosangle * (*this)[1];
        rot[2] = (*this)[2];
        return rot;
    }


	std::ostream& operator<<(std::ostream& os, const Point3d& p) noexcept
	{
		return os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
	}

	// End of Points3d implementation
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Axis3d implementation

    //////////////////////////
    // Construction:
    Axis3d::Axis3d()
    {
    }

    Axis3d::~Axis3d()
    {
    }

    ///////////////////////////
    // Operations:
    void Axis3d::getSinCos(double* pdCosFi, double* pdSinFi, double* pdCosT, double* pdSinT) const
    {
        double dR = m_sVec.diag();
        //ASSERT(gl_Acr.IsZero(dR - 1.0));

        double dr = sqrt(m_sVec.cx * m_sVec.cx + m_sVec.cy * m_sVec.cy);

        if (dr == 0)
        {
            *pdCosFi = 1;
            *pdSinFi = 0;
            *pdSinT = 0;
            *pdCosT = 1;
            return;
        }

        *pdCosFi = m_sVec.cx / dr;
        *pdSinFi = m_sVec.cy / dr;
        *pdCosT = m_sVec.cz / dR;
        *pdSinT = dr / dR;
    }

    // End of Axis3d implementaion
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}