#pragma once

#include "DataMatrix.h"

// Eigen::IndexedView<Eigen::MatrixXf, std::vector<int>, Eigen::internal::AllRange<-1>>

class DataStorage
{
public:
    // Base getters
    DataMatrix& getBaseData() { return _dataMatrix; }
    DataMatrix& getBaseFullProjection() { return _fullProjMatrix; }
    bool hasData() { return _hasBaseData; }

    // View getters
    DataMatrix& getDataView() { return _dataView; }
    DataMatrix& getFullProjectionView() { return _fullProjectionView; }
    DataMatrix& getProjectionView() { return _projectionView; }

    const std::vector<int>& getViewIndices() const { return _viewIndices; }

    // Auxilliary data getters
    std::vector<float>& getVariances() { return _variances; }

    int getNumPoints() { return _dataView.rows(); }
    int getNumDimensions() { return _dataView.cols(); }

    void setProjectionSize(float projectionSize) { _projectionSize = projectionSize; }
    float getProjectionSize() { return _projectionSize; }

    void createDataView()
    {
        _dataView = _dataMatrix;
        _fullProjectionView = _fullProjMatrix;

        _viewIndices.resize(_projectionView.rows());
        std::iota(_viewIndices.begin(), _viewIndices.end(), 0);

        _hasBaseData = true;
    }

    void createDataView(const std::vector<int>& indices)
    {
        _dataView = _dataMatrix(indices, Eigen::all);
        _fullProjectionView = _fullProjMatrix(indices, Eigen::all);

        _viewIndices = indices;
    }

    void createProjectionView(int xDim, int yDim)
    {
        getProjectionView() = getFullProjectionView()(Eigen::all, std::vector<int> { xDim, yDim });
    }

private:
    // Stored state
    // Base Data
    DataMatrix                      _dataMatrix;
    DataMatrix                      _fullProjMatrix;
    float                           _projectionSize = 0;

    // View
    DataMatrix                      _dataView;
    DataMatrix                      _fullProjectionView;
    DataMatrix                      _projectionView;

    std::vector<int>                _viewIndices;

    // Auxilliary data
    std::vector<float>              _variances;
    bool                            _hasBaseData = false;
};
