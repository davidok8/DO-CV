#include <QtOpenGL>
#ifdef __APPLE__
# include <OpenGL/GLU.h>
#else
# include <GL/glu.h>
#endif

#include <DO/Sara/Graphics/Frame.hpp>

#include <DO/Sara/Graphics/DerivedQObjects/OpenGLWindow.hpp>

#ifndef GL_MULTISAMPLE
# define GL_MULTISAMPLE  0x809D
#endif


namespace DO { namespace Sara {

  // ====================================================================== //
  // TrackBall implementation
  TrackBall::TrackBall()
  {
    pressed_ = false;
    axis_ = QVector3D(0, 1, 0);
    rotation_ = QQuaternion();
  }

  void TrackBall::push(const QPointF& p, const QQuaternion &)
  {
    rotation_ = rotation();
    pressed_ = true;
    lastPos_ = p;
  }

  static void projectToSphere(QVector3D& x)
  {
    qreal sqrZ = 1 - QVector3D::dotProduct(x, x);
    if (sqrZ > 0)
      x.setZ(std::sqrt(sqrZ));
    else
      x.normalize();
  }

  void TrackBall::move(const QPointF& p)
  {
    if (!pressed_)
      return;
    // Get the last position and project it on the sphere
    QVector3D lastPos3D = QVector3D(lastPos_.x(), lastPos_.y(), 0.0f);
    projectToSphere(lastPos3D);
    // Get the current position and project it on the sphere
    QVector3D currentPos3D = QVector3D(p.x(), p.y(), 0.0f);
    projectToSphere(currentPos3D);
    // Compute the new axis by cross product
    axis_ = QVector3D::crossProduct(lastPos3D, currentPos3D);
    axis_.normalize();
    // Compose the old rotation with the new rotation.
    // Remember that quaternions do not commute.
    rotation_ = QQuaternion::fromAxisAndAngle(axis_, 2.0) * rotation_;
    // Remember the current position as the last position when move is called again.
    lastPos_ = p;
  }

  void TrackBall::release(const QPointF& p)
  {
    move(p);
    pressed_ = false;
  }

  QQuaternion TrackBall::rotation() const
  {
    if (pressed_)
      return rotation_;
    return  QQuaternion::fromAxisAndAngle(axis_, 2.0) * rotation_;
  }

  // ====================================================================== //
  // OpenGLWindow implementation
  OpenGLWindow::OpenGLWindow(int width, int height,
                             const QString& windowTitle,
                             int x, int y,
                             QWidget* parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    , m_scale(1.0f)
    , m_backgroundColor(QColor::fromCmykF(0.39, 0.39, 0.0, 0.0))
    , m_color(QColor::fromCmykF(0.40, 0.0, 1.0, 0.0))
  {
    setAttribute(Qt::WA_DeleteOnClose);

    // Set event listener.
    m_eventListeningTimer.setSingleShot(true);
    connect(&m_eventListeningTimer, SIGNAL(timeout()),
            this, SLOT(eventListeningTimerStopped()));

    if(x != -1 && y != -1)
      move(x,y);
    setWindowTitle(windowTitle);
    resize(width, height);
    show();
    // Needed to correctly mix OpenGL commands and QPainter drawing commands.
    setAutoFillBackground(false);

    m_displayFrame = false;
  }

  void OpenGLWindow::setMesh(const SimpleTriangleMesh3f& mesh)
  {
    m_mesh = mesh;
    m_center = mesh.center();
    update();
  }

  void OpenGLWindow::displayMesh()
  {
    glBegin(GL_TRIANGLES);
    {
      for(size_t t = 0; t != m_mesh.faces().size(); ++t)
      {
        for (int v = 0; v < 3; ++v)
        {
          size_t vInd = m_mesh.face(t)(v);
          glNormal3fv(m_mesh.normal(vInd).data());
          glVertex3fv(m_mesh.vertex(vInd).data());
        }
      }
    }
    glEnd();
  }

  void OpenGLWindow::waitForEvent(int ms)
  {
    m_eventListeningTimer.setInterval(ms);
    m_eventListeningTimer.start();
  }

  void OpenGLWindow::eventListeningTimerStopped()
  {
    emit sendEvent(no_event());
  }

  void OpenGLWindow::initializeGL()
  {
    // Set background color
    qglClearColor(m_backgroundColor);

    glShadeModel(GL_SMOOTH);  // Enable smooth shading

    // Set up the cosmic background radiation.
    glEnable(GL_LIGHTING);    // Enable lighting
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // Set up light source 0
    GLfloat light0Pos[]      = { 0.0f, 0.0f, 10.0f, 1.0f };
    GLfloat light0SpotDir[]  = { 0.0f, 0.0f,-1.0f, 1.0f };
    GLfloat diffuseLight0[]  = { 0.8f, 0.5f, 0.5f, 0.8f };
    GLfloat specularLight0[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0SpotDir);
    glEnable(GL_LIGHT0);

    // Set up color material
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 100);
    glEnable(GL_COLOR_MATERIAL);

    // ?
    glEnable(GL_MULTISAMPLE);

    // Normalize the vector for the lighting
    glEnable(GL_NORMALIZE);
  }

  static void multMatrix(const QMatrix4x4& m)
  {
    // static to prevent glMultMatrixf to fail on certain drivers
    static GLfloat mat[16];
    const float *data = m.constData();
    for (int index = 0; index < 16; ++index)
      mat[index] = data[index];
    glMultMatrixf(mat);
  }

