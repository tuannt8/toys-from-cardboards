#include "glhelper.h"

#include <QOpenGLTexture>


#include "QMap"

#define addVec3(buffer, v) \
    buffer.push_back(v[0]); \
    buffer.push_back(v[1]); \
    buffer.push_back(v[2]);


void GLManager::drawSimple(floatVector points, floatVector colors,
                           QMatrix4x4 transform, GLuint mode)
{
    m_simpleShader->bind();
    m_simpleShader->enableAttributeArray("posAttr");
    m_simpleShader->enableAttributeArray("colAttr");

    m_simpleShader->setAttributeArray("posAttr", GL_FLOAT, points.data(),3);
    m_simpleShader->setAttributeArray("colAttr", GL_FLOAT, colors.data(),3);

    QMatrix4x4 model;

     model = m_globalModelMatrix * transform;


    m_simpleShader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * model);


    glDrawArrays(mode, 0, points.size()/3);

    m_simpleShader->disableAttributeArray("posAttr");
    m_simpleShader->disableAttributeArray("colAttr");
    m_simpleShader->release();
}

GLManager::GLManager()
{
    m_lightPos = QVector3D(0,3,0);
    m_globalModelMatrix.setToIdentity();
}

GLManager::~GLManager()
{
    if(m_gouraud_shader)
    {

    }
}

QVector3D GLManager::eyeLocalPos(QMatrix4x4 &localModel)
{
    QMatrix4x4 MVP = m_globalModelMatrix * localModel;
    QMatrix4x4 invVP = MVP.inverted();

    QVector3D eye(cam_viewDistane,0,0);
    eye = invVP * eye;

    return eye;
}

QVector3D GLManager::mouseRay(int posx, int posy, QMatrix4x4 &localModel)
{
    float mouseX = posx / (m_width / DPI_scale  * 0.5f) - 1.0f;
    float mouseY = 1 - posy / (m_height / DPI_scale * 0.5f);

    QMatrix4x4 proj = m_projectionMatrix;
    QMatrix4x4 view = m_viewMatrix;
    QMatrix4x4 MVP = proj * view * m_globalModelMatrix * localModel;
    QMatrix4x4 invVP = MVP.inverted();

    QVector3D screenPos(mouseX, mouseY, 1);
    QVector3D worldPos = invVP * screenPos;
    QVector3D dir = worldPos.normalized();

    return dir;
}

void GLManager::Init()
{
    // Simple shader
    // main shader
    m_simpleShader = new QOpenGLShaderProgram();
    if(! m_simpleShader->addCacheableShaderFromSourceFile(
            QOpenGLShader::Vertex, ":/resource/shader/VertexShader.glsl"))
        qInfo() << "Fail shader";
    m_simpleShader->addCacheableShaderFromSourceFile(
          QOpenGLShader::Fragment, ":/resource/shader/FragmentShader.glsl");

    m_simpleShader->link();


    // gouraud_shader
    m_gouraud_shader = new QOpenGLShaderProgram();
    if(! m_gouraud_shader->addCacheableShaderFromSourceFile(
            QOpenGLShader::Vertex, ":/resource/shader/gouraud.vert"))
        qInfo() << "Fail shader Gourand";
    if(!m_gouraud_shader->addCacheableShaderFromSourceFile(
           QOpenGLShader::Fragment, ":/resource/shader/gouraud.frag"))
        qInfo() << "Fail shader Gourand";

    m_gouraud_shader->link();

    // Texture shader
    /// Texture
    m_texture_shader = new QOpenGLShaderProgram();
    m_texture_shader->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                "attribute highp vec4 vertex;\n"
                                                "attribute mediump vec4 texCoord;\n"
                                                "varying mediump vec4 texc;\n"
                                                "uniform mediump mat4 matrix;\n"
                                                "void main(void)\n"
                                                "{\n"
                                                "    gl_Position = matrix * vertex;\n"
                                                "    texc = texCoord;\n"
                                                "}\n"
                );
    m_texture_shader->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                "uniform sampler2D texture;\n"
                                                "varying mediump vec4 texc;\n"
                                                "void main(void)\n"
                                                "{\n"
                                                "    gl_FragColor = texture2D(texture, texc.st);\n"
                                                "}\n");
    m_texture_shader->bindAttributeLocation("vertex", 0);
    m_texture_shader->bindAttributeLocation("texCoord", 1);
    m_texture_shader->link();

    // surface shader
    m_surfaceShader = new QOpenGLShaderProgram();
    if(! m_surfaceShader->addCacheableShaderFromSourceFile(
            QOpenGLShader::Vertex, ":/resource/shader/surfaceVert.glsl"))
        qInfo() << "Fail shader surface";
    if(!m_surfaceShader->addCacheableShaderFromSourceFile(
           QOpenGLShader::Fragment, ":/resource/shader/surfaceFrag.glsl"))
        qInfo() << "Fail shader surface";

    m_surfaceShader->link();
}

