#pragma once

#include <actions/GroupAction.h>

#include "actions/DatasetPickerAction.h"

using namespace mv::gui;

class GradientExplorerPlugin;

class LoadedDatasetsAction : public GroupAction
{
    Q_OBJECT
//protected:
//
//    class Widget : public WidgetActionWidget {
//    public:
//        Widget(QWidget* parent, LoadedDatasetsAction* currentDatasetAction, const std::int32_t& widgetFlags);
//    };
//
//    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
//        return new Widget(parent, this, widgetFlags);
//    };

public:
    Q_INVOKABLE LoadedDatasetsAction(QObject* parent, const QString& title);

    void initialize(GradientExplorerPlugin* scatterplotPlugin);

public: // Serialization

    /**
     * Load widget action from variant map
     * @param Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */
    QVariantMap toVariantMap() const override;

private:
    GradientExplorerPlugin* _plugin;                        /** Pointer to scatterplot plugin */
    DatasetPickerAction     _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;
    DatasetPickerAction     _sliceDatasetPickerAction;

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(LoadedDatasetsAction)

inline const auto loadedDatasetsActionMetaTypeId = qRegisterMetaType<LoadedDatasetsAction*>("LoadedDatasetsAction");
