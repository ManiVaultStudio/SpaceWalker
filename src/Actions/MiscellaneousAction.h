#pragma once

#include <actions/VerticalGroupAction.h>
#include <actions/ColorAction.h>

using namespace mv::gui;

class QMenu;

class GradientExplorerPlugin;

/**
 * Miscellaneous action class
 *
 * Action class for configuring miscellaneous settings (such as the background color)
 *
 * @author Thomas Kroes
 */
class MiscellaneousAction : public VerticalGroupAction
{
public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE MiscellaneousAction(QObject* parent, const QString& title);

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

    ColorAction& getBackgroundColorAction() { return _backgroundColorAction; }

private:
    GradientExplorerPlugin* _plugin;                    /** Pointer to scatter plot plugin */
    ColorAction             _backgroundColorAction;     /** Color action for settings the background color action */

    static const QColor DEFAULT_BACKGROUND_COLOR;

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(MiscellaneousAction)

inline const auto miscellaneousActionMetaTypeId = qRegisterMetaType<MiscellaneousAction*>("MiscellaneousAction");
