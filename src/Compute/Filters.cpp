#include "Filters.h"

#include "FloodFill.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"

#include <numeric>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace hdps;

void findPointsInRadius(Vector2f center, float radius, const DataMatrix& projMatrix, std::vector<int>& indices)
{
    float radiusSqr = radius * radius;
    for (int i = 0; i < projMatrix.rows(); i++)
    {
        Vector2f pos(projMatrix(i, 0), projMatrix(i, 1));

        Vector2f diff = center - pos;
        float sqrLen = diff.x * diff.x + diff.y * diff.y;

        if (sqrLen < radiusSqr)
        {
            indices.push_back(i);
        }
    }
}

void computeDimensionAverage(const DataMatrix& data, const std::vector<int>& indices, std::vector<float>& averages)
{
    int numDimensions = data.cols();
    averages.resize(numDimensions, 0);

#pragma omp parallel for
    for (int d = 0; d < numDimensions; d++)
    {
        for (const int& index : indices)
        {
            float v = data(index, d);
            averages[d] += v;
        }
        averages[d] /= indices.size();
    }
}

void maskPoints(std::vector<int>& indices, const std::vector<int>& mask, std::vector<int>& intersection)
{
    // Find the intersection of points that are selected, and the given indices
    std::vector<int>& smallVector = indices;
    const std::vector<int>& largeVector = mask;

    // Sort the smallest vector
    sort(smallVector.begin(), smallVector.end());

    // Loop over large vector and binary search which values are in the smaller vector
    std::copy_if(largeVector.begin(), largeVector.end(), std::back_inserter(intersection), [smallVector](int x)
    {
        return std::binary_search(smallVector.begin(), smallVector.end(), x);
    });
}

namespace filters
{
    SpatialPeakFilter::SpatialPeakFilter() :
        _innerFilterRadius(0.025f),
        _outerFilterRadius(0.05f)
    {

    }

    void SpatialPeakFilter::setInnerFilterRadius(float radius)
    {
        _innerFilterRadius = radius;
    }

    void SpatialPeakFilter::setOuterFilterRadius(float radius)
    {
        _outerFilterRadius = radius;
    }

    void SpatialPeakFilter::computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking)
    {
        int numDimensions = dataMatrix.cols();

        // Small and large circle averages
        std::vector<std::vector<float>> averages(2);
        std::vector<std::vector<int>> circleIndices(2);

        Vector2f center = Vector2f(projMatrix(pointId, 0), projMatrix(pointId, 1));

        findPointsInRadius(center, _innerFilterRadius * projSize, projMatrix, circleIndices[0]);
        computeDimensionAverage(dataMatrix, circleIndices[0], averages[0]);
        findPointsInRadius(center, _outerFilterRadius * projSize, projMatrix, circleIndices[1]);
        computeDimensionAverage(dataMatrix, circleIndices[1], averages[1]);

        std::vector<float> diffAverages(numDimensions);
        for (int d = 0; d < numDimensions; d++)
        {
            diffAverages[d] = 0;
            if (variances[d] > 0)
                diffAverages[d] = (averages[0][d] - averages[1][d]);// / variances[d];
        }

        // Sort averages from high to low
        dimRanking.resize(numDimensions);
        std::iota(dimRanking.begin(), dimRanking.end(), 0);

        std::stable_sort(dimRanking.begin(), dimRanking.end(), [&diffAverages](size_t i1, size_t i2) {return diffAverages[i1] > diffAverages[i2]; });
    }

    void SpatialPeakFilter::computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const DataMatrix& projMatrix, float projSize, std::vector<int>& dimRanking, const std::vector<int>& mask)
    {
        int numDimensions = dataMatrix.cols();

        // Small and large circle averages
        std::vector<std::vector<float>> averages(2);
        std::vector<std::vector<int>> circleIndices(2);

        Vector2f center = Vector2f(projMatrix(pointId, 0), projMatrix(pointId, 1));

        findPointsInRadius(center, _innerFilterRadius * projSize, projMatrix, circleIndices[0]);
        // Apply mask
        //std::vector<int> maskedIndicesInner;
        //maskPoints(circleIndices[0], mask, maskedIndicesInner);
        computeDimensionAverage(dataMatrix, circleIndices[0], averages[0]);

        findPointsInRadius(center, _outerFilterRadius * projSize, projMatrix, circleIndices[1]);
        // Apply mask
        //std::vector<int> maskedIndicesOuter;
        //maskPoints(circleIndices[1], mask, maskedIndicesOuter);
        computeDimensionAverage(dataMatrix, circleIndices[1], averages[1]);

        std::vector<float> diffAverages(numDimensions);
        for (int d = 0; d < numDimensions; d++)
        {
            diffAverages[d] = 0;
            if (variances[d] > 0)
                diffAverages[d] = (averages[0][d] - averages[1][d]);// / variances[d];
        }

        // Sort averages from high to low
        dimRanking.resize(numDimensions);
        std::iota(dimRanking.begin(), dimRanking.end(), 0);

        std::stable_sort(dimRanking.begin(), dimRanking.end(), [&diffAverages](size_t i1, size_t i2) {return diffAverages[i1] > diffAverages[i2]; });
    }

    HDFloodPeakFilter::HDFloodPeakFilter() :
        _innerFilterSize(5)
        //_outerFilterSize(10)
    {

    }

    void HDFloodPeakFilter::setInnerFilterSize(int size)
    {
        _innerFilterSize = size;
    }

    //void HDFloodPeakFilter::setOuterFilterSize(int size)
    //{
    //    _outerFilterSize = size;
    //}

    void HDFloodPeakFilter::computeDimensionRanking(int pointId, const DataMatrix& dataMatrix, const std::vector<float>& variances, const FloodFill& floodFill, std::vector<int>& dimRanking)
    {
        int numDimensions = dataMatrix.cols();

        std::vector<int> nearIndices;
        std::vector<int> farIndices;

        for (int wave = 0; wave < _innerFilterSize; wave++)
        {
            nearIndices.insert(nearIndices.end(), floodFill.getWaves()[wave].begin(), floodFill.getWaves()[wave].end());
        }
        for (int wave = _innerFilterSize; wave < floodFill.getNumWaves() - 1; wave++)
        {
            farIndices.insert(farIndices.end(), floodFill.getWaves()[wave].begin(), floodFill.getWaves()[wave].end());
        }

        std::vector<float> nearAverages;
        std::vector<float> farAverages;

        computeDimensionAverage(dataMatrix, nearIndices, nearAverages);
        computeDimensionAverage(dataMatrix, farIndices, farAverages);

        std::vector<float> diffAverages(numDimensions);
        for (int d = 0; d < numDimensions; d++)
        {
            diffAverages[d] = 0;
            if (variances[d] > 0)
                diffAverages[d] = (nearAverages[d] - farAverages[d]);// / variances[d];
        }

        // Sort averages from high to low
        dimRanking.resize(numDimensions);
        std::iota(dimRanking.begin(), dimRanking.end(), 0);

        std::stable_sort(dimRanking.begin(), dimRanking.end(), [&diffAverages](size_t i1, size_t i2) {return diffAverages[i1] > diffAverages[i2]; });
    }

}
