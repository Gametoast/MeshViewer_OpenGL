#pragma once
#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector>
#include "FileInterface.h"


struct DrawInformation {
	unsigned int offset;
	unsigned int size;
	unsigned int textureIndex;
	QMatrix4x4 modelMatrix;
};

class GeometryEngine : public QObject, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GeometryEngine(QObject *parent = Q_NULLPTR);
	virtual ~GeometryEngine();

// attributes
private:
	QOpenGLBuffer m_arrayBuf;
	QOpenGLBuffer m_indexBuf;
	QVector<Material>* m_materials = Q_NULLPTR;
	Material* m_defaultMaterial;
	BoundingBox m_boundings;
	QVector<DrawInformation> m_drawList;

// functions
private:
	void clearData();
	void setupPipeline(QOpenGLShaderProgram * program);

public:
	void drawGeometry(QOpenGLShaderProgram *program);
	void loadFile(QString filePath);

// signals
signals:
	void requestResetView();
	void requestUpdate();
};

