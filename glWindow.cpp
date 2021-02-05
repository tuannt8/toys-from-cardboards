#include "glWindow.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

#include "glhelper.h"

#include <vector>


#include <QElapsedTimer>

extern int appState;
extern int mainAxis;
bool isAssitView = false;

bool isCutDoubleTri = true;

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

void GLWindow::reduceMesh(float ratio)
{
    m_renderer->reduceMesh(ratio);
    emit UIChanged();
}

void GLWindow::setModelHeight(float h)
{
    meshInstance().modelHeight() = h;
}

void GLWindow::setMainAxis(int ax)
{
    mainAxis = ax;
}

void GLWindow::start()
{
    appState = 1;
    GL()->resetRotation();

    meshInstance().buildCutOrderDoubleTri2();
    updateCoordInfo();

    emit UIChanged();
}

void GLWindow::changeCutMode()
{
    isCutDoubleTri = !isCutDoubleTri;
}

bool GLWindow::isCutting2faces()
{
    return isCutDoubleTri;
}

void GLWindow::pre()
{
    if(isCutDoubleTri)
        meshInstance().changeCurrentEdge(-1);
    else
        meshInstance().changeCurrentFace(-1);
    updateCoordInfo();

    emit UIChanged();
}

void GLWindow::next()
{
    if(isCutDoubleTri)
        meshInstance().changeCurrentEdge(1);
    else
        meshInstance().changeCurrentFace(1);
    updateCoordInfo();

    emit UIChanged();
}

