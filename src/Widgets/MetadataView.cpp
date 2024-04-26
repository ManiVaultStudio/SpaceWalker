#include "MetadataView.h"

#include <QHBoxLayout>

MetadataView::MetadataView()
{
    _listWidget = new QListWidget();

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(_listWidget);

    setLayout(layout);
}

void MetadataView::addListItem(Dataset<Clusters> dataset)
{
    _listWidget->addItem(dataset->getGuiName());
}
