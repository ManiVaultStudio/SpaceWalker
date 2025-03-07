#pragma once

#include <ViewPlugin.h>

#include "util/PixelSelectionTool.h"

#include "DataStore.h"
#include "Types.h"
#include "Common.h"

#include "Actions/SettingsAction.h"
#include "graphics/Vector3f.h"
#include "Graph/GraphView.h"

#include "DataMatrix.h"
#include "Logging.h"

#include <actions/HorizontalToolbarAction.h>
#include <actions/ColorMap1DAction.h>

#include "Compute/FloodFill.h"
#include "Compute/KnnIndex.h"
#include "Compute/KnnGraph.h"
#include "Compute/Filters.h"

#include <QPoint>

using namespace mv::plugin;
using namespace mv::util;

class Points;
class Clusters;

class ScatterplotWidget;
class ProjectionView;

namespace mv
{
    class CoreInterface;
    class Vector2f;

    namespace gui {
        class DropWidget;
    }
}

enum class OverlayType
{
    NONE,
    DIM_VALUES,
    LOCAL_DIMENSIONALITY,
    DIRECTIONS
};

class SpaceWalkerPlugin : public ViewPlugin
{
    Q_OBJECT
    
public:
    SpaceWalkerPlugin(const PluginFactory* factory);
    ~SpaceWalkerPlugin() override;

    bool isDataInitialized() { return _dataInitialized; }

    void init() override;
    void resetState();

    void onDataEvent(mv::DatasetEvent* dataEvent);
    void onPointSelection();

    void computeGraphs();

    void computeStaticData();

    /**
     * Load one (or more datasets in the view)
     * @param datasets Dataset(s) to load
     */
    void loadData(const Datasets& datasets) override;

    /** Get number of points in the position dataset */
    std::uint32_t getNumberOfPoints() const;

public:
    void createSubset(const bool& fromSourceData = false, const QString& name = "");

public: // Dimension picking
    void setXDimension(const std::int32_t& dimensionIndex) { updateProjectionData(); }
    void setYDimension(const std::int32_t& dimensionIndex) { updateProjectionData(); }

public: // Data loading

    /** Invoked when the position points dataset changes */
    void positionDatasetChanged();

public: // Miscellaneous

    /** Get smart pointer to points dataset for point position */
    Dataset<Points>& getPositionDataset()               { return _positionDataset; }

    /** Get smart pointer to source of the points dataset for point position (if any) */
    Dataset<Points>& getPositionSourceDataset()         { return _positionSourceDataset; }

    Dataset<Clusters>& getSliceDataset()                { return _sliceDataset; }

    void setOverlayType(OverlayType type)               { _overlayType = type; }
    void setFilterLabelText(QString text)               { _filterLabel->setText(text); }
    void setFilterType(filters::FilterType type)        { _filterType = type; }

    SettingsAction& getSettingsAction()                 { return _settingsAction; }

protected:

    /** Updates the window title (displays the name of the view and the GUI name of the loaded points dataset) */
    void updateWindowTitle();

    /** Updates the scalar range in the color map */
    void updateColorMapActionScalarRange();

public:

    /** Get reference to the scatter plot widget */
    ScatterplotWidget& getScatterplotWidget()           { return *_scatterPlotWidget; }
    std::vector<ProjectionView*>& getProjectionViews()  { return _projectionViews; }
    ProjectionView*& getSelectedView()                  { return _selectedView; }

    filters::SpatialPeakFilter& getSpatialPeakFilter()  { return _spatialPeakFilter; }
    filters::HDFloodPeakFilter& getHDPeakFilter()       { return _hdFloodPeakFilter; }
    
    DataStorage& getDataStore()                         { return _dataStore; }
    float getProjectionSize()                           { return _dataStore.getProjectionSize(); }

public: // Flood fill
    void createKnnIndex(bool precise = true);
    void computeKnnGraph();
    void rebuildKnnGraph(int floodNeighbours) { _knnGraph.build(_dataStore.getBaseData(), _knnIndex, floodNeighbours); }

    FloodFill& getFloodFill() { return _floodFill; }

    void setFloodSteps(int numFloodSteps)
    {
        _floodFill.setNumWaves(numFloodSteps);
        _settingsAction.getFilterAction().setFloodSteps(numFloodSteps);
    }

