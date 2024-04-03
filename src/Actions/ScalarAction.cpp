#include "ScalarAction.h"
#include "Models/ScalarSourceModel.h"
#include "SpaceWalkerPlugin.h"

#include <QHBoxLayout>

using namespace mv::gui;

ScalarAction::ScalarAction(QObject* parent, const QString& title, const float& minimum /*= 0.0f*/, const float& maximum /*= 100.0f*/, const float& value /*= 0.0f*/) :
    GroupAction(parent, title),
    _magnitudeAction(this, title, minimum, maximum, value),
    _sourceAction(this, QString("%1 source").arg(title))
{
    setDefaultWidgetFlags(GroupAction::Horizontal);
    setShowLabels(false);

    addAction(&_magnitudeAction);
    addAction(&_sourceAction);

    connect(&_sourceAction.getPickerAction(), &OptionAction::currentIndexChanged, this, [this](const std::uint32_t& currentIndex) {
        emit sourceSelectionChanged(currentIndex);
    });

    connect(&_magnitudeAction, &DecimalAction::valueChanged, this, [this](const float& value) {
        emit magnitudeChanged(value);
    });

    connect(&_sourceAction, &ScalarSourceAction::scalarRangeChanged, this, [this](const float& minimum, const float& maximum) {
        emit scalarRangeChanged(minimum, maximum);
    });

    connect(&_sourceAction.getOffsetAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        emit offsetChanged(value);
    });
}

void ScalarAction::addDataset(const Dataset<DatasetImpl>& dataset)
{
    auto& sourceModel = _sourceAction.getModel();

    sourceModel.addDataset(dataset);

    connect(&sourceModel.getDatasets().last(), &Dataset<DatasetImpl>::dataChanged, this, [this, dataset]() {
        const auto currentDataset = getCurrentDataset();

        if (!currentDataset.isValid())
            return;

        _sourceAction.updateScalarRange();

        if (currentDataset == dataset)
            emit sourceDataChanged(dataset);
    });

    connect(&_magnitudeAction, &DecimalAction::valueChanged, this, [this, dataset](const float& value) {
        emit magnitudeChanged(value);
    });
}

void ScalarAction::removeAllDatasets()
{
    _sourceAction.getModel().removeAllDatasets();
}

Dataset<DatasetImpl> ScalarAction::getCurrentDataset()
{
    auto& scalarSourceModel = _sourceAction.getModel();

    const auto currentSourceIndex = _sourceAction.getPickerAction().getCurrentIndex();

    if (currentSourceIndex < ScalarSourceModel::DefaultRow::DatasetStart)
        return Dataset<DatasetImpl>();

    return scalarSourceModel.getDataset(currentSourceIndex);
}

void ScalarAction::setCurrentDataset(const Dataset<DatasetImpl>& dataset)
{
    const auto datasetRowIndex = _sourceAction.getModel().rowIndex(dataset);

    if (datasetRowIndex >= 0)
        _sourceAction.getPickerAction().setCurrentIndex(datasetRowIndex);
}

void ScalarAction::setCurrentSourceIndex(bool sourceIndex)
{
    _sourceAction.getPickerAction().setCurrentIndex(sourceIndex);
}

bool ScalarAction::isSourceConstant() const
{
    return _sourceAction.getPickerAction().getCurrentIndex() == ScalarSourceModel::DefaultRow::Constant;
}

bool ScalarAction::isSourceSelection() const
{
    return _sourceAction.getPickerAction().getCurrentIndex() == ScalarSourceModel::DefaultRow::Selection;
}

bool ScalarAction::isSourceDataset() const
{
    return _sourceAction.getPickerAction().getCurrentIndex() >= ScalarSourceModel::DefaultRow::DatasetStart;
}

void ScalarAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicScalarAction = dynamic_cast<ScalarAction*>(publicAction);

    Q_ASSERT(publicScalarAction != nullptr);

    if (publicScalarAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_magnitudeAction, &publicScalarAction->getMagnitudeAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_sourceAction, &publicScalarAction->getSourceAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void ScalarAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_magnitudeAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_sourceAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void ScalarAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _magnitudeAction.fromParentVariantMap(variantMap);
    _sourceAction.fromParentVariantMap(variantMap);
}

QVariantMap ScalarAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _magnitudeAction.insertIntoVariantMap(variantMap);
    _sourceAction.insertIntoVariantMap(variantMap);

    return variantMap;
}