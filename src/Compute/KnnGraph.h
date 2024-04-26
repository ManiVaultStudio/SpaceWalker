#pragma once

#include "Types.h"
#include "KnnIndex.h"

#include <QString>

class KnnGraphImporter;
class KnnGraphExporter;

class KnnGraph
{
public:
    KnnGraph();

    const std::vector<std::vector<nint>>& getNeighbours() const { return _neighbours; }
    int getNumNeighbours() const { return _numNeighbours; }

    void build(const KnnGraph& graph, int numNeighbours);
    void build(const DataMatrix& data, const knn::Index& index, int numNeighbours);
    void build(const KnnGraph& graph, int numNeighbours, bool shared);

    void readFromFile(QString filePath);
    void writeToFile();

private:
    std::vector<std::vector<nint>> _neighbours;
    int _numNeighbours;

    friend class SpaceWalkerPlugin;
    friend class KnnGraphImporter;
    friend class KnnGraphExporter;
};