void GLManager::StartGLOrtho(int width, int height)
{
    glClearColor(0., 0., 0., 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_width = width;
    m_height = height;

    m_viewMatrix.setToIdentity();
    m_projectionMatrix.setToIdentity();

    m_viewMatrix.ortho(0,0,1,1,-1, 1);
    if(width > height)
        m_projectionMatrix.scale((float)height/width,1,1);
    else
        m_projectionMatrix.scale(1,(float)width/height,1);
//    m_projectionMatrix.perspective(45, (float)width/height, 0.1f, 1000);

    glViewport(0, 0, width, height);

    glClearColor(0., 0., 0., 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void GLManager::StartGL(int width, int height)
{
    m_width = width;
    m_height = height;

    m_viewMatrix.setToIdentity();
    m_projectionMatrix.setToIdentity();

    float eyeDistance = cam_viewDistane;

    QVector3D center = cam_center;
    QVector3D eye = center + cam_eye * eyeDistance;
    QVector3D up = cam_up;

//    m_viewMatrix.lookAt(eye, center, up);
//    m_projectionMatrix.perspective(45, (float)width/height, 0.1f, 1000);

    glViewport(0, 0, width, height);


    m_lightPos = QVector3D(0,-4,0);// m_viewMatrix * m_projectionMatrix * QVector3D(0,3,0);

//    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // The Type Of Depth Test To Do
//    glShadeModel(GL_SMOOTH);  // Enables Smooth Color Shading
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(0.3, 0.4, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);



//    m_simpleShader->bind();
}

void GLManager::EndGL()
{
//    m_simpleShader->release();
}

void GLManager::rotateModel(float dx, float dy)
{
    dx *= 0.2;
    dy *= 0.2;

    QVector3D z,y;
    for(int i = 0; i < 3; i++)
    {
        z[i] = m_globalModelMatrix(2,i);
        y[i] = m_globalModelMatrix(1,i);
    }
    // dx rotate by the global z axis
    m_globalModelMatrix.rotate(dx, z[0], z[1], z[2]);

    // dy rotate by the global y axis
    m_globalModelMatrix.rotate(dy, y[0], y[1], y[2]);
}

void GLManager::drawGourand(const std::vector<float> &point, const std::vector<float> &normal, GLenum drawType, QMatrix4x4 *modelMatrix, float alpha, colorCode cc)
{

    static QVector<QVector<QVector3D>> colors;
    if(colors.size() == 0)
    {
        colors.resize(colorCodeCount);
        colors[colorCodeCyan] = {
            {0.15,0.4,0.5}, // ambient
            {0.2f, 0.3f, 0.4f}, // diffuse
            {0.2f, 0.3f, 0.4f} // spectacular
        };
        colors[colorCodeRed] = {
            {0.7,0.4,0.5}, // ambient
            {0.8f, 0.1f, 0.1f}, // diffuse
            {0.9f, 0.3f, 0.4f} // spectacular
        };
        colors[colorCodeGreen] = {
            {0.15,1,0.5}, // ambient
            {0.2f, 1, 0.4f}, // diffuse
            {0.2f, 1, 0.4f} // spectacular
        };
        colors[colorCodeBlue] = {
            {0.15,0.4,1}, // ambient
            {0.2f, 0.3f, 1}, // diffuse
            {0.2f, 0.3f, 1} // spectacular
        };
    }


    QMatrix4x4 localModel;
    if(modelMatrix)
        localModel =  m_globalModelMatrix * (*modelMatrix);
    else
        localModel = m_globalModelMatrix;

    QMatrix4x4 modelViewMatrix = m_viewMatrix * localModel ;
    QMatrix4x4 normalMatrix = modelViewMatrix.inverted();
    QMatrix4x4 modelViewProjectionMatrix = m_projectionMatrix * modelViewMatrix;

    m_gouraud_shader->bind();

    m_gouraud_shader->setUniformValue("lightPos", m_lightPos);

    m_gouraud_shader->setUniformValue( "MVPMatrix", modelViewProjectionMatrix);
    m_gouraud_shader->setUniformValue( "MVMatrix", modelViewMatrix);
    m_gouraud_shader->setUniformValue( "NormalMatrix", normalMatrix);

    // Will different depend
    m_gouraud_shader->setUniformValue("ambientMat", QVector4D(colors[cc][0], alpha));
    m_gouraud_shader->setUniformValue("diffuseMat", QVector4D(colors[cc][1], alpha));
    m_gouraud_shader->setUniformValue("specMat", QVector4D(colors[cc][2], alpha));


    m_gouraud_shader->enableAttributeArray("position");
    m_gouraud_shader->setAttributeArray("position", GL_FLOAT, point.data(), 3);

    m_gouraud_shader->enableAttributeArray("vector");
    m_gouraud_shader->setAttributeArray("vector", GL_FLOAT, normal.data(), 3);

    glDrawArrays(drawType, 0, point.size()/3);

    m_gouraud_shader->disableAttributeArray(0);
    m_gouraud_shader->disableAttributeArray(1);
    m_gouraud_shader->release();
}

void GLManager::drawLineSimple(vec3 p0, vec3 p1, vec3 color,
                               QMatrix4x4 * modelMatrix)
{
//    m_simpleShader->bind();
//    m_simpleShader->enableAttributeArray("posAttr");
//    m_simpleShader->enableAttributeArray("colAttr");

    floatVector points, colors;
    addVec3(points, p0);
    addVec3(points, p1);
    addVec3(colors, color);
    addVec3(colors, color);

//    m_simpleShader->setAttributeArray("posAttr", GL_FLOAT, points.data(),3);
//    m_simpleShader->setAttributeArray("colAttr", GL_FLOAT, colors.data(),3);

    QMatrix4x4 model;
    model.setToIdentity();
    if(modelMatrix)
        model = (*modelMatrix);

    drawSimple(points, colors, model, GL_LINES);

//    m_simpleShader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * model);


//    glDrawArrays(GL_LINES, 0, points.size()/3);

//    m_simpleShader->disableAttributeArray("posAttr");
//    m_simpleShader->disableAttributeArray("colAttr");
//    m_simpleShader->release();
}

void GLManager::DrawPoint(QVector<QVector3D> pos, QMatrix4x4 * modelMatrix)
{
//    m_simpleShader->bind();
//    m_simpleShader->enableAttributeArray("posAttr");
//    m_simpleShader->enableAttributeArray("colAttr");

    QVector3D color(1,0,0);
    floatVector points, colors;

    for(auto p : pos)
    {
        for(int i = 0; i < 3; i++)
        {
            points.push_back(p[i]);
            colors.push_back(color[i]);
        }
    }

    QMatrix4x4 model;
    model.setToIdentity();
    if(modelMatrix)
        model = (*modelMatrix);
    drawSimple(points, colors, model, GL_POINTS);

//    m_simpleShader->setAttributeArray("posAttr", GL_FLOAT, points.data(),3);
//    m_simpleShader->setAttributeArray("colAttr", GL_FLOAT, colors.data(),3);

//    QMatrix4x4 model;
//    if(modelMatrix)
//        model = m_globalModelMatrix * (*modelMatrix);
//    else
//        model = m_globalModelMatrix;
//    m_simpleShader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * model);

//    glDrawArrays(GL_POINTS, 0, points.size()/3);

//    m_simpleShader->disableAttributeArray("posAttr");
//    m_simpleShader->disableAttributeArray("colAttr");
//    m_simpleShader->release();
}

void GLManager::drawWiredCube(QMatrix4x4 modelMatrix, QVector3D color)
{
    floatVector points;
    glUtility::GetIdentityWiredBox(points);

    floatVector colors;
    colors.resize(points.size());
    for(int i = 0; i < points.size() / 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            colors[i*3 + j] = color[j];
        }
    }

    // draw
    drawSimple(points, colors, modelMatrix, GL_LINES);
}



void GLManager::drawLineSegment(QVector<QVector3D> segments, vec3 color, QMatrix4x4 *modelMatrix)
{
    m_simpleShader->bind();
    m_simpleShader->enableAttributeArray("posAttr");
    m_simpleShader->enableAttributeArray("colAttr");

    floatVector points, colors;

    for(auto p : segments)
    {
        for(int i = 0; i < 3; i++)
        {
            points.push_back(p[i]);
            colors.push_back(color[i]);
        }
    }

    m_simpleShader->setAttributeArray("posAttr", GL_FLOAT, points.data(),3);
    m_simpleShader->setAttributeArray("colAttr", GL_FLOAT, colors.data(),3);

    QMatrix4x4 model;
    if(modelMatrix)
        model = m_globalModelMatrix * (*modelMatrix);
    else
        model = m_globalModelMatrix;
    m_simpleShader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * model);

    glDrawArrays(GL_LINES, 0, points.size()/3);

    m_simpleShader->disableAttributeArray("posAttr");
    m_simpleShader->disableAttributeArray("colAttr");
    m_simpleShader->release();
}

void GLManager::drawCircle(int axis, float radius,
                           QMatrix4x4 * modelMatrix)
{
    m_simpleShader->bind();
    m_simpleShader->enableAttributeArray("posAttr");
    m_simpleShader->enableAttributeArray("colAttr");

    static floatVector pointsz = {
1, 0, 0, 0.95324, 0.3022, 0, 0.95324, 0.3022, 0, 0.81735, 0.57614, 0, 0.81735, 0.57614, 0, 0.60502, 0.79621, 0, 0.60502, 0.79621, 0, 0.33612, 0.94182, 0, 0.33612, 0.94182, 0, 0.035789, 0.99936, 0, 0.035789, 0.99936, 0, -0.26789, 0.96345, 0, -0.26789, 0.96345, 0, -0.54652, 0.83745, 0, -0.54652, 0.83745, 0, -0.77404, 0.63313, 0, -0.77404, 0.63313, 0, -0.92919, 0.36961, 0, -0.92919, 0.36961, 0, -0.99744, 0.071532, 0, -0.99744, 0.071532, 0, -0.97242, -0.23324, 0, -0.97242, -0.23324, 0, -0.85647, -0.5162, 0, -0.85647, -0.5162, 0, -0.66043, -0.75089, 0, -0.66043, -0.75089, 0, -0.40263, -0.91536, 0, -0.40263, -0.91536, 0, -0.10718, -0.99424, 0, -0.10718, -0.99424, 0, 0.19829, -0.98014, 0, 0.19829, -0.98014, 0, 0.48522, -0.87439, 0, 0.48522, -0.87439, 0, 0.72677, -0.68688, 0, 0.72677, -0.68688, 0, 0.90037, -0.43513, 0, 0.90037, -0.43513, 0, 0.98977, -0.1427, 0, 0.98977, -0.1427, 0, 0.98661, 0.16308, 0
    };
    static floatVector colorsz = {
       0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1
    };

    static floatVector pointsx = {0, 1, 0, 0, 0.95324, 0.3022, 0, 0.95324, 0.3022, 0, 0.81735, 0.57614, 0, 0.81735, 0.57614, 0, 0.60502, 0.79621, 0, 0.60502, 0.79621, 0, 0.33612, 0.94182, 0, 0.33612, 0.94182, 0, 0.035789, 0.99936, 0, 0.035789, 0.99936, 0, -0.26789, 0.96345, 0, -0.26789, 0.96345, 0, -0.54652, 0.83745, 0, -0.54652, 0.83745, 0, -0.77404, 0.63313, 0, -0.77404, 0.63313, 0, -0.92919, 0.36961, 0, -0.92919, 0.36961, 0, -0.99744, 0.071532, 0, -0.99744, 0.071532, 0, -0.97242, -0.23324, 0, -0.97242, -0.23324, 0, -0.85647, -0.5162, 0, -0.85647, -0.5162, 0, -0.66043, -0.75089, 0, -0.66043, -0.75089, 0, -0.40263, -0.91536, 0, -0.40263, -0.91536, 0, -0.10718, -0.99424, 0, -0.10718, -0.99424, 0, 0.19829, -0.98014, 0, 0.19829, -0.98014, 0, 0.48522, -0.87439, 0, 0.48522, -0.87439, 0, 0.72677, -0.68688, 0, 0.72677, -0.68688, 0, 0.90037, -0.43513, 0, 0.90037, -0.43513, 0, 0.98977, -0.1427, 0, 0.98977, -0.1427, 0, 0.98661, 0.16308};

    static floatVector colorsx = {
        1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0
    };

    static floatVector pointsy = {1, 0, 0, 0.95324, 0, 0.3022, 0.95324, 0, 0.3022, 0.81735, 0, 0.57614, 0.81735, 0, 0.57614, 0.60502, 0, 0.79621, 0.60502, 0, 0.79621, 0.33612, 0, 0.94182, 0.33612, 0, 0.94182, 0.035789, 0, 0.99936, 0.035789, 0, 0.99936, -0.26789, 0, 0.96345, -0.26789, 0, 0.96345, -0.54652, 0, 0.83745, -0.54652, 0, 0.83745, -0.77404, 0, 0.63313, -0.77404, 0, 0.63313, -0.92919, 0, 0.36961, -0.92919, 0, 0.36961, -0.99744, 0, 0.071532, -0.99744, 0, 0.071532, -0.97242, 0, -0.23324, -0.97242, 0, -0.23324, -0.85647, 0, -0.5162, -0.85647, 0, -0.5162, -0.66043, 0, -0.75089, -0.66043, 0, -0.75089, -0.40263, 0, -0.91536, -0.40263, 0, -0.91536, -0.10718, 0, -0.99424, -0.10718, 0, -0.99424, 0.19829, 0, -0.98014, 0.19829, 0, -0.98014, 0.48522, 0, -0.87439, 0.48522, 0, -0.87439, 0.72677, 0, -0.68688, 0.72677, 0, -0.68688, 0.90037, 0, -0.43513, 0.90037, 0, -0.43513, 0.98977, 0, -0.1427, 0.98977, 0, -0.1427, 0.98661, 0, 0.16308};

    static floatVector colorsy = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};

    floatVector points;
    floatVector colors;

    if(axis == 0)
    {
        points = pointsx;
        colors = colorsx;
    }
    else if(axis == 1)
    {
        points = pointsy;
        colors = colorsy;
    }
    else if(axis == 2)
    {
        points = pointsz;
        colors = colorsz;
    }

    for(auto & p : points)
        p *= radius;


    m_simpleShader->setAttributeArray("posAttr", GL_FLOAT, points.data(),3);
    m_simpleShader->setAttributeArray("colAttr", GL_FLOAT, colors.data(),3);

    QMatrix4x4 model;
    if(modelMatrix)
        model = m_globalModelMatrix * (*modelMatrix);
    else
        model = m_globalModelMatrix;

    m_simpleShader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * model);


    glDrawArrays(GL_LINES, 0, points.size()/3);

    m_simpleShader->disableAttributeArray("posAttr");
    m_simpleShader->disableAttributeArray("colAttr");
    m_simpleShader->release();
}

