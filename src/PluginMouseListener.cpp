#include "GradientExplorerPlugin.h"

#include "Widgets/MainView.h"

#include "ClusterData/ClusterData.h"

#include <algorithm>
#include <iostream>

namespace
{
    Matrix3f createProjectionToUVMatrix(const Bounds& projectionBounds, const QSizeF& widgetDimensions)
    {
        // Bounds variables
        float left = projectionBounds.getLeft();
        float right = projectionBounds.getRight();
        float bottom = projectionBounds.getBottom();
        float top = projectionBounds.getTop();

        // Widget dimension variables
        const auto w = widgetDimensions.width();
        const auto h = widgetDimensions.height();
        const auto size = w < h ? w : h;

        // Mapping from bounds to [-1, 1]
        Matrix3f ortho;
        ortho[0] = 2.0f / (right - left);
        ortho[4] = 2.0f / (top - bottom);
        ortho[6] = -(right + left) / (right - left);
        ortho[7] = -(top + bottom) / (top - bottom);

        // Flipping the y-axis
        Matrix3f flipMatrix;
        flipMatrix[4] = -1;

        // Scaling and biasing [-1, 1] to [0, 1]
        Matrix3f scale;
        scale[0] = 0.5f;
        scale[4] = 0.5f;

        Matrix3f bias;
        bias[6] = 0.5f;
        bias[7] = 0.5f;

        // Scaling by widget square size
        Matrix3f widgetScale;
        widgetScale[0] = size;
        widgetScale[4] = size;

        // Translating the coordinate system by the difference between the widget dimensions and the square size inside it
        Matrix3f widgetOffset;
        widgetOffset[6] = (w - size) / 2.0f;
        widgetOffset[7] = (h - size) / 2.0f;

        return widgetOffset * widgetScale * bias * scale * flipMatrix * ortho;
    }
}

int findClosestPointToMouse(const DataMatrix& projection, const Bounds& bounds, const QSizeF& widgetDimensions, Vector2f mousePos, std::vector<int> mask)
{
    int closestIndex = 0;
    float minDist = std::numeric_limits<float>::max();

    Matrix3f boundsToWidgetCoords = createProjectionToUVMatrix(bounds, widgetDimensions);

    for (int i = 0; i < mask.size(); i++)
    {
        int maskIndex = mask[i];
        Vector2f pointInProjectionCoords(projection(maskIndex, 0), projection(maskIndex, 1));
        Vector2f pointInWidgetCoords = boundsToWidgetCoords * pointInProjectionCoords;

        // Find closest point to mouse coordinates
        float dist = (pointInWidgetCoords - mousePos).sqrMagnitude();

        if (dist < minDist)
        {
            closestIndex = i;
            minDist = dist;
        }
    }
    return closestIndex;
}

void GradientExplorerPlugin::notifyNewSelectedPoint()
{
    int selectedPoint = _globalSelectedPoint;

    // If we're looking at a subset of the projection, apply subset indirection to selected index
    if (!_positionDataset->isFull())
        selectedPoint = _positionDataset->indices[selectedPoint];

    // Create vector for target selection indices
    std::vector<std::uint32_t> targetSelectionIndices;
    targetSelectionIndices.push_back(selectedPoint);

    // Apply the selection indices
    _positionDataset->setSelectionIndices(targetSelectionIndices);

    // Notify others that the selection changed
    events().notifyDatasetDataSelectionChanged(_positionDataset);
}

void GradientExplorerPlugin::mousePositionChanged(Vector2f mousePos)
{
    mv::Bounds bounds = getUI().getMainView().getBounds();

    // Loop over either all points or only the masked points and establish whether they are selected or not
    std::vector<int> full(_dataStore.getNumPoints());
    std::iota(full.begin(), full.end(), 0);
    QSizeF widgetDimensions(getUI().getMainView().width(), getUI().getMainView().height());
    int selectedPoint = findClosestPointToMouse(_dataStore.getProjectionView(), bounds, widgetDimensions, mousePos, _mask.empty() ? full : _mask);

    // Check if the selected point is the same as the previous, then dont update
    if (selectedPoint == _selectedPoint)
        return;

    _selectedPoint = selectedPoint;
    if (!_dataStore.getViewIndices().empty())
        _globalSelectedPoint = _dataStore.getViewIndices()[_selectedPoint];
    else
        _globalSelectedPoint = _mask.empty() ? _selectedPoint : _mask[_selectedPoint];

    notifyNewSelectedPoint();
}

void GradientExplorerPlugin::onKeyPressed(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        _showingMetadata = true;

        getUI().getMainView().setColoringMode(MainView::PointColoring::METADATA);
    }
}

void GradientExplorerPlugin::onKeyReleased(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        _showingMetadata = false;
        getUI().getMainView().setColoringMode(MainView::PointColoring::FLOODFILL);
    }
}

bool GradientExplorerPlugin::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::Resize:
    {
        const auto resizeEvent = static_cast<QResizeEvent*>(event);

        break;
    }

    // Keyboard input
    case QEvent::KeyPress:
    {
        onKeyPressed(static_cast<QKeyEvent*>(event));
        break;
    }

    case QEvent::KeyRelease:
    {
        onKeyReleased(static_cast<QKeyEvent*>(event));
        break;
    }

    // Mouse input
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        qDebug() << "Mouse button press" << mouseEvent->button();

        _mousePressed = true;

        _selectedViewIndex = 0;
        updateViewScalars();

        break;
    }

    case QEvent::MouseButtonRelease:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        _mousePressed = false;

        break;
    }

    case QEvent::Wheel:
    {
        auto wheelEvent = static_cast<QWheelEvent*>(event);

        if (!_sliceDataset.isValid())
            break;

        int mv = wheelEvent->angleDelta().y() > 0 ? -1 : 1;
        _currentSliceIndex += mv;
        _currentSliceIndex = std::max(std::min(_currentSliceIndex, (int)_sliceDataset->getClusters().size() - 1), 0);

        onSliceIndexChanged();

        break;
    }

    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        if (!_mousePressed)
            break;

        Vector2f mousePos = Vector2f(mouseEvent->position().x(), mouseEvent->position().y());

        if (_positionDataset.isValid())
            mousePositionChanged(mousePos);

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
