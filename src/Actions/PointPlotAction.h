#pragma once

#include <actions/VerticalGroupAction.h>

#include "ScalarAction.h"

class ScatterplotPlugin;

using namespace hdps::gui;

class PointPlotAction : public VerticalGroupAction
{
    Q_OBJECT

public:
    
    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE PointPlotAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p scatterplotPlugin
     * @param scatterplotPlugin Pointer to scatterplot plugin
     */
    void initialize(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

    /**
     * Override to show/hide child actions
     * @param visible Whether the action is visible or not
     */
    void setVisible(bool visible);

    /**
     * Add point size dataset
     * @param pointSizeDataset Smart pointer to point size dataset
     */
    void addPointSizeDataset(const Dataset<DatasetImpl>& pointSizeDataset);

    /**
     * Add point opacity dataset
     * @param pointOpacityDataset Smart pointer to point opacity dataset
     */
    void addPointOpacityDataset(const Dataset<DatasetImpl>& pointOpacityDataset);

protected:

    /** Update default datasets (candidates are children of points type and with matching number of points) */
    void updateDefaultDatasets();

    /** Update the scatter plot widget point size scalars */
    void updateScatterPlotWidgetPointSizeScalars();

    /** Update the scatter plot widget point opacity scalars */
    void updateScatterPlotWidgetPointOpacityScalars();

protected: // Linking

    /**
     * Connect this action to a public action
     * @param publicAction Pointer to public action to connect to
     * @param recursive Whether to also connect descendant child actions
     */
    void connectToPublicAction(WidgetAction* publicAction, bool recursive) override;

    /**
     * Disconnect this action from its public action
     * @param recursive Whether to also disconnect descendant child actions
     */
    void disconnectFromPublicAction(bool recursive) override;

public: // Serialization

    /**
     * Load widget action from variant map
     * @param Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */
    QVariantMap toVariantMap() const override;

public: // Action getters

    ScalarAction& getSizeAction() { return _sizeAction; }
    ScalarAction& getOpacityAction() { return _opacityAction; }
    ToggleAction& getFocusSelection() { return _focusSelection; }

private:
    ScatterplotPlugin*      _scatterplotPlugin;         /** Pointer to scatterplot plugin */
    ScalarAction            _sizeAction;                /** Point size action */
    ScalarAction            _opacityAction;             /** Point opacity action */
    std::vector<float>      _pointSizeScalars;          /** Cached point size scalars */
    std::vector<float>      _pointOpacityScalars;       /** Cached point opacity scalars */
    ToggleAction            _focusSelection;            /** Focus selection action */
    std::int32_t            _lastOpacitySourceIndex;    /** Last opacity source index that was selected */

    static constexpr double DEFAULT_POINT_SIZE      = 10.0;     /** Default point size */
    static constexpr double DEFAULT_POINT_OPACITY   = 100.0;    /** Default point opacity */

    friend class ScatterplotPlugin;
    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(PointPlotAction)

inline const auto pointPlotActionMetaTypeId = qRegisterMetaType<PointPlotAction*>("PointPlotAction");
