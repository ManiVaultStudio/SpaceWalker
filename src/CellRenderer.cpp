#include "CellRenderer.h"

#include <limits>
#define JC_VORONOI_IMPLEMENTATION
#include <jc_voronoi.h>

#include <QDebug>

namespace mv
{
    namespace gui
    {
        namespace
        {
            /**
             * Builds an orthographic projection matrix that transforms the given bounds
             * to the range [-1, 1] in both directions.
             */
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

        void CellArrayObject::init()
        {
            initializeOpenGLFunctions();

            // Generate a VAO for all instanced points
            glGenVertexArrays(1, &_handle);
            glBindVertexArray(_handle);

            // Vertex buffer
            _triangleBuffer.create();
            _triangleBuffer.bind();
            //_triangleBuffer.setData(std::vector<Triangle>());
            glVertexAttribPointer(ATTRIBUTE_VERTICES, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(ATTRIBUTE_VERTICES);

            // Position buffer
            _positionBuffer.create();
            _positionBuffer.bind();
            glVertexAttribPointer(ATTRIBUTE_POSITIONS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(ATTRIBUTE_POSITIONS);

            // Color buffer, disabled by default
            _colorBuffer.create();
            _colorBuffer.bind();

            glVertexAttribPointer(ATTRIBUTE_COLORS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glVertexAttribDivisor(ATTRIBUTE_COLORS, 1);

            // Scalar buffer for point color, disabled by default
            _colorScalarBuffer.create();
            _colorScalarBuffer.bind();

            glVertexAttribPointer(ATTRIBUTE_SCALARS_COLOR, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        //bool pointOnClipper(jcv_point* min, jcv_point* max, jcv_point* p)
        //{
        //    if (p->x == min->x)
        //}

        float computeArea(jcv_point v0, jcv_point v1, jcv_point v2)
        {
            return 0.5 * (v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y));
        }

        void CellArrayObject::computeVoronoiDiagram(const std::vector<Vector2f>& positions)
        {
            jcv_diagram diagram;
            memset(&diagram, 0, sizeof(jcv_diagram));
            jcv_diagram_generate((int) positions.size(), (jcv_point*) positions.data(), 0, 0, &diagram);

            const jcv_site* sites = jcv_diagram_get_sites(&diagram);
            for (int i = 0; i < diagram.numsites; ++i)
            {
                const jcv_site* site = &sites[i];
                
                const jcv_graphedge* e = site->edges;
                while (e)
                {
                    //float area = computeArea(site->p, e->pos[0], e->pos[1]);
                    
                    //if (fabs(site->p.x - e->pos[0].x) < 1.1f && fabs(site->p.y - e->pos[0].y) < 1.1f && fabs(site->p.x - e->pos[1].x) < 1.1f && fabs(site->p.y - e->pos[1].y) < 1.1f && fabs(e->pos[0].x - e->pos[1].x) < 1.1f && fabs(e->pos[0].y - e->pos[1].y) < 1.1f)
                    //{
                        _triangles.push_back(Triangle{ Vector2f(site->p.x, site->p.y), Vector2f(e->pos[0].x, e->pos[0].y), Vector2f(e->pos[1].x, e->pos[1].y) });
                        _ids.push_back(site->index);
                    //}
                    e = e->next;
                }
            }

            jcv_diagram_free(&diagram);
        }

        void CellArrayObject::setPositions(const std::vector<Vector2f>& positions)
        {
            _triangles.clear();
            _ids.clear();
            computeVoronoiDiagram(positions);

            _positions.resize(_triangles.size() * 3);
            for (int i = 0; i < _triangles.size(); i++)
            {
                int id = _ids[i];
                _positions[i * 3 + 0] = positions[id];
                _positions[i * 3 + 1] = positions[id];
                _positions[i * 3 + 2] = positions[id];
            }

            _dirtyVertices = true;
        }

        void CellArrayObject::setScalars(const std::vector<float>& scalars)
        {
            _colorScalarsRange.x = std::numeric_limits<float>::max();
            _colorScalarsRange.y = -std::numeric_limits<float>::max();

            // Determine scalar range
            for (const float& scalar : scalars)
            {
                if (scalar < _colorScalarsRange.x)
                    _colorScalarsRange.x = scalar;

                if (scalar > _colorScalarsRange.y)
                    _colorScalarsRange.y = scalar;
            }

            _colorScalarsRange.z = _colorScalarsRange.y - _colorScalarsRange.x;

            if (_colorScalarsRange.z < 1e-07f)
                _colorScalarsRange.z = 1e-07f;

            _colorScalars.resize(_triangles.size() * 3);
            for (int i = 0; i < _triangles.size(); i++)
            {
                int id = _ids[i];
                _colorScalars[i * 3 + 0] = scalars[id];
                _colorScalars[i * 3 + 1] = scalars[id];
                _colorScalars[i * 3 + 2] = scalars[id];
            }

            _dirtyColorScalars = true;
        }

        void CellArrayObject::setColors(const std::vector<Vector3f>& colors)
        {
            _colors = colors;

            _dirtyColors = true;
        }

        void CellArrayObject::enableAttribute(uint index, bool enable)
        {
            glBindVertexArray(_handle);
            if (enable)
                glEnableVertexAttribArray(index);
            else
                glDisableVertexAttribArray(index);
        }

        void CellArrayObject::draw()
        {
            glBindVertexArray(_handle);

            if (_dirtyVertices)
            {
                _triangleBuffer.bind();
                _triangleBuffer.setData(_triangles);
                _positionBuffer.bind();
                _positionBuffer.setData(_positions);

                _dirtyVertices = false;
            }

            if (_dirtyColors)
            {
                _colorBuffer.bind();
                _colorBuffer.setData(_colors);
                enableAttribute(ATTRIBUTE_COLORS, true);

                _dirtyColors = false;
            }

            if (_dirtyColorScalars)
            {
                _colorScalarBuffer.bind();
                _colorScalarBuffer.setData(_colorScalars);
                //for (int i = 0; i < _colorScalars.size(); i++)
                //{
                //    qDebug() << _colorScalars[i];
                //}

                enableAttribute(ATTRIBUTE_SCALARS_COLOR, true);
                qDebug() << "Uploading scalars";

                _dirtyColorScalars = false;
            }

            if (!_triangles.empty())
            {
                glDrawArrays(GL_TRIANGLES, 0, (int) _triangles.size() * 3);
            }
            glBindVertexArray(0);
        }

        void CellArrayObject::destroy()
        {
            glDeleteVertexArrays(1, &_handle);
            _triangleBuffer.destroy();
        }

        void CellRenderer::setData(const std::vector<Vector2f>& positions)
        {
            _gpuPoints.setPositions(positions);
        }

        void CellRenderer::setColorChannelScalars(const std::vector<float>& scalars)
        {
            _gpuPoints.setScalars(scalars);
        }

        void CellRenderer::setColors(const std::vector<Vector3f>& colors)
        {
            _gpuPoints.setColors(colors);
        }

        void CellRenderer::setScalarEffect(const ScalarEffect effect)
        {
            _pointEffect = effect;
        }

        void CellRenderer::setColormap(const QImage& image)
        {
            _colormap.loadFromImage(image);
        }

        void CellRenderer::setBounds(const Bounds& bounds)
        {
            _bounds = bounds;
        }

        void CellRenderer::loadShaders()
        {
            bool loaded = true;
            loaded &= _shader.loadShaderFromFile(":shaders/TrianglePlot.vert", ":shaders/TrianglePlot.frag");

            if (!loaded) {
                qCritical() << "Failed to load one of the Scatterplot shaders";
            }
            else
            {
                qDebug() << "Loaded shaders";
            }
        }

        void CellRenderer::reloadShaders()
        {
            _needsReload = true;
        }

        void CellRenderer::init()
        {
            initializeOpenGLFunctions();

            _gpuPoints.init();

            loadShaders();
        }

        void CellRenderer::resize(QSize renderSize)
        {
            int w = renderSize.width();
            int h = renderSize.height();

            _windowSize.setWidth(w);
            _windowSize.setHeight(h);
        }

        void CellRenderer::render()
        {
            if (_needsReload)
            {
                loadShaders();
                _needsReload = false;
            }

            int w = _windowSize.width();
            int h = _windowSize.height();
            int size = w < h ? w : h;
            glViewport(w / 2 - size / 2, h / 2 - size / 2, size, size);

            // World to clip transformation
            _orthoM = createProjectionMatrix(_bounds);

            _shader.bind();

            // Point size uniforms
            _shader.uniformMatrix3f("orthoM", _orthoM);
            qDebug() << _gpuPoints.hasColorScalars();
            _shader.uniform1i("hasScalars", _gpuPoints.hasColorScalars());
            _shader.uniform1i("hasColors", _gpuPoints.hasColors());
            _shader.uniform1i("numSelectedPoints", _numSelectedPoints);
            _shader.uniform1i("scalarEffect", 1);

            if (_gpuPoints.hasColorScalars())
                _shader.uniform3f("colorMapRange", _gpuPoints.getColorMapRange());

            if (_pointEffect == ScalarEffect::Color) {
                _colormap.bind(0);
                _shader.uniform1i("colormap", 0);
                qDebug() << "Colormap";
            }

            _gpuPoints.draw();
        }

        void CellRenderer::destroy()
        {
            _gpuPoints.destroy();
        }

        mv::Vector3f CellRenderer::getColorMapRange() const
        {
            return _gpuPoints.getColorMapRange();
        }

        void CellRenderer::setColorMapRange(const float& min, const float& max)
        {
            return _gpuPoints.setColorMapRange(Vector3f(min, max, max - min));
        }

    } // namespace gui

} // namespace mv
