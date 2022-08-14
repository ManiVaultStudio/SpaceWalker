#pragma once

#include <actions/VerticalGroupAction.h>

#include "actions/DatasetPickerAction.h"

using namespace hdps::gui;

class ScatterplotPlugin;

class LoadedDatasetsAction : public VerticalGroupAction
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

    void initialize(ScatterplotPlugin* scatterplotPlugin);

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
    ScatterplotPlugin*      _scatterplotPlugin;             /** Pointer to scatterplot plugin */
    DatasetPickerAction     _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;
    DatasetPickerAction     _sliceDatasetPickerAction;

    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(LoadedDatasetsAction)

inline const auto loadedDatasetsActionMetaTypeId = qRegisterMetaType<LoadedDatasetsAction*>("LoadedDatasetsAction");
