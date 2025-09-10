#include "pch.h"

#include "StringUtils.h"
#include "Angles.h"

namespace nzg
{
     
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// AngleType implementation
    namespace StringUtils
    {
        std::string asString(AngleType e) noexcept
        {
            switch (e)
            {
            case AngleType::Unknown:    return "Unknown";
            case AngleType::Rad:        return "radians";
            case AngleType::Deg:        return "degrees";
            case AngleType::SemiCircle: return "semi-circles";
            case AngleType::Sin:        return "sin";
            case AngleType::Cos:        return "cos";
            default:                    return "???";
            }
        }


        AngleType asAngleType(const std::string& s) noexcept
        {
            std::string lc(nzg::StringUtils::lowerCase(s));
            if (lc == "unknown")
                return AngleType::Unknown;
            if (lc == "radians")
                return AngleType::Rad;
            if (lc == "degrees")
                return AngleType::Deg;
            if (lc == "semi-circles")
                return AngleType::SemiCircle;
            if (lc == "sin")
                return AngleType::Sin;
            if (lc == "cos")
                return AngleType::Cos;
            return AngleType::Unknown;
        }
    } // namespace StringUtils

	// End of AngleType implementation
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AngleReduced implementation
    AngleReduced::AngleReduced()
        : sine(std::numeric_limits<double>::quiet_NaN()),
        cosine(std::numeric_limits<double>::quiet_NaN())
    {}


    void AngleReduced::setValue(double v, AngleType t)
    {
        double radians;
        switch (t)
        {
        case AngleType::Rad:
            sine = ::sin(v);
            cosine = ::cos(v);
            break;
        case AngleType::Deg:
            radians = v * DEG2RAD;
            sine = ::sin(radians);
            cosine = ::cos(radians);
            break;
        case AngleType::SemiCircle:
            radians = v * PI;
            sine = ::sin(radians);
            cosine = ::cos(radians);
            break;
        case AngleType::Sin:
            sine = v;
            cosine = ::sqrt(1 - sine * sine);
            break;
        case AngleType::Cos:
            cosine = v;
            sine = ::sqrt(1 - cosine * cosine);
            break;
        default:
            NZG_THROW(Exception("Invalid type in setValue"));
            break;
        }
    }

    std::string AngleReduced::asString() const
    {
        std::ostringstream s;
        s << *this;
        return s.str();
    }


    // End of AngleReduced implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Angle implementation
    Angle::Angle()
        : radians(std::numeric_limits<double>::quiet_NaN()),
        degrees(std::numeric_limits<double>::quiet_NaN()),
        tangent(std::numeric_limits<double>::quiet_NaN()),
        semicircles(std::numeric_limits<double>::quiet_NaN())
    {
    }


    Angle::Angle(double s, double c)
        : AngleReduced(s, c)
    {
        radians = atan2(s, c);
        degrees = radians * RAD2DEG;
        semicircles = radians / PI;
        tangent = ::tan(radians);
    }


    void Angle::setValue(double v, AngleType t)
    {
        switch (t)
        {
        case AngleType::Rad:
            radians = v;
            degrees = v * RAD2DEG;
            semicircles = v / PI;
            sine = ::sin(radians);
            cosine = ::cos(radians);
            tangent = ::tan(radians);
            break;
        case AngleType::Deg:
            radians = v * DEG2RAD;
            degrees = v;
            semicircles = v / 180.0;
            sine = ::sin(radians);
            cosine = ::cos(radians);
            tangent = ::tan(radians);
            break;
        case AngleType::SemiCircle:
            radians = v * PI;
            degrees = v * 180.0;
            semicircles = v;
            sine = ::sin(radians);
            cosine = ::cos(radians);
            tangent = ::tan(radians);
            break;
        case AngleType::Sin:
            radians = asin(v);
            degrees = radians * RAD2DEG;
            semicircles = radians / PI;
            sine = v;
            cosine = ::sqrt(1 - v * v);
            tangent = ::tan(radians);
            break;
        case AngleType::Cos:
            radians = acos(v);
            degrees = radians * RAD2DEG;
            semicircles = radians / PI;
            sine = ::sqrt(1 - v * v);
            cosine = v;
            tangent = ::tan(radians);
            break;
        default:
            NZG_THROW(Exception("Invalid type in setValue"));
            break;
        }
    } // setValue


    Angle Angle::operator-(const Angle& right) const
    {
        double newrad = radians - right.radians;
        Angle rv(newrad, AngleType::Rad);
        return rv;
    }


    Angle Angle::operator+(const Angle& right) const
    {
        double newrad = radians + right.radians;
        Angle rv(newrad, AngleType::Rad);
        return rv;
    }


    std::string Angle::asString() const
    {
        std::ostringstream s;
        s << *this;
        return s.str();
    }

    // End of Angle implementation
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ThetaPhi
    Point3d ThetaPhi::unitPoint() const
    {
        double ct = ::cos(theta);
        double st = ::sin(theta);
        double cp = ::cos(phi);
        double sp = ::sin(phi);
        return Point3d(st * cp, st * sp, ct);
    }

    // End of ThetaPhi
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
}