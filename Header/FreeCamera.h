#pragma once
#include "CameraInterface.h"


class FreeCamera : public CameraInterface
{
public:
	explicit FreeCamera();
	virtual ~FreeCamera();

// attributes
private:
	QVector3D m_translation;
	QQuaternion m_rotation;

// functions
public:
	virtual void rotateAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void moveAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void wheelAction(double value) Q_DECL_OVERRIDE;
	virtual void recalculateMatrix() Q_DECL_OVERRIDE;
	virtual void resetView() Q_DECL_OVERRIDE;
};