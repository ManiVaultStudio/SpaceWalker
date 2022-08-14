#include "PlotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

PlotAction::PlotAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _scatterplotPlugin(nullptr),
    _pointPlotAction(this, "Point"),
    _densityPlotAction(this, "Density")
{
    setToolTip("Plot settings");
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("paint-brush"));
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_pointPlotAction.getSizeAction());
    addAction(&_pointPlotAction.getOpacityAction());
    //addAction(&_pointPlotAction.getFocusSelection());
    
    addAction(&_densityPlotAction.getSigmaAction());
    addAction(&_densityPlotAction.getContinuousUpdatesAction());
}

void PlotAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    _pointPlotAction.initialize(_scatterplotPlugin);
    _densityPlotAction.initialize(_scatterplotPlugin);

    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();

    const auto updateRenderMode = [this, &scatterplotWidget]() -> void {
        _pointPlotAction.setVisible(scatterplotWidget.getRenderMode() == ScatterplotWidget::SCATTERPLOT);
        _densityPlotAction.setVisible(scatterplotWidget.getRenderMode() != ScatterplotWidget::SCATTERPLOT);
    };

    updateRenderMode();

    connect(&scatterplotWidget, &ScatterplotWidget::renderModeChanged, this, updateRenderMode);
}

QMenu* PlotAction::getContextMenu()
{
    if (_scatterplotPlugin == nullptr)
        return nullptr;

    switch (_scatterplotPlugin->getScatterplotWidget().getRenderMode())
    {
        case ScatterplotWidget::RenderMode::SCATTERPLOT:
            return _pointPlotAction.getContextMenu();
            break;

        case ScatterplotWidget::RenderMode::DENSITY:
        case ScatterplotWidget::RenderMode::LANDSCAPE:
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
