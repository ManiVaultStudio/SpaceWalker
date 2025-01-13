#include "OverlayAction.h"

#include "SpaceWalkerPlugin.h"
#include "ScatterplotWidget.h"

#include <QMenu>
#include <QGroupBox>

OverlayAction::OverlayAction(QObject* parent, const QString& title) :
    WidgetAction(parent, "Overlay Settings"),
    _spaceWalkerPlugin(nullptr),
    _computeKnnGraphAction(this, "Compute Floods"),
    _approximateKnnAction(this, "(recommended for large data)", false),
    _floodDecimal(this, "Flood nodes", 10, 500, 10),
    _floodStepsAction(this, "Flood steps", 2, 50, 10),
    _sharedDistAction(this, " ", false), // intentionally empty title, use label in OverlayAction::Widget
    _floodOverlayAction(this, "Flood Steps"),
    _dimensionOverlayAction(this, "Top Dimension Values"),
    _dimensionalityOverlayAction(this, "Local Dimensionality")
    //_overlayGroupAction(this, true)
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("image"));

    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    //_triggers << TriggersAction::Trigger("Flood Steps", "Color flood points by closeness to seed point in HD space");
    //_triggers << TriggersAction::Trigger("Top Dimension Values", "Color flood points by values of top ranked dimension");
    //_triggers << TriggersAction::Trigger("Local Dimensionality", "Color flood points by local intrinsic dimensionality");
    //_triggers << TriggersAction::Trigger("Directions", "Show major eigenvector directions over flood points");

    //_overlayGroupAction.setText("Flood Nodes Overlay");
    //_overlayGroupAction.setShowLabels(false);

    //TriggersAction* overlayTriggers = new TriggersAction(&_overlayGroupAction, "Overlay Triggers", _triggers);

    //connect(overlayTriggers, &TriggersAction::triggered, this, [spaceWalkerPlugin](int32_t triggerIndex)
    //{
    //    spaceWalkerPlugin->getScatterplotWidget().showDirections(false);
    //    switch (triggerIndex)
    //    {
    //    case 0: spaceWalkerPlugin->setOverlayType(OverlayType::NONE); break;
    //    case 1: spaceWalkerPlugin->setOverlayType(OverlayType::DIM_VALUES); break;
    //    case 2: spaceWalkerPlugin->setOverlayType(OverlayType::LOCAL_DIMENSIONALITY); break;
    //    case 3: {spaceWalkerPlugin->setOverlayType(OverlayType::DIRECTIONS); spaceWalkerPlugin->getScatterplotWidget().showDirections(true); break; }
    //    }
    //});
}

void OverlayAction::initialize(SpaceWalkerPlugin* spaceWalkerPlugin)
{
    connect(&_computeKnnGraphAction, &TriggerAction::triggered, this, [spaceWalkerPlugin, this](bool enabled)
    {
        const bool preciseKnn = !_approximateKnnAction.isChecked();
        spaceWalkerPlugin->createKnnIndex(preciseKnn);
        spaceWalkerPlugin->computeKnnGraph();
    });

    connect(&_floodDecimal, &IntegralAction::valueChanged, this, [spaceWalkerPlugin](int32_t value)
    {
        spaceWalkerPlugin->rebuildKnnGraph(value);
    });

    connect(&_floodStepsAction, &IntegralAction::valueChanged, this, [spaceWalkerPlugin](int32_t value)
    {
        spaceWalkerPlugin->setFloodSteps(value);
        spaceWalkerPlugin->onPointSelection();
    });

    connect(&_sharedDistAction, &ToggleAction::toggled, this, [spaceWalkerPlugin](bool enabled)
    {
        spaceWalkerPlugin->useSharedDistances(enabled);
    });

    // Overlay buttons
    connect(&_floodOverlayAction, &TriggerAction::triggered, this, [spaceWalkerPlugin](bool enabled)
    {
        spaceWalkerPlugin->setOverlayType(OverlayType::NONE);
        spaceWalkerPlugin->onPointSelection();
    });
    connect(&_dimensionOverlayAction, &TriggerAction::triggered, this, [spaceWalkerPlugin](bool enabled)
    {
        spaceWalkerPlugin->setOverlayType(OverlayType::DIM_VALUES);
        spaceWalkerPlugin->onPointSelection();
    });
    connect(&_dimensionalityOverlayAction, &TriggerAction::triggered, this, [spaceWalkerPlugin](bool enabled)
    {
        spaceWalkerPlugin->setOverlayType(OverlayType::LOCAL_DIMENSIONALITY);
        spaceWalkerPlugin->onPointSelection();
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
    addActionToMenu(&_approximateKnnAction);
    addActionToMenu(&_floodDecimal);
    addActionToMenu(&_floodStepsAction);
    addActionToMenu(&_sharedDistAction);

    return menu;
}

void OverlayAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _approximateKnnAction.fromParentVariantMap(variantMap);
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

    _approximateKnnAction.insertIntoVariantMap(variantMap);
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

    QLabel* approximateKnnLabel = new QLabel("Approximate kNN:", parent);
    approximateKnnLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(approximateKnnLabel, 1, 0);
    layout->addWidget(overlayAction->getApproximateKnnAction().createWidget(this), 1, 1);

    layout->addWidget(overlayAction->getFloodDecimalAction().createLabelWidget(this), 2, 0);
    layout->addWidget(overlayAction->getFloodDecimalAction().createWidget(this), 2, 1);

    layout->addWidget(overlayAction->getFloodStepsAction().createLabelWidget(this), 3, 0);
    layout->addWidget(overlayAction->getFloodStepsAction().createWidget(this), 3, 1);

    QLabel* sharedDistancesLabel = new QLabel("Shared distances:", parent);
    sharedDistancesLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(sharedDistancesLabel, 4, 0);
    layout->addWidget(overlayAction->getSharedDistAction().createWidget(this), 4, 1);

    layout->addWidget(new QLabel("Color flood nodes by:", parent), 5, 0);
    layout->addWidget(overlayAction->getFloodOverlayAction().createWidget(this), 6, 0);
    layout->addWidget(overlayAction->getDimensionOverlayAction().createWidget(this), 6, 1);
    layout->addWidget(overlayAction->getDimensionalityOverlayAction().createWidget(this), 6, 2);

    setLayout(layout);
}
