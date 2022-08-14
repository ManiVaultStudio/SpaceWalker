#pragma once

#include "DataMatrix.h"
#include "graphics/Vector2f.h"

#include <vector>

class KnnGraph;

namespace hdps
{
    namespace compute
    {
        int binarySearchCDF(const std::vector<float>& cdf, float u);

        void doRandomWalks(const DataMatrix& highDim, const DataMatrix& spatialMap, int selectedPoint, std::vector<std::vector<Vector2f>>& randomWalks);

        void doRandomWalksKNN(const DataMatrix& highDim, const DataMatrix& spatialMap, const KnnGraph& knnGraph, int selectedPoint, std::vector<std::vector<int>>& randomWalks);

        void traceLineage(const DataMatrix& data, const std::vector<std::vector<int>>& floodFill, std::vector<Vector2f>& positions, int seedIndex, std::vector<int>& lineage);
    }
}
