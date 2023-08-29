#include "PointPlotAction.h"
#include "ScalarSourceAction.h"
#include "SpaceWalkerPlugin.h"
#include "ScatterplotWidget.h"
#include "ProjectionView.h"

#include <DataHierarchyItem.h>

using namespace gui;

PointPlotAction::PointPlotAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _spaceWalkerPlugin(nullptr),
    _sizeAction(this, "Point size", 0.0, 40.0, DEFAULT_POINT_SIZE),
    _opacityAction(this, "Point opacity", 0.0, 100.0, DEFAULT_POINT_OPACITY),
    _pointSizeScalars(),
    _pointOpacityScalars(),
    _focusSelection(this, "Focus selection"),
    _lastOpacitySourceIndex(-1)
{
    setToolTip("Point plot settings");
    setConfigurationFlag(WidgetAction::ConfigurationFlag::NoLabelInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_sizeAction);
    addAction(&_opacityAction);

    _sizeAction.setConnectionPermissionsToNone();
    _opacityAction.setConnectionPermissionsToNone();

    _sizeAction.getMagnitudeAction().setSuffix("px");
    _opacityAction.getMagnitudeAction().setSuffix("%");

    _opacityAction.getSourceAction().getOffsetAction().setSuffix("%");

    _focusSelection.setToolTip("Put focus on selected points by modulating the point opacity");
    _focusSelection.setDefaultWidgetFlags(ToggleAction::CheckBox);

    connect(&_sizeAction, &ScalarAction::sourceSelectionChanged, this, [this](const std::uint32_t& sourceSelectionIndex) {
        switch (sourceSelectionIndex)
        {
            case ScalarSourceModel::DefaultRow::Constant:
            {
                _sizeAction.getSourceAction().getOffsetAction().setValue(0.0f);

                break;
            }

            case ScalarSourceModel::DefaultRow::Selection:
            {
                _sizeAction.getSourceAction().getOffsetAction().setValue(_sizeAction.getMagnitudeAction().getValue());

                break;
            }

            case ScalarSourceModel::DefaultRow::DatasetStart:
                break;

            default:
                break;
        }
    });

    connect(&_opacityAction, &ScalarAction::sourceSelectionChanged, this, [this](const std::uint32_t& sourceSelectionIndex) {
        switch (sourceSelectionIndex)
        {
            case ScalarSourceModel::DefaultRow::Constant:
            {
                _opacityAction.getMagnitudeAction().setValue(100.0f);
                _opacityAction.getSourceAction().getOffsetAction().setValue(0.0f);
                _focusSelection.setChecked(false);

                break;
            }

            case ScalarSourceModel::DefaultRow::Selection:
            {
                _opacityAction.getMagnitudeAction().setValue(10.0f);
                _opacityAction.getSourceAction().getOffsetAction().setValue(90.0f);
                _focusSelection.setChecked(true);

                break;
            }

            case ScalarSourceModel::DefaultRow::DatasetStart:
            {
                _focusSelection.setChecked(false);

                break;
            }

            default:
                break;
        }
    });

    connect(&_focusSelection, &ToggleAction::toggled, this, [this](const bool& toggled) {
        const auto opacitySourceIndex  = _opacityAction.getSourceAction().getPickerAction().getCurrentIndex();

        if (toggled) {

            if (opacitySourceIndex != ScalarSourceModel::DefaultRow::Selection)
                _opacityAction.setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Selection);

            _lastOpacitySourceIndex = opacitySourceIndex;
        }
        else {
            if (_lastOpacitySourceIndex != ScalarSourceModel::DefaultRow::Selection)
                _opacityAction.setCurrentSourceIndex(_lastOpacitySourceIndex);
        }
    });
}

void PointPlotAction::initialize(SpaceWalkerPlugin* spaceWalkerPlugin)
{
    Q_ASSERT(spaceWalkerPlugin != nullptr);

    if (spaceWalkerPlugin == nullptr)
        return;

    _spaceWalkerPlugin = spaceWalkerPlugin;

    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this]() {

        const auto positionDataset = _spaceWalkerPlugin->getPositionDataset();

        if (!positionDataset.isValid())
            return;

        _sizeAction.removeAllDatasets();
        _opacityAction.removeAllDatasets();

        _sizeAction.addDataset(positionDataset);
        _opacityAction.addDataset(positionDataset);

        const auto positionSourceDataset = _spaceWalkerPlugin->getPositionSourceDataset();

        if (positionSourceDataset.isValid()) {
            _sizeAction.addDataset(positionSourceDataset);
            _opacityAction.addDataset(positionSourceDataset);
        }

        updateDefaultDatasets();

        updateScatterPlotWidgetPointSizeScalars();
        updateScatterPlotWidgetPointOpacityScalars();

        _sizeAction.getSourceAction().getPickerAction().setCurrentIndex(0);
        _opacityAction.getSourceAction().getPickerAction().setCurrentIndex(0);
    });

    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::childAdded, this, &PointPlotAction::updateDefaultDatasets);
    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::childRemoved, this, &PointPlotAction::updateDefaultDatasets);
    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);

    connect(&_sizeAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::offsetChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::sourceSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);

    connect(&_opacityAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::offsetChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::sourceSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
}

