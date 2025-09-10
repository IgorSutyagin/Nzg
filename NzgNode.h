#pragma once

#include "Node.h"

namespace nzg
{
	class Strat
	{
	public:
		enum class Type
		{
			yes = 0,		// Cooperated always
			no = 1,			// Defect always
			friedman = 3,	// Holds grudges
			joss = 4,		// Sneaky (10% of time gets sneaky and defects)
			graaskamp = 5,	// Same as joss but defects not 10% but 50th round
			titfortat = 6,	// Optimal, starts coop, copy the last move of the opponent
			random = 7,
			maxType
		};
		static const char* c_types[(int)Type::maxType+1];
		

		Strat();
		Strat(Type t) : type(t), score(0), totalScore(0) {}
		virtual ~Strat();

		// Attributes:
	public:
		Type type;
		int score;
		int totalScore;

		virtual bool play(std::vector<bool>& my, std::vector<bool>& his);

		DWORD getColor() const;
	};

	class StratMatrix
	{
	// Construction:
	public:
		StratMatrix();
		~StratMatrix();
		void resize(int rows_, int cols_);

	// Operations:
	public:
		std::shared_ptr<Strat> operator()(int row, int col) const {
			return sts[row * cols + col];
		}
		std::shared_ptr<Strat>& operator()(int row, int col){
			return sts[row * cols + col];
		}
		int getRows() const { return rows; }
		int getCols() const { return cols; }

	// Attributes:
	protected:
		int rows;
		int cols;
		std::vector<std::shared_ptr<Strat>> sts;
	};

	////////////////////////////////////////////////////////////////////////////////
	// NzgNode interface
	class NzgNode : public Node
	{
	// Construction:
	public:
		NzgNode();
		~NzgNode();

		enum class MapType
		{
			random = 0,
			ordered = 1
		};

	// Attributes:
	public:
		StratMatrix sts;

	// Operations:
	public:
		void setMap(MapType mt, int rows, int cols);

		void play();
		void resetTotalScores();
		void updateStrat();
		void reset(int rows, int cols, MapType mt);

	// Overrides:
	public:
		virtual std::string getName() const { return "Nzg"; }

	};
}