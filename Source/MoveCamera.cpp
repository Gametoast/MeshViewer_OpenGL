#include "..\Header\MoveCamera.h"
#include <QVector2D>
#include <qmath.h>


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

MoveCamera::MoveCamera()
{
	resetView();
}

MoveCamera::~MoveCamera()
{

}


/////////////////////////////////////////////////////////////////////////
// functions

void MoveCamera::rotateAction(QVector2D diff)
{
	m_phi -= diff.x() * 0.01;
	m_theta += diff.y() * 0.01;

	m_theta = qMax(qMin(M_PI - 0.0001, m_theta), 0.0001);
}

void MoveCamera::moveAction(QVector2D diff)
{
	if (diff.y() > 0)
		m_sidewardSpeed = 1;
	else if (diff.y() < 0)
		m_sidewardSpeed = -1;
	else
		m_sidewardSpeed = 0;
}

void MoveCamera::wheelAction(double value)
{
	if (value > 0)
		m_forwardSpeed = 1;
	else if (value < 0)
		m_forwardSpeed = -1;
	else
		m_forwardSpeed = 0;
}

void MoveCamera::recalculateMatrix()
{
	m_matrix = QMatrix4x4();

	// different coordinate (spherical -> world) X->Z | Y->X | Z->Y
	QVector3D tmpdirection(
		qSin(m_theta) * qSin(m_phi),
		qCos(m_theta),
		qSin(m_theta) * qCos(m_phi)
	);

	QVector3D tmpRight(
		qSin(m_phi - M_PI_2),
		0,
		qCos(m_phi - M_PI_2)
	);

	m_position += m_forwardSpeed * m_zSpeed * 0.1 * tmpdirection;
	m_position += m_sidewardSpeed * m_zSpeed * 0.1 * tmpRight;

	m_matrix.lookAt(m_position, m_position + tmpdirection, QVector3D::crossProduct(tmpRight, tmpdirection));
}

void MoveCamera::resetView()
{
	m_position = { 0,0,4 };
	m_phi = M_PI;
	m_theta = M_PI_2;
	m_forwardSpeed = 0;
	m_sidewardSpeed = 0;
	CameraInterface::resetView();
}
