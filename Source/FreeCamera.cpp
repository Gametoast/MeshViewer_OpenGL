#include "..\Header\FreeCamera.h"
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

FreeCamera::FreeCamera()
{
	resetView();
}

FreeCamera::~FreeCamera()
{

}


/////////////////////////////////////////////////////////////////////////
// functions

void FreeCamera::rotateAction(QVector2D diff)
{
	m_rotation = QQuaternion::fromAxisAndAngle(QVector3D(diff.y(), diff.x(), 0.0).normalized(), diff.length() * 0.5) * m_rotation;
}

void FreeCamera::moveAction(QVector2D diff)
{
	m_translation += {(float)(diff.x() * 0.01), (float)(diff.y() * -0.01), 0.0};
}

void FreeCamera::wheelAction(double value)
{
	m_translation += {0.0, 0.0, (float) (m_zSpeed * value / 240)};
}

void FreeCamera::recalculateMatrix()
{
	m_matrix = QMatrix4x4();

	m_matrix.translate(m_translation);
	m_matrix.rotate(m_rotation);
}

void FreeCamera::resetView()
{
	m_translation = { 0, 0, -4 };
	m_rotation = QQuaternion();

	CameraInterface::resetView();
}