void GLWindow::updateCoordInfo()
{
    progressString = meshInstance().progressMessage();

    float screen_scale;
    vec2 screen_trans;

    if(windowSize.width() > windowSize.height())
    {
        float gap = (windowSize.width() - windowSize.height()) / 2.0;
        screen_scale = windowSize.height() / 2;
        screen_trans = vec2(gap + windowSize.height()/2, windowSize.height()/2);
    }
    else
    {
        float gap =  (-windowSize.width() + windowSize.height()) / 2.0;
        screen_scale = windowSize.width() / 2;
        screen_trans = vec2(windowSize.width()/2, gap + windowSize.width()/2);
    }

    auto m = meshInstance().manifold();


    ///////////////////////////////////////
    // Update the coord of single face
//    if(!isCutDoubleTri)
    {
        auto pos2 = meshInstance().get_flated_tri_pos_single();
        if(pos2.size() > 0)
        {
            auto hew = m.walker(meshInstance().currentFace());
            f = QString::number(hew.face().get_index());
            f0 = QString::number(hew.opp().face().get_index());
            f1 = QString::number(hew.next().opp().face().get_index());
            f2 = QString::number(hew.next().next().opp().face().get_index());

            vec2 p = (pos2[0] + pos2[1] + pos2[2])/3  * screen_scale;
            x_f = screen_trans[0] + p[0];
            y_f = screen_trans[1] - p[1];

            vec2 p0 = (pos2[0] + pos2[1])/2  * screen_scale ;
            x_f0 = screen_trans[0] + p0[0];
            y_f0 = screen_trans[1] - p0[1];

            vec2 p1 = (pos2[1] + pos2[2])/2  * screen_scale ;
            x_f1 = screen_trans[0] + p1[0];
            y_f1 = screen_trans[1] - p1[1];

            vec2 p2 = (pos2[2] + pos2[0])/2  * screen_scale ;
            x_f2 = screen_trans[0] + p2[0];
            y_f2 = screen_trans[1] - p2[1];
        }
    }
//    else
    {

        ///////////////////////////////////////
        // Update the coord of double face
        auto hew = m.walker(meshInstance().currentEdge());

        auto pos2 = meshInstance().get_flated_tri_pos();
        // 0-1-2. 3,1,0

        t1 = QString::number(hew.face().get_index());
        n11 = QString::number(hew.next().opp().face().get_index());
        n12 = QString::number(hew.next().next().opp().face().get_index());
        t2 = QString::number(hew.opp().face().get_index());
        n21 = QString::number(hew.opp().next().opp().face().get_index());
        n22 = QString::number(hew.opp().next().next().opp().face().get_index());

        vec2 p1 = (pos2[0] + pos2[1] + pos2[2])/3  * screen_scale;
        x_t1 = screen_trans[0] + p1[0];
        y_t1 = screen_trans[1] - p1[1];

        vec2 p2 = (pos2[3] + pos2[1] + pos2[0])/3  * screen_scale;
        x_t2 = screen_trans[0] + p2[0];
        y_t2 = screen_trans[1] - p2[1];

        vec2 p11 = (pos2[2] + pos2[0])/2  * screen_scale ;
        x_n11 = screen_trans[0] + p11[0];
        y_n11 = screen_trans[1] - p11[1];

        vec2 p12 = (pos2[2] + pos2[1])/2  * screen_scale;
        x_n12 = screen_trans[0] + p12[0];
        y_n12 = screen_trans[1] - p12[1];

        vec2 p21 = (pos2[1] + pos2[3])/2  * screen_scale;
        x_n21 =screen_trans[0] +  p21[0];
        y_n21 = screen_trans[1] - p21[1];

        vec2 p22 = (pos2[0] + pos2[3])/2  * screen_scale;
        x_n22 = screen_trans[0] + p22[0];
        y_n22 = screen_trans[1] - p22[1];


        auto norm0 = HMesh::normal(m, hew.face());
        auto norm1 = HMesh::normal(m, hew.opp().face());

        float angle = acos(CGLA::dot(norm0,norm1)) * 180 / 3.14159;
        if(angle > 90) // convex
        {
            cutline = "Cut opposite: " + QString::number(angle);
        }
        else
        {
            cutline = "---------: " + QString::number(angle);;
        }
    }

    emit UIChanged();
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

QString GLWindow::meshInfo()
{
    auto m = meshInstance().manifold();
    return QString("Mesh: ") + QString::number(m.no_vertices()) + " vertices; "
                + QString::number(m.no_faces()) + " faces";
}

int GLWindow::getAppState()
{
    return appState;
}

bool GLWindow::isAssistCutView()
{
    return isAssitView;
}

void GLWindow::setAssistView(bool isAssist)
{
    isAssitView = isAssist;
    if(!isAssitView)
        GL()->resetRotation();
}

int GLWindow::getMainAxis()
{
    return mainAxis;
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

    windowSize = window()->size();

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
        meshInstance().draw();

        drawCoordinate();

        meshInstance().drawBased();
    }
    else if(appState == 1)
    {
        if(isAssitView)
        {
            GL()->StartGL(m_viewportSize.width(), m_viewportSize.height());
            GL()->rotateModel(2,2);

            meshInstance().draw_ordered_cutEdges();
            meshInstance().drawActiveEdge();

            meshInstance().draw_ordered_cutFaces();
        }
        else
        {
            GL()->StartGLOrtho(m_viewportSize.width(), m_viewportSize.height());
            if(isCutDoubleTri)
                meshInstance().drawTriangle();
            else
                meshInstance().drawTriangleSingle();
//            drawCoordinate(true);
        }
    }

    GL()->EndGL();

    // Not strictly needed for this example, but generally useful for when
    // mixing with raw OpenGL.
    m_window->resetOpenGLState();

    emit instance->UIChanged();
}

void SquircleRenderer::drawCoordinate(bool isOne)
{
    // Global coord
    float dis = isOne? 1 : GL()->view_distance() / 3;
    QVector<QVector3D> coords = {
        QVector3D(dis,0,0), QVector3D(0,dis,0), QVector3D(0,0,dis)
    };
    vec3 origin(-1,0,0);
    for(auto c : coords)
    {
        GL()->drawLineSimple(origin, origin+c, c);
    }
}

void SquircleRenderer::reduceMesh(float ratio)
{
    meshInstance().reduceMesh(ratio);

}
