#include "PositionAction.h"
#include "SpaceWalkerPlugin.h"

#include <QMenu>

using namespace mv::gui;

PositionAction::PositionAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _xDimensionPickerAction(this, "X"),
    _yDimensionPickerAction(this, "Y")
{
    setIcon(mv::util::StyledIcon("ruler-combined"));
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_xDimensionPickerAction);
    addAction(&_yDimensionPickerAction);

    _xDimensionPickerAction.setToolTip("X dimension");
    _yDimensionPickerAction.setToolTip("Y dimension");

    auto spaceWalkerPlugin = dynamic_cast<SpaceWalkerPlugin*>(parent->parent());

    if (spaceWalkerPlugin == nullptr)
        return;

    connect(&_xDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, spaceWalkerPlugin](const std::uint32_t& currentDimensionIndex) {
        if (spaceWalkerPlugin->isDataInitialized())
            spaceWalkerPlugin->setXDimension(currentDimensionIndex);
    });

    connect(&_yDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, spaceWalkerPlugin](const std::uint32_t& currentDimensionIndex) {
        if (spaceWalkerPlugin->isDataInitialized())
            spaceWalkerPlugin->setYDimension(currentDimensionIndex);
    });

    connect(&spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, spaceWalkerPlugin]() {
        _xDimensionPickerAction.setPointsDataset(spaceWalkerPlugin->getPositionDataset());
        _yDimensionPickerAction.setPointsDataset(spaceWalkerPlugin->getPositionDataset());

        _xDimensionPickerAction.setCurrentDimensionIndex(0);

        const auto yIndex = _xDimensionPickerAction.getNumberOfDimensions() >= 2 ? 1 : 0;

        _yDimensionPickerAction.setCurrentDimensionIndex(yIndex);
    });

    const auto updateReadOnly = [this, spaceWalkerPlugin]() -> void {
        setEnabled(spaceWalkerPlugin->getPositionDataset().isValid());
    };

    updateReadOnly();

    connect(&spaceWalkerPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateReadOnly);
}

QMenu* PositionAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Position", parent);

    auto xDimensionMenu = new QMenu("X dimension");
    auto yDimensionMenu = new QMenu("Y dimension");

    xDimensionMenu->addAction(&_xDimensionPickerAction);
    yDimensionMenu->addAction(&_yDimensionPickerAction);

    menu->addMenu(xDimensionMenu);
    menu->addMenu(yDimensionMenu);

    return menu;
}

std::int32_t PositionAction::getDimensionX() const
{
    return _xDimensionPickerAction.getCurrentDimensionIndex();
}

std::int32_t PositionAction::getDimensionY() const
{
    return _yDimensionPickerAction.getCurrentDimensionIndex();
}

void PositionAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicPositionAction = dynamic_cast<PositionAction*>(publicAction);

    Q_ASSERT(publicPositionAction != nullptr);

    if (publicPositionAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_xDimensionPickerAction, &publicPositionAction->getXDimensionPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_yDimensionPickerAction, &publicPositionAction->getYDimensionPickerAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void PositionAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_xDimensionPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_yDimensionPickerAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void PositionAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _xDimensionPickerAction.fromParentVariantMap(variantMap);
    _yDimensionPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap PositionAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _xDimensionPickerAction.insertIntoVariantMap(variantMap);
    _yDimensionPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
