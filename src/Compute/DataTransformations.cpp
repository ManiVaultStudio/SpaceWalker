#include "DataTransformations.h"

void standardizeData(DataMatrix& dataMatrix, std::vector<float>& variances)
{
    int numPoints = dataMatrix.rows();
    int numDimensions = dataMatrix.cols();

    std::vector<float> means(numDimensions);
    variances.resize(numDimensions);

#pragma omp parallel for
    for (int d = 0; d < numDimensions; d++)
    {
        // Compute mean
        float mean = dataMatrix.col(d).mean();

        //mean /= numPoints;
        means[d] = mean;

        // Compute variance
        variances[d] = (dataMatrix.col(d).array() - mean).square().sum() / numPoints;

        // If variance is 0, then don't try to divide the data by it
        if (variances[d] <= 0) continue;

        // Standardize data
        float invStddev = 1.0f / sqrt(variances[d]);

        for (int i = 0; i < numPoints; i++)
        {
            dataMatrix(i, d) -= mean;
            dataMatrix(i, d) *= invStddev;
        }
    }

    //// Print means and variances
    //for (int d = 0; d < numDimensions; d++)
    //    std::cout << "Mean " << d << " " << means[d] << std::endl;
    //for (int d = 0; d < numDimensions; d++)
    //    std::cout << "Variance " << d << " " << variances[d] << std::endl;
}

void normalizeData(const DataMatrix& dataMatrix, std::vector<std::vector<float>>& normalizedData)
{
    normalizedData.resize(dataMatrix.cols(), std::vector<float>(dataMatrix.rows()));
#pragma omp parallel for
    for (int d = 0; d < dataMatrix.cols(); d++)
    {
        auto col = dataMatrix.col(d);

        float minVal = *std::min_element(col.begin(), col.end());
        float maxVal = *std::max_element(col.begin(), col.end());
        float range = maxVal - minVal;
        if (range == 0) range = 1;

        for (int i = 0; i < dataMatrix.rows(); i++)
            normalizedData[d][i] = std::min(0.99999f, (col(i) - minVal) / range);
    }
}
