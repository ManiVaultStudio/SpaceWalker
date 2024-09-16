#include "GraphView.h"

#include "Timer.h"

using namespace mv;

namespace
{
    QColor grey = QColor(128, 128, 128, 50);
    QColor selected = QColor(255, 0, 0, 255);
    QColor primary = QColor(255, 128, 0, 255);
    QColor secondary = QColor(255, 128, 0, 128);
}

GraphView::GraphView() :
    _lineVao(0),
    _lineVbo(0),
    _colorIdVbo(0),
    _numLineVertices(0),
    _colorMapAction(this, "Color map", "RdYlBu")
{
    installEventFilter(this);
    setMouseTracking(true);
}

void GraphView::reset()
{

}

void GraphView::setTopDimensions(int d1, int d2)
{
    _topDimension1 = d1;
    _topDimension2 = d2;
}

void GraphView::setBins(const std::vector<std::vector<int>>& bins)
{
    SpaceWalker::Timer timer;
    timer.start();

    Q_ASSERT(!bins.empty());
    Q_ASSERT(!bins[0].empty());

    int numDimensions = bins.size();
    int numSteps = bins[0].size();

    _lineVertices.clear(); // Clear the list to avoid copying values
    _lineVertices.resize(bins.size() * numSteps * 2); // Resize it to accodomate the two vertices of each line segment

    _numLineVertices = 0;
    int binTotal = 0;

    for (int d = 0; d < bins.size(); d++)
    {
        binTotal = 0;

        Vector2f currentPoint(0, 0);
        for (int i = 0; i < bins[d].size(); i++)
        {
            binTotal += bins[d][i];

            _lineVertices[_numLineVertices++] = currentPoint;
            currentPoint.set(binTotal, i / 30.0f);
            _lineVertices[_numLineVertices++] = currentPoint;
        }
    }

    _xRange.set(0, binTotal);
    _yRange.set(0, 1);
    _numVerticesPerLine = bins[0].size() * 2;

    //// Update axes to proper values
    //_xAxis->setRange(0, binTotal);
    //_yAxis->setRange(0, 1);

    //for (int d = 0; d < bins.size(); d++)
    //    _seriesArray[d]->replace(pointLists[d]);

    //updateChartColors();
    update();

    timer.finish("Graph inner");
}

void GraphView::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    glLineWidth(2);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders
    bool loaded = true;
    loaded &= _graphShader.loadShaderFromFile(":shaders/Graph.vert", ":shaders/Graph.frag");
    
    if (!loaded) {
        qCritical() << "Failed to load graph shader.";
    }

    // Upload colormap
    QImage image = _colorMapAction.getColorMapImage().mirrored(false, true);
    _colormap.loadFromImage(image);

    // Set up line VAO and VBO
    glGenVertexArrays(1, &_lineVao);
    glBindVertexArray(_lineVao);

    glGenBuffers(1, &_lineVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _lineVbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &_colorIdVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _lineVbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Build orthographic matrix
    float left = -0.1;
    float right = 1.1;
    float bottom = -0.1;
    float top = 1.1;

    _projMatrix.setToIdentity();
    _projMatrix.data()[0] = 2.0f / (right - left);
    _projMatrix.data()[5] = 2.0f / (top - bottom);
    _projMatrix.data()[12] = -(right + left) / (right - left);
    _projMatrix.data()[13] = -(top + bottom) / (top - bottom);

    _graphShader.bind();
    _graphShader.uniform4f("mainColor", grey.redF(), grey.greenF(), grey.blueF(), grey.alphaF());
    _graphShader.uniform4f("selectedColor", selected.redF(), selected.greenF(), selected.blueF(), selected.alphaF());
    _graphShader.uniform4f("primaryColor", primary.redF(), primary.greenF(), primary.blueF(), primary.alphaF());
    _graphShader.uniform4f("secondaryColor", secondary.redF(), secondary.greenF(), secondary.blueF(), secondary.alphaF());
}

void GraphView::resizeGL(int w, int h)
{

}

void GraphView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    _graphShader.bind();

    _graphShader.uniformMatrix4f("projMatrix", _projMatrix.data());
    _graphShader.uniform2f("xRange", _xRange.x, _xRange.y);
    _graphShader.uniform2f("yRange", _yRange.x, _yRange.y);
    _graphShader.uniform1i("numVerticesPerLine", _numVerticesPerLine);
    _graphShader.uniform1i("primaryLine", _topDimension1);
    _graphShader.uniform1i("secondaryLine", _topDimension2);
    _graphShader.uniform1i("lineHover", _lineHover);
    _graphShader.uniform1i("lineSelection", _lineSelection);

    _colormap.bind(0);
    _graphShader.uniform1i("colormap", 0);

    glBindVertexArray(_lineVao);

    glBindBuffer(GL_ARRAY_BUFFER, _lineVbo);
    glBufferData(GL_ARRAY_BUFFER, _lineVertices.size() * sizeof(Vector2f), _lineVertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, _numLineVertices);
}

bool GraphView::eventFilter(QObject* target, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        if (_lineHover != -1)
        {
            _lineSelection = _lineHover;
            emit lineClicked(_lineSelection);
            update();
        }

        break;
    }
    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        Vector2f mousePos = Vector2f(mouseEvent->position().x(), mouseEvent->position().y());
        
        Vector2f uvPos(mousePos.x / width(), 1 - (mousePos.y / height()));
        uvPos = uvPos * 1.2f - 0.1f;

        Vector2f tPos((uvPos.x + _xRange.x) * _xRange.y, (uvPos.y + _yRange.x) * _yRange.y);

        float minDist = std::numeric_limits<float>::max();

        for (int i = 0; i < _lineVertices.size(); i += 2)
        {
            Vector2f& v1 = _lineVertices[i+0];
            Vector2f& v2 = _lineVertices[i+1];
            
            if (tPos.x > v1.x && tPos.x < v2.x && tPos.y > v1.y && tPos.y < v2.y)
            {
                Vector2f dir = v2 - v1;
                Vector2f v = (tPos - v1);
                Vector2f vProj = v1 + dir * dot(v, normalize(dir));
                float dist = (tPos - vProj).sqrMagnitude();

                if (dist < minDist)
                {
                    _lineHover = i / _numVerticesPerLine;
                    minDist = dist;
                    update();
                }
            }
        }

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