QMenu* PointPlotAction::getContextMenu()
{
    if (_spaceWalkerPlugin == nullptr)
        return nullptr;

    auto menu = new QMenu("Plot settings");

    const auto renderMode = _spaceWalkerPlugin->getScatterplotWidget().getRenderMode();

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sizeAction);
    addActionToMenu(&_opacityAction);

    return menu;
}

void PointPlotAction::setVisible(bool visible)
{
    _sizeAction.setVisible(visible);
    _opacityAction.setVisible(visible);
    _focusSelection.setVisible(visible);
}

void PointPlotAction::addPointSizeDataset(const Dataset<DatasetImpl>& pointSizeDataset)
{
    if (!pointSizeDataset.isValid())
        return;

    _sizeAction.addDataset(pointSizeDataset);
}

void PointPlotAction::addPointOpacityDataset(const Dataset<DatasetImpl>& pointOpacityDataset)
{
    if (!pointOpacityDataset.isValid())
        return;

    _opacityAction.addDataset(pointOpacityDataset);
}

void PointPlotAction::updateDefaultDatasets()
{
    if (_spaceWalkerPlugin == nullptr)
        return;

    auto positionDataset = Dataset<Points>(_spaceWalkerPlugin->getPositionDataset());

    if (!positionDataset.isValid())
        return;

    const auto children = positionDataset->getDataHierarchyItem().getChildren();

    for (auto child : children) {
        const auto childDataset = child->getDataset();
        const auto dataType     = childDataset->getDataType();

        if (dataType != PointType)
            continue;

        auto points = Dataset<Points>(childDataset);

        _sizeAction.addDataset(points);
        _opacityAction.addDataset(points);
    }
}

void PointPlotAction::updateScatterPlotWidgetPointSizeScalars()
{
    if (_spaceWalkerPlugin == nullptr)
        return;

    //if (!_spaceWalkerPlugin->getDataStore().hasData())
    //    return;

    _spaceWalkerPlugin->getScatterplotWidget().setPointSize(_sizeAction.getMagnitudeAction().getValue());



    const auto numberOfPoints = _spaceWalkerPlugin->getDataStore().getProjectionView().rows();

    //if (numberOfPoints != _pointSizeScalars.size())
    //    _pointSizeScalars.resize(numberOfPoints);

    //std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), _sizeAction.getMagnitudeAction().getValue());

    _spaceWalkerPlugin->getProjectionViews()[0]->setSourcePointSize(_sizeAction.getMagnitudeAction().getValue());
    _spaceWalkerPlugin->getProjectionViews()[1]->setSourcePointSize(_sizeAction.getMagnitudeAction().getValue());
    _spaceWalkerPlugin->getSelectedView()->setSourcePointSize(_sizeAction.getMagnitudeAction().getValue());

    return;

    if (_sizeAction.isSourceSelection()) {
        auto positionDataset = _spaceWalkerPlugin->getPositionDataset();

        std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), _sizeAction.getMagnitudeAction().getValue());

        const auto pointSizeSelectedPoints = _sizeAction.getMagnitudeAction().getValue() + _sizeAction.getSourceAction().getOffsetAction().getValue();

        std::vector<bool> selected;

        positionDataset->selectedLocalIndices(positionDataset->getSelection<Points>()->indices, selected);

        for (int i = 0; i < selected.size(); i++) {
            if (!selected[i])
                continue;

            _pointSizeScalars[i] = pointSizeSelectedPoints;
        }
    }

    if (_sizeAction.isSourceDataset()) {

        auto pointSizeSourceDataset = Dataset<Points>(_sizeAction.getCurrentDataset());

        if (pointSizeSourceDataset.isValid() && pointSizeSourceDataset->getNumPoints() == _spaceWalkerPlugin->getPositionDataset()->getNumPoints())
        {
            pointSizeSourceDataset->visitData([this, pointSizeSourceDataset, numberOfPoints](auto pointData) {
                const auto currentDimensionIndex    = _sizeAction.getSourceAction().getDimensionPickerAction().getCurrentDimensionIndex();
                const auto rangeMin                 = _sizeAction.getSourceAction().getRangeAction().getMinimum();
                const auto rangeMax                 = _sizeAction.getSourceAction().getRangeAction().getMaximum();
                const auto rangeLength              = rangeMax - rangeMin;

                if (rangeLength > 0) {
                    for (std::uint32_t pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
                        auto pointValue = static_cast<float>(pointData[pointIndex][currentDimensionIndex]);

                        const auto pointValueClamped    = std::max(rangeMin, std::min(rangeMax, pointValue));
                        const auto pointValueNormalized = (pointValueClamped - rangeMin) / rangeLength;

                        _pointSizeScalars[pointIndex] = _sizeAction.getSourceAction().getOffsetAction().getValue() + (pointValueNormalized * _sizeAction.getMagnitudeAction().getValue());
                    }
                }
                else {
                    std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), _sizeAction.getSourceAction().getOffsetAction().getValue() + (rangeMin * _sizeAction.getMagnitudeAction().getValue()));
                }
            });
        }
    }

    _spaceWalkerPlugin->getScatterplotWidget().setPointSizeScalars(_pointSizeScalars);
}

