#pragma once
#include <QOpenGlTexture>
#include <QFile>
#include <QVector>
#include <QVector2D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QRegExp>

struct BoundingBox {
	QQuaternion rotation;
	QVector3D center;
	QVector3D extents;
};

struct VertexData
{
	QVector3D position;
	QVector2D texCoord;
	QVector3D vertexNormal;
	QVector3D polygonNormal;
	QVector3D tangent;
	QVector3D bitangent;
};

struct Segment {
	quint32 textureIndex = 0;
	QVector<VertexData> vertices;
	QVector<GLuint> indices;
};

struct Model {
	QString name = "";
	QString parent = "";
	QMatrix4x4 m4x4Translation;
	QQuaternion quadRotation;
	QVector<Segment*> segmList;
};

struct Material {
	QString name;
	QString tx0d;
	QString tx1d;
	QString tx2d;
	QString tx3d;
	QOpenGLTexture* texture0 = Q_NULLPTR;
	QOpenGLTexture* texture1 = Q_NULLPTR;
	QVector4D specularColor = { 0.1f, 0.1f, 0.1f, 1.0 };
	QVector4D diffuseColor = { 1.0, 0.0, 0.0, 1.0 };
	QVector4D ambientColor = { 1.0, 1.0, 1.0, 1.0 };
	float shininess = 1;
	bool flags[8] = { false };
	bool transparent = false;
	quint8 rendertype = 0;
	quint8 dataValues[2] = { 0 };
};

class FileInterface
{

public:
	explicit FileInterface(QString path)
		: m_models(new QVector<Model*>)
		, m_materials(new QVector<Material>)
	{
		//open file
		m_file.setFileName(path);

		if (!m_file.open(QIODevice::ReadOnly))
			throw std::invalid_argument(std::string("ERROR: file not found: ") += path.toStdString());

		m_filepath = path.left(path.lastIndexOf(QRegExp("/|\\\\")));

	};

	virtual ~FileInterface()
	{
		// close file
		m_file.close();

		//clean up
		for (Model* modelIt : *m_models)
		{
			for (Segment* segIt : modelIt->segmList)
			{
				segIt->indices.clear();
				segIt->vertices.clear();
				delete segIt;
			}
			modelIt->segmList.clear();

			delete modelIt;
		}
		m_models->clear();
		delete m_models;
	};

protected:
	QVector<Model*>* m_models;
	QFile m_file;
	QVector<Material>* m_materials;
	BoundingBox m_sceneBbox;
	QString m_filepath;

	virtual void import() = 0;

public:
	virtual QVector<Model*>* getModels() const { return m_models; };
	virtual QVector<Material>* getMaterials() const { return m_materials; };
	virtual BoundingBox getBoundingBox() const { return m_sceneBbox; };

	static Material* getDefaultMaterial() {
		Material* defMaterial = new Material;

		QImage img(1, 1, QImage::Format_RGB32);
		img.fill(Qt::red);

		QOpenGLTexture* new_texture = new QOpenGLTexture(img.mirrored());

		// Set nearest filtering mode for texture minification
		new_texture->setMinificationFilter(QOpenGLTexture::Nearest);

		// Set bilinear filtering mode for texture magnification
		new_texture->setMagnificationFilter(QOpenGLTexture::Linear);

		// Wrap texture coordinates by repeating
		// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
		new_texture->setWrapMode(QOpenGLTexture::Repeat);

		defMaterial->texture0 = new_texture;
		defMaterial->name = "Default Material";

		return defMaterial;
	};

};