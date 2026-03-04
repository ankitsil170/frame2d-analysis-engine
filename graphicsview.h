#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent = nullptr);

signals:
    void mouseClicked(const QPointF& scenePos);
    void leftClicked(const QPointF& scenePos);
    void rightClicked(const QPointF& scenePos);
    void leftPressed(const QPointF& scenePos);
    void mouseMoved(const QPointF& scenePos);
    void mouseReleased();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif
