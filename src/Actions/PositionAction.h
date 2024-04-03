#pragma once

#include <actions/VerticalGroupAction.h>

#include <PointData/DimensionPickerAction.h>

#include <QHBoxLayout>
#include <QLabel>

using namespace mv::gui;

/**
 * Position action class
 *
 * Action class for picking data dimensions for the point positions
 *
 * @author Thomas Kroes
 */
class PositionAction : public VerticalGroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE PositionAction(QObject* parent, const QString& title);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /** Get current x-dimension */
    std::int32_t getDimensionX() const;

    /** Get current y-dimension */
    std::int32_t getDimensionY() const;

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

    DimensionPickerAction& getXDimensionPickerAction() { return _xDimensionPickerAction; }
    DimensionPickerAction& getYDimensionPickerAction() { return _yDimensionPickerAction; }

private:
    DimensionPickerAction    _xDimensionPickerAction;   /** X-dimension picker action */
    DimensionPickerAction    _yDimensionPickerAction;   /** Y-dimension picker action */

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(PositionAction)

inline const auto positionActionMetaTypeId = qRegisterMetaType<PositionAction*>("PositionAction");
