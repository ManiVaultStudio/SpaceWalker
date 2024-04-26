#include "DataMatrix.h"

#include "PointData/DimensionsPickerAction.h"

void convertToEigenMatrix(mv::Dataset<Points> dataset, mv::Dataset<Points> sourceDataset, DataMatrix& dataMatrix)
{
    // Compute num points
    std::vector<bool> enabledDims = sourceDataset->getDimensionsPickerAction().getEnabledDimensions();
    int numPoints = sourceDataset->getNumPoints();
    int numDimensions = sourceDataset->getNumDimensions();
    int numEnabledDims = std::count(enabledDims.begin(), enabledDims.end(), true);

    // Create list of enabled dimensions
    std::vector<int> enabledDimensions(numEnabledDims);
    int col = 0;
    for (int d = 0; d < numDimensions; d++)
        if (enabledDims[d])
            enabledDimensions[col++] = d;

    DataMatrix fullDataMatrix;
    fullDataMatrix.resize(numPoints, numEnabledDims);

#pragma omp parallel for
    for (int d = 0; d < enabledDimensions.size(); d++)
    {
        int dim = enabledDimensions[d];

        std::vector<float> dimData;
        sourceDataset->extractDataForDimension(dimData, dim);
        for (int i = 0; i < numPoints; i++)
            fullDataMatrix(i, d) = dimData[i];
    }

    // If the dataset was a subset or subset chain, take only a portion of the matrix by indexing
    if (!dataset->isFull())
    {
        // FIXME Might need to change this into getIndicesIntoFullDataset,
        // because now it also goes down the subset chain of the non-derived data
        std::vector<uint32_t> indices;
        dataset->getGlobalIndices(indices);

        dataMatrix = fullDataMatrix(indices, Eigen::all);
    }
    else
        dataMatrix = fullDataMatrix;
}

void convertToEigenMatrixProjection(mv::Dataset<Points> dataset, DataMatrix& dataMatrix)
{
    mv::Dataset<Points> fullDataset = dataset->getFullDataset<Points>();

    // Compute num points
    std::vector<bool> enabledDims = dataset->getDimensionsPickerAction().getEnabledDimensions();
    int numPoints = dataset->getNumPoints();
    int numPointsOfFull = fullDataset->getNumPoints();
    int numDimensions = dataset->getNumDimensions();
    int numEnabledDims = std::count(enabledDims.begin(), enabledDims.end(), true);

    // Create list of enabled dimensions
    std::vector<int> enabledDimensions(numEnabledDims);
    int col = 0;
    for (int d = 0; d < numDimensions; d++)
        if (enabledDims[d])
            enabledDimensions[col++] = d;

    DataMatrix fullDataMatrix;
    fullDataMatrix.resize(numPointsOfFull, numEnabledDims);

#pragma omp parallel for
    for (int d = 0; d < numEnabledDims; d++)
    {
        int dim = enabledDimensions[d];

        std::vector<float> dimData;
        fullDataset->extractDataForDimension(dimData, dim);
        for (int i = 0; i < numPointsOfFull; i++)
            fullDataMatrix(i, d) = dimData[i];
    }

    // If the dataset was a subset or subset chain, take only a portion of the matrix by indexing
    if (!dataset->isFull())
    {
        // FIXME Might need to change this into getIndicesIntoFullDataset,
        // because now it also goes down the subset chain of the non-derived data
        std::vector<uint32_t> indices;
        dataset->getGlobalIndices(indices);

        dataMatrix = fullDataMatrix(indices, Eigen::all);
    }
    else
        dataMatrix = fullDataMatrix;
}
