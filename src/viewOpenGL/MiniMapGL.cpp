/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "MiniMapGL.h"

#include "utils/Utils.h"

#include <QPainter>

#include "GLScope.h"
#include "render/GLElementRender.h"
#include "data/GLElementRectangleFactory.h"

const QRectF MiniMapGL::DEFAULT_BOUNDS =
        QRectF(0.0, 0.0,
        Globals::minimap_height,
        Globals::minimap_width);

MiniMapGL::MiniMapGL(QObject* parent)
    : ViewItemGL(parent),
      m_selecting(false),
      m_bounds(DEFAULT_BOUNDS),
      m_transform(),
      m_scene(),
      m_view()
{
    m_sceneColor = Globals::minimap_scene_color;
    m_viewColor = Globals::minimap_view_color;
}

MiniMapGL::~MiniMapGL()
{

}

void MiniMapGL::setBounds(const QRectF& bounds)
{
    if (m_bounds != bounds) {
        m_bounds = bounds;
    }
}

void MiniMapGL::setScene(const QRectF& scene)
{
    // early out
    if (!scene.isValid()) {
        return;
    }
    
    QRectF scaled = QRectF(m_bounds.topLeft(),
                           scene.size().scaled(m_bounds.size(), Qt::KeepAspectRatio));

    if (m_scene != scaled) {
        m_scene = scaled;
        updateTransform(scene);
        rebuildMinimapData();
    }
}

void MiniMapGL::setView(const QRectF& view)
{
    // early out
    if (!view.isValid()) {
        return;
    }

    const QRectF transformedView = m_transform.mapRect(view);
    if (m_view != transformedView) {
        m_view = transformedView;
        rebuildMinimapData();
    }
}

void MiniMapGL::updateTransform(const QRectF& scene)
{
    // early out
    if (!m_bounds.isValid() || !m_scene.isValid() || !scene.isValid()) {
        // set to identity matrix
        m_transform = QTransform();
        return;
    }
    const QPointF s1 = QPointF(scene.width(), scene.height());
    const QPointF s2 = QPointF(m_scene.width(), m_scene.height());
    const qreal s11 = (s2.x() / s1.x());
    const qreal s22 = (s2.y() / s1.y());
    m_transform =
        QTransform::fromTranslate(-scene.x(), -scene.y()) // align
        * QTransform(s11, 0.0, 0.0, s22, 0.0, 0.0);   // scale
}

void MiniMapGL::render(QPainter* painter)
{
    GL::GLElementRender renderer;
    painter->beginNativePainting();
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        GL::GLscope glBlendScope(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderer.render(m_data, m_queue);
        glPopMatrix();
    }
    painter->endNativePainting();
}

const QRectF MiniMapGL::boundingRect() const
{
    return m_scene;
}

bool MiniMapGL::contains(const QPointF& point) const
{
    return m_scene.contains(point);
}

bool MiniMapGL::mouseMoveEvent(QMouseEvent* event)
{
    // set selecting to false if release event missed
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_selecting = false;
    }

    // move
    if (m_selecting) {
        const QPointF localPoint = event->localPos();
        const QPointF scenePoint = mapToScene(localPoint);
        emit signalCenterOn(scenePoint);
        return true;
    }

    return false;
}

bool MiniMapGL::mousePressEvent(QMouseEvent* event)
{
    // center if left button is pressed down
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_selecting = true;
        const QPointF localPoint = event->localPos();
        const QPointF scenePoint = mapToScene(localPoint);
        emit signalCenterOn(scenePoint);
        return true;
    }
    return false;
}

bool MiniMapGL::mouseReleaseEvent(QMouseEvent* event)
{
    // set selecting to false if released
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_selecting = false;
    }
    // always propagate release events
    return false;
}

const QPointF MiniMapGL::mapToScene(const QPointF& point) const
{
    return m_transform.inverted().map(point);
}

void MiniMapGL::rebuildMinimapData()
{
    // clear rendering data and generate anew
    m_data.clear();
    m_queue.clear();
    generateMinimapData();
}

void MiniMapGL::generateMinimapData()
{
    const GL::GLflag flags =
            GL::GLElementShapeFactory::AutoAddColor |
            GL::GLElementShapeFactory::AutoAddConnection;

    GL::GLElementRectangleFactory factory(m_data, flags);

    // draw scene rectangle
    if (m_scene.isValid()) {

        const QPointF stl = m_scene.topLeft();
        const QPointF str = m_scene.topRight();
        const QPointF sbr = m_scene.bottomRight();
        const QPointF sbl = m_scene.bottomLeft();
        QColor4ub sceneColor = QColor4ub(m_sceneColor);
        sceneColor.setAlphaF(0.2);

        factory.setColor(sceneColor);
        factory.addShape(QRectF(stl, sbr));

        sceneColor.setAlphaF(0.8);
        factory.setColor(sceneColor); //line is 1
        factory.addShape(QRectF(stl, str));
        factory.addShape(QRectF(str, sbr));
        factory.addShape(QRectF(sbr, sbl));
        factory.addShape(QRectF(sbl, stl));
    }

    // draw view rectangle
    if (m_view.isValid()) {

        const QPointF vtl = m_view.topLeft();
        const QPointF vtr = m_view.topRight();
        const QPointF vbr = m_view.bottomRight();
        const QPointF vbl = m_view.bottomLeft();
        QColor4ub viewColor = QColor4ub(m_viewColor);

        viewColor.setAlphaF(0.2);
        factory.setColor(viewColor);
        factory.addShape(QRectF(vtl, vbr));

        viewColor.setAlphaF(0.8);
        factory.setColor(viewColor); //line is 1
        factory.addShape(QRectF(vtl, vtr));
        factory.addShape(QRectF(vtr, vbr));
        factory.addShape(QRectF(vbr, vbl));
        factory.addShape(QRectF(vbl, vtl));

    }

    // generate element data render command
    m_queue.add(GL::GLElementRenderQueue::Command
                (GL::GLElementRenderQueue::Command::RenderItemAll));   // render elements
    m_queue.end();
}