    void useSharedDistances(bool useSharedDistances) { _useSharedDistances = useSharedDistances; }

public: // Slicing
    void onSliceIndexChanged();

private: // Updating functions
    void updateProjectionData();
    void updateSelection();

    void updateViewData(std::vector<Vector2f>& positions);
    void updateViewScalars();
    void updateFloodScalarOutput(const std::vector<float>& scalars);

private: // Mouse Interaction
    void notifyNewSelectedPoint();
    void mousePositionChanged(Vector2f mousePos);
    bool eventFilter(QObject* target, QEvent* event);

public: // Import / Export
    void exportDimRankings();
    void exportFloodnodes();
    void importKnnGraph();

private slots: // Graph
    void onLineClicked(dint dim);

public: // Mask
    bool hasMaskApplied();
    void clearMask();
    void useSelectionAsMask();
    void useSelectionAsDataView(std::vector<int>& indices);

public: // Serialization
    /**
    * Load plugin from variant map
    * @param Variant map representation of the plugin
    */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
    * Save plugin to variant map
    * @return Variant map representation of the plugin
    */
    QVariantMap toVariantMap() const override;

private:
    DataStorage                     _dataStore;

    // Data
    Dataset<Points>                 _positionDataset;           /** Smart pointer to points dataset for point position */
    Dataset<Points>                 _positionSourceDataset;     /** Smart pointer to source of the points dataset for point position (if any) */
    nint                            _numPoints;                 /** Number of point positions */

    std::vector<std::vector<float>> _normalizedData;
    std::vector<QString>            _enabledDimNames;
    bool                            _dataInitialized = false;
    std::vector<nint>               _mask;
    std::vector<float>              _colorScalars;

    // Interaction
    nint                            _selectedPoint = 0;
    nint                            _globalSelectedPoint = 0;
    dint                            _selectedDimension;

    bool                            _mousePressed = false;
    int                             _selectedViewIndex = 0;
    bool                            _loadingFromProject = false;

    // Filters
    QLabel*                         _filterLabel;
    filters::FilterType             _filterType;
    filters::SpatialPeakFilter      _spatialPeakFilter;
    filters::HDFloodPeakFilter      _hdFloodPeakFilter;

    // KNN
    bool                            _computeOnLoad = false;
    bool                            _graphAvailable = false;
    knn::Index                      _knnIndex;
    KnnGraph                        _knnGraph;
    KnnGraph                        _largeKnnGraph;
    KnnGraph                        _sourceKnnGraph;
    bool                            _useSharedDistances = false;
    bool                            _preloadedKnnGraph = false;

    // Slicing
    Dataset<Clusters>               _sliceDataset;
    int                             _currentSliceIndex = 0;

    // Masked KNN
    DataMatrix                      _maskedDataMatrix;
    DataMatrix                      _maskedProjMatrix;
    knn::Index                      _maskedKnnIndex;
    KnnGraph                        _maskedKnnGraph;
    KnnGraph                        _maskedSourceKnnGraph;
    bool                            _maskedKnn = false;

    // Floodfill
    Dataset<Points>                 _floodScalars;
    FloodFill                       _floodFill;

    // Graph
    GraphView*                      _graphView;
    std::vector<std::vector<int>>   _bins;
    QTimer*                         _graphTimer;

    // Local dimensionality
    std::vector<float>              _localSpatialDimensionality;
    std::vector<float>              _localHighDimensionality;

    // Directions
    std::vector<Vector2f>           _directions;

    // Overlays
    OverlayType                     _overlayType;

protected:
    ScatterplotWidget*              _scatterPlotWidget;
    std::vector<ProjectionView*>    _projectionViews;
    ProjectionView*                 _selectedView;

    mv::gui::DropWidget*        _dropWidget;
    SettingsAction              _settingsAction;
    ColorMap1DAction            _colorMapAction;            /** Color map action */
    HorizontalToolbarAction     _primaryToolbarAction;      /** Horizontal toolbar for primary content */
    HorizontalToolbarAction     _secondaryToolbarAction;    /** Secondary toolbar for secondary content */
};

// =============================================================================
// Factory
// =============================================================================

class SpaceWalkerPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.biovault.SpaceWalkerPlugin"
                      FILE  "SpaceWalkerPlugin.json")
    
public:
    SpaceWalkerPluginFactory(void);
    ~SpaceWalkerPluginFactory(void) override {}

    ViewPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
