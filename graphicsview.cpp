#include "graphicsview.h"
#include <QMouseEvent>


GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton) {
        emit leftPressed(scenePos);   // 🔥 ADD THIS
        emit leftClicked(scenePos);
    }
    else if (event->button() == Qt::RightButton) {
        emit rightClicked(scenePos);
    }

    QGraphicsView::mousePressEvent(event); // ✅ important
}
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved(mapToScene(event->pos()));
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleased();
    QGraphicsView::mouseReleaseEvent(event);
}

