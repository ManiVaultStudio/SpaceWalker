#include "ExportImageDialog.h"
#include "GradientExplorerPlugin.h"

#include <Application.h>

#include <QVBoxLayout>

using namespace mv;

ExportImageDialog::ExportImageDialog(QWidget* parent, GradientExplorerPlugin& scatterplotPlugin) :
    QDialog(parent),
    _plugin(scatterplotPlugin),
    _exportImageAction(&scatterplotPlugin, "ExportImageAction")
{
    setWindowTitle("Export " + scatterplotPlugin.getPositionDataset()->getGuiName() + " to image(s)");
    setWindowIcon(Application::getIconFont("FontAwesome").getIcon("file-export"));
    setModal(true);

    auto layout = new QVBoxLayout();

    auto exportImageWidget = _exportImageAction.createWidget(this);

    dynamic_cast<QGridLayout*>(exportImageWidget->layout())->setColumnStretch(0, 1);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(exportImageWidget);
    layout->addStretch(1);

    setLayout(layout);

    // Reject when the cancel action is triggered
    connect(&_exportImageAction.getExportCancelAction(), &TriggersAction::triggered, this, [this](std::int32_t triggerIndex) {
        if (triggerIndex == 1)
            reject();
    });
}
