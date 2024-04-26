#include "SpaceWalkerPlugin.h"
#include "Widgets/MainView.h"
#include "Widgets/ProjectionView.h"
#include "DataHierarchyItem.h"
#include "Application.h"
#include "event/Event.h"

#include "PointData/DimensionsPickerAction.h"

#include "PointData/PointData.h"
#include "ColorData/ColorData.h"
#include "ClusterData/ClusterData.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"
#include "widgets/DropWidget.h"

#include <DatasetsMimeData.h>

#include "Compute/LocalDimensionality.h"
#include "Compute/DataTransformations.h"
#include "IO/RankingExport.h"
#include "IO/FloodNodeExport.h"
#include "Timer.h"

#include <QtCore>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QMetaType>
#include <QVector>
#include <QPoint>
#include <QFileDialog>

#include <algorithm>
#include <functional>
#include <limits>
#include <set>
#include <iostream>
#include <chrono>

Q_PLUGIN_METADATA(IID "nl.biovault.SpaceWalkerPlugin")

using namespace mv;
using namespace mv::util;

namespace
{
    void normalizeVector(std::vector<float>& v)
    {
        // Store scalars in floodfill dataset
        float scalarMin = std::numeric_limits<float>::max();
        float scalarMax = -std::numeric_limits<float>::max();

        // Compute min and max of scalars
        for (int i = 0; i < v.size(); i++)
        {
            if (v[i] < scalarMin) scalarMin = v[i];
            if (v[i] > scalarMax) scalarMax = v[i];
        }
        float scalarRange = scalarMax - scalarMin;

        if (scalarRange != 0)
        {
            float invScalarRange = 1.0f / (scalarMax - scalarMin);
            // Normalize the scalars
#pragma omp parallel for
            for (int i = 0; i < v.size(); i++)
            {
                v[i] = (v[i] - scalarMin) * invScalarRange;
            }
        }
    }
}

SpaceWalkerPlugin::SpaceWalkerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _positionDataset(),
    _positionSourceDataset(),
    _numPoints(0),
    _userInterface(this),
    _graphView(new GraphView()),
    _selectedDimension(-1),
    _floodFill(10),
    _overlayType(OverlayType::NONE),
    _graphTimer(new QTimer(this))
{
    setObjectName("SpaceWalker");

    getWidget().setFocusPolicy(Qt::ClickFocus);

    _userInterface.init();

    // Connect signals from views
    connect(getUI().getProjectionViews()[0], &ProjectionView::viewSelected, this, [this]() { _selectedViewIndex = 1; updateViewScalars(); });
    connect(getUI().getProjectionViews()[1], &ProjectionView::viewSelected, this, [this]() { _selectedViewIndex = 2; updateViewScalars(); });
    connect(&getUI().getSelectedView(), &ProjectionView::viewSelected, this, [this]() { _selectedViewIndex = 3; updateViewScalars(); });

    connect(&getUI().getMainView(), &MainView::initialized, this, [this]()
    {
            getUI().getMainView().setColorMap(getUI().getColorMapAction().getColorMapImage().mirrored(false, true));
            getUI().getMainView().setScalarEffect(PointEffect::Color);
    });
    connect(getUI().getProjectionViews()[0], &ProjectionView::initialized, this, [this]() {getUI().getProjectionViews()[0]->setColorMap(getUI().getColorMapAction().getColorMapImage().mirrored(false, true)); });
    connect(getUI().getProjectionViews()[1], &ProjectionView::initialized, this, [this]() {getUI().getProjectionViews()[1]->setColorMap(getUI().getColorMapAction().getColorMapImage().mirrored(false, true)); });
    connect(&getUI().getSelectedView(), &ProjectionView::initialized, this, [this]() {getUI().getSelectedView().setColorMap(getUI().getColorMapAction().getColorMapImage().mirrored(false, true)); });

    connect(_graphView, &GraphView::lineClicked, this, &SpaceWalkerPlugin::onLineClicked);
    _graphTimer->setSingleShot(true);
    connect(_graphTimer, &QTimer::timeout, this, &SpaceWalkerPlugin::computeGraphs);

    connect(&getUI().getMainView(), &MainView::customContextMenuRequested, this, [this](const QPoint& point) {
        if (!_positionDataset.isValid())
            return;

        auto contextMenu = getUI().getSettingsAction().getContextMenu();
        
        contextMenu->addSeparator();

        _positionDataset->populateContextMenu(contextMenu);

        contextMenu->exec(getWidget().mapToGlobal(point));
    });

    getUI().getDropWidget().setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    getUI().getDropWidget().initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->text();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType , ColorType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);

        // Points dataset is about to be dropped
        if (dataType == PointType) {

            // Get points dataset from the core
            auto candidateDataset = mv::data().getDataset<Points>(datasetId);

            // Establish drop region description
            const auto description = QString("Visualize %1 as points or density/contour map").arg(datasetGuiName);

            if (!_positionDataset.isValid()) {

                // Load as point positions when no dataset is currently loaded
                dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                    _dataInitialized = false;
                    _positionDataset = candidateDataset;
                    positionDatasetChanged();
                });
            }
            else {
                if (_positionDataset != candidateDataset && candidateDataset->getNumDimensions() >= 2) {

                    // The number of points is equal, so offer the option to replace the existing points dataset
                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _dataInitialized = false;
                        _positionDataset = candidateDataset;
                        positionDatasetChanged();
                    });
                }

                if (candidateDataset->getNumPoints() == _positionDataset->getNumPoints()) {

                    //// The number of points is equal, so offer the option to use the points dataset as source for points colors
                    //dropRegions << new DropWidget::DropRegion(this, "Point color", QString("Colorize %1 points with %2").arg(_positionDataset->getGuiName(), candidateDataset->getGuiName()), "palette", true, [this, candidateDataset]() {
                    //    _settingsAction.getColoringAction().addColorDataset(candidateDataset);
                    //    _settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                    //});

                    //// The number of points is equal, so offer the option to use the points dataset as metadata
                    //dropRegions << new DropWidget::DropRegion(this, "Point coloring", QString("Color %1 points with %2").arg(_positionDataset->getGuiName(), candidateDataset->getGuiName()), "brush", true, [this, candidateDataset]() {
                    //    _metadataDataset = candidateDataset;
                    //    onMetadataChanged();
                    //});
                }
            }
        }

        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {

            // Get clusters dataset from the core
            auto candidateDataset = mv::data().getDataset<Clusters>(datasetId);

            // Establish drop region description
            const auto description = QString("Use %1 as mask clusters").arg(candidateDataset->getGuiName());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_positionDataset.isValid())
            {
                // Use the clusters set for points color
                dropRegions << new DropWidget::DropRegion(this, "Mask", description, "palette", true, [this, candidateDataset]()
                {
                    _currentSliceIndex = 0;
                    _sliceDataset = candidateDataset;
                    onSliceIndexChanged();
                });
                // Offer the option to use the points dataset as metadata
                dropRegions << new DropWidget::DropRegion(this, "Point coloring", QString("Color %1 points with %2").arg(_positionDataset->getGuiName(), candidateDataset->getGuiName()), "brush", true, [this, candidateDataset]() {
                    _metadataDataset = candidateDataset;
                    onMetadataChanged();
                });
            }
            else {

                // Only allow user to color by clusters when there is a positions dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", "exclamation-circle", false);
            }
        }

        return dropRegions;
    });

    getUI().getMainView().installEventFilter(this);
}

