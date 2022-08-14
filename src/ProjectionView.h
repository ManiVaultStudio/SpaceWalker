#pragma once

#include "renderers/PointRenderer.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"
#include "graphics/Bounds.h"

#include <Eigen/Eigen>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

using namespace hdps;
using namespace hdps::gui;

/**
 * Projection view class
 *
 * View for rendering points of a projection in a scatterplot
 *
 * @author Julian Thijssen
 */
class ProjectionView : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    ProjectionView();
    ~ProjectionView();

    /** Returns true when the widget was initialized and is ready to be used. */
    bool isInitialized();

    void setData(const std::vector<Vector2f>* data);

    void setScalars(const std::vector<float>& scalars, int selectedPoint);
    void setScalars(const Eigen::Block<Eigen::MatrixXf, -1, 1, true>& scalars, int selectedPoint);
    //void setColors(const std::vector<Vector3f>& colors);

    void setCurrentPosition(Vector2f pos) { _currentPoint = pos; update(); }
    int getShownDimension() { return _shownDimension; }
    void setShownDimension(int dimension) { _shownDimension = dimension; }

    void setProjectionName(QString name);
    void setSourcePointSize(float sourcePointSize);

    /** Assign a color map image to the point renderer */
    void setColorMap(const QImage& colorMapImage);
    void setColorMapRange(const float& min, const float& max);

    /////
        /**
     * Set point size scalars
     * @param pointSizeScalars Point size scalars
     */
    void setPointSizeScalars(const std::vector<float>& pointSizeScalars);

    /**
     * Set point opacity scalars
     * @param pointOpacityScalars Point opacity scalars (assume the values are normalized)
     */
    void setPointOpacityScalars(const std::vector<float>& pointOpacityScalars);

    void selectView(bool selected)
    {
        _clicked = selected;
        update();
    }

signals:
    void initialized();
    void viewSelected();

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

private:
    PointRenderer   _pointRenderer;
    Bounds          _dataBounds;             /** Bounds of the loaded data */
    QSize           _windowSize;             /** Size of the scatterplot widget */

    QString         _projectionName;

    QImage          _colorMapImage;

    Vector2f        _currentPoint;

    bool            _isInitialized = false;

    bool            _clicked = false;
    int             _shownDimension = 0;
};
