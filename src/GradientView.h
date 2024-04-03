#pragma once

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

using namespace mv;

/**
 * Projection view class
 *
 * 
 *
 * @author Julian Thijssen
 */
class GradientView : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    GradientView();
    ~GradientView();

    void setData(const std::vector<Vector2f>& data);

    void setColors(const std::vector<Vector3f>& colors);
};
