#include "UserInterface.h"

UserInterface::UserInterface(QObject* parent) :
    _mainView(new MainView()),
    _projectionViews(2, nullptr),
    _selectedView(),
    _dropWidget(nullptr),
    _settingsAction(parent, "SettingsAction"),
    _colorMapAction(parent, "Color map", "RdYlBu"),
    _primaryToolbarAction(parent, "PrimaryToolbar"),
    _secondaryToolbarAction(parent, "SecondaryToolbar"),
    _filterLabel(nullptr)
{

}

void UserInterface::init()
{
    _primaryToolbarAction.addAction(&_settingsAction.getRenderModeAction(), 4, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getPlotAction(), 7, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getPositionAction(), 10, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getFilterAction(), 0, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getOverlayAction(), 0, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getExportAction(), 0, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getSelectionAsMaskAction());
    _primaryToolbarAction.addAction(&_settingsAction.getClearMaskAction());

    _dropWidget = new DropWidget(_mainView);

    for (int i = 0; i < _projectionViews.size(); i++)
    {
        _projectionViews[i] = new ProjectionView();
    }
    _selectedView = new ProjectionView();
}

void UserInterface::initializeLabels()
{
    _filterLabel = new QLabel("Spatial Peak Ranking");
    _dimensionSelectionLabel = new QLabel("Dimension Selection");
    _sortedExpressionGraphLabel = new QLabel("Sorted Expression Graph");

    QFont sansFont("Helvetica [Cronyx]", 18);
    _filterLabel->setFont(sansFont);
    _dimensionSelectionLabel->setFont(sansFont);
    _sortedExpressionGraphLabel->setFont(sansFont);
}