SpaceWalkerPlugin::~SpaceWalkerPlugin()
{

}

void SpaceWalkerPlugin::init()
{
    auto layout = new QVBoxLayout();
    auto gradientViewLayout = new QVBoxLayout();
    auto dimensionViewsLayout = new QHBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(getUI().getPrimaryToolbar().createWidget(&getWidget()));

    getUI().initializeLabels();

    QFont sansFont("Helvetica [Cronyx]", 18);
    getUI().getFilterLabel().setFont(sansFont);

    gradientViewLayout->setContentsMargins(6, 0, 6, 0);
    gradientViewLayout->addWidget(&getUI().getFilterLabel());
    gradientViewLayout->addWidget(getUI().getProjectionViews()[0], 50);
    gradientViewLayout->addWidget(getUI().getProjectionViews()[1], 50);

    // Dimension selection
    gradientViewLayout->addWidget(&getUI().getDimensionSelectionLabel());
    gradientViewLayout->addWidget(&getUI().getSelectedView(), 50);

    // Expression graph
    gradientViewLayout->addWidget(&getUI().getSortedExpressionGraphLabel());
    gradientViewLayout->addWidget(_graphView, 70);
    gradientViewLayout->addWidget(&_metadataView, 10);

    auto leftPanel = new QVBoxLayout();
    leftPanel->addWidget(&getUI().getMainView(), 90);

    auto centralPanel = new QHBoxLayout();
    centralPanel->addLayout(leftPanel, 80);
    centralPanel->addLayout(gradientViewLayout, 20);

    layout->addLayout(centralPanel, 100);

    auto bottomToolbarWidget = new QWidget();
    auto bottomToolbarLayout = new QHBoxLayout();

    bottomToolbarWidget->setAutoFillBackground(true);
    bottomToolbarWidget->setLayout(bottomToolbarLayout);

    bottomToolbarLayout->setContentsMargins(0, 0, 0, 0);
    //bottomToolbarLayout->addWidget(_settingsAction.getColoringAction().getColorMapAction().createLabelWidget(&getWidget()));
    //bottomToolbarLayout->addWidget(_settingsAction.getColoringAction().getColorMapAction().createWidget(&getWidget()));
    bottomToolbarLayout->addWidget(getUI().getSettingsAction().getPlotAction().getPointPlotAction().getFocusSelection().createWidget(&getWidget()));
    bottomToolbarLayout->addStretch(1);
    //bottomToolbarLayout->addWidget(_settingsAction.getExportImageAction().createWidget(&getWidget()));
    bottomToolbarLayout->addWidget(getUI().getSettingsAction().getMiscellaneousAction().createCollapsedWidget(&getWidget()));

    layout->addWidget(_userInterface.getSecondaryToolbar().createWidget(&getWidget()));

    getWidget().setLayout(layout);

    // Update the data when the scatter plot widget is initialized
    //connect(_mainView, &MainView::initialized, this, &ScatterplotPlugin::updateProjectionData);

    //_eventListener.setEventCore(Application::core());
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&SpaceWalkerPlugin::onDataEvent, this, std::placeholders::_1));

    // Load points when the pointer to the position dataset changes
    //connect(&_positionDataset, &Dataset<Points>::changed, this, &ScatterplotPlugin::positionDatasetChanged);

    // Update points when the position dataset data changes
    //connect(&_positionDataset, &Dataset<Points>::dataChanged, this, &ScatterplotPlugin::updateData);

    // Update point selection when the position dataset data changes
    connect(&_positionDataset, &Dataset<Points>::dataSelectionChanged, this, &SpaceWalkerPlugin::updateSelection);

    // Update the window title when the GUI name of the position dataset changes
    //connect(&_positionDataset, &Dataset<Points>::datasetTextChanged, this, &ScatterplotPlugin::updateWindowTitle);

    // Do an initial update of the window title
    updateWindowTitle();
}

