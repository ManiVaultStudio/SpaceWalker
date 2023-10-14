#pragma once

#include "Types.h"

#include <actions/ColorMap1DAction.h>

#include "graphics/Shader.h"
#include "graphics/Vector2f.h"
#include "graphics/Texture.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>

class GraphView : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    GraphView();

    void reset();
    void setTopDimensions(int d1, int d2);
    void setBins(const std::vector<std::vector<int>>& bins);

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;

private:
    bool eventFilter(QObject* target, QEvent* event);

signals:
    void lineClicked(dint dim);

private:
    dint _topDimension1 = -1;
    dint _topDimension2 = -1;
    dint _selectedDimension = -1;

    mv::gui::ColorMap1DAction _colorMapAction;

private: // Rendering stuff
    GLuint _lineVao;
    GLuint _lineVbo;
    GLuint _colorIdVbo;

    std::vector<mv::Vector2f> _lineVertices;
    uint _numLineVertices;
    uint _numVerticesPerLine = 1;
    int _lineHover = -1;
    int _lineSelection = -1;

    QMatrix4x4 _projMatrix;

    mv::Vector2f _xRange;
    mv::Vector2f _yRange;

    mv::ShaderProgram _graphShader;

    mv::Texture2D _colormap;
};
