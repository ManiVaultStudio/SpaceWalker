#pragma once

#include <QString>

class DataStorage;
class FloodFill;
class KnnGraph;

namespace filters
{
    class Filters;
}

void writeDimensionRanking(const std::vector<std::vector<int>>& ranking, const std::vector<QString>& names);

void exportRankings(DataStorage& dataStore, FloodFill& floodFill, KnnGraph& knnGraph, const filters::Filters& filters, bool restrictToFloodNodes, const std::vector<QString>& names);
