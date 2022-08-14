#pragma once

#include "renderers/Renderer.h"

#include "graphics/BufferObject.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"
#include "graphics/Matrix3f.h"
#include "graphics/Bounds.h"
#include "graphics/Texture.h"

#include <QRectF>

namespace hdps
{
    namespace gui
    {
        class Triangle
        {
        public:
            Vector2f v0, v1, v2;
        };

        enum class ScalarEffect {
            None, Color
        };

        struct CellArrayObject : private QOpenGLFunctions_3_3_Core
        {
        public:
            GLuint _handle;

            BufferObject _triangleBuffer;
            BufferObject _positionBuffer;
            BufferObject _colorScalarBuffer;
            BufferObject _colorBuffer;

            void init();
            void setPositions(const std::vector<Vector2f>& positions);
            void setScalars(const std::vector<float>& scalars);
            void setColors(const std::vector<Vector3f>& colors);

            void enableAttribute(uint index, bool enable);

            bool hasColorScalars() const { return !_colorScalars.empty(); }
            bool hasColors() const { return !_colors.empty(); }

            void computeVoronoiDiagram(const std::vector<Vector2f>& positions);

            Vector3f getColorMapRange() const {
                return _colorScalarsRange;
            }

            void setColorMapRange(const hdps::Vector3f& colorMapRange) {
                _colorScalarsRange = colorMapRange;
            }

            void draw();
            void destroy();

        private:
            /** Vertex array indices */
            const uint ATTRIBUTE_VERTICES           = 0;
            const uint ATTRIBUTE_POSITIONS          = 1;
            const uint ATTRIBUTE_SCALARS_COLOR      = 3;
            const uint ATTRIBUTE_COLORS             = 4;

            /* Point attributes */
            std::vector<Triangle>   _triangles;
            std::vector<Vector2f>   _positions;
            std::vector<Vector3f>   _colors;

            std::vector<int>        _ids;

            /** Scalar channels */
            std::vector<float>  _colorScalars;      /** Point color scalar channel */

            /** Scalar ranges */
            Vector3f    _colorScalarsRange;     /** Scalar range of the point color scalars */

            bool _dirtyVertices         = false;
            bool _dirtyColorScalars     = false;
            bool _dirtyColors           = false;
        };

        class CellRenderer : public Renderer
        {
        public:
            void setData(const std::vector<Vector2f>& points);
            void setColorChannelScalars(const std::vector<float>& scalars);
            void setColors(const std::vector<Vector3f>& colors);

            void setScalarEffect(const ScalarEffect effect);
            void setColormap(const QImage& image);
            void setBounds(const Bounds& bounds);

            void init() override;
            void resize(QSize renderSize) override;
            void render() override;
            void destroy() override;

            void loadShaders();
            void reloadShaders();

            Vector3f getColorMapRange() const;
            void setColorMapRange(const float& min, const float& max);

        private:
            /* Point properties */
            ScalarEffect   _pointEffect = ScalarEffect::None;

            /* Window properties */
            QSize _windowSize;

            /* Rendering variables */
            ShaderProgram _shader;
            bool _needsReload = false;

            CellArrayObject _gpuPoints;
            Texture2D _colormap;

            Matrix3f _orthoM;
            Bounds _bounds = Bounds(-1, 1, -1, 1);

            std::int32_t    _numSelectedPoints;     /** Number of selected (highlighted points) */
        };

    } // namespace gui

} // namespace hdps