void GLManager::draw3DObj(QString name, colorCode cc, QMatrix4x4 * transformMat)
{
//    floatVector positions, normals;
//    Geometric::obj_load(":/3DOBJ/resource/3D-models/" + name + ".obj", positions, normals);
//    drawGourand(positions, normals, GL_TRIANGLES, transformMat, 1.0, cc);

}

void GLManager::drawImage(QString name, QVector3D pos, float size)
{
    QMatrix4x4 imodel = m_globalModelMatrix.inverted();
    QVector3D eyePos = imodel * (cam_eye * view_distance());
    QVector3D eyeDirect = imodel * cam_up;

    QVector3D yy = eyePos - pos; yy.normalize();
    QVector3D zz = eyeDirect; zz.normalize();
    QVector3D xx = QVector3D::crossProduct(yy,zz); xx.normalize();

    QMatrix4x4 localTranMat;
    localTranMat.setColumn(0, QVector4D(xx, 0));
    localTranMat.setColumn(1, QVector4D(yy, 0));
    localTranMat.setColumn(2, QVector4D(zz, 0));
    localTranMat.setColumn(3, QVector4D(pos, 1));
    localTranMat.scale(size, size, size);

    static QMap<QString, QOpenGLTexture*> textureCached;

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if(textureCached.find(name) == textureCached.end())
    {
        QString path = ":/2Dtexture/resource/texture/" + name + ".png";
        QOpenGLTexture * tex = new QOpenGLTexture(QImage(path).mirrored());
        tex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        tex->setMagnificationFilter(QOpenGLTexture::Linear);
        textureCached.insert(name, tex);
    }

    QOpenGLTexture * texture = textureCached[name];

    std::vector<float> vertices = {
        -0.5, 0, -0.5,
        0.5, 0, -0.5,
        -0.5, 0, 0.5,

        0.5, 0, 0.5,
        0.5, 0, -0.5,
        -0.5, 0, 0.5
    };
    std::vector<float> texCoord = {
        0,0,
        1,0,
        0,1,

        1,1,
        1,0,
        0,1
    };


    m_texture_shader->bind();
    m_texture_shader->enableAttributeArray("vertex");
    m_texture_shader->enableAttributeArray("texCoord");
    m_texture_shader->setAttributeArray("vertex", GL_FLOAT, vertices.data(),3);
    m_texture_shader->setAttributeArray("texCoord", GL_FLOAT, texCoord.data(),2);
    m_texture_shader->setUniformValue("matrix", m_projectionMatrix * m_viewMatrix * m_globalModelMatrix * localTranMat);

    texture->bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/3);

    m_texture_shader->disableAttributeArray("vertex");
    m_texture_shader->disableAttributeArray("texCoord");
    m_texture_shader->disableAttributeArray("matrix");
    m_texture_shader->release();
}

