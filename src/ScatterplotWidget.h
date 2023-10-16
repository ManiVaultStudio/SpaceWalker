#pragma once

#include "renderers/PointRenderer.h"
#include "CellRenderer.h"
#include "renderers/DensityRenderer.h"
#include "util/PixelSelectionTool.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"
#include "graphics/Matrix3f.h"
#include "graphics/Bounds.h"
#include "graphics/Selection.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

#include <QMouseEvent>
#include <QMenu>

using namespace mv;
using namespace mv::gui;
using namespace mv::util;

class ScatterplotWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    enum RenderMode {
        SCATTERPLOT,
        DENSITY,
        LANDSCAPE,
        CELL
    };

    /** The way that point colors are determined */
    enum class ColoringMode {
        Constant,      /** Point color is a constant color */
        Data                /** Determined by external dataset */
    };

public:
    ScatterplotWidget();
    ~ScatterplotWidget();

    /** Returns true when the widget was initialized and is ready to be used. */
    bool isInitialized();

    /** Get/set render mode */
    RenderMode getRenderMode() const;
    void setRenderMode(const RenderMode& renderMode);

    /** Get/set background color */
    QColor getBackgroundColor();
    void setBackgroundColor(QColor color);

    /** Get/set coloring mode */
    //ColoringMode getColoringMode() const;
    //void setColoringMode(const ColoringMode& coloringMode);

    ///** Get reference to the pixel selection tool */
    //PixelSelectionTool& getPixelSelectionTool();

    /**
     * Feed 2-dimensional data to the scatterplot.
     */
    void setData(const std::vector<Vector2f>* data);
    void setHighlights(const std::vector<char>& highlights, const std::int32_t& numSelectedPoints);
    void setScalars(const std::vector<float>& scalars);

    /**
     * Set colors for each individual data point
     * @param colors Vector of colors (size must match that of the loaded points dataset)
     */
    void setColors(const std::vector<Vector3f>& colors);

    void setPointSize(float pointSize);

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

    void setScalarEffect(PointEffect effect);
    void setPointScaling(mv::gui::PointScaling scalingMode);

    void setCurrentPosition(Vector2f pos) { _currentPoint = pos; update(); }
    void setFilterRadii(Vector2f radii) { _radii = radii; update(); }
    void showFiltersCircles(bool show) { _showFilterCircles = show; }

    /**
     * Set sigma value for kernel density esitmation.
     * @param sigma kernel width as a fraction of the output square width. Typical values are [0.01 .. 0.5]
     */
    void setSigma(const float sigma);

    Bounds getBounds() const {
        return _dataBounds;
    }

    Vector3f getColorMapRange() const;
    void setColorMapRange(const float& min, const float& max);

    /**
     * Create screenshot
     * @param width Width of the screen shot (in pixels)
     * @param height Height of the screen shot (in pixels)
     * @param backgroundColor Background color of the screen shot
     */
    void createScreenshot(std::int32_t width, std::int32_t height, const QString& fileName, const QColor& backgroundColor);

    void setRandomWalks(const std::vector<std::vector<Vector2f>>& randomWalks);
    void setProjectionName(QString name) { _projectionName = name; }
    void setColoredBy(QString coloredBy) { _coloredBy = coloredBy; }
    void setClusterName(QString clusterName) { _clusterName = clusterName; }
    void setDirections(const std::vector<Vector2f>& directions);

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();
    
public: // Const access to renderers

    const PointRenderer& getPointRenderer() const { 
        return _pointRenderer;
    }

    const DensityRenderer& getDensityRenderer() const {
        return _densityRenderer;
    }

public:

    /** Assign a color map image to the point and density renderers */
    void setColorMap(const QImage& colorMapImage);

signals:
    void initialized();

    /**
     * Signals that the render mode changed
     * @param renderMode Signals that the render mode has changed
     */
    void renderModeChanged(const RenderMode& renderMode);

    /**
     * Signals that the coloring mode changed
     * @param coloringMode Signals that the coloring mode has changed
     */
    //void coloringModeChanged(const ColoringMode& coloringMode);

    /** Signals that the density computation has started */
    void densityComputationStarted();

    /** Signals that the density computation has ended */
    void densityComputationEnded();

public slots:
    void computeDensity();

    void showRandomWalk()
    {
        _showRandomWalk = !_showRandomWalk;
        update();
    }

    void showDirections(bool show)
    {
        _showDirections = show;
        update();
    }

private:
    const Matrix3f          toClipCoordinates = Matrix3f(2, 0, 0, 2, -1, -1);
    Matrix3f                toNormalisedCoordinates;
    Matrix3f                toIsotropicCoordinates;
    bool                    _isInitialized = false;
    RenderMode              _renderMode = SCATTERPLOT;
    QColor                  _backgroundColor;
    //ColoringMode            _coloringMode = ColoringMode::Constant;
    PointRenderer           _pointRenderer;
    CellRenderer            _cellRenderer;
    DensityRenderer         _densityRenderer;
    QSize                   _windowSize;                        /** Size of the scatterplot widget */
    Bounds                  _dataBounds;                        /** Bounds of the loaded data */
    QImage                  _colorMapImage;
    //PixelSelectionTool      _pixelSelectionTool;
    std::vector<std::vector<Vector2f>> _randomWalks;
    std::vector<Vector2f>   _directions;
    bool                    _showRandomWalk;
    bool                    _showDirections;
    bool                    _showFilterCircles = true;

    QString                 _projectionName;
    QString                 _coloredBy;
    QString                 _clusterName;

    Vector2f                _currentPoint;
    Vector2f                _radii;
};
