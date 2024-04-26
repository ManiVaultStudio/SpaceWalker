#include "ScalarSourceModel.h"

#include <DataHierarchyItem.h>
#include <Application.h>
#include <Set.h>

using namespace mv;

ScalarSourceModel::ScalarSourceModel(QObject* parent /*= nullptr*/) :
    QAbstractListModel(parent),
    _datasets(),
    _showFullPathName(true)
{
}

int ScalarSourceModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    // Constant point size option plus the number of available datasets
    return _datasets.count() + DefaultRow::DatasetStart;
}

int ScalarSourceModel::rowIndex(const Dataset<DatasetImpl>& dataset) const
{
    // Only proceed if we have a valid dataset
    if (!dataset.isValid())
        return -1;

    // Return the index of the dataset and add one for the constant point size option
    return _datasets.indexOf(dataset) + DefaultRow::DatasetStart;
}

int ScalarSourceModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant ScalarSourceModel::data(const QModelIndex& index, int role) const
{
    // Get row/column of and smart pointer to the dataset
    const auto row              = index.row();
    const auto column           = index.column();
    const auto scalarDataset    = getDataset(row);

    switch (role)
    {
        // Return ruler icon for constant point size and dataset icon otherwise
        case Qt::DecorationRole:
        {
            if (row == DefaultRow::Constant)
                return Application::getIconFont("FontAwesome").getIcon("ruler");

            if (row == DefaultRow::Selection)
                return Application::getIconFont("FontAwesome").getIcon("mouse-pointer");

            if (row >= DefaultRow::DatasetStart)
                return scalarDataset->getIcon();

            break;
        }

         // Return 'Constant' for constant point size and dataset (full path) GUI name otherwise
        case Qt::DisplayRole:
        {
            if (row >= DefaultRow::DatasetStart)
            {
                if (row == 2)
                    return scalarDataset->text();
                else
                    return _showFullPathName ? scalarDataset->getLocation() : scalarDataset->text();
            } else {
                if (row == DefaultRow::Constant)
                    return "Constant";

                if (row == DefaultRow::Selection)
                    return "Selection";
            }
        }

        default:
            break;
    }

    return QVariant();
}

void ScalarSourceModel::addDataset(const Dataset<DatasetImpl>& dataset)
{
    // Avoid duplicates
    if (rowIndex(dataset) >= DefaultRow::DatasetStart)
        return;

    // Insert row into model
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    {
        // Add the dataset
        _datasets << dataset;
    }
    endInsertRows();

    // Get smart pointer to last added dataset
    auto& addedDataset = _datasets.last();

    // Remove a dataset from the model when it is about to be deleted
    connect(&addedDataset, &Dataset<DatasetImpl>::aboutToBeRemoved, this, [this, &addedDataset]() {
        removeDataset(addedDataset);
    });

    // Notify others that the model has updated when the dataset GUI name changes
    connect(addedDataset.get(), &DatasetImpl::textChanged, this, [this, &addedDataset]() {

        // Get row index of the dataset
        const auto colorDatasetRowIndex = rowIndex(addedDataset);

        // Only proceed if we found a valid row index
        if (colorDatasetRowIndex < 0)
            return;

        // Establish model index
        const auto modelIndex = index(colorDatasetRowIndex, 0);

        // Only proceed if we have a valid model index
        if (!modelIndex.isValid())
            return;

        // Notify others that the data changed
        emit dataChanged(modelIndex, modelIndex);
    });
}

void ScalarSourceModel::removeDataset(const Dataset<DatasetImpl>& dataset)
{
    // Get row index of the dataset
    const auto datasetRowIndex = rowIndex(dataset);

    // Update model
    beginRemoveRows(QModelIndex(), datasetRowIndex, datasetRowIndex);
    {
        // Remove dataset from internal vector
        _datasets.removeOne(dataset);
    }
    endRemoveRows();
}

void ScalarSourceModel::removeAllDatasets()
{
    // Remove row from model
    beginRemoveRows(QModelIndex(), 0, rowCount() - DefaultRow::DatasetStart);
    {
        // Remove all datasets
        _datasets.clear();
    }
    endRemoveRows();

    // And update model data with altered datasets
    updateData();
}

const Datasets& ScalarSourceModel::getDatasets() const
{
    return _datasets;
}

Dataset<DatasetImpl> ScalarSourceModel::getDataset(const std::int32_t& rowIndex) const
{
    // Return empty smart pointer when out of range
    if (rowIndex < DefaultRow::DatasetStart || rowIndex > (DefaultRow::DatasetStart + _datasets.count()))
        return Dataset<DatasetImpl>();

    // Subtract the constant point size row
    return _datasets[rowIndex - DefaultRow::DatasetStart];
}

void ScalarSourceModel::setDatasets(const Datasets& datasets)
{
    // Notify others that the model layout is about to be changed
    emit layoutAboutToBeChanged();

    // Add datasets
    for (const auto& dataset : datasets)
        addDataset(dataset);

    // Notify others that the model layout is changed
    emit layoutChanged();

    // And update model data with datasets
    updateData();
}

bool ScalarSourceModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ScalarSourceModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;

    updateData();
}

void ScalarSourceModel::updateData()
{
    // Update the datasets string list model
    for (auto dataset : _datasets) {

        // Continue if the dataset is not valid
        if (!dataset.isValid())
            continue;

        // Get dataset model index
        const auto datasetModelIndex = index(_datasets.indexOf(dataset), 0);

        // Notify others that the data changed
        emit dataChanged(datasetModelIndex, datasetModelIndex);
    }
}
