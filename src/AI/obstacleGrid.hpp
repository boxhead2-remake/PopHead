#pragma once

#include <vector>

namespace ph {

class ObstacleGrid
{
public:
	ObstacleGrid() = default;
	ObstacleGrid(size_t columns, size_t rows);
	ObstacleGrid(std::vector<bool> obstacles, size_t columns, size_t rows);
	ObstacleGrid(std::vector<std::vector<bool>> obstacles, size_t columns, size_t rows);

	void registerObstacle(size_t column, size_t row);
	bool isObstacle(i32 column, i32 row) const;

	size_t getColumnsCount() const { return mColumns; } 
	size_t getRowsCount() const { return mRows; }

private:
	size_t internalIndex(size_t column, size_t row) const;

private:
	std::vector<bool> mObstacles;
	size_t mColumns = 0;
	size_t mRows = 0;
};

}