GLManager *GLManager::GetInstance()
{
    static GLManager instance;
    return &instance;
}

void GLManager::drawTest()
{
    drawCircle(1,1);
}

GLManager *GL()
{
    return GLManager::GetInstance();
}

void glUtility::GetIdentityBox(std::vector<float> &points, std::vector<float> &norms)
{
    /*
     *  7 ---- 6
     *  |     |
     *  |     |
     *  4 ---- 5
     */
    /*
     *  3 ---- 2
     *  |     |
     *  |     |
     *  0 ---- 1
     */
    /*
     *     ^ z
     *     |
     *     ----> y
     *   /
     * x
     */
    static std::vector<float> pps =
    {
        0,0,0,
        1,0,0,
        1,0,1,
        0,0,1,

        0,1,0,
        1,1,0,
        1,1,1,
        0,1,1,
    };

    static std::vector<int> tris =
    {
        // down
        0, 3, 1,
        1, 3, 2,
        // right
        1, 2, 5,
        5, 2, 6,
        // left
        0, 4, 7,
        0, 7, 3,
        // back
        2, 7, 6,
        2, 3, 7,
        // front
        0, 1, 4,
        4, 1, 5,
        // up
        4, 5, 7,
        5, 6, 7
    };

    static std::vector<vec3> norm = {
        vec3(0,0,-1),
        vec3(0,1,0),
        vec3(0,-1,0),
        vec3(-1,0,0),
        vec3(1,0,0),
        vec3(0,0,1)
    };

    // Tranform it to [-1 1]
    static std::vector<float> pointstatic, normstatic;
    if(pointstatic.size() == 0)
    {
        for(auto & p : pps)
        {
            p -= 0.5;
//            p *= 2;
        }

        for(int i = 0; i < (int)tris.size()/3; i++) // triangle
        {
            vec3 n = norm[std::floor(i/2)];
            for(int j = 0; j < 3; j++) // 3 vertices
            {
                // three points
                int pidx = tris[3*i + j];

                for(int k = 0; k < 3; k++) // x,y,z
                {
                    normstatic.push_back(n[k]);
                    pointstatic.push_back(pps[pidx * 3 + k]);
                }
            }
        }
    }

    points = pointstatic;
    norms = normstatic;
}

