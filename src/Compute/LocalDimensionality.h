#pragma once

#include "DataMatrix.h"

#include <vector>

class KnnGraph;

namespace mv
{
    namespace compute
    {
        float computeProjectionDiameter(const DataMatrix& projection, int xDim, int yDim);

        void findNeighbourhood(const DataMatrix& projection, int centerId, float radius, std::vector<int>& neighbourhood, int xDim, int yDim);

        void computeSpatialLocalDimensionality(DataMatrix& dataMatrix, DataMatrix& projMatrix, std::vector<float>& colors);

        void computeHDLocalDimensionality(DataMatrix& dataMatrix, KnnGraph& knnGraph, std::vector<float>& normLD);
    }
}
