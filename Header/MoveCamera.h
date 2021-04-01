#pragma once
#include "CameraInterface.h"


class MoveCamera : public CameraInterface
{
public:
	explicit MoveCamera();
	virtual ~MoveCamera();

	// attributes
private:
	QVector3D m_position;
	double m_phi;
	double m_theta;
	int m_forwardSpeed;
	int m_sidewardSpeed;

	// functions
public:
	virtual void rotateAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void moveAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void wheelAction(double value) Q_DECL_OVERRIDE;
	virtual void recalculateMatrix() Q_DECL_OVERRIDE;
	virtual void resetView() Q_DECL_OVERRIDE;
};