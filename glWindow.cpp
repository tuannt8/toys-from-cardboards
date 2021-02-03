#include "glWindow.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

#include "glhelper.h"

#include <vector>


#include <QElapsedTimer>

extern int appState;

using namespace std;

#define POINT_TEST
#define NBPOINTS 1e5

//BoxBC b;
//BoxBC b1;

std::vector<float> pointTest;
std::vector<float> colorTest;
std::vector<float> normsTest;
float frameRateV;

bool bMouseActive;
int numTri;
GLWindow * instance = nullptr;

GLWindow::GLWindow()
    : m_t(0)
    , m_renderer(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &GLWindow::handleWindowChanged);

//    setAcceptTouchEvents(true);
//    setAcceptedMouseButtons(Qt::LeftButton);

//    sshFileReceiver()->loadTestData();

    instance = this;

}

void GLWindow::setT(qreal t)
{

    if (t == m_t)
        return;

    m_t = t;
    emit tChanged();

    if (window())
        window()->update();
}

void GLWindow::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &GLWindow::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &GLWindow::cleanup, Qt::DirectConnection);
        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void GLWindow::cleanup()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = nullptr;
    }
}


SquircleRenderer::~SquircleRenderer()
{
    delete m_program;
}

void GLWindow::sync()
{
    if (!m_renderer) {
        m_renderer = new SquircleRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &SquircleRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setT(m_t);
    m_renderer->setWindow(window());


   GL()->DPI_scale = (float)window()->devicePixelRatio();
}

void SquircleRenderer::paint()
{
    if (!m_program) {
        initializeOpenGLFunctions();
        GL()->Init();
    }

    if(appState == 0)
    {
        GL()->StartGL(m_viewportSize.width(), m_viewportSize.height());
        GL()->rotateModel(2,2);
        mesh.draw();
    }
    else
    {
        GL()->StartGLOrtho(m_viewportSize.width(), m_viewportSize.height());
        mesh.drawTriangle();
    }

    GL()->EndGL();

    // Not strictly needed for this example, but generally useful for when
    // mixing with raw OpenGL.
    m_window->resetOpenGLState();

    emit instance->UIChanged();
}

void SquircleRenderer::reduceMesh(float ratio)
{
    mesh.reduceMesh(ratio);
}
