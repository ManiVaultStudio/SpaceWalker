#pragma once

#include <actions/VerticalGroupAction.h>

#include "PointPlotAction.h"
#include "DensityPlotAction.h"

class SpaceWalkerPlugin;

using namespace mv::gui;

/**
 * Plot action class
 *
 * Action class for configuring plot settings
 *
 * @author Thomas Kroes
 */
class PlotAction : public VerticalGroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE PlotAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p spaceWalkerPlugin
     * @param spaceWalkerPlugin Pointer to scatterplot plugin
     */
    void initialize(SpaceWalkerPlugin* spaceWalkerPlugin);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

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

    PointPlotAction& getPointPlotAction() { return _pointPlotAction; }
    DensityPlotAction& getDensityPlotAction() { return _densityPlotAction; }

private:
    SpaceWalkerPlugin*  _spaceWalkerPlugin;     /** Pointer to scatterplot plugin */
    PointPlotAction     _pointPlotAction;       /** Point plot action */
    DensityPlotAction   _densityPlotAction;     /** Density plot action */

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(PlotAction)

inline const auto plotActionMetaTypeId = qRegisterMetaType<PlotAction*>("PlotAction");
