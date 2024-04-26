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

        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking) const;
        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking, const std::vector<int>& mask) const;

    private:
        float _innerFilterRadius;
        float _outerFilterRadius;
    };

    class HDFloodPeakFilter
    {
    public:
        HDFloodPeakFilter();

        void setInnerFilterSize(int size);

        void computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const FloodFill& floodFill, std::vector<int>& dimRanking) const;
    private:
        int _innerFilterSize;
    };

    class Filters
    {
    public:
        Filters() :
            _type(filters::FilterType::SPATIAL_PEAK)
        {

        }

    public:
        void setFilterType(filters::FilterType type) { _type = type; }

        // Non-const member functions
        filters::SpatialPeakFilter&         getSpatialPeakFilter()  { return _spatialFilter; }
        filters::HDFloodPeakFilter&         getHDPeakFilter()       { return _highdimFilter; }

        // Const member functions
        filters::FilterType                 getFilterType()         const { return _type; }
        const filters::SpatialPeakFilter&   getSpatialPeakFilter()  const { return _spatialFilter; }
        const filters::HDFloodPeakFilter&   getHDPeakFilter()       const { return _highdimFilter; }

    private:
        filters::FilterType                 _type;
        filters::SpatialPeakFilter          _spatialFilter;
        filters::HDFloodPeakFilter          _highdimFilter;
    };
}
