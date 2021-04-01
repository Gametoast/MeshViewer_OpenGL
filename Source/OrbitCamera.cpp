#include "..\Header\OrbitCamera.h"
#include <QVector2D>
#include <qmath.h>
#include <iostream>


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

OrbitCamera::OrbitCamera()
{
	resetView();
}

OrbitCamera::~OrbitCamera()
{

}


/////////////////////////////////////////////////////////////////////////
// functions

void OrbitCamera::rotateAction(QVector2D diff)
{
	m_phi -= diff.x() * 0.01;
	m_theta -= diff.y() * 0.01;

	//m_theta = qMax(qMin(M_PI - 0.0001, m_theta), 0.0001);
}

void OrbitCamera::moveAction(QVector2D diff)
{

}

void OrbitCamera::wheelAction(double value)
{
	m_roh -= m_zSpeed * value / 240;
	m_roh = qMax(m_roh, 0.0);
}

void OrbitCamera::recalculateMatrix()
{
	m_matrix = QMatrix4x4();

	// different coordinate (spherical -> world) X->Z | Y->X | Z->Y
	QVector3D tmpPosition(
		qSin(m_theta) * qSin(m_phi),
		qCos(m_theta),
		qSin(m_theta) * qCos(m_phi)
	);

	QVector3D tmpRight(
		qSin(m_phi - M_PI_2),
		0,
		qCos(m_phi - M_PI_2)
	);

	m_matrix.lookAt(m_roh * tmpPosition, QVector3D(0,0,0), QVector3D::crossProduct(tmpRight, tmpPosition));
}

void OrbitCamera::resetView()
{
	m_roh = 4;
	m_phi = 0;
	m_theta = M_PI_2;
	CameraInterface::resetView();
}
