#include "KnnIndex.h"

#include <QDebug>
#include <iostream>
#include <iomanip>

#include <fstream>
#include <sstream>

void writeDataMatrixToDisk(const DataMatrix& dataMatrix)
{
    uint32_t numPoints = (uint32_t)dataMatrix.rows();
    uint32_t numDimensions = (uint32_t)dataMatrix.cols();

    // Linearize data
    std::vector<float> linearData(numPoints * numDimensions);
    int c = 0;
    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
            linearData[c++] = dataMatrix(i, j);
    }
    std::cout << "Linearized data for export" << std::endl;
    // Write to file
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream fileName;
    fileName << "data_matrix";
    fileName << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    fileName << ".data";
    std::cout << "Writing to file: " << fileName.str() << std::endl;
    std::ofstream myfile(fileName.str(), std::ios::out | std::ios::binary);
    if (!myfile) {
        std::cout << "Cannot open file for writing data matrix!" << std::endl;
        return;
    }
    myfile.write((char*)&numPoints, sizeof(uint32_t));
    myfile.write((char*)&numDimensions, sizeof(uint32_t));

    for (size_t i = 0; i < linearData.size(); i++)
    {
        if (i % 10000 == 0) std::cout << "Progress: " << i << std::endl;
        myfile.write((char*)&linearData[i], sizeof(uint32_t));
    }

    myfile.close();
    std::cout << "Data matrix written to file" << std::endl;
}

void createFaissIndex(faiss::IndexFlat*& index, int numDimensions, knn::Metric metric)
{
    switch (metric)
    {
    case knn::Metric::MANHATTAN:
        index = new faiss::IndexFlat(numDimensions, faiss::METRIC_L1); break;
    case knn::Metric::EUCLIDEAN:
        index = new faiss::IndexFlat(numDimensions, faiss::METRIC_L2); break;
    case knn::Metric::COSINE:
        index = new faiss::IndexFlat(numDimensions, faiss::METRIC_INNER_PRODUCT); break;
    }
}

void createAnnoyIndex(AnnoyIndex*& index, int numDimensions)
{
    index = new AnnoyIndex(numDimensions);
}

namespace knn
{
    Index::Index() :
        _metric(Metric::EUCLIDEAN)
    {

    }

    Index::~Index()
    {
        if (_annoyIndex != nullptr)
            delete _annoyIndex;
        if (_faissIndex != nullptr)
            delete _faissIndex;
    }

    void Index::create(int numDimensions, Metric metric)
    {
        _metric = metric;
        if (_preciseKnn)
            createFaissIndex(_faissIndex, numDimensions, metric);
        else
            createAnnoyIndex(_annoyIndex, numDimensions);
    }

    void Index::addData(const DataMatrix& data)
    {
        size_t numPoints = data.rows();
        size_t numDimensions = data.cols();

        std::vector<float> indexData;
        linearizeData(data, indexData);

        if (_preciseKnn)
        {
            _faissIndex->add(numPoints, indexData.data());
        }
        else
        {
            //_annoyIndex->load("test.ann");
            //writeDataMatrixToDisk(data);
            //_annoyIndex->on_disk_build("test.ann");
            for (size_t i = 0; i < numPoints; ++i) {
                if (i % 10000 == 0) qDebug() << "Add Progress: " << i;
                _annoyIndex->add_item((int) i, indexData.data() + (i * numDimensions));
                if (i % 100 == 0)
                    std::cout << "Loading objects ...\t object: " << i + 1 << "\tProgress:" << std::fixed << std::setprecision(2) << (double)i / (double)(numPoints + 1) * 100 << "%\r";
            }

            std::cout << "Building index.." << std::endl;
            _annoyIndex->build((int) (10 * numDimensions));
            //_annoyIndex->save("test.ann");
        }
    }

    void Index::search(const DataMatrix& data, int k, std::vector<int>& indices, std::vector<float>& distances) const
    {
        int numPoints = data.rows();
        int numDimensions = data.cols();

        // Put eigen matrix into flat float vector
        std::vector<float> query;
        linearizeData(data, query);

        // Initialize result vectors
        size_t resultSize = numPoints * k;
        indices.resize(resultSize);
        distances.resize(resultSize);

        if (_preciseKnn)
        {
            idx_t* I = new idx_t[resultSize];

            _faissIndex->search(numPoints, query.data(), k, distances.data(), I);

            indices.assign(I, I + resultSize);

            delete[] I;
        }
        else
        {

            int search_k = -1; // defaults to n_trees * k
#pragma omp parallel for
            for (int i = 0; i < numPoints; i++)
            {
                std::vector<int> tempIndices;
                std::vector<float> tempDistances;

                _annoyIndex->get_nns_by_item(i, k, -1, &tempIndices, &tempDistances);

                for (unsigned int m = 0; m < static_cast<unsigned int>(k); m++) {
                    indices[i * k + m] = tempIndices[m];
                    distances[i * k + m] = tempDistances[m];
                }

            }

        }
    }

    void Index::linearizeData(const DataMatrix& data, std::vector<float>& highDimArray) const
    {
        size_t numPoints = data.rows();
        size_t numDimensions = data.cols();

        // Put eigen matrix into flat float vector
        highDimArray.resize(numPoints * numDimensions);

        int idx = 0;
        for (int i = 0; i < numPoints; i++)
        {
            if (_metric == Metric::COSINE)
            {
                double len = 0;
                for (int d = 0; d < numDimensions; d++)
                {
                    double dd = data(i, d);
                    len += dd * dd;
                }
                len = sqrt(len);

                for (int d = 0; d < numDimensions; d++)
                {
                    highDimArray[idx++] = data(i, d) / len;
                }
            }
            else
            {
                for (int d = 0; d < numDimensions; d++)
                    highDimArray[idx++] = data(i, d);
            }
            if (i % 10000 == 0) qDebug() << "Convert Data Progress: " << i;
        }
    }
}