void SpaceWalkerPlugin::resetState()
{
    _dataStore = DataStorage();

    _positionSourceDataset.reset();
    _numPoints = 0;

    _normalizedData.clear();
    _enabledDimNames.clear();
    _dataInitialized = false;

    // Interaction
    _selectedPoint = 0;
    _globalSelectedPoint = 0;
    _selectedDimension = -1;
    _mousePressed = false;
    _graphTimer->stop();
    _mask.clear();
    _selectedViewIndex = 0;
    _colorScalars.clear();

    // Filters
    // ... Should be ok

    // KNN
    _computeOnLoad = false;
    _graphAvailable = false;
    _knnIndex = knn::Index();
    _knnGraph = KnnGraph();
    _largeKnnGraph = KnnGraph();
    _sourceKnnGraph = KnnGraph();
    _preloadedKnnGraph = false;

    // Slicing
    _sliceDataset.reset();
    _currentSliceIndex = 0;

    // MaskedKNN
    _maskedDataMatrix = DataMatrix();
    _maskedProjMatrix = DataMatrix();
    _maskedKnnIndex = knn::Index();
    _maskedKnnGraph = KnnGraph();
    _maskedSourceKnnGraph = KnnGraph();
    _maskedKnn = false;

    // Floodfill
    _floodFill = FloodFill(10);

    // Graph
    _graphView->reset();
    _bins.clear();

    // Local dimensionality
    _localSpatialDimensionality.clear();
    _localHighDimensionality.clear();

    // Directions
    _directions.clear();

    // Overlays
    _overlayType = OverlayType::NONE;
}

void SpaceWalkerPlugin::positionDatasetChanged()
{
    // Only proceed if we have a valid position dataset
    if (!_positionDataset.isValid())
        return;

    qDebug() << "New data dropped on gradient explorer..";
    // Only reset state if we aren't loading the plugin from a saved project
    if (!_loadingFromProject)
        resetState();

    // Reset dataset references
    _positionSourceDataset.reset();

    // Set position source dataset reference when the position dataset is derived
    if (_positionDataset->isDerivedData())
        _positionSourceDataset = _positionDataset->getSourceDataset<Points>();

    // Do not show the drop indicator if there is a valid point positions dataset
    getUI().getDropWidget().setShowDropIndicator(!_positionDataset.isValid());

    // Update the window title to reflect the position dataset change
    updateWindowTitle();

    // Collect metadata
    auto* parentItem = dataHierarchy().getItem(_positionSourceDataset->getId());
    DataHierarchyItems childrenItems = dataHierarchy().getChildren(*parentItem);

    _metadataDatasets.clear();
    for (DataHierarchyItem* item : childrenItems)
    {
        DataType type = item->getDataType();
        if (type == ClusterType)
        {
            Dataset<Clusters> clusterDataset = item->getDataset<Clusters>();
            _metadataDatasets.append(clusterDataset);
            _metadataView.addListItem(clusterDataset);
            qDebug() << clusterDataset->getGuiName();
        }
    }
    // Compute metadata indices
    _metadataIndexing.resize(_metadataDatasets.size(), std::vector<int>(_positionDataset->getNumPoints()));
    for (int i = 0; i < _metadataDatasets.size(); i++)
    {
        Dataset<Clusters> metadata = _metadataDatasets[i];

        for (int c = 0; c < metadata->getClusters().size(); c++)
        {
            Cluster& cluster = metadata->getClusters()[c];
            for (int index : cluster.getIndices())
            {
                _metadataIndexing[i][index] = c;
            }
        }
    }

    // Create flood nodes dataset if it doesn't exist yet
    if (!_floodScalars.isValid())
    {
        if (!_loadingFromProject)
        {
            _floodScalars = mv::data().createDataset<Points>("Points", "Flood Nodes");
            
            events().notifyDatasetAdded(_floodScalars);
        }
    }
    
    computeStaticData();

    _dataInitialized = true;
}

void SpaceWalkerPlugin::updateWindowTitle()
{
    if (!_positionDataset.isValid())
        getWidget().setWindowTitle(getGuiName());
    else
        getWidget().setWindowTitle(QString("%1: %2").arg(getGuiName(), _positionDataset->getDataHierarchyItem().getLocation()));
}

void SpaceWalkerPlugin::computeStaticData()
{
    // Make sure the position dataset is valid
    if (!_positionDataset.isValid())
        return;

    // Check if source dataset names and num dimensions line up, otherwise throw warning
    if (_positionSourceDataset->getDimensionNames().size() != _positionSourceDataset->getNumDimensions())
    {
        qWarning() << "!!! Warning: Data dimensions (" << _positionSourceDataset->getNumDimensions() << ") do not line up with number of dimension names (" << _positionSourceDataset->getDimensionNames().size() << ")";
        qWarning() << "!!! Shown dimension names may not be correct.";
    }

    ge::util::Timer timer;
    timer.start();

    {
        logger() << "Converting data to internal format...";

        convertToEigenMatrix(_positionDataset, _positionSourceDataset, _dataStore.getBaseData());
        convertToEigenMatrixProjection(_positionDataset, _dataStore.getBaseFullProjection());
    }
    timer.mark("Data preparation");

    {
        logger() << "Standardizing data...";
        standardizeData(_dataStore.getBaseData(), _dataStore.getVariances());
    }

    {
        logger() << "Normalizing data...";
        normalizeData(_dataStore.getBaseData(), _normalizedData);
    }

    _dataStore.createDataView();
    // Update projection matrix and views
    updateProjectionData();

    timer.mark("Data transformations");

    if (_computeOnLoad)
    {
        logger() << "Computing KNN graph";
        createKnnIndex();
        timer.mark("Computing KNN index");
        computeKnnGraph();
        timer.mark("Computing KNN graph");
        _largeKnnGraph.writeToFile();
    }

    {
        //compute::computePCAData(_dataStore.getData());

        //_localSpatialDimensionality.clear();
        //_localHighDimensionality.clear();

        //timer.mark("Local dimensionality");

        //compute::computeHDLocalDimensionality(_dataStore.getData(), _largeKnnGraph, _localHighDimensionality);
        //getUI().getMainView().setColorMap(_colorMapAction.getColorMapImage());
        //getUI().getMainView().setColorMapRange(0, 1);
        //getUI().getMainView().setScalars(_localHighDimensionality);

        //Dataset<Points> localDims = _core->addDataset("Points", "Dimensionality");

        //localDims->setData(_localHighDimensionality.data(), _localHighDimensionality.size(), 1);

        //_core->notifyDatasetAdded(localDims);

        //computeDirection(_dataMatrix, _projMatrix, _knnGraph, _directions);
        //getUI().getMainView().setDirections(_directions);
        timer.mark("Directions");
    }

    // Get enabled dimension names
    const auto& dimNames = _positionSourceDataset->getDimensionNames();
    auto enabledDimensions = _positionSourceDataset->getDimensionsPickerAction().getEnabledDimensions();

    _enabledDimNames.clear();
    for (int i = 0; i < enabledDimensions.size(); i++)
    {
        if (enabledDimensions[i])
            _enabledDimNames.push_back(dimNames[i]);
    }

    std::cout << "Number of enabled dimensions in the dataset : " << _dataStore.getNumDimensions() << std::endl;
    _bins.resize(_dataStore.getNumDimensions(), std::vector<int>(30));

    timer.finish("Graph init");
}

