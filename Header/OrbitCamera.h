#pragma once
#include "CameraInterface.h"


class OrbitCamera : public CameraInterface
{
public:
	explicit OrbitCamera();
	virtual ~OrbitCamera();

	// attributes
private:
	double m_phi;
	double m_theta;
	double m_roh;

	// functions
public:
	virtual void rotateAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void moveAction(QVector2D diff) Q_DECL_OVERRIDE;
	virtual void wheelAction(double value) Q_DECL_OVERRIDE;
	virtual void recalculateMatrix() Q_DECL_OVERRIDE;
	virtual void resetView() Q_DECL_OVERRIDE;
};