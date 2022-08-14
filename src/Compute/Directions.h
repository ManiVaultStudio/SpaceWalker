#pragma once

#include "DataMatrix.h"
#include "KnnGraph.h"

#include "graphics/Vector2f.h"

void computeDirection(DataMatrix& dataMatrix, DataMatrix& projMatrix, KnnGraph& knnGraph, int numSteps, std::vector<hdps::Vector2f>& directions);