// Is called when the x, y dimensions chosen by the user change, as well as once when dropping new data into the view
void SpaceWalkerPlugin::updateProjectionData()
{
    // Check if the scatter plot is initialized, if not, don't do anything
    if (!getUI().getMainView().isInitialized())
    {
        qCritical() << "Tried to update projection data while view widgets are uninitialized";
        exit(1);
    }

    // Subset the new projection matrix from the one with all the dimensions
    {
        logger() << "Adjusting projection matrix...";

        int xDim = getUI().getSettingsAction().getPositionAction().getDimensionX();
        int yDim = getUI().getSettingsAction().getPositionAction().getDimensionY();
        _dataStore.createProjectionView(xDim, yDim);
    }

    // Convert projection data to 2D vector list
    std::vector<Vector2f> positions(_dataStore.getProjectionView().rows());
    {
        logger() << "Convert projection data to 2D vector list...";
        for (int i = 0; i < _dataStore.getProjectionView().rows(); i++)
            positions[i].set(_dataStore.getProjectionView()(i, 0), _dataStore.getProjectionView()(i, 1));
    }

    // Update the views with the new data
    updateViewData(positions);
    // Update any selections FIXME: is this necessary?
    updateSelection();

    // Update the projection size
    Bounds bounds = getUI().getMainView().getBounds();
    _dataStore.setProjectionSize(bounds.getWidth() > bounds.getHeight() ? bounds.getWidth() : bounds.getHeight());
    std::cout << "Projection bounds: x:" << bounds.getLeft() << ", " << bounds.getRight() << " y: " << bounds.getBottom() << ", " << bounds.getTop() << std::endl;
    std::cout << "Projection size: " << _dataStore.getProjectionSize() << std::endl;
}

void SpaceWalkerPlugin::updateViewData(std::vector<Vector2f>& positions)
{
    // TODO: Can save some time here only computing data bounds once
    // Pass the 2D points to the scatter plot widget
    getUI().getMainView().setData(&positions);
    for (int i = 0; i < (int)getUI().getProjectionViews().size(); i++)
        getUI().getProjectionViews()[i]->setData(&positions);
    getUI().getSelectedView().setData(&positions);
}

void SpaceWalkerPlugin::updateColorMapActionScalarRange()
{
    // Get the color map range from the scatter plot widget
    const auto colorMapRange = getUI().getMainView().getColorMapRange();
    const auto colorMapRangeMin = colorMapRange.x;
    const auto colorMapRangeMax = colorMapRange.y;

    // Get reference to color map range action
    const auto& colorMapRangeAction = getUI().getColorMapAction().getRangeAction(ColorMapAction::Axis::X);

    // Initialize the color map range action with the color map range from the scatter plot 
    getUI().getColorMapAction().getDataRangeAction(ColorMapAction::Axis::X).setRange({colorMapRangeMin, colorMapRangeMax});
}

void SpaceWalkerPlugin::loadData(const Datasets& datasets)
{
    // Exit if there is nothing to load
    if (datasets.isEmpty())
        return;

    // Load the first dataset
    _positionDataset = datasets.first();
}

void SpaceWalkerPlugin::onDataEvent(mv::DatasetEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DatasetDataSelectionChanged)
    {
        if (dataEvent->getDataset() == _positionDataset)
        {
            if (_positionDataset->isDerivedData())
            {
                onPointSelection();
            }
        }
    }
}

std::uint32_t SpaceWalkerPlugin::getNumberOfPoints() const
{
    if (!_positionDataset.isValid())
        return 0;

    return _positionDataset->getNumPoints();
}

void SpaceWalkerPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    auto subsetPoints = fromSourceData ? _positionDataset->getSourceDataset<Points>() : _positionDataset;

    // Create the subset
    auto subset = subsetPoints->createSubsetFromSelection(_positionDataset->getGuiName(), _positionDataset);

    // Notify others that the subset was added
    events().notifyDatasetAdded(subset);

    // And select the subset
    subset->getDataHierarchyItem().select();
}

