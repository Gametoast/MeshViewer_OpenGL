#pragma once
#include <QMatrix4x4>

class CameraInterface
{
public:
	explicit CameraInterface() {};
	virtual ~CameraInterface() {};

// attributes
protected:
	QMatrix4x4 m_matrix;
	double m_zSpeed = 1.0;

// functions
public:
	virtual void setZoomSpeed(int percent) { m_zSpeed = (double) percent / 100; };

	virtual void rotateAction(QVector2D diff) = 0;
	virtual void moveAction(QVector2D diff) = 0;
	virtual void wheelAction(double value) = 0;
	virtual void resetView() { m_matrix = QMatrix4x4(); };

	virtual void recalculateMatrix() = 0;
	virtual QMatrix4x4 getMatrix() { recalculateMatrix(); return m_matrix; };
};