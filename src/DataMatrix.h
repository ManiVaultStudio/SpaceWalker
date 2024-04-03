#pragma once

#include "Set.h"
#include "PointData/PointData.h"

#include <Eigen/Eigen>
#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>

using DataMatrix = Eigen::Matrix<float, -1, -1, Eigen::ColMajor>;

void convertToEigenMatrix(mv::Dataset<Points> dataset, mv::Dataset<Points> sourceDataset, DataMatrix& dataMatrix);

void convertToEigenMatrixProjection(mv::Dataset<Points> dataset, DataMatrix& dataMatrix);
