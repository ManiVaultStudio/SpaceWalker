#pragma once

#include <actions/WidgetAction.h>

#include <actions/TriggerAction.h>
#include <actions/IntegralAction.h>
#include <actions/ToggleAction.h>

using namespace mv::gui;

class SpaceWalkerPlugin;

class OverlayAction : public WidgetAction
{
    Q_OBJECT
protected: // Widget
    class Widget : public WidgetActionWidget
    {
    public:
        Widget(QWidget* parent, OverlayAction* overlayAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override
    {
        return new Widget(parent, this, widgetFlags);
    }

public:
    Q_INVOKABLE OverlayAction(QObject* parent, const QString& title);

    void initialize(SpaceWalkerPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    /**
     *
     *
     */

public: // Serialization
    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

public: // Action getters
    TriggerAction& getComputeKnnGraphAction() { return _computeKnnGraphAction; }

    IntegralAction& getFloodDecimalAction() { return _floodDecimal; }
    IntegralAction& getFloodStepsAction() { return _floodStepsAction; }
    ToggleAction& getSharedDistAction() { return _sharedDistAction; }

    TriggerAction& getFloodOverlayAction() { return _floodOverlayAction; }
    TriggerAction& getDimensionOverlayAction() { return _dimensionOverlayAction; }
    TriggerAction& getDimensionalityOverlayAction() { return _dimensionalityOverlayAction; }

    //GroupAction& getOverlayGroupAction() { return _overlayGroupAction; }

private:
    SpaceWalkerPlugin* _plugin;             /** Pointer to scatterplot plugin */
    TriggerAction           _computeKnnGraphAction;

    IntegralAction          _floodDecimal;
    IntegralAction          _floodStepsAction;
    ToggleAction            _sharedDistAction;

    TriggerAction           _floodOverlayAction;
    TriggerAction           _dimensionOverlayAction;
    TriggerAction           _dimensionalityOverlayAction;

    //QVector<TriggersAction::Trigger>    _triggers;
    //GroupAction _overlayGroupAction;
};

Q_DECLARE_METATYPE(OverlayAction)

inline const auto overlayActionMetaTypeId = qRegisterMetaType<OverlayAction*>("OverlayAction");
