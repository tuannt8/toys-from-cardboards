#ifndef GLHELPER_H
#define GLHELPER_H

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <vector>


enum colorCode{
    colorCodeRed = 0,
    colorCodeGreen = 1,
    colorCodeBlue = 2,
    colorCodeCyan,
    colorCodeCount
};

typedef std::vector<float> floatVector ;
typedef QVector3D vec3 ;

class GLManager
{
    // gourand shader
    QVector3D m_lightPos;
    QOpenGLShaderProgram * m_gouraud_shader = nullptr;
    QMatrix4x4 m_projectionMatrix, // perspective
                m_globalModelMatrix, // User rotate
                m_viewMatrix; // lookat

    // Simple shader
    QOpenGLShaderProgram * m_simpleShader = nullptr;

    // Draw image
    QOpenGLShaderProgram * m_texture_shader;

    // surface shader
    QOpenGLShaderProgram * m_surfaceShader  = nullptr;

    int m_width, m_height;

    // camera
    QVector3D cam_center = QVector3D(0.,0.,0.);
    QVector3D cam_up = QVector3D(0.,0.,1);
    QVector3D cam_eye = QVector3D(1,0.,0);
    float cam_viewDistane = 5;

public:
    void drawSimple(floatVector points, floatVector colors,
                    QMatrix4x4 transform, GLuint mode);

public:
    float DPI_scale;

    float view_distance(){return cam_viewDistane;}

    GLManager();
    ~GLManager();

    QVector3D eyeLocalPos(QMatrix4x4 & localModel);
    QVector3D mouseRay(int posx, int posy, QMatrix4x4 & localModel);

    void Init(); // Load shader

    void StartGLOrtho(int width, int height);
    void StartGL(int width, int height); // in main loop
    void EndGL();

    void rotateModel(float x, float y); // Control camera

    void drawGourand(const std::vector<float> & point,
              const std::vector<float> & normal,
              GLenum drawType,
              QMatrix4x4 * modelMatrix = nullptr, float alpha = 1, colorCode cc = colorCodeCyan);

    void drawLineSimple(vec3 p0, vec3 p1, vec3 color,
                        QMatrix4x4 * modelMatrix = nullptr);

    void DrawPoint(QVector<QVector3D> pos, QMatrix4x4 * modelMatrix = nullptr);

    void drawWiredCube(QMatrix4x4 modelMatrix, QVector3D color);
    void drawVoxel(QVector3D pos, int type);

    // {l00,l01, l10,l11, l20,l21 ...}
    void drawLineSegment(QVector<QVector3D> segments, vec3 color, QMatrix4x4 * modelMatrix = nullptr);

    void drawCircle(int axis, float radius,
                    QMatrix4x4 * modelMatrix = nullptr);


    void draw3DObj(QString name, colorCode cc, QMatrix4x4 * transformMat = nullptr);
    void drawImage(QString name, QVector3D pos, float size);


    static GLManager * GetInstance();


    void drawTest();
};

namespace glUtility {
    // Cube [-0.5, 0.5]
    void GetIdentityBox(std::vector<float> & points, std::vector<float> & norms);
    void GetIdentityWiredBox(std::vector<float> & points);
}

GLManager * GL();

#endif // GLHELPER_H
