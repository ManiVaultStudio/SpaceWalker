#include "OverlayAction.h"

#include "GradientExplorerPlugin.h"
#include "Widgets/MainView.h"

#include <QMenu>
#include <QGroupBox>

OverlayAction::OverlayAction(QObject* parent, const QString& title) :
    WidgetAction(parent, "Overlay Settings"),
    _plugin(nullptr),
    _computeKnnGraphAction(this, "Compute Floods"),
    _floodDecimal(this, "Flood nodes", 10, 500, 10),
    _floodStepsAction(this, "Flood steps", 2, 50, 10),
    _sharedDistAction(this, "Shared distances", false),
    _floodOverlayAction(this, "Flood Steps"),
    _dimensionOverlayAction(this, "Top Dimension Values"),
    _dimensionalityOverlayAction(this, "Local Dimensionality")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("image"));

    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
}

void OverlayAction::initialize(GradientExplorerPlugin* scatterplotPlugin)
{
    connect(&_computeKnnGraphAction, &TriggerAction::triggered, this, [scatterplotPlugin](bool enabled)
    {
        scatterplotPlugin->createKnnIndex();
        scatterplotPlugin->computeKnnGraph();
    });

    connect(&_floodDecimal, &IntegralAction::valueChanged, this, [scatterplotPlugin](int32_t value)
    {
        scatterplotPlugin->rebuildKnnGraph(value);
    });

    connect(&_floodStepsAction, &IntegralAction::valueChanged, this, [scatterplotPlugin](int32_t value)
    {
        scatterplotPlugin->setFloodSteps(value);
        scatterplotPlugin->onPointSelection();
    });

    connect(&_sharedDistAction, &ToggleAction::toggled, this, [scatterplotPlugin](bool enabled)
    {
        scatterplotPlugin->useSharedDistances(enabled);
    });

    // Overlay buttons
    connect(&_floodOverlayAction, &TriggerAction::triggered, this, [scatterplotPlugin](bool enabled)
    {
        scatterplotPlugin->setOverlayType(OverlayType::NONE);
        scatterplotPlugin->onPointSelection();
    });
    connect(&_dimensionOverlayAction, &TriggerAction::triggered, this, [scatterplotPlugin](bool enabled)
    {
        scatterplotPlugin->setOverlayType(OverlayType::DIM_VALUES);
        scatterplotPlugin->onPointSelection();
    });
    connect(&_dimensionalityOverlayAction, &TriggerAction::triggered, this, [scatterplotPlugin](bool enabled)
    {
        scatterplotPlugin->setOverlayType(OverlayType::LOCAL_DIMENSIONALITY);
        scatterplotPlugin->onPointSelection();
    });
}

QMenu* OverlayAction::getContextMenu()
{
    QMenu* menu = new QMenu("Overlay settings");

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_computeKnnGraphAction);
    addActionToMenu(&_floodDecimal);
    addActionToMenu(&_floodStepsAction);
    addActionToMenu(&_sharedDistAction);

    return menu;
}

void OverlayAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _floodDecimal.fromParentVariantMap(variantMap);
    _floodStepsAction.fromParentVariantMap(variantMap);
    _sharedDistAction.fromParentVariantMap(variantMap);

    _floodOverlayAction.fromParentVariantMap(variantMap);
    _dimensionOverlayAction.fromParentVariantMap(variantMap);
    _dimensionalityOverlayAction.fromParentVariantMap(variantMap);
}

QVariantMap OverlayAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _floodDecimal.insertIntoVariantMap(variantMap);
    _floodStepsAction.insertIntoVariantMap(variantMap);
    _sharedDistAction.insertIntoVariantMap(variantMap);

    _floodOverlayAction.insertIntoVariantMap(variantMap);
    _dimensionOverlayAction.insertIntoVariantMap(variantMap);
    _dimensionalityOverlayAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

OverlayAction::Widget::Widget(QWidget* parent, OverlayAction* overlayAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, overlayAction, widgetFlags)
{
    setToolTip("Overlay settings");
    //setStyleSheet("QToolButton { width: 36px; height: 36px; qproperty-iconSize: 18px; }");

    auto layout = new QGridLayout();

    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(overlayAction->getComputeKnnGraphAction().createLabelWidget(this), 0, 0);
    layout->addWidget(overlayAction->getComputeKnnGraphAction().createWidget(this), 0, 1);

    layout->addWidget(overlayAction->getFloodDecimalAction().createLabelWidget(this), 1, 0);
    layout->addWidget(overlayAction->getFloodDecimalAction().createWidget(this), 1, 1);

    layout->addWidget(overlayAction->getFloodStepsAction().createLabelWidget(this), 2, 0);
    layout->addWidget(overlayAction->getFloodStepsAction().createWidget(this), 2, 1);

    layout->addWidget(overlayAction->getSharedDistAction().createLabelWidget(this), 3, 0);
    layout->addWidget(overlayAction->getSharedDistAction().createWidget(this), 3, 1);

    layout->addWidget(new QLabel("Color flood nodes by:", parent), 4, 0);
    layout->addWidget(overlayAction->getFloodOverlayAction().createWidget(this), 5, 0);
    layout->addWidget(overlayAction->getDimensionOverlayAction().createWidget(this), 5, 1);
    layout->addWidget(overlayAction->getDimensionalityOverlayAction().createWidget(this), 5, 2);

    setLayout(layout);
}
