#include "RandomWalks.h"

#include "KnnGraph.h"

#include <QDebug>
#include <random>

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);

namespace mv
{
    namespace compute
    {
        int binarySearchCDF(const std::vector<float>& cdf, float u)
        {
            // Binary search for u
            int itemFromLeft = 0;
            int itemFromRight = (int) cdf.size() - 1;
            while (itemFromLeft < itemFromRight)
            {
                int m = itemFromLeft + (itemFromRight - itemFromLeft) / 2;
                if (cdf[m] < u)
                    itemFromLeft = m + 1;
                else
                    itemFromRight = m;
            }
            return itemFromLeft;
        }

        void doRandomWalks(const DataMatrix& highDim, const DataMatrix& spatialMap, int selectedPoint, std::vector<std::vector<Vector2f>>& randomWalks)
        {
            int numDimensions = highDim.cols();

            //auto seedPoint = highDim.row(selectedPoint);

            randomWalks.resize(1);
            // Start X number of random walks
            for (int w = 0; w < 1; w++)
            {
                // Perform given number of iterations of a random walk
                auto currentNode = selectedPoint;

                std::vector<float> distances(highDim.rows(), std::numeric_limits<float>::max());
                std::vector<float> probs(highDim.rows());
                std::vector<float> cdf(highDim.rows(), 0);

                randomWalks[w].resize(10);

                // Iterations
                for (int i = 0; i < 10; i++)
                {
                    randomWalks[w][i] = Vector2f(spatialMap(currentNode, 0), spatialMap(currentNode, 1));
                    float probSum = 0;
                    float cdfSum = 0;
                    for (int j = 0; j < highDim.rows(); j++)
                    {
                        if (currentNode == j) { distances[j] = 1000; continue; }
                        distances[j] = (highDim.row(currentNode) - highDim.row(j)).lpNorm<1>();

                        probs[j] = (distances[j] == 0) ? 1 : 1 / (distances[j] * distances[j]);
                        probSum += probs[j];
                    }

                    // Normalize probabilities
                    for (int j = 0; j < probs.size(); j++)
                    {
                        probs[j] *= 1 / probSum;
                        cdfSum += probs[j];
                        cdf[j] = cdfSum;
                        //qDebug() << probs[j];
                    }

                    float u = distribution(generator);

                    int xi = binarySearchCDF(cdf, u);
                    float weight = probs[xi];

                    currentNode = xi;
                }
            }
        }

        void doRandomWalksKNN(const DataMatrix& highDim, const DataMatrix& spatialMap, const KnnGraph& knnGraph, int selectedPoint, std::vector<std::vector<int>>& randomWalks)
        {
            int numDimensions = highDim.cols();

            int k = (int) knnGraph.getNumNeighbours();
            int numWalks = 100;
            int numSteps = 10;

            randomWalks.resize(numWalks);
            // Start X number of random walks
            for (int w = 0; w < numWalks; w++)
            {
                int prevNode = -1;
                // Perform given number of iterations of a random walk
                auto currentNode = selectedPoint;

                randomWalks[w].resize(numSteps);

                // Iterations
                for (int i = 0; i < numSteps; i++)
                {
                    randomWalks[w][i] = currentNode;

                    int newNode = -1;
                    do
                    {
                        float u = distribution(generator) * 0.999f;
                        int knnIndex = (int)(u * k);

                        newNode = knnGraph.getNeighbours()[currentNode][knnIndex];
                    } while (newNode == prevNode);

                    prevNode = currentNode;
                    currentNode = newNode;
                }
            }
        }

        void traceLineage(const DataMatrix& data, const std::vector<std::vector<int>>& floodFill, std::vector<Vector2f>& positions, int seedIndex, std::vector<int>& lineage)
        {
            bigint numFloodNodes = 0;
            for (int i = 0; i < floodFill.size(); i++)
            {
                numFloodNodes += floodFill[i].size();
            }

            // Store all flood nodes together
            std::vector<nint> floodNodes(numFloodNodes);
            nint n = 0;
            for (int i = 0; i < floodFill.size(); i++)
            {
                for (int j = 0; j < floodFill[i].size(); j++)
                {
                    floodNodes[n++] = floodFill[i][j];
                }
            }

            int currentNode = seedIndex;
            Vector2f currentNodePos = positions[seedIndex];

            // Determine initial direction
            Vector2f sumPos(0, 0);
            for (int i = 0; i < floodNodes.size(); i++)
            {
                sumPos += positions[floodNodes[i]];
            }
            sumPos = sumPos / floodNodes.size();
            Vector2f initDir(0, 0);// = (sumPos - currentNodePos) / (sumPos - currentNodePos).length();
            Vector2f currentDir = initDir;

            // Find neighbours
            while (lineage.size() < 10)
            {
                std::vector<int> neighbours;
                for (int i = 0; i < floodNodes.size(); i++)
                {
                    int floodIndex = floodNodes[i];

                    if (floodIndex == currentNode) continue;

                    Vector2f nodePos = positions[floodIndex];
                    if (abs(nodePos.x - currentNodePos.x) < 1.1f && abs(nodePos.y - currentNodePos.y) < 1.1f)
                    {
                        neighbours.push_back(floodIndex);
                    }
                }

                if (neighbours.size() < 1) return;

                // Calculate probabilities based on cosine similarity to current direction
                std::vector<float> probs(neighbours.size());
                std::vector<Vector2f> dirs(neighbours.size());
                float totalProb = 0;
                for (int i = 0; i < neighbours.size(); i++)
                {
                    Vector2f loc = positions[neighbours[i]];
                    Vector2f dir = (loc - currentNodePos) / (loc - currentNodePos).length();
                    dirs[i] = dir;

                    float sim = (currentDir.x * dir.x + currentDir.y * dir.y) / (dir.length());
                    if (sim <= 0) sim = 0.001f;
                    probs[i] = sim;
                    totalProb += sim;
                }
                if (totalProb == 0) return;
                // Normalize probabilities
                float cdfSum = 0;
                std::vector<float> cdf(probs.size());
                for (int i = 0; i < probs.size(); i++)
                {
                    probs[i] /= totalProb;
                    cdfSum += probs[i];
                    cdf[i] = cdfSum;
                }

                // Pick a neighbour based on probabilities
                float u = distribution(generator) * 0.999;

                int xi = binarySearchCDF(cdf, u);
                float weight = probs[xi];

                currentNode = neighbours[xi];
                currentNodePos = positions[currentNode];
                currentDir = dirs[xi];

                lineage.push_back(currentNode);
            }
        }
    }
}
