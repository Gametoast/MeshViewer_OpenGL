#pragma once
#include "..\Header\FileInterface.h"
#include <QList>

struct ChunkHeader {
	QString name;
	quint32 size;
	qint64 position;
};

enum ModelTyp {
	null,
	dynamicMesh,
	cloth,
	bone,
	staticMesh,
	shadowMesh = 6
};

class MshFile : public FileInterface
{
public:
	explicit MshFile(QString path);
	virtual ~MshFile();

private:
	ModelTyp m_currentType = ModelTyp::null;
	std::int32_t m_currentRenderFlag = -1;

	virtual void import() Q_DECL_OVERRIDE Q_DECL_FINAL;

	void loadChunks(QList<ChunkHeader*> &destination, qint64 start, const quint32 length);

	void analyseMsh2Chunks(QList<ChunkHeader*> &chunkList);

	void analyseMatdChunks(QList<ChunkHeader*> &chunkList);

	void analyseModlChunks(Model* dataDestination, QList<ChunkHeader*> &chunkList);
	void analyseGeomChunks(Model* dataDestination, QList<ChunkHeader*> &chunkList);
	void analyseSegmChunks(Model* dataDestination, QList<ChunkHeader*> &chunkList);
	void analyseClthChunks(Model* dataDestination, QList<ChunkHeader*> &chunkList);

	void readVertex(Segment* dataDestination, qint64 position);
	void readUV(Segment* dataDestination, qint64 position);

	void loadTexture(QOpenGLTexture*& destination, QString filepath, QString& filename);

	QMatrix4x4 getParentMatrix(QString parent) const;
	QQuaternion getParentRotation(QString parent) const;
};