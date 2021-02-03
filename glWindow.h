#ifndef GLWINDOWS_H
#define GLWINDOWS_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>

#include "meshobject.h"

class SquircleRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

    MeshObject mesh;
public:
    double horizon_angle = 1.57, vertical_angle = 0;
    double eyeDistance = 3;

    QMatrix4x4 model;

    void RotateModel(double dx, double dy);
    void ResetToCenter(double x, double y, double z){
        model.setToIdentity();
        model.translate(-x,-y,-z);
    }

public:
    SquircleRenderer() : m_t(0), m_program(0) {
        model.setToIdentity();
    }
    ~SquircleRenderer();

    void setT(qreal t) { m_t = t; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }



    int nbSegment = 10;
    void drawTest();

public slots:
    void paint();

private:
    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;

    QOpenGLShaderProgram * m_gouraud_shader;


    QQuickWindow *m_window;
};

class GLWindow : public QQuickItem
{


    Q_OBJECT
    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)


private:
    QPoint mouse_pre_pos;

//    void touchEvent(QTouchEvent *ev) override;

//    void mousePressEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    void mouseReleaseEvent(QMouseEvent *event) override;

//    void wheelEvent(QWheelEvent *event) override;

public:
    GLWindow();

    qreal t() const { return m_t; }
    void setT(qreal t);

    Q_PROPERTY(QString fr MEMBER frameRate NOTIFY tChanged)
    QString frameRate;

     Q_INVOKABLE void doubleRes(){m_renderer->nbSegment *= 2;}
signals:
    void tChanged();
    void UIChanged();

public slots:
    void sync();
    void cleanup();


private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    qreal m_t;
    SquircleRenderer *m_renderer;
};

#endif // GLWINDOWS_H
