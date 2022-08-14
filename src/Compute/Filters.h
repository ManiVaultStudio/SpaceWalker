#pragma once

#include "DataMatrix.h"

#include <vector>
#include <QString>

void writeFloodNodes(const std::vector<std::vector<int>>& floodNodes);
void writeDimensionRanking(const std::vector<std::vector<int>>& ranking, const std::vector<QString>& names);

class FloodFill;

namespace filters
{
    enum class FilterType
    {
        SPATIAL_PEAK,
        HD_PEAK
    };

    class SpatialPeakFilter
    {
    public:
        SpatialPeakFilter();

        float getInnerFilterRadius() const { return _innerFilterRadius; }
        float getOuterFilterRadius() const { return _outerFilterRadius; }
        void setInnerFilterRadius(float size);
        void setOuterFilterRadius(float size);

        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking);
        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking, const std::vector<int>& mask);

    private:
        float _innerFilterRadius;
        float _outerFilterRadius;
    };

    class HDFloodPeakFilter
    {
    public:
        HDFloodPeakFilter();

        void setInnerFilterSize(int size);
        //void setOuterFilterSize(int size);

        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const FloodFill& floodFill, std::vector<int>& dimRanking);
    private:
        int _innerFilterSize;
    };
}
