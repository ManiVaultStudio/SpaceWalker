#include "DensityPlotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

DensityPlotAction::DensityPlotAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _sigmaAction(this, "Sigma", 0.01f, 0.5f, DEFAULT_SIGMA, 3),
    _continuousUpdatesAction(this, "Live Updates", DEFAULT_CONTINUOUS_UPDATES)
{
    setToolTip("Density plot settings");
    setConfigurationFlag(WidgetAction::ConfigurationFlag::NoLabelInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_sigmaAction);
    addAction(&_continuousUpdatesAction);
}

void DensityPlotAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    const auto computeDensity = [this]() -> void {
        if (static_cast<std::int32_t>(_scatterplotPlugin->getSettingsAction().getRenderModeAction().getCurrentIndex()) == ScatterplotWidget::RenderMode::SCATTERPLOT)
            return;

        _scatterplotPlugin->getScatterplotWidget().setSigma(_sigmaAction.getValue());

        const auto maxDensity = _scatterplotPlugin->getScatterplotWidget().getDensityRenderer().getMaxDensity();

        //if (maxDensity > 0)
            //_scatterplotPlugin->getSettingsAction().getColoringAction().getColorMap1DAction().getRangeAction(ColorMapAction::Axis::X).setRange({ 0.0f, maxDensity });
    };

    connect(&_sigmaAction, &DecimalAction::valueChanged, this, computeDensity);

    const auto updateSigmaAction = [this]() {
        _sigmaAction.setUpdateDuringDrag(_continuousUpdatesAction.isChecked());
    };

    connect(&_continuousUpdatesAction, &ToggleAction::toggled, updateSigmaAction);

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, updateSigmaAction, computeDensity](DatasetImpl* dataset) {
        updateSigmaAction();
        computeDensity();
    });

    connect(&_scatterplotPlugin->getSettingsAction().getRenderModeAction(), &OptionAction::currentIndexChanged, this, computeDensity);

    updateSigmaAction();
    computeDensity();
}

QMenu* DensityPlotAction::getContextMenu()
{
    if (_scatterplotPlugin == nullptr)
        return nullptr;

    auto menu = new QMenu("Plot settings");

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sigmaAction);
    addActionToMenu(&_continuousUpdatesAction);

    return menu;
}

void DensityPlotAction::setVisible(bool visible)
{
    _sigmaAction.setVisible(visible);
    _continuousUpdatesAction.setVisible(visible);
}

void DensityPlotAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicDensityPlotAction = dynamic_cast<DensityPlotAction*>(publicAction);

    Q_ASSERT(publicDensityPlotAction != nullptr);

    if (publicDensityPlotAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_sigmaAction, &publicDensityPlotAction->getSigmaAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_continuousUpdatesAction, &publicDensityPlotAction->getContinuousUpdatesAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void DensityPlotAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_sigmaAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_continuousUpdatesAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void DensityPlotAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _sigmaAction.fromParentVariantMap(variantMap);
    _continuousUpdatesAction.fromParentVariantMap(variantMap);
}

QVariantMap DensityPlotAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _sigmaAction.insertIntoVariantMap(variantMap);
    _continuousUpdatesAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