void SpaceWalkerPlugin::onPointSelection()
{
    ge::util::Timer timer;

    if (!_positionDataset.isValid() || !_positionSourceDataset.isValid() || !_dataInitialized)
        return;

    mv::Dataset<Points> selection = _positionSourceDataset->getSelection();

    if (selection->indices.size() > 0)
    {
timer.start();
        computeCellMetadata();
timer.mark("Metadata");

        Vector2f center = Vector2f(_dataStore.getProjectionView()(_selectedPoint, 0), _dataStore.getProjectionView()(_selectedPoint, 1));

        KnnGraph& knnGraph = !_maskedKnn ? _knnGraph : _maskedKnnGraph;
        DataMatrix& dataMatrix = _mask.empty() ? _dataStore.getDataView() : _maskedDataMatrix;
        DataMatrix& projMatrix = _mask.empty() ? _dataStore.getProjectionView() : _maskedProjMatrix;
        const std::vector<float>& variances = _dataStore.getVariances();

        float projectionSize = _dataStore.getProjectionSize();
        int numPoints = _dataStore.getNumPoints();
        int numDimensions = _dataStore.getNumDimensions();

        getUI().getMainView().setCurrentPosition(center);
        getUI().getProjectionViews()[0]->setCurrentPosition(center);
        getUI().getProjectionViews()[1]->setCurrentPosition(center);
        getUI().getSelectedView().setCurrentPosition(center);
        getUI().getMainView().setFilterRadii(Vector2f(_filters.getSpatialPeakFilter().getInnerFilterRadius() * projectionSize, _filters.getSpatialPeakFilter().getOuterFilterRadius() * projectionSize));

        //////////////////
        // Do floodfill //
        //////////////////
        const std::vector<int>& viewIndices = _dataStore.getViewIndices();
        int selectedPoint = viewIndices.size() > 0 ? viewIndices[_selectedPoint] : _selectedPoint;
        if (_graphAvailable)
            _floodFill.compute(knnGraph, selectedPoint);

timer.mark("Floodfill");

        /////////////////////
        // Gradient picker //
        /////////////////////
        std::vector<int> dimRanking;
        switch (_filters.getFilterType())
        {
        case filters::FilterType::SPATIAL_PEAK:
        {
            if (getUI().getSettingsAction().getFilterAction().getRestrictToFloodAction().isChecked())
                _filters.getSpatialPeakFilter().computeDimensionRanking(_selectedPoint, dataMatrix, variances, projMatrix, projectionSize, dimRanking, _floodFill.getAllNodes());
            else
                _filters.getSpatialPeakFilter().computeDimensionRanking(_selectedPoint, dataMatrix, variances, projMatrix, projectionSize, dimRanking);
            break;
        }
        case filters::FilterType::HD_PEAK:
        {
            _filters.getHDPeakFilter().computeDimensionRanking(_selectedPoint, _dataStore.getBaseData(), variances, _floodFill, dimRanking);
            break;
        }
        }
timer.mark("Ranking");

        // Set appropriate coloring of gradient view, FIXME use colormap later
        for (int pi = 0; pi < getUI().getProjectionViews().size(); pi++)
        {
            const auto dimValues = dataMatrix(Eigen::all, dimRanking[pi]);
            std::vector<float> dimV(dimValues.data(), dimValues.data() + dimValues.size());
            getUI().getProjectionViews()[pi]->setShownDimension(dimRanking[pi]);
            getUI().getProjectionViews()[pi]->setScalars(dimV, _globalSelectedPoint);
            getUI().getProjectionViews()[pi]->setProjectionName(_enabledDimNames[dimRanking[pi]]);
        }
        // Set selected gradient view
        if (_selectedDimension >= 0)
        {
            qDebug() << "SEL DIM:" << _selectedDimension;
            const auto dimValues = dataMatrix(Eigen::all, _selectedDimension);
            std::vector<float> dimV(dimValues.data(), dimValues.data() + dimValues.size());
            getUI().getSelectedView().setShownDimension(_selectedDimension);
            getUI().getSelectedView().setScalars(dimV, _globalSelectedPoint);
            getUI().getSelectedView().setProjectionName(_enabledDimNames[_selectedDimension]);
        }

        _graphView->setTopDimensions(dimRanking[0], dimRanking[1]);

timer.mark("Filter");

        /////////////////////
        // Coloring        //
        /////////////////////
        _colorScalars.clear();
        _colorScalars.resize(_positionDataset->getNumPoints(), 0);

        if (_graphAvailable)
        {
            switch (_overlayType)
            {
            case OverlayType::NONE:
            {
                if (_floodFill.getNumWaves() > 0)
                {
                    getUI().getMainView().setColoredBy("Colored by - Flood fill step");
                    for (int i = 0; i < _floodFill.getNumWaves(); i++)
                    {
                        for (int j = 0; j < _floodFill.getWaves()[i].size(); j++)
                        {
                            int index = _floodFill.getWaves()[i][j];
                            _colorScalars[_mask.empty() ? index : _mask[index]] = 1 - (1.0f / _floodFill.getNumWaves()) * i;
                        }
                    }
                }
                else
                    getUI().getMainView().setColoredBy("Colored by - None");

                break;
            }
            case OverlayType::DIM_VALUES:
            {
                getUI().getMainView().setColoredBy("Colored by - Dim: " + _enabledDimNames[dimRanking[0]]);
                for (int i = 0; i < _floodFill.getTotalNumNodes(); i++)
                {
                    int node = _floodFill.getAllNodes()[i];
                    int index = _mask.empty() ? node : _mask[node];
                    _colorScalars[index] = _normalizedData[dimRanking[0]][index];
                }
                // TEMP
                //const auto dimValues = _dataStore.getFullData()(Eigen::all, dimRanking[0]);
                //std::vector<float> dimV(dimValues.data(), dimValues.data() + dimValues.size());
                //_colorScalars.assign(dimV.begin(), dimV.end());
                break;
            }
            case OverlayType::LOCAL_DIMENSIONALITY:
            {
                getUI().getMainView().setColoredBy("Colored by - Local Dimensionality");
                if (_localHighDimensionality.empty()) break;

                for (int i = 0; i < _floodFill.getTotalNumNodes(); i++)
                {
                    int node = _floodFill.getAllNodes()[i];
                    int index = _mask.empty() ? node : _mask[node];
                    _colorScalars[index] = _localHighDimensionality[node];
                }
                break;
            }
            case OverlayType::DIRECTIONS:
            {
                std::vector<float> scalars(dataMatrix.rows(), 0);
                std::vector<Vector2f> directions(_floodFill.getTotalNumNodes() * 2);

                for (int i = 0; i < _floodFill.getTotalNumNodes(); i++)
                {
                    float idx = _floodFill.getAllNodes()[i];
                    directions[i * 2 + 0] = _directions[idx * 2 + 0];
                    directions[i * 2 + 1] = _directions[idx * 2 + 1];
                    scalars[idx] = 0.5f;
                }
                getUI().getMainView().setDirections(directions);

                break;
            }
            }
        }

        // Set color scalars in the main view, if we are looking at a data view, subset the scalars first
        if (viewIndices.size() > 0)
        {
            std::vector<float> viewScalars(numPoints);
#pragma omp parallel for
            for (int i = 0; i < numPoints; i++)
            {
                viewScalars[i] = _colorScalars[viewIndices[i]];
            }
            getUI().getMainView().setScalars(viewScalars);
        }
        else
            getUI().getMainView().setScalars(_colorScalars);

        timer.mark("Compute color scalars");

        // Store scalars in floodfill dataset
        normalizeVector(_colorScalars);
        updateFloodScalarOutput(_colorScalars);

        timer.mark("Publish color scalars");

        /////////////////////
        // Graphs          //
        /////////////////////

        // Start a timer to compute the graphs in 100ms, if the timer is restarted before graphs are not computed
        _graphTimer->start(100);

        timer.finish("Graphs");
    }
}

