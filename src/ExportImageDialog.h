#pragma once

#include "Actions/ExportImageAction.h"

#include <QDialog>

class SpaceWalkerPlugin;

/**
 * Export image dialog class
 *
 * Dialog for exporting to image/video
 *
 * @author Thomas Kroes
 */
class ExportImageDialog : public QDialog
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent widget
     * @param spaceWalkerPlugin Reference to scatterplot plugin
     */
    ExportImageDialog(QWidget* parent, SpaceWalkerPlugin& spaceWalkerPlugin);

    /** Get preferred size */
    QSize sizeHint() const override {
        return QSize(600, 500);
    }

    /** Get minimum size hint*/
    QSize minimumSizeHint() const override {
        return sizeHint();
    }

protected:
    SpaceWalkerPlugin&      _spaceWalkerPlugin;     /** Reference to scatterplot plugin */
    ExportImageAction       _exportImageAction;     /** Export to image action */
};
