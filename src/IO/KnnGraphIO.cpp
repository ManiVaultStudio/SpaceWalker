#include "KnnGraphIO.h"

#include "Types.h"
#include "Compute/KnnGraph.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

void KnnGraphImporter::read(QString filePath, KnnGraph& graph)
{
    std::ifstream myfile(filePath.toStdString(), std::ios::in | std::ios::binary);
    if (!myfile) {
        std::cout << "Cannot open file for reading KNN graph!" << std::endl;
        return;
    }

    std::cout << "Reading KNN file: " << filePath.toStdString() << std::endl;
    auto& neighbours = graph._neighbours;

    char intType;
    bigint numPoints;
    int numNeighbours;

    //myfile.read((char*)&intType, sizeof(char));

    myfile.read((char*)&numPoints, sizeof(bigint));
    myfile.read((char*)&numNeighbours, sizeof(bigint));
    
    std::cout << "Reading " << numPoints << " points with " << numNeighbours << " neighbours" << std::endl;
    neighbours.resize(numPoints, std::vector<int>(numNeighbours));
    graph._numNeighbours = numNeighbours;

    for (int i = 0; i < numPoints; i++)
    {
        if (i % (numPoints / 50) == 0) std::cout << "Reading progress: " << i << "/" << numPoints << std::endl;
        for (int j = 0; j < numNeighbours; j++)
        {
            myfile.read((char*)&neighbours[i][j], sizeof(int));
        }
    }
    myfile.close();
    std::cout << "KNN graph imported!" << std::endl;
}

void KnnGraphExporter::write(const KnnGraph& graph)
{
    const std::vector<std::vector<int>>& neighbours = graph.getNeighbours();
    uint32_t numPoints = (uint32_t) neighbours.size();
    uint32_t numNeighbours = (uint32_t) graph.getNumNeighbours();

    // Linearize data
    std::vector<int> linearNeighbours(numPoints * numNeighbours);
    int c = 0;
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& n = neighbours[i];
        for (int j = 0; j < numNeighbours; j++)
            linearNeighbours[c++] = neighbours[i][j];
    }
    std::cout << "Linearized data for export" << std::endl;
    // Write to file
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream fileName;
    fileName << "knngraph";
    fileName << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    fileName << ".knn";
    std::cout << "Writing to file: " << fileName.str() << std::endl;
    std::ofstream myfile(fileName.str(), std::ios::out | std::ios::binary);
    if (!myfile) {
        std::cout << "Cannot open file for writing KNN graph!" << std::endl;
        return;
    }
    myfile.write((char*)&numPoints, sizeof(uint32_t));
    myfile.write((char*)&numNeighbours, sizeof(uint32_t));

    for (size_t i = 0; i < linearNeighbours.size(); i++)
    {
        if (i % 10000 == 0) std::cout << "Progress: " << i << std::endl;
        myfile.write((char*)&linearNeighbours[i], sizeof(uint32_t));
    }

    myfile.close();
    std::cout << "Knn graph written to file" << std::endl;
}
