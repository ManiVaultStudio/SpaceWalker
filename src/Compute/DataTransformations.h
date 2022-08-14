#pragma once

#include "DataMatrix.h"

#include <iostream>

void standardizeData(DataMatrix& dataMatrix, std::vector<float>& variances);
void normalizeData(const DataMatrix& dataMatrix, std::vector<std::vector<float>>& normalizedData);
