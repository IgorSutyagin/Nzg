#include "pch.h"

#include "NzgNode.h"

namespace nzg
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Strat implementation
	
	const char* Strat::c_types[(int)Type::maxType+1] = {
		"yes",
		"no",
		"friedman",
		"joss",
		"graaskamp",
		"titfortat",
		"random",
		"maxType"
	};

	Strat::Strat() : type (Type::maxType), score(0), totalScore(0)
	{
	}

	Strat::~Strat()
	{
	}

	bool Strat::play(std::vector<bool>& my, std::vector<bool>& his)
	{
		if (type == Type::yes)
			return true;
		else if (type == Type::no)
			return false;
		else if (type == Type::friedman)
		{
			if (my.size() == 0)
				return true;
			int rand = std::rand();

			if ((double)rand / (double)RAND_MAX < 0.1)
				return false;
			return his.back();
		}
		else if (type == Type::graaskamp)
		{
			if (my.size() == 0)
				return true;
			if (my.size() == 49)
				return false;
			return his.back();
		}
		else if (type == Type::titfortat)
		{
			if (my.size() == 0)
				return true;
			return his.back();
		}
		else if (type == Type::random)
		{
			int rand = std::rand();
			if (rand < RAND_MAX / 2)
				return true;
			return false;
		}
		return false;
	}

	DWORD Strat::getColor() const
	{
		COLORREF clrs[56] =
			/*
		{
			0xFFB300,    // Vivid Yellow
			0x803E75,    // Strong Purple
			0xFF6800,    // Vivid Orange
			0xA6BDD7,    // Very Light Blue
			0xC10020,    // Vivid Red
			0xCEA262,    // Grayish Yellow
			0x817066,    // Medium Gray

			0x007D34,    // Vivid Green
			0xF6768E,    // Strong Purplish Pink
			0x00538A,    // Strong Blue
			0xFF7A5C,    // Strong Yellowish Pink
			0x53377A,    // Strong Violet
			0xFF8E00,    // Vivid Orange Yellow
			0xB32851,    // Strong Purplish Red
			0xF4C800,    // Vivid Greenish Yellow
			0x7F180D,    // Strong Reddish Brown
			0x93AA00,    // Vivid Yellowish Green
			0x593315,    // Deep Yellowish Brown
			0xF13A13,    // Vivid Reddish Orange
			0x232C16,    // Dark Olive Green
		};
		*/
		{
			0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0x000000,
			0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0x808080,
			0xC00000, 0x00C000, 0x0000C0, 0xC0C000, 0xC000C0, 0x00C0C0, 0xC0C0C0,
			0x400000, 0x004000, 0x000040, 0x404000, 0x400040, 0x004040, 0x404040,
			0x200000, 0x002000, 0x000020, 0x202000, 0x200020, 0x002020, 0x202020,
			0x600000, 0x006000, 0x000060, 0x606000, 0x600060, 0x006060, 0x606060,
			0xA00000, 0x00A000, 0x0000A0, 0xA0A000, 0xA000A0, 0x00A0A0, 0xA0A0A0,
			0xE00000, 0x00E000, 0x0000E0, 0xE0E000, 0xE000E0, 0x00E0E0, 0xE0E0E0
		};

		int nt = (int)type;
		return clrs[nt % 56];
	}
	// End of Strat implementation
	////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// StratMatrix implementation
	StratMatrix::StratMatrix()
	{
		rows = cols = 0;
	}

	StratMatrix::~StratMatrix()
	{
	}

	void StratMatrix::resize(int rows_, int cols_)
	{
		rows = rows_;
		cols = cols_;
		sts.resize(rows * cols);
	}


	// End of StratMatrix implementation
	////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// NzgNode implementation
	NzgNode::NzgNode()
	{
		m_ent = entNzg;
		setMap(MapType::random, 10, 10);
	}

	NzgNode::~NzgNode()
	{
	}

	void NzgNode::setMap(MapType mt, int rows, int cols)
	{
		sts.resize(rows, cols);

		if (mt == MapType::random)
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					double r = std::rand() / (double)RAND_MAX;
					Strat::Type nt = (Strat::Type)(((int)Strat::Type::maxType - 1) * r);
					if (nt == Strat::Type::maxType)
						nt = Strat::Type::titfortat;
					sts(i, j) = std::shared_ptr<Strat>(new Strat(nt));
				}
			}
		}
		else
		{
			for (int i = 0; i < rows; i++)
			{
				Strat::Type nt = (Strat::Type)(((int)Strat::Type::maxType-1) * i / rows);
				for (int j = 0; j < cols; j++)
				{
					sts(i, j) = std::shared_ptr<Strat>(new Strat(nt));
				}
			}
		}
	}

	void NzgNode::play()
	{
		CWaitCursor wc;

		std::pair<int, int> neis[8] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1} };
		for (int i = 0; i < sts.getRows(); i++)
		{
			for (int j = 0; j < sts.getCols(); j++)
			{
				Strat* ps1 = sts(i, j).get();

				for (int k = 0; k < 8; k++)
				{
					int m = i + neis[k].first;
					int n = j + neis[k].second;

					if (m < 0)
						m += sts.getRows();
					if (m >= sts.getRows())
						m -= sts.getRows();
					if (n < 0)
						n += sts.getCols();
					if (n >= sts.getCols())
						n -= sts.getCols();

					Strat* ps2 = sts(m, n).get();

					std::vector<bool> r1;
					std::vector<bool> r2;
					for (int p = 0; p < 50; p++)
					{
						bool b1 = ps1->play(r1, r2);
						bool b2 = ps2->play(r2, r1);
						r1.push_back(b1);
						r2.push_back(b2);
						int s1 = 0;
						int s2 = 0;
						if (b1 && b2)
						{
							s1 = s2 = 3;
						}
						else if (b1 && !b2)
						{
							s1 = 0;
							s2 = 5;
						}
						else if (!b1 && b2)
						{
							s1 = 5;
							s2 = 0;
						}
						else if (!b1 && !b2)
						{
							s1 = 1;
							s2 = 1;
						}
						ps1->score += s1;
						ps1->totalScore += s1;
						ps2->score += s2;
						ps2->totalScore += s2;
					}
					ps1->score = 0;
					ps2->score = 0;
				}
			}
		}
	}

	void NzgNode::resetTotalScores()
	{
		for (int i = 0; i < sts.getRows(); i++)
		{
			for (int j = 0; j < sts.getCols(); j++)
			{
				sts(i, j)->totalScore = 0;
				sts(i, j)->score = 0;
			}
		}
	}

	void NzgNode::updateStrat()
	{
		std::map<std::pair<int, int>, Strat::Type> newSts;
		std::pair<int, int> neis[8] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1} };
		for (int i = 0; i < sts.getRows(); i++)
		{
			for (int j = 0; j < sts.getCols(); j++)
			{
				Strat* ps1 = sts(i, j).get();
				bool updateStr = true;
				int maxScore = 0;
				Strat::Type bestType = Strat::Type::maxType;
				for (int k = 0; k < 8; k++)
				{
					int m = i + neis[k].first;
					int n = j + neis[k].second;

					if (m < 0)
						m += sts.getRows();
					if (m >= sts.getRows())
						m -= sts.getRows();
					if (n < 0)
						n += sts.getCols();
					if (n >= sts.getCols())
						n -= sts.getCols();

					Strat* ps2 = sts(m, n).get();
					if (ps1->totalScore > ps2->totalScore)
					{
						updateStr = false;
						break;
					}

					if (ps2->totalScore > maxScore)
					{
						maxScore = ps2->totalScore;
						bestType = ps2->type;
					}
				}

				newSts[std::pair<int, int>(i, j)] = updateStr ? bestType : Strat::Type::maxType;

			}
		}

		for (int i = 0; i < sts.getRows(); i++)
		{
			for (int j = 0; j < sts.getCols(); j++)
			{
				if (newSts[{i, j}] == Strat::Type::maxType)
					continue;

				sts(i, j)->type = newSts[{i, j}];
			}
		}
	}

	void NzgNode::reset(int rows, int cols, MapType mt)
	{
		setMap(mt, rows, cols);
	}

	// End of NzgNode implementation
	////////////////////////////////////////////////////////////////////////////////////////////////////
}