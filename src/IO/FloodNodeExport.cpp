#include "FloodNodeExport.h"

#include "Compute/FloodFill.h"
#include "Compute/KnnGraph.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma warning(push)
#pragma warning(disable:4996) // Disable security warning of localtime
void writeFloodNodes(const std::vector<std::vector<int>>& floodNodes)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream fileName;
    fileName << "flood_nodes";
    fileName << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    fileName << ".csv";

    std::ofstream myfile;
    myfile.open(fileName.str());
    for (int i = 0; i < floodNodes.size(); i++)
    {
        for (int d = 0; d < floodNodes[i].size(); d++)
        {
            if (d != 0) myfile << ',';
            myfile << floodNodes[i][d];
        }
        myfile << std::endl;
    }

    myfile.close();
    std::cout << "Flood nodes written to file" << std::endl;
}
#pragma warning(pop)

void exportFloodNodes(int numPoints, FloodFill& floodFill, KnnGraph& knnGraph)
{
    std::vector<std::vector<int>> perPointFloodNodes(numPoints);
    for (int p = 0; p < numPoints; p++)
    {
        FloodFill exportFloodFill(floodFill.getNumWaves());
        exportFloodFill.compute(knnGraph, p);

        // Store all flood nodes together
        perPointFloodNodes[p].resize(exportFloodFill.getTotalNumNodes() + exportFloodFill.getNumWaves());
        int n = 0;
        for (int i = 0; i < exportFloodFill.getNumWaves(); i++)
        {
            perPointFloodNodes[p][n++] = -1;
            for (int j = 0; j < exportFloodFill.getWaves()[i].size(); j++)
            {
                perPointFloodNodes[p][n++] = exportFloodFill.getWaves()[i][j];
            }
        }
    }

    writeFloodNodes(perPointFloodNodes);
}