void SpaceWalkerPlugin::onSliceIndexChanged()
{
    std::vector<uint32_t>& uindices = _sliceDataset->getClusters()[_currentSliceIndex].getIndices();
    std::vector<int> indices;
    indices.assign(uindices.begin(), uindices.end());

    QString clusterName = _sliceDataset->getClusters()[_currentSliceIndex].getName();
    getUI().getMainView().setClusterName(QString::number(_currentSliceIndex) + ": " + clusterName);

    useSelectionAsDataView(indices);
}

void SpaceWalkerPlugin::onMetadataChanged()
{
    _colors.clear();
    _colors.resize(_positionDataset->getNumPoints(), 0);
    for (Cluster cluster : _metadataDataset->getClusters())
    {
        mv::Vector3f color(cluster.getColor().redF(), cluster.getColor().greenF(), cluster.getColor().blueF());
        for (const int& index : cluster.getIndices())
        {
            _colors[index] = color;
        }
    }

    getUI().getMainView().setColors(_colors);
}

void SpaceWalkerPlugin::computeCellMetadata()
{
    int selectedCell = _globalSelectedPoint;

    QStringList metadataList;

    for (int i = 0; i < _metadataDatasets.size(); i++)
    {
        Dataset<Clusters> dataset = _metadataDatasets[i];

        int clusterIndex = _metadataIndexing[i][selectedCell];

        QString clusterName = dataset->getClusters()[clusterIndex].getName();
        metadataList.append(dataset->getGuiName() + ": " + clusterName);
    }

    getUI().getMainView().setMetadataList(metadataList);
    qDebug() << metadataList;
}

