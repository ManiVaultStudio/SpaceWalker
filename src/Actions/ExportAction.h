#pragma once

#include <actions/WidgetAction.h>

#include <Actions/TriggerAction.h>

using namespace hdps::gui;

class SpaceWalkerPlugin;

class ExportAction : public WidgetAction
{
    Q_OBJECT
protected: // Widget
    class Widget : public WidgetActionWidget
    {
    public:
        Widget(QWidget* parent, ExportAction* exportAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override
    {
        return new Widget(parent, this, widgetFlags);
    }

public:
    Q_INVOKABLE ExportAction(QObject* parent, const QString& title);

    void initialize(SpaceWalkerPlugin* spaceWalkerPlugin);

    QMenu* getContextMenu();

    /**
     *
     *
     */

public: // Action getters
    TriggerAction& getExportRankingsAction() { return _exportRankingsAction; }
    TriggerAction& getExportFloodnodesAction() { return _exportFloodnodesAction; }
    TriggerAction& getImportKnnGraphAction() { return _importKnnGraphAction; }

protected:
    TriggerAction       _exportRankingsAction;
    TriggerAction       _exportFloodnodesAction;
    TriggerAction       _importKnnGraphAction;
};

Q_DECLARE_METATYPE(ExportAction)

inline const auto exportActionMetaTypeId = qRegisterMetaType<ExportAction*>("ExportAction");
