#include "LoadedDatasetsAction.h"
#include "SpaceWalkerPlugin.h"

#include "PointData/PointData.h"
#include "ColorData/ColorData.h"
#include "ClusterData/ClusterData.h"

#include <QMenu>

using namespace mv;
using namespace mv::gui;

LoadedDatasetsAction::LoadedDatasetsAction(QObject* parent, const QString& title) :
    GroupAction(parent, "Loaded datasets"),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color"),
    _sliceDatasetPickerAction(this, "Cluster")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("database"));
    setToolTip("Manage loaded datasets for position and/or color");

    _positionDatasetPickerAction.setFilterFunction([](const mv::Dataset<DatasetImpl>& dataset) -> bool {
        return dataset->getDataType() == PointType;
        });

    _colorDatasetPickerAction.setFilterFunction([](const mv::Dataset<DatasetImpl>& dataset) -> bool {
        return dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType;
        });


    //connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
    //    _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(pickedDataset);
    //});
    
    //connect(&_scatterplotPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
    //    _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    //});
}

void LoadedDatasetsAction::initialize(SpaceWalkerPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _plugin = scatterplotPlugin;

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
        _plugin->getPositionDataset() = pickedDataset;
        //_scatterplotPlugin->positionDatasetChanged();
    });

    connect(&_plugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });

    connect(&_plugin->getSliceDataset(), &Dataset<Clusters>::changed, this, [this](DatasetImpl* dataset) -> void {
        _sliceDatasetPickerAction.setCurrentDataset(dataset);
    });
}

void LoadedDatasetsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _positionDatasetPickerAction.fromParentVariantMap(variantMap);
    _colorDatasetPickerAction.fromParentVariantMap(variantMap);
    _sliceDatasetPickerAction.fromParentVariantMap(variantMap);

    // Load position dataset
    auto positionDataset = _positionDatasetPickerAction.getCurrentDataset();
    if (positionDataset.isValid())
    {
        Dataset pickedDataset = mv::data().getDataset(positionDataset.getDatasetId());
        _plugin->getPositionDataset() = pickedDataset;
    }

    // Load slice dataset
    auto sliceDataset = _sliceDatasetPickerAction.getCurrentDataset();
    if (sliceDataset.isValid())
    {
        
        qDebug() << ">>>>> Found a slice dataset " << sliceDataset->getGuiName();
        Dataset pickedDataset = mv::data().getDataset(sliceDataset.getDatasetId());
        _plugin->getSliceDataset() = pickedDataset;
    }

    //_scatterplotPlugin->positionDatasetChanged();
}

QVariantMap LoadedDatasetsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);
    _sliceDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