void SpaceWalkerPlugin::updateSelection()
{
    if (!_positionDataset.isValid())
        return;

    auto selection = _positionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    //_positionDataset->selectedLocalIndices(selection->indices, selected);

    if (!_dataStore.getViewIndices().empty())
    {
        // In an array the size of the full raw data, mark selected points as true
        std::vector<bool> globalSelection(_positionSourceDataset->getNumRawPoints(), false);

        for (const unsigned int& selectionIndex : selection->indices)
            globalSelection[selectionIndex] = true;

        // For all local points find out which are selected
        selected.resize(_dataStore.getViewIndices().size(), false);
        for (int i = 0; i < _dataStore.getViewIndices().size(); i++)
        {
            if (globalSelection[_dataStore.getViewIndices()[i]])
                selected[i] = true;
        }
    }

    highlights.resize(_positionDataset->getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    getUI().getMainView().setHighlights(highlights, static_cast<std::int32_t>(selection->indices.size()));
}

void SpaceWalkerPlugin::updateViewScalars()
{
    getUI().getProjectionViews()[0]->selectView(false);
    getUI().getProjectionViews()[1]->selectView(false);
    getUI().getSelectedView().selectView(false);

    ProjectionView* selectedView = nullptr;
    if (_selectedViewIndex == 1) selectedView = getUI().getProjectionViews()[0];
    else if (_selectedViewIndex == 2) selectedView = getUI().getProjectionViews()[1];
    else if (_selectedViewIndex == 3) selectedView = &getUI().getSelectedView();

    if (selectedView != nullptr)
    {
        selectedView->selectView(true);

        int selectedDimension = selectedView->getShownDimension();
        const auto dimValues = _dataStore.getDataView()(Eigen::all, selectedDimension);
        std::vector<float> dimV(dimValues.data(), dimValues.data() + dimValues.size());
        getUI().getMainView().setScalars(dimV);
        getUI().getMainView().setProjectionName("Dimension View: " + _enabledDimNames[selectedDimension]);
        getUI().getMainView().setColoredBy("");

        // Put full dimension scalars in output dataset
        const auto fullDimValues = _dataStore.getBaseData()(Eigen::all, selectedDimension);
        std::vector<float> fullDimV(fullDimValues.data(), fullDimValues.data() + fullDimValues.size());
        normalizeVector(fullDimV);
        updateFloodScalarOutput(fullDimV);

        //setProjectionName(_enabledDimNames[_selectedDimension]);
    }
    else
    {
        getUI().getMainView().setProjectionName("Floodfill View");
    }
}

void SpaceWalkerPlugin::updateFloodScalarOutput(const std::vector<float>& scalars)
{
    std::cout << "Flood scalars: " << scalars[1000] << std::endl;

    _floodScalars->setData<float>(scalars.data(), scalars.size(), 1);
    events().notifyDatasetDataChanged(_floodScalars);
}

/******************************************************************************
 * Graphs
 ******************************************************************************/

void SpaceWalkerPlugin::computeGraphs()
{
    // Binning
    int numBins = (int)_bins.size();
    int binSteps = (int)_bins[0].size();

    for (int d = 0; d < numBins; d++)
        std::fill(_bins[d].begin(), _bins[d].end(), 0);

#pragma omp parallel for
    for (int d = 0; d < numBins; d++)
    {
        int* const bins_d = &_bins[d][0];
        float* const norm_d = &_normalizedData[d][0];

        for (bigint i = 0; i < _floodFill.getTotalNumNodes(); i++)
        {
            const float& f = norm_d[_floodFill.getAllNodes()[i]];
            bins_d[(bigint)(f * binSteps)]++;
        }
    }
    qDebug() << "Graphs computed";

    _graphView->setBins(_bins);
}

void SpaceWalkerPlugin::onLineClicked(dint dim)
{
    qDebug() << "Dim: " << dim;
    _selectedDimension = dim;

    getUI().getSelectedView().setProjectionName(_enabledDimNames[_selectedDimension]);

    onPointSelection();
    updateViewScalars();
}

/******************************************************************************
 * Import / Export
 ******************************************************************************/

void SpaceWalkerPlugin::exportDimRankings()
{
    bool restrictToFloodNodes = getUI().getSettingsAction().getFilterAction().getRestrictToFloodAction().isChecked();
    exportRankings(_dataStore, _floodFill, _knnGraph, _filters, restrictToFloodNodes, _enabledDimNames);
}

void SpaceWalkerPlugin::exportFloodnodes()
{
    exportFloodNodes(_dataStore.getNumPoints(), _floodFill, _knnGraph);
}

void SpaceWalkerPlugin::importKnnGraph()
{
    QString fileName = QFileDialog::getOpenFileName(&getWidget(),
        tr("Open Knn Graph"), "", tr("KNN Files (*.knn)"));

    _largeKnnGraph.readFromFile(fileName);
    _knnGraph.build(_largeKnnGraph, 10);

    _graphAvailable = true;
}

/******************************************************************************
 * Flooding
 ******************************************************************************/

void SpaceWalkerPlugin::createKnnIndex()
{
    qDebug() << "Creating index";
    if (_dataStore.getNumDimensions() <= 200)
        _knnIndex.create(_dataStore.getNumDimensions(), knn::Metric::MANHATTAN);
    else
        _knnIndex.create(_dataStore.getNumDimensions(), knn::Metric::COSINE);
    qDebug() << "Adding data";
    _knnIndex.addData(_dataStore.getBaseData());
    qDebug() << "Done creating index";
}

void SpaceWalkerPlugin::computeKnnGraph()
{
    qDebug() << "Building KNN Graph..";
    if (!_preloadedKnnGraph)
    {
        _largeKnnGraph.build(_dataStore.getBaseData(), _knnIndex, 30);

        if (_useSharedDistances)
        {
            _sourceKnnGraph.build(_dataStore.getBaseData(), _knnIndex, 100);
            _largeKnnGraph.build(_sourceKnnGraph, 30, true);
            _knnGraph.build(_sourceKnnGraph, 10, true);
        }
        else
        {
            _largeKnnGraph.build(_dataStore.getBaseData(), _knnIndex, 30);
            _knnGraph.build(_largeKnnGraph, 10);
        }
    }
    else
        _knnGraph.build(_largeKnnGraph, 10);

    _graphAvailable = true;
    qDebug() << "Done building KNN Graph! Ready for flood-fill.";
    //_largeKnnGraph.writeToFile();
}

/******************************************************************************
 * Serialization
 ******************************************************************************/

void SpaceWalkerPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    _loadingFromProject = true;

    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "SettingsAction");

    getUI().getSettingsAction().fromVariantMap(variantMap["SettingsAction"].toMap());

    _overlayType = static_cast<OverlayType>(variantMap["OverlayType"].toInt());

    positionDatasetChanged();

    // Load flood nodes
    QString floodNodeId = variantMap["floodNodeGuid"].toString();
    _floodScalars = mv::data().getDataset<Points>(floodNodeId);

    // Load potential kNN graph from project
    bool knnAvailable = static_cast<bool>(variantMap["knnAvailable"].toBool());
    if (knnAvailable)
    {
        int numPoints = static_cast<size_t>(variantMap["numPoints"].toInt());
        int numNeighbours = static_cast<size_t>(variantMap["numNeighbours"].toInt());
        const auto qneighbours = variantMap["largeKnnGraph"].toMap();

        std::vector<int> linearNeighbours(numPoints * numNeighbours);

        populateDataBufferFromVariantMap(qneighbours, (char*)linearNeighbours.data());

        std::vector<std::vector<int>> neighbours(numPoints, std::vector<int>(numNeighbours));
        int c = 0;
        for (int i = 0; i < neighbours.size(); i++)
        {
            for (int j = 0; j < neighbours[i].size(); j++)
            {
                neighbours[c / numNeighbours][c % numNeighbours] = linearNeighbours[c];
                c++;
            }
        }

        _largeKnnGraph._neighbours = neighbours;
        _largeKnnGraph._numNeighbours = numNeighbours;

        _preloadedKnnGraph = true;

        computeKnnGraph();
    }

    // Load slice index from project if slice dataset has been set
    if (_sliceDataset.isValid())
    {
        _currentSliceIndex = variantMap["currentSliceIndex"].toInt();
        onSliceIndexChanged();
    }

    // Load selected point from project
    {
        _selectedPoint = variantMap["selectedPoint"].toInt();
        _globalSelectedPoint = variantMap["globalSelectedPoint"].toInt();

        // Notify core of new selected point
        notifyNewSelectedPoint();
    }

    _loadingFromProject = false;
}

