#include "ProjectionView.h"

#include "util/Exception.h"

#include <vector>
#include <algorithm>

#include <QSize>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QWindow>

constexpr float MIN_POINT_SIZE = 0.5f;

namespace
{
    Bounds getDataBounds(const std::vector<Vector2f>& points)
    {
        Bounds bounds = Bounds::Max;

        for (const Vector2f& point : points)
        {
            bounds.setLeft(std::min(point.x, bounds.getLeft()));
            bounds.setRight(std::max(point.x, bounds.getRight()));
            bounds.setBottom(std::min(point.y, bounds.getBottom()));
            bounds.setTop(std::max(point.y, bounds.getTop()));
        }

        return bounds;
    }
}

ProjectionView::ProjectionView() :
    _pointRenderer(),
    _pixelRatio(1.0)
{
    //setContextMenuPolicy(Qt::CustomContextMenu);
    //setAcceptDrops(true);
    //setMouseTracking(true);
    //setFocusPolicy(Qt::ClickFocus);
    //setMinimumHeight(200);

    _pointRenderer.setPointScaling(Relative);
    _pointRenderer.setPointSize(1.5f);

    QSurfaceFormat surfaceFormat;

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);

#ifdef __APPLE__
    // Ask for an OpenGL 3.3 Core Context as the default
    surfaceFormat.setVersion(3, 3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    //QSurfaceFormat::setDefaultFormat(defaultFormat);
#else
    // Ask for an OpenGL 4.3 Core Context as the default
    surfaceFormat.setVersion(4, 3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
#endif

#ifdef _DEBUG
    surfaceFormat.setOption(QSurfaceFormat::DebugContext);
#endif

    surfaceFormat.setSamples(16);

    setFormat(surfaceFormat);

    // we connect screenChanged to updating the pixel ratio
    // this is necessary in case the window is moved between hi and low dpi screens
    // e.g., from a laptop display to a projector
    winId(); // This is needed to produce a valid windowHandle
    QObject::connect(windowHandle(), &QWindow::screenChanged, this, &ProjectionView::updatePixelRatio);

    setFocusPolicy(Qt::ClickFocus);
    installEventFilter(this);
}

ProjectionView::~ProjectionView()
{
    disconnect(QOpenGLWidget::context(), &QOpenGLContext::aboutToBeDestroyed, this, &ProjectionView::cleanup);
    cleanup();
}

bool ProjectionView::isInitialized()
{
    return _isInitialized;
}

void ProjectionView::setData(const std::vector<Vector2f>* points)
{
    auto dataBounds = getDataBounds(*points);

    dataBounds.ensureMinimumSize(1e-07f, 1e-07f);
    dataBounds.makeSquare();
    dataBounds.expand(0.1f);

    _dataBounds = dataBounds;

    // Pass bounds and data to renderer
    _pointRenderer.setBounds(_dataBounds);
    _pointRenderer.setData(*points);

    _pointRenderer.setSelectionOutlineColor(Vector3f(1, 0, 0));
    _pointRenderer.setAlpha(1.0f);
    //_pointRenderer.setPointScaling(PointScaling::Relative);

    update();
}

void ProjectionView::setScalars(const std::vector<float>& scalars, int selectedPoint)
{
    _pointRenderer.setColorChannelScalars(scalars);
    _pointRenderer.setScalarEffect(PointEffect::Color);
    
    update();
}

void ProjectionView::setScalars(const Eigen::Block<Eigen::MatrixXf, -1, 1, true>& scalars, int selectedPoint)
{
    float minV = *std::min_element(scalars.begin(), scalars.end());
    float maxV = *std::max_element(scalars.begin(), scalars.end());

    std::vector<Vector3f> colors(scalars.size());
    for (int i = 0; i < scalars.size(); i++)
    {
        float dimValue = scalars[i] / (maxV - minV);

        colors[i] = (i == selectedPoint) ? Vector3f(1, 0, 0) : Vector3f(1 - dimValue, 1 - dimValue, 1 - dimValue);
    }

    _pointRenderer.setColors(colors);
    _pointRenderer.setScalarEffect(PointEffect::Color);

    update();
}

//void ProjectionView::setColors(const std::vector<Vector3f>& colors)
//{
//    _pointRenderer.setColors(colors);
//    _pointRenderer.setScalarEffect(PointEffect::None);
//
//    update();
//}

void ProjectionView::setProjectionName(QString name)
{
    _projectionName = name;
}

void ProjectionView::setSourcePointSize(float sourcePointSize)
{
    _pointRenderer.setPointSize(std::max(sourcePointSize / 3.5f, MIN_POINT_SIZE));
}

void ProjectionView::setPointSizeScalars(const std::vector<float>& pointSizeScalars)
{
    _pointRenderer.setSizeChannelScalars(pointSizeScalars);
    _pointRenderer.setPointSize(*std::max_element(pointSizeScalars.begin(), pointSizeScalars.end()));

    update();
}

void ProjectionView::setPointOpacityScalars(const std::vector<float>& pointOpacityScalars)
{
    _pointRenderer.setOpacityChannelScalars(pointOpacityScalars);

    update();
}

void ProjectionView::initializeGL()
{
    initializeOpenGLFunctions();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ProjectionView::cleanup);

    // Initialize renderers
    _pointRenderer.init();

    // Set a default color map for both renderers
    _pointRenderer.setScalarEffect(PointEffect::Color);

    // OpenGL is initialized
    _isInitialized = true;

    emit initialized();
}

