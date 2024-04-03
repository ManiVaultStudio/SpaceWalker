#include "LoadedDatasetsAction.h"
#include "SpaceWalkerPlugin.h"

#include "PointData/PointData.h"
#include "ColorData/ColorData.h"
#include "ClusterData/ClusterData.h"

#include <QMenu>

using namespace mv;
using namespace mv::gui;

LoadedDatasetsAction::LoadedDatasetsAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, "Loaded datasets"),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color"),
    _sliceDatasetPickerAction(this, "Cluster")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("database"));
    setToolTip("Manage loaded datasets for position and/or color");

    _positionDatasetPickerAction.setDatasetsFilterFunction([](const mv::Datasets& datasets) -> Datasets {
        Datasets pointDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == PointType)
                pointDatasets << dataset;

        return pointDatasets;
    });

    _colorDatasetPickerAction.setDatasetsFilterFunction([](const mv::Datasets& datasets) -> Datasets {
        Datasets colorDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType)
                colorDatasets << dataset;

        return colorDatasets;
    });

    //connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
    //    _spaceWalkerPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(pickedDataset);
    //});
    
    //connect(&_spaceWalkerPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
    //    _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    //});
}

void LoadedDatasetsAction::initialize(SpaceWalkerPlugin* spaceWalkerPlugin)
{
    Q_ASSERT(spaceWalkerPlugin != nullptr);

    if (spaceWalkerPlugin == nullptr)
        return;

    _spaceWalkerPlugin = spaceWalkerPlugin;

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
        _spaceWalkerPlugin->getPositionDataset() = pickedDataset;
        //_spaceWalkerPlugin->positionDatasetChanged();
    });

    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });

    connect(&_spaceWalkerPlugin->getSliceDataset(), &Dataset<Clusters>::changed, this, [this](DatasetImpl* dataset) -> void {
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
        Dataset pickedDataset = mv::data().createDataset<Points>("Points", positionDataset.getDatasetId());
        _spaceWalkerPlugin->getPositionDataset() = pickedDataset;
    }

    // Load slice dataset
    auto sliceDataset = _sliceDatasetPickerAction.getCurrentDataset();
    if (sliceDataset.isValid())
    {
        qDebug() << ">>>>> Found a slice dataset " << sliceDataset->getGuiName();
        Dataset pickedDataset = mv::data().createDataset<Clusters>("Cluster", sliceDataset.getDatasetId());
        _spaceWalkerPlugin->getSliceDataset() = pickedDataset;
    }

    //_spaceWalkerPlugin->positionDatasetChanged();
}

QVariantMap LoadedDatasetsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    //variantMap.insert(
    //    {
    //    { "Value", _positionDatasetPickerAction.getCurrentDatasetGuid() }
    //    }
    //    "dataset", _positionDatasetPickerAction.getCurrentDataset());
    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);
    _sliceDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

//LoadedDatasetsAction::Widget::Widget(QWidget* parent, LoadedDatasetsAction* currentDatasetAction, const std::int32_t& widgetFlags) :
//    WidgetActionWidget(parent, currentDatasetAction)
//{
//    setFixedWidth(300);
//
//    auto layout = new QGridLayout();
//
//    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createLabelWidget(this), 0, 0);
//    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createWidget(this), 0, 1);
//    layout->addWidget(currentDatasetAction->_colorDatasetPickerAction.createLabelWidget(this), 1, 0);
//    layout->addWidget(currentDatasetAction->_colorDatasetPickerAction.createWidget(this), 1, 1);
//
//    if (widgetFlags & PopupLayout)
//    {
//        setPopupLayout(layout);
//            
//    } else {
//        layout->setContentsMargins(0, 0, 0, 0);
//        setLayout(layout);
//    }
//}
