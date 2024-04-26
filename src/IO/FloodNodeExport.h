#pragma once

#include <vector>

class FloodFill;
class KnnGraph;

void writeFloodNodes(const std::vector<std::vector<int>>& floodNodes);

void exportFloodNodes(int numPoints, FloodFill& floodFill, KnnGraph& knnGraph);