void ProjectionView::resizeGL(int w, int h)
{
    // we need this here as we do not have the screen yet to get the actual devicePixelRatio when the view is created
    _pixelRatio = devicePixelRatio();

    // Pixelration tells us how many pixels map to a point
    // That is needed as macOS calculates in points and we do in pixels
    // On macOS high dpi displays pixel ration is 2
    w *= _pixelRatio;
    h *= _pixelRatio;

    _windowSize.setWidth(w);
    _windowSize.setHeight(h);

    _pointRenderer.resize(QSize(w, h));

    // Set matrix for normalizing from pixel coordinates to [0, 1]
    //toNormalisedCoordinates = Matrix3f(1.0f / w, 0, 0, 1.0f / h, 0, 0);

    // Take the smallest dimensions in order to calculate the aspect ratio
    int size = w < h ? w : h;

    float wAspect = (float)w / size;
    float hAspect = (float)h / size;
    float wDiff = ((wAspect - 1) / 2.0);
    float hDiff = ((hAspect - 1) / 2.0);

    //toIsotropicCoordinates = Matrix3f(wAspect, 0, 0, hAspect, -wDiff, -hDiff);
}

void ProjectionView::updatePixelRatio()
{
    float pixelRatio = devicePixelRatio();

#ifdef SCATTER_PLOT_WIDGET_VERBOSE
    qDebug() << "Window moved to screen " << window()->screen() << ".";
    qDebug() << "Pixelratio before was " << _pixelRatio << ". New pixelratio is: " << pixelRatio << ".";
#endif // SCATTER_PLOT_WIDGET_VERBOSE

    // we only update if the ratio actually changed
    if (_pixelRatio != pixelRatio)
    {
        _pixelRatio = pixelRatio;
        resizeGL(width(), height());
        update();
    }
}

namespace {
    Matrix3f createProjectionMatrix(Bounds bounds)
    {
        Matrix3f m;
        m.setIdentity();
        m[0] = 2 / bounds.getWidth();
        m[4] = 2 / bounds.getHeight();
        m[6] = -((bounds.getRight() + bounds.getLeft()) / bounds.getWidth());
        m[7] = -((bounds.getTop() + bounds.getBottom()) / bounds.getHeight());
        return m;
    }
}


void ProjectionView::paintGL()
{
    try {
        QPainter painter;

        // Begin mixed OpenGL/native painting
        if (!painter.begin(this))
            throw std::runtime_error("Unable to begin painting");

        // Draw layers with OpenGL
        painter.beginNativePainting();
        {
            // Bind the framebuffer belonging to the widget
            //glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

            // Clear the widget to the background color
            glClearColor(0.1f, 0.1f, 0.1f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Reset the blending function
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            _pointRenderer.render();
        }
        painter.endNativePainting();

        QFont font = painter.font();
        font.setPointSize(font.pointSize() * 2);
        painter.setFont(font);
        painter.setPen(QPen(Qt::white));
        painter.drawText(12, 30, "Dim: " + _projectionName);

        Matrix3f orthoM = createProjectionMatrix(_dataBounds);

        int size = std::min(width(), height());
        Matrix3f toScreen;
        toScreen.setIdentity();
        toScreen[0] = size / 2;
        toScreen[4] = size / 2;
        toScreen[6] = width() / 2;
        toScreen[7] = height() / 2;

        Matrix3f invM;
        invM.setIdentity();
        invM[0] = 1;
        invM[4] = -1;

        Vector2f cp = toScreen * invM * orthoM * _currentPoint;

        // Render selection dot
        painter.setBrush(Qt::red);
        painter.drawEllipse(QPointF(cp.x, cp.y), 5, 5);
        painter.setBrush(Qt::BrushStyle::NoBrush);

        // Render clicked frame
        if (_clicked)
        {
            QPen pen(Qt::green);
            pen.setWidth(10);
            painter.setPen(pen);
            painter.drawRect(0, 0, width(), height());
        }

        painter.end();
    }
    catch (std::exception& e)
    {
        mv::util::exceptionMessageBox("Rendering failed", e);
    }
    catch (...) {
        mv::util::exceptionMessageBox("Rendering failed");
    }
}

void ProjectionView::setColorMap(const QImage& colorMapImage)
{
    _colorMapImage = colorMapImage;

    // Do not update color maps of the renderers when OpenGL is not initialized
    if (!_isInitialized)
        return;

    // Activate OpenGL context
    makeCurrent();

    // Apply color maps to renderers
    _pointRenderer.setColormap(_colorMapImage);

    // Render
    update();
}

void ProjectionView::setColorMapRange(const float& min, const float& max)
{
    _pointRenderer.setColorMapRange(min, max);

    update();
}

void ProjectionView::cleanup()
{
    qDebug() << "Deleting projection view, performing clean up...";
    _isInitialized = false;

    makeCurrent();
    _pointRenderer.destroy();
}

bool ProjectionView::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        qDebug() << "Mouse button press";
        _clicked = true;

        emit viewSelected();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
