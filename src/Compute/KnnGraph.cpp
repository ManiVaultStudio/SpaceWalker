#include "KnnGraph.h"

#include "SecondaryDistanceMeasures.h"
#include "IO/KnnGraphIO.h"

#include <algorithm>
#include <iostream>

KnnGraph::KnnGraph() :
    _numNeighbours(1)
{

}

void printIndices(std::string name, idx_t* indices, int k)
{
    std::cout << "Indices (5 first results): " << name << std::endl;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < k+1; j++)
            printf("%5zd ", indices[i * (k + 1) + j]);
        printf("\n");
    }

    //printf("FAISS I (5 last results)=\n");
    //for (int i = data.rows() - 5; i < data.rows(); i++) {
    //    for (int j = 0; j < k; j++)
    //        printf("%5zd ", I[i * (k + 1) + j]);
    //    printf("\n");
    //}
}

void printIndices(std::string name, std::vector<int>& indices, int k)
{
    std::cout << "Indices (5 first results): " << name << std::endl;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < k + 1; j++)
            printf("%5zd ", indices[i * (k + 1) + j]);
        printf("\n");
    }
}

void printDistances(std::string name, float* distances, int k)
{
    std::cout << "Distances (5 first results): " << name << std::endl;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < k + 1; j++)
            printf("%7g ", distances[i * (k + 1) + j]);
        printf("\n");
    }
}

// Build KNN sub-graph from bigger graph
void KnnGraph::build(const KnnGraph& graph, int numNeighbours)
{
    assert(graph.getNumNeighbours() > numNeighbours);

    _numNeighbours = numNeighbours;

    const auto& graphNeighbours = graph.getNeighbours();
    _neighbours.resize(graphNeighbours.size(), std::vector<int>(numNeighbours));

    for (int i = 0; i < _neighbours.size(); i++)
    {
        for (int k = 0; k < numNeighbours; k++)
        {
            _neighbours[i][k] = graphNeighbours[i][k];
        }
    }
}

void KnnGraph::build(const DataMatrix& data, const knn::Index& index, int numNeighbours)
{
    std::vector<int> indices;
    std::vector<float> distances;

    int k = numNeighbours + 1; // Plus one to account for the query point itself being in the results

    index.search(data, k, indices, distances);

    // print results
    printIndices("INDEX", indices, k);
    //printDistances("INDEX", distances.data(), k);

    _numNeighbours = numNeighbours;
    _neighbours.clear();
    _neighbours.resize(data.rows(), std::vector<int>(_numNeighbours));

    int progressTick = std::max(1LL, data.rows() / 100);
#pragma omp parallel for
    for (int i = 0; i < data.rows(); i++)
    {
        if (i % progressTick == 0) std::cout << "Building graph: " << i << "/" << data.rows() << std::endl;
        for (int j = 0; j < _numNeighbours; j++)
        {
            _neighbours[i][j] = indices[i * k + j + 1];
        }
    }
}

void KnnGraph::build(const KnnGraph& graph, int numNeighbours, bool shared)
{
    _numNeighbours = numNeighbours;
    _neighbours.clear();
    _neighbours.resize(graph.getNeighbours().size(), std::vector<int>(_numNeighbours));

    computeSharedNeighboursBitset(graph.getNeighbours(), _neighbours, numNeighbours);
}

void KnnGraph::readFromFile(QString filePath)
{
    KnnGraphImporter::read(filePath, *this);
}

void KnnGraph::writeToFile()
{
    KnnGraphExporter::write(*this);
}
