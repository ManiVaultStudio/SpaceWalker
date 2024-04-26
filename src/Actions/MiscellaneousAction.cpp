#include "MiscellaneousAction.h"
#include "SpaceWalkerPlugin.h"
#include "Widgets/MainView.h"

using namespace mv::gui;

const QColor MiscellaneousAction::DEFAULT_BACKGROUND_COLOR = qRgb(22, 22, 22);

MiscellaneousAction::MiscellaneousAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _plugin(dynamic_cast<SpaceWalkerPlugin*>(parent->parent())),
    _backgroundColorAction(this, "Background color")
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_backgroundColorAction);

    _backgroundColorAction.setColor(DEFAULT_BACKGROUND_COLOR);

    const auto updateBackgroundColor = [this]() -> void {
        _plugin->getUI().getMainView().setBackgroundColor(_backgroundColorAction.getColor());
    };

    connect(&_backgroundColorAction, &ColorAction::colorChanged, this, [this, updateBackgroundColor](const QColor& color) {
        updateBackgroundColor();
    });

    updateBackgroundColor();
}

QMenu* MiscellaneousAction::getContextMenu()
{
    auto menu = new QMenu("Miscellaneous");

    menu->addAction(&_backgroundColorAction);

    return menu;
}

void MiscellaneousAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicMiscellaneousAction = dynamic_cast<MiscellaneousAction*>(publicAction);

    Q_ASSERT(publicMiscellaneousAction != nullptr);

    if (publicMiscellaneousAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_backgroundColorAction, &publicMiscellaneousAction->getBackgroundColorAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void MiscellaneousAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_backgroundColorAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void MiscellaneousAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _backgroundColorAction.fromParentVariantMap(variantMap);
}

QVariantMap MiscellaneousAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _backgroundColorAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
