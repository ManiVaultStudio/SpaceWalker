#include "Directions.h"

#include "FloodFill.h"
#include "graphics/Vector2f.h"

void computeDirection(DataMatrix& dataMatrix, DataMatrix& projMatrix, KnnGraph& knnGraph, int numSteps, std::vector<mv::Vector2f>& directions)
{
    for (int p = 0; p < dataMatrix.rows(); p++)
    {
        FloodFill directionsFloodFill(numSteps);
        directionsFloodFill.compute(knnGraph, p);

        int depth = 3;
        bigint numNodes = 0;
        for (int i = 0; i < depth; i++)
        {
            numNodes += directionsFloodFill.getWaves()[i].size();
        }

        Eigen::MatrixXf nodeLocations(numNodes, 2);
        int n = 0;
        for (int i = 0; i < depth; i++)
        {
            for (int j = 0; j < directionsFloodFill.getWaves()[i].size(); j++)
            {
                int index = directionsFloodFill.getWaves()[i][j];

                nodeLocations(n, 0) = projMatrix(index, 0);
                nodeLocations(n, 1) = projMatrix(index, 1);
                n++;
            }
        }

        //
        // Mean centering data.
        Eigen::MatrixXf centered = nodeLocations.rowwise() - nodeLocations.row(0);
        // Compute the covariance matrix.
        Eigen::MatrixXf cov = centered.adjoint() * centered;
        cov = cov / (nodeLocations.rows() - 1);
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
        // Normalize eigenvalues to make them represent percentages.
        Eigen::MatrixXf evecs = eig.eigenvectors();
        // Get the two major eigenvectors and omit the others.
        mv::Vector2f majorEigenVector;
        if (eig.eigenvalues()(0) > eig.eigenvalues()(1))
            majorEigenVector.set(evecs(0, 0), evecs(1, 0));
        else
            majorEigenVector.set(evecs(0, 1), evecs(1, 1));

        directions.push_back(mv::Vector2f(projMatrix(p, 0), projMatrix(p, 1)));
        directions.push_back(majorEigenVector);
    }
}
