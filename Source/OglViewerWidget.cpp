#include "..\Header\OglViewerWidget.h"
#include "..\Header\MainWindow.h"
#include "..\Header\FreeCamera.h"
#include "..\Header\OrbitCamera.h"
#include "..\Header\MoveCamera.h"
#include "..\Header\SettingsManager.h"
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>

#include "..\Header\Profiler.h"

#define DEFAULT_Z_DISTANCE -4.0


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

OglViewerWidget::OglViewerWidget(QWidget *parent)
	: QOpenGLWidget(parent)
	, m_dataEngine(Q_NULLPTR)
	, m_camera(new FreeCamera)
{
	setFocus();
	setAcceptDrops(true);

	// settings window
	setDefaultValues();
	m_settings = new SettingsWindow(this);

	connect(m_settings, &SettingsWindow::updateBGColorOff, this, &OglViewerWidget::setBGColorOff);
	connect(m_settings, &SettingsWindow::updateBGColorOn, this, &OglViewerWidget::setBGColorOn);
	connect(m_settings, &SettingsWindow::updateLightColor, this, &OglViewerWidget::setLightColor);
	connect(m_settings, &SettingsWindow::updateAttFac, this, &OglViewerWidget::setAttFac);
	connect(m_settings, &SettingsWindow::updateAmbCoef, this, &OglViewerWidget::setAmbCoef);
	connect(m_settings, &SettingsWindow::sendHeadlight, this, &OglViewerWidget::setHeadlight);
	connect(m_settings, &SettingsWindow::sendBackfaceCulling, this, &OglViewerWidget::setBackfaceCulling);
	connect(m_settings, &SettingsWindow::sendZommSpeed, [this](int value) {m_camera->setZoomSpeed(value); });
}

OglViewerWidget::~OglViewerWidget()
{
	// Make sure the context is current when deleting the texture
	// and the buffers.
	makeCurrent();
	delete m_dataEngine;
	doneCurrent();

	delete m_camera;
	delete m_settings;
}


/////////////////////////////////////////////////////////////////////////
// functions

void OglViewerWidget::setDefaultValues()
{
	SettingsManager* sm = SettingsManager::getInstance(this);

	m_backgroundColorOn = QVector4D(sm->getBgColorOn() / 255, 1.0f);
	m_backgroundColorOff = QVector4D(sm->getBgColorOff() / 255, 1.0f);

	m_wireframe = false;
	m_lightOn = sm->isLight();
	m_backfaceCulling = sm->isBfCulling();

	if (sm->getLightType() == 1)	// directional
		m_light.position = { 0.0,0.0,0.0,0.0 };
	else						// point
		m_light.position = { 0.0,0.0,0.0,1.0 };
	m_light.intensities = sm->getLightColor() / 255;
	m_light.attenuationFactor = sm->getAttenuation();
	m_light.ambientCoefficient = sm->getAmbient();
	m_light.headlight = sm->isHeadlight();

	connect(this, &OglViewerWidget::lightChanged, sm, &SettingsManager::setLight);

}

void OglViewerWidget::initShaders()
{
	// Compile vertex shader
	if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader.glsl"))
		close();

	// Compile fragment shader
	if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl"))
		close();

	// Link shader pipeline
	if (!m_program.link())
		close();

	// Bind shader pipeline for use
	if (!m_program.bind())
		close();
}

void OglViewerWidget::resetView()
{
	m_camera->resetView();

	if (m_light.headlight)
		updateLightPosition();
	update();
}

void OglViewerWidget::updateLightPosition()
{
	QVector4D lightPosition = { 0,0,0,1 };

	lightPosition = m_camera->getMatrix().inverted() * lightPosition;

	m_light.position.setX(lightPosition.x());
	m_light.position.setY(lightPosition.y());
	m_light.position.setZ(lightPosition.z());
}

// OpenGL ///////////////////////////////////////////////////////////////

void OglViewerWidget::initializeGL()
{
	initializeOpenGLFunctions();
	initShaders();

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_dataEngine = new GeometryEngine(this);
	connect(m_dataEngine, &GeometryEngine::requestResetView, this, &OglViewerWidget::resetView);
	connect(m_dataEngine, &GeometryEngine::requestUpdate, this, static_cast<void(OglViewerWidget::*)(void)>(&OglViewerWidget::update));
}

void OglViewerWidget::resizeGL(int w, int h)
{
	// Calculate aspect ratio
	qreal aspect = qreal(w) / qreal(h ? h : 1);

	// Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
	const qreal zNear = 0.1, zFar = 100.0, fov = 45.0;

	// Reset projection
	m_projection.setToIdentity();

	// Set perspective projection
	m_projection.perspective(fov, aspect, zNear, zFar);
}

