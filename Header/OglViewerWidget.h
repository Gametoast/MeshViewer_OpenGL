#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include "GeometryEngine.h"
#include "SettingsWindow.h"
#include "CameraInterface.h"


class GeometryEngine;

class OglViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	explicit OglViewerWidget(QWidget *parent = 0);
	~OglViewerWidget();

// attributes
private:
	QOpenGLShaderProgram m_program;
	GeometryEngine *m_dataEngine;

	QVector4D m_backgroundColorOn;
	QVector4D m_backgroundColorOff;

	bool m_wireframe;
	bool m_lightOn;
	bool m_backfaceCulling;

	struct {
		QVector4D position;
		QVector3D intensities;
		float attenuationFactor;
		float ambientCoefficient;
		bool headlight;
	} m_light;

	struct {
		bool left = false;
		bool right = false;
		QVector2D position;
	} m_mouse;

	QMatrix4x4 m_projection;
	CameraInterface* m_camera;

	SettingsWindow* m_settings;

// functions
private:
	void setDefaultValues();
	void initShaders();
	void resetView();
	void updateLightPosition();

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;

	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
	
	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

	void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
	void dropEvent(QDropEvent * event) Q_DECL_OVERRIDE;

// slots
public slots:
	void loadFile(QString name);
	void useFreeCamera();
	void useOrbitCamera();
	void useMoveCamera();
	void toggleWireframe();
	void toggleLight();
	void showSettings();

	void setBGColorOff(QVector3D value);
	void setBGColorOn(QVector3D value);
	void setLightColor(QVector3D value);
	void setAttFac(double value);
	void setAmbCoef(double value);
	void setHeadlight(bool value);
	void setBackfaceCulling(bool value);

// signals
signals:
	void lightChanged(bool value);


};

