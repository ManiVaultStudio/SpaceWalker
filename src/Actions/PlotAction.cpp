#include "PlotAction.h"
#include "GradientExplorerPlugin.h"
#include "Widgets/MainView.h"

using namespace mv::gui;

PlotAction::PlotAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _plugin(nullptr),
    _pointPlotAction(this, "Point"),
    _densityPlotAction(this, "Density")
{
    setToolTip("Plot settings");
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("paint-brush"));
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_pointPlotAction.getSizeAction());
    addAction(&_pointPlotAction.getOpacityAction());
    //addAction(&_pointPlotAction.getFocusSelection());
    
    addAction(&_densityPlotAction.getSigmaAction());
    addAction(&_densityPlotAction.getContinuousUpdatesAction());
}

void PlotAction::initialize(GradientExplorerPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _plugin = scatterplotPlugin;

    _pointPlotAction.initialize(_plugin);
    _densityPlotAction.initialize(_plugin);

    auto& mainView = _plugin->getUI().getMainView();

    const auto updateRenderMode = [this, &mainView]() -> void {
        _pointPlotAction.setVisible(mainView.getRenderMode() == MainView::SCATTERPLOT);
        _densityPlotAction.setVisible(mainView.getRenderMode() != MainView::SCATTERPLOT);
    };

    updateRenderMode();

    connect(&mainView, &MainView::renderModeChanged, this, updateRenderMode);
}

QMenu* PlotAction::getContextMenu()
{
    if (_plugin == nullptr)
        return nullptr;

    switch (_plugin->getUI().getMainView().getRenderMode())
    {
        case MainView::RenderMode::SCATTERPLOT:
            return _pointPlotAction.getContextMenu();
            break;

        case MainView::RenderMode::DENSITY:
        case MainView::RenderMode::LANDSCAPE:
            return _densityPlotAction.getContextMenu();
            break;

        default:
            break;
    }

    return new QMenu("Plot");
}

void PlotAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicPlotAction = dynamic_cast<PlotAction*>(publicAction);

    Q_ASSERT(publicPlotAction != nullptr);

    if (publicPlotAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_pointPlotAction, &publicPlotAction->getPointPlotAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_densityPlotAction, &publicPlotAction->getDensityPlotAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void PlotAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_pointPlotAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_densityPlotAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void PlotAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _pointPlotAction.fromParentVariantMap(variantMap);
    _densityPlotAction.fromParentVariantMap(variantMap);
}

QVariantMap PlotAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _pointPlotAction.insertIntoVariantMap(variantMap);
    _densityPlotAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