void OglViewerWidget::paintGL()
{
	// set background color, last value is dirtybit
	if (m_lightOn && m_backgroundColorOn[3] == 1.0)
	{
		glClearColor(m_backgroundColorOn[0], m_backgroundColorOn[1], m_backgroundColorOn[2], 0.0000f);
		m_backgroundColorOn[3] = 0.0;
	}
	else if (!m_lightOn && m_backgroundColorOff[3] == 1.0)
	{
		glClearColor(m_backgroundColorOff[0], m_backgroundColorOff[1], m_backgroundColorOff[2], 0.0000f);
		m_backgroundColorOff[3] = 0.0;
	}

	// Clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set view-projection matrix
	m_program.setUniformValue("viewProjection", m_projection * m_camera->getMatrix());

	// Set Light values
	m_program.setUniformValue("useLight", m_lightOn);
	m_program.setUniformValue("light.position", m_light.position);
	m_program.setUniformValue("light.intensities", m_light.intensities);
	m_program.setUniformValue("light.attenuationFactor", m_light.attenuationFactor);
	m_program.setUniformValue("light.ambientCoefficient", m_light.ambientCoefficient);

	// Set camera position
	m_program.setUniformValue("cameraPosition", (m_camera->getMatrix().inverted() * QVector4D(0,0,0,1)).toVector3D());

	// Draw cube geometry
	if (m_backfaceCulling)
		glEnable(GL_CULL_FACE);

	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_dataEngine->drawGeometry(&m_program);

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Inputs ///////////////////////////////////////////////////////////////

void OglViewerWidget::mousePressEvent(QMouseEvent *e)
{
	// Save mouse press position
	m_mouse.position = QVector2D(e->localPos());

	// Which button has been pressed?
	if (e->button() == Qt::LeftButton)
		m_mouse.left = true;
	else if (e->button() == Qt::RightButton)
		m_mouse.right = true;
}

void OglViewerWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		m_mouse.left = false;
	else if (e->button() == Qt::RightButton)
		m_mouse.right = false;
}

void OglViewerWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouse.left)
	{
		// get the difference between last press and now
		m_camera->rotateAction(QVector2D(e->localPos()) - m_mouse.position);

		// update the new position
		m_mouse.position = QVector2D(e->localPos());

		// request an update
		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (m_mouse.right)
	{
		// get the difference between last press and now
		m_camera->moveAction(QVector2D(e->localPos()) - m_mouse.position);

		// update the new position
		m_mouse.position = QVector2D(e->localPos());

		// request an update
		if (m_light.headlight)
			updateLightPosition();
		update();
	}
}

void OglViewerWidget::wheelEvent(QWheelEvent *e)
{
	m_camera->wheelAction(e->angleDelta().y());
	
	if (m_light.headlight)
		updateLightPosition();
	update();
}

void OglViewerWidget::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Space)
	{
		resetView();
	}
	else if (e->key() == Qt::Key_W)
	{
		emit m_camera->wheelAction(1);

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (e->key() == Qt::Key_S)
	{
		emit m_camera->wheelAction(-1);

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (e->key() == Qt::Key_A)
	{
		emit m_camera->moveAction(QVector2D(0, -1));

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (e->key() == Qt::Key_D)
	{
		emit m_camera->moveAction(QVector2D(0, 1));

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (e->key() == Qt::Key_Escape)
	{
		parentWidget()->close();
	}
	else if (e->key() == Qt::Key_L)
	{
		updateLightPosition();
		update();
	}
}

void OglViewerWidget::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_W || e->key() == Qt::Key_S)
	{
		emit m_camera->wheelAction(0);

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
	else if (e->key() == Qt::Key_A || e->key() == Qt::Key_D)
	{
		emit m_camera->moveAction(QVector2D(0, 0));

		if (m_light.headlight)
			updateLightPosition();
		update();
	}
}

void OglViewerWidget::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls())
		if(e->mimeData()->urls().size() == 1)
			if(e->mimeData()->urls().first().toLocalFile().endsWith(".msh"))
				e->acceptProposedAction();

}

void OglViewerWidget::dropEvent(QDropEvent * e)
{
	m_dataEngine->loadFile(e->mimeData()->urls().first().toLocalFile());
}


/////////////////////////////////////////////////////////////////////////
// public slots

void OglViewerWidget::loadFile(QString name)
{
	m_dataEngine->loadFile(name);
}

void OglViewerWidget::useFreeCamera()
{
	delete m_camera;
	m_camera = new FreeCamera;

	if (m_lightOn)
		updateLightPosition();
	update();
}

void OglViewerWidget::useOrbitCamera()
{
	delete m_camera;
	m_camera = new OrbitCamera;

	if (m_lightOn)
		updateLightPosition();
	update();
}

void OglViewerWidget::useMoveCamera()
{
	delete m_camera;
	m_camera = new MoveCamera;

	if (m_lightOn)
		updateLightPosition();
	update();
}

void OglViewerWidget::toggleWireframe()
{
	m_wireframe = !m_wireframe;
	update();
}

void OglViewerWidget::toggleLight()
{
	m_lightOn = !m_lightOn;

	if (m_lightOn)
	{
		m_backgroundColorOn[3] = 1.0;
		updateLightPosition();
	}
	else
	{
		m_backgroundColorOff[3] = 1.0;
	}

	emit lightChanged(m_lightOn);

	update();
}

void OglViewerWidget::showSettings()
{
	m_settings->show();
}

void OglViewerWidget::setBGColorOff(QVector3D value)
{
	m_backgroundColorOff = QVector4D(value / 255, 1.0f);

	if (!m_lightOn)
		update();
}

void OglViewerWidget::setBGColorOn(QVector3D value)
{
	m_backgroundColorOn = QVector4D(value / 255, 1.0f);

	if (m_lightOn)
		update();
}

void OglViewerWidget::setLightColor(QVector3D value)
{
	m_light.intensities = value / 255;

	if (m_lightOn)
		update();
}

void OglViewerWidget::setAttFac(double value)
{
	m_light.attenuationFactor = (float)value;

	if (m_lightOn)
		update();
}

void OglViewerWidget::setAmbCoef(double value)
{
	m_light.ambientCoefficient = (float)value;

	if (m_lightOn)
		update();
}

void OglViewerWidget::setHeadlight(bool value)
{
	m_light.headlight = value;

	if (m_lightOn && value)
	{
		updateLightPosition();
		update();
	}
}

void OglViewerWidget::setBackfaceCulling(bool value)
{
	m_backfaceCulling = value;
	update();
}
