#ifndef GLWINDOWS_H
#define GLWINDOWS_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>

#include "meshobject.h"

class SquircleRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
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
    int instanceC;
    SquircleRenderer() : m_t(0), m_program(0) {
        model.setToIdentity();

        static int count = 0;
        instanceC = count++;
    }
    ~SquircleRenderer();

    void setT(qreal t) { m_t = t; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }




    int nbSegment = 10;
    void drawTest();

    void reduceMesh(float ratio);

public slots:
    void paint();


private:
    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;

    QOpenGLShaderProgram * m_gouraud_shader;


    QQuickWindow *m_window;

    void drawCoordinate(bool isOne = false);
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

    Q_PROPERTY(QString meshInfo READ meshInfo() NOTIFY UIChanged)
    QString meshInfo();

    Q_PROPERTY(int appState READ getAppState() NOTIFY UIChanged)
    int getAppState();

    Q_PROPERTY(QString progress MEMBER progressString NOTIFY UIChanged)
    QString progressString;

    Q_PROPERTY(QString cutline MEMBER cutline NOTIFY UIChanged)
    QString cutline;

    Q_PROPERTY(bool isAssistView READ isAssistCutView WRITE setAssistView NOTIFY UIChanged)
    bool isAssistCutView();
    void setAssistView(bool isAssist);

    Q_INVOKABLE int modelHeight(){return meshInstance().modelHeight();}
    Q_INVOKABLE int getMainAxis();

    Q_INVOKABLE void reduceMesh(float ratio);
    Q_INVOKABLE void setModelHeight(float h);
    Q_INVOKABLE void setMainAxis(int ax);

    Q_INVOKABLE void start();

    Q_INVOKABLE void changeCutMode();
    Q_PROPERTY(bool isCutting2faces READ isCutting2faces NOTIFY UIChanged)
    bool isCutting2faces();

public:
#define idxStr(v) \
    Q_PROPERTY(QString v MEMBER v NOTIFY UIChanged) \
    QString v; \
    Q_PROPERTY(int x_##v MEMBER x_##v NOTIFY UIChanged)\
    Q_PROPERTY(int y_##v MEMBER y_##v NOTIFY UIChanged)\
    int x_##v, y_##v;

    idxStr(t1)
    idxStr(t2)
    idxStr(n11)
    idxStr(n12)
    idxStr(n21)
    idxStr(n22)

    idxStr(f)
    idxStr(f0)
    idxStr(f1)
    idxStr(f2)

    Q_INVOKABLE void pre();
    Q_INVOKABLE void next();
    void updateCoordInfo();

    QSize windowSize;

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
