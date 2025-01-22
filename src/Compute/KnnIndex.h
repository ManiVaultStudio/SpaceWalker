#pragma once

#include "DataMatrix.h"

#pragma warning(push, 0) // annoylib.h has some warnings that are 'annoy'ing
#define ANNOYLIB_MULTITHREADED_BUILD
#include <annoylib.h>
#include <kissrandom.h>
#pragma warning(pop)

#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>

using AnnoyIndex = Annoy::AnnoyIndex<int, float, Annoy::Angular, Annoy::Kiss32Random, Annoy::AnnoyIndexMultiThreadedBuildPolicy>;

using idx_t = int64_t;

namespace knn
{
    enum class Metric
    {
        EUCLIDEAN, MANHATTAN, COSINE, ANGULAR
    };

    class Index
    {
    public:
        Index();
        ~Index();

        void setPrecise(bool precise) { _preciseKnn = precise; }

        void create(int numDimensions, Metric metric);
        void addData(const DataMatrix& data);
        void search(const DataMatrix& data, int numNeighbours, std::vector<int>& indices, std::vector<float>& distances) const;

    private:
        void linearizeData(const DataMatrix& data, std::vector<float>& highDimArray) const;

    private:
        AnnoyIndex*                         _annoyIndex     = nullptr;
        faiss::IndexFlat*                   _faissIndex     = nullptr;

        Metric                              _metric;

        bool                                _preciseKnn     = true;
    };
}
