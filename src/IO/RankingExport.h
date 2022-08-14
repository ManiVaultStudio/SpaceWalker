#pragma once

#include <QString>

class DataStorage;
class FloodFill;
class KnnGraph;

namespace filters
{
    enum class FilterType;
    class SpatialPeakFilter;
    class HDFloodPeakFilter;
}

void writeDimensionRanking(const std::vector<std::vector<int>>& ranking, const std::vector<QString>& names);

void exportRankings(DataStorage& dataStore, FloodFill& floodFill, KnnGraph& knnGraph, filters::FilterType filterType, filters::SpatialPeakFilter spatialFilter, filters::HDFloodPeakFilter hdFilter, bool restrictToFloodNodes, const std::vector<QString>& names);
