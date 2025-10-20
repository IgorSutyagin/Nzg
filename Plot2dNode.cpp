#include "pch.h"

#include "Plot2dNode.h"

namespace nzg
{
	void Plot2dNode::getCurve(std::vector<Point2d>& pts)
	{
		pts.clear();
		double dx = 0.01;
		for (double x = 0; x <= 2 * PI * 4; x+=dx)
		{
			Point2d pt(x, sin(x));
			pts.push_back(pt);
		}
	}
}