void glUtility::GetIdentityWiredBox(std::vector<float> &points)
{
    /*
     *  7 ---- 6
     *  |     |
     *  |     |
     *  4 ---- 5
     */
    /*
     *  3 ---- 2
     *  |     |
     *  |     |
     *  0 ---- 1
     */
    /*
     *     ^ z
     *     |
     *     ----> y
     *   /
     * x
     */
    static std::vector<float> pps =
    {
        0,0,0,
        1,0,0,
        1,0,1,
        0,0,1,

        0,1,0,
        1,1,0,
        1,1,1,
        0,1,1,
    };

    static std::vector<int> lines =
    {
        // down
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        // left
        0, 3,
        3, 7,
        7, 4,
        4, 0,
        // right
        1, 2,
        2, 6,
        6, 5,
        5, 1,
        // up
        4, 5,
        5, 6,
        6, 7,
        7, 4
    };

    static std::vector<float> pointstatic;
    if(pointstatic.size() == 0)
    {
        for(auto & p : pps)
        {
            p -= 0.5;
        }

        for(int i = 0; i < (int)lines.size()/2; i++) // triangle
        {
            for(int j = 0; j < 2; j++) // 3 vertices
            {
                // three points
                int pidx = lines[2*i + j];

                for(int k = 0; k < 3; k++) // x,y,z
                {
                    pointstatic.push_back(pps[pidx * 3 + k]);
                }
            }
        }
    }

    points = pointstatic;
}
