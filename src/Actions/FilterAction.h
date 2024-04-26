#pragma once

#include <actions/WidgetAction.h>

#include <actions/TriggerAction.h>
#include <actions/ToggleAction.h>
#include <actions/DecimalAction.h>
#include <actions/IntegralAction.h>

using namespace mv::gui;

class GradientExplorerPlugin;

class FilterAction : public WidgetAction
{
    Q_OBJECT
protected: // Widget
    class Widget : public WidgetActionWidget
    {
    public:
        Widget(QWidget* parent, FilterAction* filterAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override
    {
        return new Widget(parent, this, widgetFlags);
    }

public:
    Q_INVOKABLE FilterAction(QObject* parent, const QString& title);

    void initialize(GradientExplorerPlugin* scatterplotPlugin);

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
    void setFloodSteps(int numSteps);

    TriggerAction& getSpatialPeakFilterAction() { return _spatialPeakFilterAction; }
    TriggerAction& getHDPeakFilterAction() { return _hdPeakFilterAction; }

    ToggleAction& getRestrictToFloodAction() { return _restrictToFloodAction; }

    DecimalAction& getInnerFilterSizeAction() { return _innerFilterSizeAction; }
    DecimalAction& getOuterFilterSizeAction() { return _outerFilterSizeAction; }

    IntegralAction& getHDInnerFilterSizeAction() { return _hdInnerFilterSizeAction; }
    //IntegralAction& getHDOuterFilterSizeAction() { return _hdOuterFilterSizeAction; }

protected:
    GradientExplorerPlugin* _plugin;             /** Pointer to scatterplot plugin */
    TriggerAction           _spatialPeakFilterAction;
    TriggerAction           _hdPeakFilterAction;

    ToggleAction            _restrictToFloodAction;

    DecimalAction           _innerFilterSizeAction;
    DecimalAction           _outerFilterSizeAction;

    IntegralAction          _hdInnerFilterSizeAction;
    //IntegralAction        _hdOuterFilterSizeAction;
};

Q_DECLARE_METATYPE(FilterAction)

inline const auto filterActionMetaTypeId = qRegisterMetaType<FilterAction*>("FilterAction");
