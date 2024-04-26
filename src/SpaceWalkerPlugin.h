#pragma once

#include <ViewPlugin.h>

#include "UserInterface.h"
#include "DataStore.h"
#include "Types.h"

#include "graphics/Vector3f.h"
#include "Widgets/GraphView.h"
#include "Widgets/MetadataView.h"
#include "Logging.h"

#include "Compute/FloodFill.h"
#include "Compute/KnnIndex.h"
#include "Compute/KnnGraph.h"
#include "Compute/Filters.h"

#include <vector>

using namespace mv::plugin;
using namespace mv::util;

class Points;
class Clusters;

class MainView;
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

public: // User interface
    /** Get reference to the user interface elements */
    UserInterface&                  getUI()             { return _userInterface; }

protected:

    /** Updates the window title (displays the name of the view and the GUI name of the loaded points dataset) */
    void updateWindowTitle();

    /** Updates the scalar range in the color map */
    void updateColorMapActionScalarRange();

public:
    DataStorage& getDataStore()                         { return _dataStore; }
    float getProjectionSize()                           { return _dataStore.getProjectionSize(); }

    filters::Filters& getFilters()                      { return _filters; }

public: // Flood fill
    void createKnnIndex();
    void computeKnnGraph();
    void rebuildKnnGraph(int floodNeighbours) { _knnGraph.build(_dataStore.getBaseData(), _knnIndex, floodNeighbours); }

    FloodFill& getFloodFill() { return _floodFill; }

    void setFloodSteps(int numFloodSteps)
    {
        _floodFill.setNumWaves(numFloodSteps);
        getUI().getSettingsAction().getFilterAction().setFloodSteps(numFloodSteps);
    }

    void useSharedDistances(bool useSharedDistances) { _useSharedDistances = useSharedDistances; }

private: // Slicing
    void onSliceIndexChanged();

private: // Metadata display
    void onMetadataChanged();
    void computeCellMetadata();

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

private: // Key Interaction
    void onKeyPressed(QKeyEvent* event);
    void onKeyReleased(QKeyEvent* event);

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
    std::vector<Vector3f>           _colors;

    // Interaction
    nint                            _selectedPoint = 0;
    nint                            _globalSelectedPoint = 0;
    dint                            _selectedDimension;

    bool                            _mousePressed = false;
    int                             _selectedViewIndex = 0;
    bool                            _loadingFromProject = false;

    // Filters
    filters::Filters                _filters;

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

    // Metadata visualisation
    Dataset<Clusters>               _metadataDataset;
    bool                            _showingMetadata = false;
    QVector<Dataset<Clusters>>      _metadataDatasets;
    MetadataView                    _metadataView;
    std::vector<std::vector<int>>   _metadataIndexing;

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

    UserInterface                   _userInterface;
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
    SpaceWalkerPluginFactory(void) {}
    ~SpaceWalkerPluginFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    ViewPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
