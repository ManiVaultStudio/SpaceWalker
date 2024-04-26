#pragma once

#include <vector>
#include <numeric>
#include <bitset>
#include <iostream>

void computeSharedNeighboursBruteForce(const std::vector<std::vector<int>>& neighbours, std::vector<std::vector<int>>& _neighbours, int k)
{
    std::cout << "Building graph.." << std::endl;

    Eigen::MatrixXi simM = Eigen::MatrixXi::Zero(neighbours.size(), neighbours.size());
    //simM.resize(neighbours.size(), neighbours.size());
    for (int i = 0; i < neighbours.size()-1; i++)
    {
        if (i % 100 == 0) std::cout << "Graph progress: " << i << "/" << neighbours.size() << std::endl;
        const std::vector<int>& knn1 = neighbours[i];

        std::vector<int> similarity(neighbours.size(), 0);
        for (int j = i + 1; j < neighbours.size(); j++)
        {
            const std::vector<int>& knn2 = neighbours[j];

            std::unordered_set<int> hmap(knn1.begin(), knn1.end());
            int count = 0;
            for (int a : knn2)
                if (hmap.find(a) != hmap.end()) {
                    count++;
                }
            
            simM(i, j) = count;
            simM(j, i) = count;
        }
    }

    for (int i = 0; i < neighbours.size(); i++)
    {
        auto row = simM.row(i);
        std::vector<int> sim(row.begin(), row.end());

        std::vector<int> idx(sim.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&sim](int i1, int i2) {return sim[i1] > sim[i2]; });

        for (int j = 0; j < k; j++)
        {
            _neighbours[i][j] = idx[j];
        }
    }
}

// ONLY WORKS UP TO 5000 POINTS, OR CAN BE CHANGED BY ALTERING THE NUMNODES PARAMETER
void computeSharedNeighboursBitset(const std::vector<std::vector<int>>& neighbours, std::vector<std::vector<int>>& _neighbours, int k)
{
    const int numNodes = 5000;
    // Convert neighbours to bitsets
    std::vector<std::bitset<numNodes>> bitsets(numNodes);
    std::cout << "Making bitsets.. " << std::endl;
    for (int i = 0; i < neighbours.size(); i++)
    {
        //std::bitset<numNodes>& bits = bitsets[i];
        const std::vector<int>& knn = neighbours[i];
        for (const int& idx : knn)
        {
            bitsets[i].set(size_t(idx), true);
        }
    }
    std::cout << "Print bits" << std::endl;
    std::cout << bitsets[0] << std::endl;

    // Make similarity matrix
    std::vector<std::vector<int>> simM(numNodes, std::vector<int>(numNodes, 0));

    // Compute similarities
    for (int i = 0; i < neighbours.size() - 1; i++)
    {
        if (i % 100 == 0) std::cout << "Graph progress: " << i << "/" << neighbours.size() << std::endl;

        for (int j = i + 1; j < neighbours.size(); j++)
        {
            std::bitset<numNodes> intersection = bitsets[i] & bitsets[j];
            int count = (int) intersection.count();
            simM[i][j] = count;
            simM[j][i] = count;
        }
    }

    for (int i = 0; i < neighbours.size(); i++)
    {
        std::vector<int> sim = simM[i];

        std::vector<int> idx(sim.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&sim](int i1, int i2) {return sim[i1] > sim[i2]; });

        for (int j = 0; j < k; j++)
        {
            _neighbours[i][j] = idx[j];
        }
    }
}
