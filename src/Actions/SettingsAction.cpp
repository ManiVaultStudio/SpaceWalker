#include "SettingsAction.h"

#include "SpaceWalkerPlugin.h"
#include "Widgets/MainView.h"
#include "PointData/PointData.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<SpaceWalkerPlugin*>(parent)),
    _currentDatasetAction(this, "Current dataset"),
    _renderModeAction(this, "Render Mode"),
    _positionAction(this, "Position"),
    _plotAction(this, "Plot"),
    //_exportImageAction(this, "Export to image/video"),
    _miscellaneousAction(this, "Miscellaneous"),
    _filterAction(this, "Filter"),
    _overlayAction(this, "Overlay"),
    _exportAction(this, "Export"),
    _selectionAsMaskAction(this, "Selection As Mask"),
    _clearMaskAction(this, "Clear Mask"),
    _sectionAction(this, "Section")
{
    setConnectionPermissionsToForceNone();

    _currentDatasetAction.initialize(_plugin);
    _renderModeAction.initialize(_plugin);
    _plotAction.initialize(_plugin);
    //_exportImageAction.initialize(_scatterplotPlugin);

    _filterAction.initialize(_plugin);
    _overlayAction.initialize(_plugin);
    _exportAction.initialize(_plugin);

    //_exportImageAction.setEnabled(false);

    const auto updateEnabled = [this]() {
        bool hasDataset = _plugin->getPositionDataset().isValid();

        _renderModeAction.setEnabled(hasDataset);
        _positionAction.setEnabled(hasDataset);
        _plotAction.setEnabled(hasDataset);
        //_exportImageAction.setEnabled(hasDataset);
        _miscellaneousAction.setEnabled(hasDataset);
        _filterAction.setEnabled(hasDataset);
        _overlayAction.setEnabled(hasDataset);
        _exportAction.setEnabled(hasDataset);
        _selectionAsMaskAction.setEnabled(hasDataset);
        _clearMaskAction.setEnabled(hasDataset);
        //setEnabled(_scatterplotPlugin->getPositionDataset().isValid());
    };

    connect(&_plugin->getPositionDataset(), &Dataset<Points>::changed, this, updateEnabled);

    updateEnabled();

    //_exportImageAction.setIcon(mv::Application::getIconFont("FontAwesome").getIcon("camera"));
    //_exportImageAction.setDefaultWidgetFlags(TriggerAction::Icon);

    connect(&_selectionAsMaskAction, &TriggerAction::triggered, this, [this]() {
        _plugin->useSelectionAsMask();
    });

    connect(&_clearMaskAction, &TriggerAction::triggered, this, [this]() {
        _plugin->clearMask();
    });
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    menu->addMenu(_renderModeAction.getContextMenu());
    menu->addMenu(_plotAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_positionAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_miscellaneousAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_filterAction.getContextMenu());
    menu->addMenu(_overlayAction.getContextMenu());
    menu->addMenu(_exportAction.getContextMenu());

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _currentDatasetAction.fromParentVariantMap(variantMap);
    _plotAction.fromParentVariantMap(variantMap);
    _positionAction.fromParentVariantMap(variantMap);
    _filterAction.fromParentVariantMap(variantMap);
    _overlayAction.fromParentVariantMap(variantMap);
    _miscellaneousAction.fromParentVariantMap(variantMap);
    _sectionAction.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _currentDatasetAction.insertIntoVariantMap(variantMap);
    _plotAction.insertIntoVariantMap(variantMap);
    _positionAction.insertIntoVariantMap(variantMap);
    _filterAction.insertIntoVariantMap(variantMap);
    _overlayAction.insertIntoVariantMap(variantMap);
    _miscellaneousAction.insertIntoVariantMap(variantMap);
    _sectionAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