void PointPlotAction::updateScatterPlotWidgetPointOpacityScalars()
{
    if (_spaceWalkerPlugin == nullptr)
        return;

    if (!_spaceWalkerPlugin->getPositionDataset().isValid())
        return;

    _opacityAction.getMagnitudeAction().setEnabled(!_spaceWalkerPlugin->hasMaskApplied());

    if (_spaceWalkerPlugin->hasMaskApplied()) return;

    const auto numberOfPoints = _spaceWalkerPlugin->getPositionDataset()->getNumPoints();

    if (numberOfPoints != _pointOpacityScalars.size())
        _pointOpacityScalars.resize(numberOfPoints);

    const auto opacityMagnitude = 0.01f * _opacityAction.getMagnitudeAction().getValue();

    std::fill(_pointOpacityScalars.begin(), _pointOpacityScalars.end(), opacityMagnitude);

    if (_opacityAction.isSourceSelection()) {
        auto positionDataset    = _spaceWalkerPlugin->getPositionDataset();
        auto selectionSet       = positionDataset->getSelection<Points>();

        std::fill(_pointOpacityScalars.begin(), _pointOpacityScalars.end(), 0.01f * _opacityAction.getMagnitudeAction().getValue());

        const auto opacityOffset                = 0.01f * _opacityAction.getSourceAction().getOffsetAction().getValue();
        const auto pointOpacitySelectedPoints   = std::min(1.0f, opacityMagnitude + opacityOffset);

        std::vector<uint32_t> localSelectionIndices;
        positionDataset->getLocalSelectionIndices(localSelectionIndices);

        for (const auto& selectionIndex : localSelectionIndices)
            _pointOpacityScalars[selectionIndex] = pointOpacitySelectedPoints;
    }

    if (_opacityAction.isSourceDataset()) {
        auto pointOpacitySourceDataset = Dataset<Points>(_opacityAction.getCurrentDataset());

        if (pointOpacitySourceDataset.isValid() && pointOpacitySourceDataset->getNumPoints() == _spaceWalkerPlugin->getPositionDataset()->getNumPoints()) {
            pointOpacitySourceDataset->visitData([this, pointOpacitySourceDataset, numberOfPoints, opacityMagnitude](auto pointData) {
                const auto currentDimensionIndex    = _opacityAction.getSourceAction().getDimensionPickerAction().getCurrentDimensionIndex();
                const auto opacityOffset            = 0.01f * _opacityAction.getSourceAction().getOffsetAction().getValue();
                const auto rangeMin                 = _opacityAction.getSourceAction().getRangeAction().getMinimum();
                const auto rangeMax                 = _opacityAction.getSourceAction().getRangeAction().getMaximum();
                const auto rangeLength              = rangeMax - rangeMin;

                if (rangeLength > 0) {
                    for (std::uint32_t pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
                        auto pointValue                 = static_cast<float>(pointData[pointIndex][currentDimensionIndex]);
                        const auto pointValueClamped    = std::max(rangeMin, std::min(rangeMax, pointValue));
                        const auto pointValueNormalized = (pointValueClamped - rangeMin) / rangeLength;

                        if (opacityOffset == 1.0f)
                            _pointOpacityScalars[pointIndex] = 1.0f;
                        else
                            _pointOpacityScalars[pointIndex] = opacityMagnitude * (opacityOffset + (pointValueNormalized / (1.0f - opacityOffset)));
                    }
                }
                else {
                    auto& rangeAction = _opacityAction.getSourceAction().getRangeAction();

                    if (rangeAction.getRangeMinAction().getValue() == rangeAction.getRangeMaxAction().getValue())
                        std::fill(_pointOpacityScalars.begin(), _pointOpacityScalars.end(), 0.0f);
                    else
                        std::fill(_pointOpacityScalars.begin(), _pointOpacityScalars.end(), 1.0f);
                }
            });
        }
    }

    _spaceWalkerPlugin->getScatterplotWidget().setPointOpacityScalars(_pointOpacityScalars);
}

void PointPlotAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicPointPlotAction = dynamic_cast<PointPlotAction*>(publicAction);

    Q_ASSERT(publicPointPlotAction != nullptr);

    if (publicPointPlotAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_sizeAction, &publicPointPlotAction->getSizeAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_opacityAction, &publicPointPlotAction->getOpacityAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_focusSelection, &publicPointPlotAction->getFocusSelection(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void PointPlotAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_sizeAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_opacityAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_focusSelection, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void PointPlotAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _sizeAction.fromParentVariantMap(variantMap);
    _opacityAction.fromParentVariantMap(variantMap);
    _focusSelection.fromParentVariantMap(variantMap);
}

QVariantMap PointPlotAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _sizeAction.insertIntoVariantMap(variantMap);
    _opacityAction.insertIntoVariantMap(variantMap);
    _focusSelection.insertIntoVariantMap(variantMap);

    return variantMap;
}
