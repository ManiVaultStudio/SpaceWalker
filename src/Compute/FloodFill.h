#pragma once

#include "KnnGraph.h"
#include "Types.h"

#include <vector>

class FloodFill
{
public:
    FloodFill(int numWaves);

    void compute(const KnnGraph& knnGraph, nint selectedPoint);
    void recompute();

    void setNumWaves(int numWaves);

    int getNumWaves() const { return (int) _waves.size(); }
    bigint getTotalNumNodes() const { return (bigint) _allNodes.size(); }

    std::vector<std::vector<nint>>& getWaves() { return _waves; }
    const std::vector<std::vector<nint>>& getWaves() const { return _waves; }
    const std::vector<nint>& getAllNodes() const { return _allNodes; }

private:
    int _numWaves;

    std::vector<std::vector<nint>> _waves;

    std::vector<nint> _allNodes;

    // Store knn graph for recomputation
    const KnnGraph* _lastKnnGraph;
    nint _lastSelectedPoint;
    int _lastNumWaves;
};
