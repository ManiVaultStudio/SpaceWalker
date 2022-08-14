#pragma once

#include "Dataset.h"

#include <QAbstractListModel>

using namespace hdps;

/**
 * Scalar source model class
 *
 * Model which defines the source(s) to size scalars (by constant or by dataset)
 *
 * @author Thomas Kroes
 */
class ScalarSourceModel : public QAbstractListModel
{
protected:

    /** (Default) constructor */
    ScalarSourceModel(QObject* parent = nullptr);

public:
    /** Default scalar options */
    enum DefaultRow {
        Constant,           /** Scale by constant */
        Selection,          /** Scale by selected */
        DatasetStart        /** Start row of the dataset(s) */
    };

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
     * Add a dataset
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

    /**
     * Set datasets (resets the model)
     * @param datasets Vector of smart pointers to datasets
     */
    void setDatasets(const Datasets& datasets);

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
    Datasets    _datasets;              /** Datasets used to size the scatter plot points with */
    bool        _showFullPathName;      /** Whether to show the full path name in the GUI */

    friend class ScalarAction;
    friend class ScalarSourceAction;
};
