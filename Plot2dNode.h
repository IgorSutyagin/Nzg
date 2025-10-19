#pragma once

#include "Node.h"
#include "Points.h"

namespace nzg
{
	class Plot2dNode : public Node
	{
	public:
		Plot2dNode() { m_ent = entPlot2d; }
		~Plot2dNode() {}

	// Operations:
	public:
		void getCurve(std::vector<Point2d>& pts);

	// Overrides:
	public:
		virtual std::string getName() const { return "Nzg"; }

	};
}