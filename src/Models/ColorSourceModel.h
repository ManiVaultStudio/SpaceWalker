#pragma once

#include "Dataset.h"

#include <QAbstractListModel>

using namespace hdps;

/**
 * Color source model class
 *
 * Model which defines the sources to color scatter plot points (by constant or by dataset)
 *
 * @author Thomas Kroes
 */
class ColorSourceModel : public QAbstractListModel
{
protected:

    /** (Default) constructor */
    ColorSourceModel(QObject* parent = nullptr);

public:

    /**
     * Get the number of row
     * @param parent Parent model index
     * @return Number of rows in the model
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /**
     * Get the row index of a dataset
     * @param parent Parent model index
     * @return Row index of the dataset
     */
    int rowIndex(const Dataset<DatasetImpl>& dataset) const;

    /**
     * Get the number of columns
     * @param parent Parent model index
     * @return Number of columns in the model
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /**
     * Get data
     * @param index Model index to query
     * @param role Data role
     * @return Data
     */
    QVariant data(const QModelIndex& index, int role) const;

    /**
     * Add dataset
     * @param dataset Smart pointer to dataset
     */
    void addDataset(const Dataset<DatasetImpl>& dataset);

    /**
     * Remove a dataset
     * @param dataset Smart pointer to dataset
     */
    void removeDataset(const Dataset<DatasetImpl>& dataset);

    /** Remove all datasets from the model */
    void removeAllDatasets();

    /**
     * Get datasets
     * @return Vector of smart pointers to datasets
     */
    const Datasets& getDatasets() const;

    /**
     * Get dataset at the specified row index
     * @param rowIndex Index of the row
     * @return Smart pointer to dataset
     */
    Dataset<DatasetImpl> getDataset(const std::int32_t& rowIndex) const;

    /** Get whether to show the full path name in the GUI */
    bool getShowFullPathName() const;

    /**
     * Set whether to show the full path name in the GUI
     * @param showFullPathName Whether to show the full path name in the GUI
     */
    void setShowFullPathName(const bool& showFullPathName);

    /** Updates the model from the datasets */
    void updateData();

protected:
    Datasets    _datasets;              /** Datasets from which can be picked */
    bool        _showFullPathName;      /** Whether to show the full path name in the GUI */

    friend class ColoringAction;
};
