#include "LocalDimensionality.h"

#include "KnnGraph.h"

#include <QDebug>

namespace hdps
{
    namespace compute
    {
        float computeProjectionDiameter(const DataMatrix& projection, int xDim, int yDim)
        {
            float rangeX = projection.col(xDim).maxCoeff() - projection.col(xDim).minCoeff();
            float rangeY = projection.col(yDim).maxCoeff() - projection.col(yDim).minCoeff();

            float diameter = rangeX > rangeY ? rangeX : rangeY;

            return diameter;
        }

        void findNeighbourhood(const DataMatrix& projection, int centerId, float radius, std::vector<int>& neighbourhood, int xDim, int yDim)
        {
            float x = projection(centerId, xDim);
            float y = projection(centerId, yDim);

            float radSquared = radius * radius;

            neighbourhood.clear();

            for (int i = 0; i < projection.rows(); i++)
            {
                float xd = projection(i, xDim) - x;
                float yd = projection(i, yDim) - y;

                float magSquared = xd * xd + yd * yd;

                if (magSquared > radSquared)
                    continue;

                neighbourhood.push_back(i);
            }
        }

        void computeSpatialLocalDimensionality(DataMatrix& dataMatrix, DataMatrix& projMatrix, std::vector<float>& colors)
        {
            float diameter = computeProjectionDiameter(projMatrix, 0, 1);

            std::vector<std::vector<int>> neighbourhoods(projMatrix.rows());

            std::vector<int> dimensionalities(projMatrix.rows());
            for (int i = 0; i < projMatrix.rows(); i++)
            {
                findNeighbourhood(projMatrix, i, diameter * 0.02f, neighbourhoods[i], 0, 1);

                if (neighbourhoods[i].size() < 2) { dimensionalities[i] = 10; continue; }

                auto neighbours = dataMatrix(neighbourhoods[i], Eigen::all);

                // Mean centering data.
                Eigen::MatrixXf centered = neighbours.rowwise() - neighbours.colwise().mean();
                // Compute the covariance matrix.
                Eigen::MatrixXf cov = centered.adjoint() * centered;
                cov = cov / (neighbours.rows() - 1);

                Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
                // Normalize eigenvalues to make them represent percentages.
                Eigen::VectorXf normalizedEigenValues = (eig.eigenvalues() / eig.eigenvalues().sum()).reverse();
                Eigen::MatrixXf evecs = eig.eigenvectors();

                // Compute how many eigenvectors contribute to 85% of variance
                float variancePercentage = 0;
                int j = 0;
                for (j = 0; j < normalizedEigenValues.size(); j++)
                {
                    variancePercentage += normalizedEigenValues(j);

                    if (variancePercentage > 0.85)
                    {
                        break;
                    }
                }

                dimensionalities[i] = j;
                //qDebug() << "Dimensionality: " << j;
            }

            // Compute colors
            colors.resize(dataMatrix.rows());

            int minDim = *std::min_element(dimensionalities.begin(), dimensionalities.end());
            int maxDim = *std::max_element(dimensionalities.begin(), dimensionalities.end());
            int range = maxDim - minDim;

            for (int i = 0; i < dimensionalities.size(); i++)
            {
                float p = (dimensionalities[i] - minDim) / (float)range;
                colors[i] = p;
            }
        }

        void computeHDLocalDimensionality(DataMatrix& dataMatrix, KnnGraph& knnGraph, std::vector<float>& normLD)
        {
            int numPoints = dataMatrix.rows();
            std::vector<int> dimensionalities(numPoints);

            for (int i = 0; i < numPoints; i++)
            {
                auto neighbours = dataMatrix(knnGraph.getNeighbours()[i], Eigen::all);

                // Mean centering data.
                Eigen::MatrixXf centered = neighbours.rowwise() - neighbours.colwise().mean();
                // Compute the covariance matrix.
                Eigen::MatrixXf cov = centered.adjoint() * centered;
                cov = cov / (neighbours.rows() - 1);

                Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
                // Normalize eigenvalues to make them represent percentages.
                Eigen::VectorXf normalizedEigenValues = (eig.eigenvalues() / eig.eigenvalues().sum()).reverse();
                Eigen::MatrixXf evecs = eig.eigenvectors();

                // Compute how many eigenvectors contribute to 85% of variance
                float variancePercentage = 0;
                int j = 0;
                for (j = 0; j < normalizedEigenValues.size(); j++)
                {
                    variancePercentage += normalizedEigenValues(j);

                    if (variancePercentage > 0.85)
                    {
                        break;
                    }
                }

                dimensionalities[i] = j;
                //qDebug() << "Dimensionality: " << j;
            }

            // Compute colors
            normLD.resize(dataMatrix.rows());

            int minDim = *std::min_element(dimensionalities.begin(), dimensionalities.end());
            int maxDim = *std::max_element(dimensionalities.begin(), dimensionalities.end());
            int range = maxDim - minDim;

            for (int i = 0; i < dimensionalities.size(); i++)
            {
                float p = (dimensionalities[i] - minDim) / (float)range;
                normLD[i] = p;
            }
        }
    }
}
