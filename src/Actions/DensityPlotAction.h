#pragma once

#include <actions/VerticalGroupAction.h>
#include <actions/DecimalAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

class SpaceWalkerPlugin;

/**
 * Density plot action class
 *
 * Action class for configuring density plot settings
 *
 * @author Thomas Kroes
 */
class DensityPlotAction : public VerticalGroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE DensityPlotAction(QObject* parent, const QString& title);

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

    /**
     * Override to show/hide child actions
     * @param visible Whether the action is visible or not
     */
    void setVisible(bool visible);

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

    DecimalAction& getSigmaAction() { return _sigmaAction; }
    ToggleAction& getContinuousUpdatesAction() { return _continuousUpdatesAction; }

private:
    SpaceWalkerPlugin*  _spaceWalkerPlugin;         /** Pointer to scatterplot plugin */
    DecimalAction       _sigmaAction;               /** Density sigma action */
    ToggleAction        _continuousUpdatesAction;   /** Live updates action */

    static constexpr double DEFAULT_SIGMA = 0.15f;
    static constexpr bool DEFAULT_CONTINUOUS_UPDATES = true;

    friend class PlotAction;
    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(DensityPlotAction)

inline const auto densityPlotActionMetaTypeId = qRegisterMetaType<DensityPlotAction*>("DensityPlotAction");
