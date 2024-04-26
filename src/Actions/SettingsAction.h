#pragma once

#include <actions/GroupAction.h>

#include "LoadedDatasetsAction.h"
#include "RenderModeAction.h"
#include "PositionAction.h"
#include "PlotAction.h"
#include "ExportImageAction.h"
#include "MiscellaneousAction.h"

#include "FilterAction.h"
#include "OverlayAction.h"
#include "ExportAction.h"

using namespace mv::gui;

class GradientExplorerPlugin;

class SettingsAction : public GroupAction
{
public:
    
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

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
    
    RenderModeAction& getRenderModeAction() { return _renderModeAction; }
    PositionAction& getPositionAction() { return _positionAction; }
    PlotAction& getPlotAction() { return _plotAction; }
    //ExportImageAction& getExportImageAction() { return _exportImageAction; }
    MiscellaneousAction& getMiscellaneousAction() { return _miscellaneousAction; }

    FilterAction& getFilterAction() { return _filterAction; }
    OverlayAction& getOverlayAction() { return _overlayAction; }
    ExportAction& getExportAction() { return _exportAction; }
    TriggerAction& getSelectionAsMaskAction() { return _selectionAsMaskAction; }
    TriggerAction& getClearMaskAction() { return _clearMaskAction; }

protected:
    GradientExplorerPlugin*     _plugin;                    /** Pointer to scatter plot plugin */
    LoadedDatasetsAction        _currentDatasetAction;
    RenderModeAction            _renderModeAction;          /** Action for configuring render mode */
    PositionAction              _positionAction;            /** Action for configuring point positions */
    PlotAction                  _plotAction;                /** Action for configuring plot settings */
    //ExportImageAction         _exportImageAction;
    MiscellaneousAction         _miscellaneousAction;       /** Action for miscellaneous settings */

    FilterAction                _filterAction;
    OverlayAction               _overlayAction;
    ExportAction                _exportAction;
    TriggerAction               _selectionAsMaskAction;
    TriggerAction               _clearMaskAction;
};