QVariantMap SpaceWalkerPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    const SettingsAction& settingsAction = _userInterface.getSettingsAction();
    settingsAction.insertIntoVariantMap(variantMap);

    variantMap.insert("OverlayType", static_cast<int>(_overlayType));

    // Store potential KNN graph in project
    variantMap.insert("knnAvailable", _graphAvailable);
    if (_graphAvailable && _largeKnnGraph.getNeighbours().size() > 0)
    {
        const std::vector<std::vector<int>>& neighbours = _largeKnnGraph.getNeighbours();

        // Linearize data
        std::vector<int> linearNeighbours(neighbours.size() * neighbours[0].size());
        int c = 0;
        for (int i = 0; i < neighbours.size(); i++)
        {
            const std::vector<int>& n = neighbours[i];
            for (int j = 0; j < neighbours[i].size(); j++)
                linearNeighbours[c++] = neighbours[i][j];
        }

        QVariantMap qneighbours = rawDataToVariantMap((char*)linearNeighbours.data(), linearNeighbours.size() * sizeof(std::int32_t), true);

        variantMap.insert("largeKnnGraph", qneighbours);
        variantMap.insert("numPoints", QVariant::fromValue(neighbours.size()));
        variantMap.insert("numNeighbours", QVariant::fromValue(neighbours[0].size()));
    }

    // Store current slice in project, if slice dataset is valid
    if (_sliceDataset.isValid())
    {
        variantMap.insert("currentSliceIndex", _currentSliceIndex);
    }

    // Store selected point in project
    {
        variantMap.insert("selectedPoint", _selectedPoint);
        variantMap.insert("globalSelectedPoint", _globalSelectedPoint);
    }

    // Store floodnode guid
    variantMap.insert("floodNodeGuid", _floodScalars.getDatasetId());

    return variantMap;
}


/******************************************************************************
 * Mask
 ******************************************************************************/

bool SpaceWalkerPlugin::hasMaskApplied()
{
    return !_mask.empty();
}

void SpaceWalkerPlugin::clearMask()
{
    _mask.clear();

    // Set point opacity
    std::vector<float> opacityScalars(_dataStore.getNumPoints(), 1.0f);
    getUI().getMainView().setPointOpacityScalars(opacityScalars);
}

void SpaceWalkerPlugin::useSelectionAsMask()
{
    // Get current selection
    // Compute the indices that are selected in this local dataset
    std::vector<uint32_t> localSelectionIndices;
    _positionDataset->getLocalSelectionIndices(localSelectionIndices);

    _mask.assign(localSelectionIndices.begin(), localSelectionIndices.end());

    // Set point opacity
    std::vector<float> opacityScalars(_dataStore.getNumPoints(), 0.2f);
    for (const int maskIndex : _mask)
        opacityScalars[maskIndex] = 1.0f;
    getUI().getMainView().setPointOpacityScalars(opacityScalars);

    _maskedDataMatrix = _dataStore.getDataView()(_mask, Eigen::all);
    _maskedProjMatrix = _dataStore.getProjectionView()(_mask, Eigen::all);

    if (_maskedKnn)
    {
        if (_maskedDataMatrix.rows() < 5000)
            _maskedKnnIndex.create(_maskedDataMatrix.cols(), knn::Metric::MANHATTAN);
        else
            _maskedKnnIndex.create(_maskedDataMatrix.cols(), knn::Metric::EUCLIDEAN);
        _maskedKnnIndex.addData(_maskedDataMatrix);

        _largeKnnGraph.build(_maskedDataMatrix, _maskedKnnIndex, 30);

        if (_maskedDataMatrix.rows() < 5000)
        {
            _maskedSourceKnnGraph.build(_maskedDataMatrix, _maskedKnnIndex, 100);
            _maskedKnnGraph.build(_maskedSourceKnnGraph, 10, true);
        }
        else
            _maskedKnnGraph.build(_maskedDataMatrix, _maskedKnnIndex, 10);
    }
}

void SpaceWalkerPlugin::useSelectionAsDataView(std::vector<int>& indices)
{
    // Get current selection
    // Compute the indices that are selected in this local dataset
    //std::vector<uint32_t> localSelectionIndices;
    //_positionDataset->getLocalSelectionIndices(localSelectionIndices);
    //std::vector<int> indices;
    //indices.assign(localSelectionIndices.begin(), localSelectionIndices.end());

    _dataStore.createDataView(indices);

    int xDim = getUI().getSettingsAction().getPositionAction().getDimensionX();
    int yDim = getUI().getSettingsAction().getPositionAction().getDimensionY();
    qDebug() << "Selected dimensions: " << xDim << yDim;
    _dataStore.createProjectionView(xDim, yDim);

    // Convert projection data to 2D vector list
    std::vector<Vector2f> positions(_dataStore.getProjectionView().rows());
    {
        logger() << "Convert projection data to 2D vector list...";
        for (int i = 0; i < _dataStore.getProjectionView().rows(); i++)
            positions[i].set(_dataStore.getProjectionView()(i, 0), _dataStore.getProjectionView()(i, 1));
    }

    // Update the views with the new data
    updateViewData(positions);

    // Update the color of the points
    if (_colorScalars.size() == _positionDataset->getNumPoints())
    {
        std::vector<float> viewScalars(indices.size());
        for (int i = 0; i < indices.size(); i++)
        {
            viewScalars[i] = _colorScalars[indices[i]];
        }
        getUI().getMainView().setScalars(viewScalars);
    }
    // Update metadata colors
    if (_colors.size() == _positionDataset->getNumPoints())
    {
        std::vector<Vector3f> viewColors(indices.size());
        for (int i = 0; i < indices.size(); i++)
        {
            viewColors[i] = _colors[indices[i]];
        }
        getUI().getMainView().setColors(viewColors);
    }
}

/******************************************************************************
 * Factory
 ******************************************************************************/

QIcon SpaceWalkerPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("braille", color);
}

ViewPlugin* SpaceWalkerPluginFactory::produce()
{
    return new SpaceWalkerPlugin(this);
}

PluginTriggerActions SpaceWalkerPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> SpaceWalkerPlugin* {
        return dynamic_cast<SpaceWalkerPlugin*>(Application::core()->getPluginManager().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    return pluginTriggerActions;
}
