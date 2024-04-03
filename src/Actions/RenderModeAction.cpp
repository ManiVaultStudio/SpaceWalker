#include "RenderModeAction.h"
#include "SpaceWalkerPlugin.h"
#include "ScatterplotWidget.h"

using namespace mv::gui;

RenderModeAction::RenderModeAction(QObject* parent, const QString& title) :
    OptionAction(parent, title, { "Scatter", "Density", "Contour", "Cell"}),
    _spaceWalkerPlugin(nullptr),
    _scatterPlotAction(this, "Scatter"),
    _densityPlotAction(this, "Density"),
    _contourPlotAction(this, "Contour"),
    _cellPlotAction(this, "Cell")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("image"));
    setDefaultWidgetFlags(OptionAction::HorizontalButtons);
    setEnabled(false);

    _scatterPlotAction.setConnectionPermissionsToForceNone(true);
    _densityPlotAction.setConnectionPermissionsToForceNone(true);
    _contourPlotAction.setConnectionPermissionsToForceNone(true);
    _cellPlotAction.setConnectionPermissionsToForceNone(true);

    _scatterPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _densityPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _contourPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _cellPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);

    _scatterPlotAction.setShortcut(QKeySequence("S"));
    _densityPlotAction.setShortcut(QKeySequence("D"));
    _contourPlotAction.setShortcut(QKeySequence("C"));
    _cellPlotAction.setShortcut(QKeySequence("V"));

    _scatterPlotAction.setToolTip("Set render mode to scatter plot (S)");
    _densityPlotAction.setToolTip("Set render mode to density plot (D)");
    _contourPlotAction.setToolTip("Set render mode to contour plot (C)");
    _cellPlotAction.setToolTip("Set render mode to cell plot (V)");
}

void RenderModeAction::initialize(SpaceWalkerPlugin* spaceWalkerPlugin)
{
    Q_ASSERT(spaceWalkerPlugin != nullptr);

    if (spaceWalkerPlugin == nullptr)
        return;

    _spaceWalkerPlugin = spaceWalkerPlugin;

    _spaceWalkerPlugin->getWidget().addAction(&_scatterPlotAction);
    //_spaceWalkerPlugin->getWidget().addAction(&_densityPlotAction);
    //_spaceWalkerPlugin->getWidget().addAction(&_contourPlotAction);
    _spaceWalkerPlugin->getWidget().addAction(&_cellPlotAction);

    const auto currentIndexChanged = [this]() {
        const auto renderMode = static_cast<RenderMode>(getCurrentIndex());

        _scatterPlotAction.setChecked(renderMode == RenderMode::ScatterPlot);
        _densityPlotAction.setChecked(renderMode == RenderMode::DensityPlot);
        _contourPlotAction.setChecked(renderMode == RenderMode::ContourPlot);
        _cellPlotAction.setChecked(renderMode == RenderMode::CellPlot);

        _spaceWalkerPlugin->getScatterplotWidget().setRenderMode(static_cast<ScatterplotWidget::RenderMode>(getCurrentIndex()));
    };

    currentIndexChanged();

    connect(this, &OptionAction::currentIndexChanged, this, currentIndexChanged);

    connect(&_scatterPlotAction, &QAction::toggled, this, [this, spaceWalkerPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::ScatterPlot));
    });

    connect(&_densityPlotAction, &QAction::toggled, this, [this, spaceWalkerPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::DensityPlot));
    });

    connect(&_contourPlotAction, &QAction::toggled, this, [this, spaceWalkerPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::ContourPlot));
    });

    connect(&_cellPlotAction, &QAction::toggled, this, [this, spaceWalkerPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::CellPlot));
    });

    setCurrentIndex(static_cast<std::int32_t>(RenderMode::ScatterPlot));

    const auto updateReadOnly = [this]() -> void {
        setEnabled(_spaceWalkerPlugin->getPositionDataset().isValid());
    };

    updateReadOnly();

    connect(&_spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateReadOnly);
}

QMenu* RenderModeAction::getContextMenu()
{
    auto menu = new QMenu("Render mode");

    menu->addAction(&_scatterPlotAction);
    menu->addAction(&_densityPlotAction);
    menu->addAction(&_contourPlotAction);
    menu->addAction(&_cellPlotAction);

    return menu;
}

void RenderModeAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicRenderModeAction = dynamic_cast<RenderModeAction*>(publicAction);

    Q_ASSERT(publicRenderModeAction != nullptr);

    if (publicRenderModeAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_scatterPlotAction, &publicRenderModeAction->getScatterPlotAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_densityPlotAction, &publicRenderModeAction->getDensityPlotAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_contourPlotAction, &publicRenderModeAction->getContourPlotAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_cellPlotAction, &publicRenderModeAction->getCellPlotAction(), recursive);
    }

    OptionAction::connectToPublicAction(publicAction, recursive);
}

void RenderModeAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_scatterPlotAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_densityPlotAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_contourPlotAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_cellPlotAction, recursive);
    }

    OptionAction::disconnectFromPublicAction(recursive);
}

void RenderModeAction::fromVariantMap(const QVariantMap& variantMap)
{
    OptionAction::fromVariantMap(variantMap);

    _scatterPlotAction.fromParentVariantMap(variantMap);
    _densityPlotAction.fromParentVariantMap(variantMap);
    _contourPlotAction.fromParentVariantMap(variantMap);
    _cellPlotAction.fromParentVariantMap(variantMap);
}

QVariantMap RenderModeAction::toVariantMap() const
{
    auto variantMap = OptionAction::toVariantMap();

    _scatterPlotAction.insertIntoVariantMap(variantMap);
    _densityPlotAction.insertIntoVariantMap(variantMap);
    _contourPlotAction.insertIntoVariantMap(variantMap);
    _cellPlotAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
