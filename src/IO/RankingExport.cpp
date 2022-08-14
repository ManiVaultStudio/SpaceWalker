#include "RankingExport.h"

#include "DataStore.h"
#include "Compute/FloodFill.h"
#include "Compute/KnnGraph.h"
#include "Compute/Filters.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma warning(push)
#pragma warning(disable:4996) // Disable security warning of localtime
void writeDimensionRanking(const std::vector<std::vector<int>>& ranking, const std::vector<QString>& names)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream fileName;
    fileName << "rankings";
    fileName << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    fileName << ".csv";

    std::ofstream myfile;
    myfile.open(fileName.str());
    for (int i = 0; i < ranking.size(); i++)
    {
        for (int d = 0; d < ranking[i].size(); d++)
        {
            if (d != 0) myfile << ',';
            myfile << names[ranking[i][d]].toStdString();
        }
        myfile << std::endl;
    }

    myfile.close();
    std::cout << "Rankings written to file" << std::endl;
}
#pragma warning(pop)

void exportRankings(DataStorage& dataStore, FloodFill& floodFill, KnnGraph& knnGraph, filters::FilterType filterType, filters::SpatialPeakFilter spatialFilter, filters::HDFloodPeakFilter hdFilter, bool restrictToFloodNodes, const std::vector<QString>& names)
{
    std::vector<std::vector<int>> perPointDimRankings(dataStore.getNumPoints());
    for (int i = 0; i < dataStore.getNumPoints(); i++)
    {
        FloodFill exportFloodFill(floodFill.getNumWaves());
        exportFloodFill.compute(knnGraph, i);

        switch (filterType)
        {
        case filters::FilterType::SPATIAL_PEAK:
            if (restrictToFloodNodes)
                spatialFilter.computeDimensionRanking(i, dataStore.getDataView(), dataStore.getVariances(), dataStore.getProjectionView(), dataStore.getProjectionSize(), perPointDimRankings[i], exportFloodFill.getAllNodes());
            else
                spatialFilter.computeDimensionRanking(i, dataStore.getDataView(), dataStore.getVariances(), dataStore.getProjectionView(), dataStore.getProjectionSize(), perPointDimRankings[i]);
            break;
        case filters::FilterType::HD_PEAK:
            hdFilter.computeDimensionRanking(i, dataStore.getDataView(), dataStore.getVariances(), exportFloodFill, perPointDimRankings[i]);
            break;
        }
    }

    writeDimensionRanking(perPointDimRankings, names);
}
