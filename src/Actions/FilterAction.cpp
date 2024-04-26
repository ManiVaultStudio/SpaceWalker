#include "FilterAction.h"

#include "SpaceWalkerPlugin.h"
#include "Widgets/MainView.h"

#include <QMenu>
#include <QGroupBox>

FilterAction::FilterAction(QObject* parent, const QString& title) :
    WidgetAction(parent, "Filter Settings"),
    _plugin(nullptr),
    _spatialPeakFilterAction(this, "Spatial Peak Filter"),
    _hdPeakFilterAction(this, "HD Peak Filter"),
    _restrictToFloodAction(this, "Restrict to flood nodes", true),
    _innerFilterSizeAction(this, "Inner Filter Radius", 1, 10, 2.5f, 2),
    _outerFilterSizeAction(this, "Outer Filter Radius", 2, 20, 5, 2),
    _hdInnerFilterSizeAction(this, "HD Inner Filter Size", 1, 10, 5)
    //_hdOuterFilterSizeAction(this, "HD Outer Filter Size", 2, 10, 10)
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("bullseye"));

    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
}

void FilterAction::initialize(SpaceWalkerPlugin* plugin)
{
    filters::Filters& filters = plugin->getFilters();

    auto& spatialPeakFilter = filters.getSpatialPeakFilter();
    auto& hdPeakFilter = filters.getHDPeakFilter();

    connect(&_spatialPeakFilterAction, &TriggerAction::triggered, this, [plugin, &filters]() {
        filters.setFilterType(filters::FilterType::SPATIAL_PEAK);
        plugin->getUI().getFilterLabel().setText("Spatial Peak Ranking");
        plugin->getUI().getMainView().showFiltersCircles(true);
        plugin->getUI().getMainView().update();
        });
    connect(&_hdPeakFilterAction, &TriggerAction::triggered, this, [plugin, &filters]() {
        if (plugin->getFloodFill().getNumWaves() == 0)
        {
            qDebug() << "No flood-fill loaded, cannot do HD ranking";
            return;
        }
        
        filters.setFilterType(filters::FilterType::HD_PEAK);
        plugin->getUI().getFilterLabel().setText("HD Peak Ranking");
        plugin->getUI().getMainView().showFiltersCircles(false);
        plugin->getUI().getMainView().update();
        });

    connect(&_innerFilterSizeAction, &DecimalAction::valueChanged, [plugin, &spatialPeakFilter](const float& value) {
        float projSize = plugin->getProjectionSize();
        spatialPeakFilter.setInnerFilterRadius(value * 0.01f);
        plugin->getUI().getMainView().setFilterRadii(Vector2f(spatialPeakFilter.getInnerFilterRadius() * projSize, spatialPeakFilter.getOuterFilterRadius() * projSize));
        plugin->onPointSelection();
        });
    connect(&_outerFilterSizeAction, &DecimalAction::valueChanged, [plugin, &spatialPeakFilter](const float& value) {
        float projSize = plugin->getProjectionSize();
        spatialPeakFilter.setOuterFilterRadius(value * 0.01f);
        plugin->getUI().getMainView().setFilterRadii(Vector2f(spatialPeakFilter.getInnerFilterRadius() * projSize, spatialPeakFilter.getOuterFilterRadius() * projSize));
        plugin->onPointSelection();
        });

    connect(&_hdInnerFilterSizeAction, &IntegralAction::valueChanged, [&hdPeakFilter](int value) { hdPeakFilter.setInnerFilterSize(value); });
    //connect(&_hdOuterFilterSizeAction, &IntegralAction::valueChanged, [&hdPeakFilter](int value) { hdPeakFilter.setOuterFilterSize(value); });
}

QMenu* FilterAction::getContextMenu()
{
    QMenu* menu = new QMenu("Filter settings");

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_innerFilterSizeAction);
    addActionToMenu(&_outerFilterSizeAction);

    return menu;
}

void FilterAction::setFloodSteps(int numSteps)
{
    if (_hdInnerFilterSizeAction.getValue() > numSteps - 1)
        _hdInnerFilterSizeAction.setValue(numSteps - 1);

    _hdInnerFilterSizeAction.setMaximum(numSteps-1);
}

void FilterAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _innerFilterSizeAction.fromParentVariantMap(variantMap);
    _outerFilterSizeAction.fromParentVariantMap(variantMap);

    _hdInnerFilterSizeAction.fromParentVariantMap(variantMap);
}

QVariantMap FilterAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _innerFilterSizeAction.insertIntoVariantMap(variantMap);
    _outerFilterSizeAction.insertIntoVariantMap(variantMap);

    _hdInnerFilterSizeAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

FilterAction::Widget::Widget(QWidget* parent, FilterAction* filterAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, filterAction, widgetFlags)
{
    setToolTip("Filter settings");
    //setStyleSheet("QToolButton { width: 36px; height: 36px; qproperty-iconSize: 18px; }");
    
    // Add widgets
    auto layout = new QGridLayout();

    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(filterAction->getSpatialPeakFilterAction().createWidget(this), 0, 0);
    layout->addWidget(filterAction->getHDPeakFilterAction().createWidget(this), 0, 1);

    layout->addWidget(filterAction->getRestrictToFloodAction().createWidget(this), 1, 0);

    layout->addWidget(filterAction->getInnerFilterSizeAction().createLabelWidget(this), 2, 0);
    QWidget* w = filterAction->getInnerFilterSizeAction().createWidget(this);
    w->setMinimumWidth(200);
    layout->addWidget(w, 2, 1);

    layout->addWidget(filterAction->getOuterFilterSizeAction().createLabelWidget(this), 3, 0);
    layout->addWidget(filterAction->getOuterFilterSizeAction().createWidget(this), 3, 1);

    layout->addWidget(filterAction->getHDInnerFilterSizeAction().createLabelWidget(this), 4, 0);
    layout->addWidget(filterAction->getHDInnerFilterSizeAction().createWidget(this), 4, 1);

    //layout->addWidget(filterAction->getHDOuterFilterSizeAction().createLabelWidget(this), 5, 0);
    //layout->addWidget(filterAction->getHDOuterFilterSizeAction().createWidget(this), 5, 1);

    setLayout(layout);
}