  void OpenGLWindow::paintEvent(QPaintEvent *)
  {
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup the viewing mode for the mesh
    glPolygonMode(GL_FRONT, GL_FILL); // we make each front face filled
    glPolygonMode(GL_BACK, GL_LINE);  // we draw only the edges of the back face
    glEnable(GL_DEPTH_TEST);  // For depth consistent drawing
    // Model-view transform.
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -15.0f);
    // Display the world frame is at z=-15 w.r.t. the camera frame.
    //frame_.draw(5, 0.1);
    // Scale the model
    glScalef(m_scale, m_scale, m_scale);
    // Rotate the model with the trackball.
    QMatrix4x4 m;
    m.rotate(m_trackball.rotation());
    multMatrix(m);
    // Display the mesh.
    glPushMatrix();
    {
      // Center the model
      glTranslatef(-m_center.x(), -m_center.y(), -m_center.z());
      // Draw the model
      displayMesh();
    }
    glPopMatrix();
    // Object-centered frame.
    if (m_displayFrame)
      m_frame.draw(5, 0.1);

    // Disable the following to properly display the drawing with QPainter.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Start drawing the text information on the window.
    QPainter p(this);
    // Rendering options
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    // The text to display.
    QString text = tr("Use the mouse wheel to zoom and the mouse left button to rotate the scene.\nHit 'F' to toggle object-centered frame display");
    // Set the font style
    setFont(QFont("Helvetica [Cronyx]", 10, QFont::Bold));
    // Draw the bounding box within which the text will be drawn.
    QFontMetrics metrics = QFontMetrics(font());
    int border = qMax(4, metrics.leading());
    QRect rect = metrics.boundingRect(
      0, 0, width() - 2*border, int(height()*0.125),
      Qt::AlignCenter | Qt::TextWordWrap, text);
    p.fillRect(QRect(0, 0, width(), rect.height() + 2*border),
      QColor(0, 0, 0, 127));
    // Draw the text.
    p.setPen(Qt::white);
    p.fillRect(QRect(0, 0, width(), rect.height() + 2*border),
      QColor(0, 0, 0, 127));
    p.setFont(font());
    p.drawText((width() - rect.width())/2, border,
      rect.width(), rect.height(),
      Qt::AlignCenter | Qt::TextWordWrap, text);
    p.end();
  }

  void OpenGLWindow::resizeGL(int width, int height)
  {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio = width/static_cast<double>(height);
    gluPerspective(60.0, ratio, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
  }

  QPointF OpenGLWindow::normalizePos(const QPointF& localPos) const
  {
    QPointF pos(localPos);
    pos.rx() -=  width()/2.; pos.rx() /= width()/2.;
    pos.ry() -= height()/2.; pos.ry() /= height()/2.; pos.ry() *= -1;
    return pos;
  }

  void OpenGLWindow::mousePressEvent(QMouseEvent *event)
  {
    QGLWidget::mousePressEvent(event);
    if (event->isAccepted())
      return;

    QPointF pos(normalizePos(event->localPos()));
    if (event->buttons() & Qt::LeftButton) {
      m_trackball.push(pos, m_trackball.rotation());
      event->accept();
    }
    update();
  }

  void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
  {
    QGLWidget::mouseReleaseEvent(event);
    if (event->isAccepted())
      return;

    QPointF pos(normalizePos(event->localPos()));
    if (event->button() == Qt::LeftButton) {
      m_trackball.release(pos);
      event->accept();
    }
    update();
  }

  void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
  {
    QGLWidget::mouseMoveEvent(event);
    if (event->isAccepted())
    {
      qDebug() << "mouse move event already accepted";
      return;
    }

    QPointF pos(normalizePos(event->localPos()));
    if (event->buttons() & Qt::LeftButton) {
      m_trackball.move(pos);
      event->accept();
    } else {
      m_trackball.release(pos);
    }
    update();
  }

  void OpenGLWindow::wheelEvent(QWheelEvent* event)
  {
    QGLWidget::wheelEvent(event);

    if (!event->isAccepted()) {
      event->angleDelta().y() > 0 ? m_scale += 0.05f * m_scale :
                                    m_scale -= 0.05f*m_scale;
      update();
    }
  }

  void OpenGLWindow::keyPressEvent(QKeyEvent *event)
  {
    emit pressedKey(event->key());
    if (event->key() == Qt::Key_F)
    {
      m_displayFrame=!m_displayFrame;
      update();
    }
    if (m_eventListeningTimer.isActive())
    {
      m_eventListeningTimer.stop();
      emit sendEvent(key_pressed(event->key(), event->modifiers()));
    }
  }

  void OpenGLWindow::keyReleaseEvent(QKeyEvent *event)
  {
    emit releasedKey(event->key());
    if (m_eventListeningTimer.isActive())
    {
      m_eventListeningTimer.stop();
      emit sendEvent(key_released(event->key(), event->modifiers()));
    }
  }

  void OpenGLWindow::closeEvent(QCloseEvent *event)
  {
    if(event->spontaneous())
    {
      qWarning() << "\n\nWarning: you closed a window unexpectedly!\n\n";
      qWarning() << "Graphical application is terminating...";
      qApp->exit(0);
    }
  }

} /* namespace Sara */
} /* namespace DO */
