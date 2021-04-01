#include "..\Header\GeometryEngine.h"
#include "..\Header\MshFile.h"
#include "..\Header\OglViewerWidget.h"
#include "..\Header\MainWindow.h"
#include "..\Header\OutputDevice.h"
#include <QRegExp>


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

GeometryEngine::GeometryEngine(QObject *parent)
	: QObject(parent)
	, m_indexBuf(QOpenGLBuffer::IndexBuffer)
	, m_defaultMaterial(FileInterface::getDefaultMaterial())
{
	initializeOpenGLFunctions();
}

GeometryEngine::~GeometryEngine()
{
	clearData();
	delete m_defaultMaterial->texture0;
	delete m_defaultMaterial;
}


/////////////////////////////////////////////////////////////////////////
// functions

void GeometryEngine::clearData()
{
	if (m_arrayBuf.isCreated())
		m_arrayBuf.destroy();
	if (m_indexBuf.isCreated())
		m_indexBuf.destroy();

	if (m_materials != Q_NULLPTR)
	{
		for (auto it : *m_materials)
		{
			if (it.texture0 != Q_NULLPTR)
				delete it.texture0;

			if (it.texture1 != Q_NULLPTR)
				delete it.texture1;
		}
		m_materials->clear();
		delete m_materials;
	}
	m_drawList.clear();
}

void GeometryEngine::setupPipeline(QOpenGLShaderProgram *program)
{
	// Offset for position
	quintptr offset = 0;

	// Tell OpenGL programmable pipeline how to locate vertex position data
	int vertexLocation = program->attributeLocation("a_position");
	program->enableAttributeArray(vertexLocation);
	program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

	// Offset for texture coordinate
	offset += sizeof(QVector3D);

	// Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
	int texcoordLocation = program->attributeLocation("a_texcoord");
	program->enableAttributeArray(texcoordLocation);
	program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

	//Offset for vertexNormal
	offset += sizeof(QVector2D);

	// Tell OpenGL programmable pipeline how to locate vertex normal data
	int vertNormLocation = program->attributeLocation("a_normal");
	program->enableAttributeArray(vertNormLocation);
	program->setAttributeBuffer(vertNormLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

	//Offset for polygonNormal
	offset += sizeof(QVector3D);

	// Tell OpenGL programmable pipeline how to locate polygon normal data
	int polyNormLocation = program->attributeLocation("a_polyNorm");
	program->enableAttributeArray(polyNormLocation);
	program->setAttributeBuffer(polyNormLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

	//Offset for polygonTangent
	offset += sizeof(QVector3D);

	// Tell OpenGL programmable pipeline how to locate polygon tangent data
	int polyTanLocation = program->attributeLocation("a_polyTan");
	program->enableAttributeArray(polyTanLocation);
	program->setAttributeBuffer(polyTanLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

	//Offset for polygonBitangent
	offset += sizeof(QVector3D);

	// Tell OpenGL programmable pipeline how to locate polygon bitangent data
	int polyBiTanLocation = program->attributeLocation("a_polyBiTan");
	program->enableAttributeArray(polyBiTanLocation);
	program->setAttributeBuffer(polyBiTanLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

}

void GeometryEngine::drawGeometry(QOpenGLShaderProgram *program)
{
	if (!m_arrayBuf.isCreated() || !m_indexBuf.isCreated())
		return;

	// Setup
	// Tell OpenGL which VBOs to use
	m_arrayBuf.bind();
	m_indexBuf.bind();

	// Allways normalize by this
	QMatrix4x4 normMatrix;
	float maxExtent = std::max(std::max(m_boundings.extents[0], m_boundings.extents[1]), m_boundings.extents[2]);
	normMatrix.scale(1 / maxExtent);
	normMatrix.translate(-m_boundings.center[0], -m_boundings.center[1], -m_boundings.center[2]);
	program->setUniformValue("normalizeModel", normMatrix);

	// Allways use texture unit 0 and 1
	program->setUniformValue("tx0", 0);
	program->setUniformValue("tx1", 1);

	//setup the pipeline
	setupPipeline(program);
	
	// Paint

	for (auto& it : m_drawList)
	{
		bool tmp_transparent(false);
		bool tmp_specular(false);
		bool tmp_normalmap(false);
		bool tmp_glow(false);
		float shininess(0.0);
		QVector3D specularColor;

		// bind the correct texture
		if (it.textureIndex < (unsigned)m_materials->size() && m_materials->at(it.textureIndex).texture0 != Q_NULLPTR)
		{
			m_materials->at(it.textureIndex).texture0->bind(0);
			tmp_transparent = m_materials->at(it.textureIndex).transparent;
			tmp_specular = m_materials->at(it.textureIndex).flags[7];
			shininess = m_materials->at(it.textureIndex).shininess;
			specularColor = m_materials->at(it.textureIndex).specularColor.toVector3D();

			if (m_materials->at(it.textureIndex).rendertype == 27 || m_materials->at(it.textureIndex).rendertype == 28)
			{
				if (m_materials->at(it.textureIndex).texture1 != Q_NULLPTR)
				{
					tmp_normalmap = true;
					m_materials->at(it.textureIndex).texture1->bind(1);
				}
			}

			if (m_materials->at(it.textureIndex).flags[0] || m_materials->at(it.textureIndex).flags[1] || m_materials->at(it.textureIndex).rendertype == 1)
				tmp_glow = true;
		}
		else
		{
			m_defaultMaterial->texture0->bind(0);
			tmp_transparent = m_defaultMaterial->transparent;
		}

		// Set model matrix
		program->setUniformValue("modelMatrix", it.modelMatrix);

		// Set normal matrix
		program->setUniformValue("normalMatrix", (normMatrix * it.modelMatrix).normalMatrix());

		// set some material attributes
		program->setUniformValue("material.shininess", shininess);
		program->setUniformValue("material.specularColor", specularColor);
		program->setUniformValue("material.isTransparent", tmp_transparent);
		program->setUniformValue("material.hasSpecularmap", tmp_specular);
		program->setUniformValue("material.hasNormalmap", tmp_normalmap);
		program->setUniformValue("material.isGlow", tmp_glow);

		// Draw cube geometry using indices from VBO 1
		glDrawElements(GL_TRIANGLES, it.size, GL_UNSIGNED_INT, (void*)(it.offset * sizeof(GLuint)));

	}
}

void GeometryEngine::loadFile(QString filePath)
{
	// cleanup old stuff and recreate buffers
	clearData();
	m_arrayBuf.create();
	m_indexBuf.create();

	//reset view
	emit requestResetView();
	OutputDevice::getInstance()->print("loading file..", 0);

	try
	{
		QVector<Model*>* models;
		QVector<VertexData> vertexData;
		QVector<GLuint> indexData;

		// open file and get the information
		MshFile file(filePath);

		models = file.getModels();
		m_materials = file.getMaterials();
		m_boundings = file.getBoundingBox();

		// collect data
		unsigned int indexOffset(0);
		unsigned int vertexOffset(0);
		for (auto& modelIterator : *models)
		{
			for (auto& segmentIterator : modelIterator->segmList)
			{
				// get draw information
				DrawInformation new_info;
				new_info.offset = indexOffset;
				new_info.size = segmentIterator->indices.size();
				new_info.textureIndex = segmentIterator->textureIndex;
				new_info.modelMatrix = modelIterator->m4x4Translation;
				new_info.modelMatrix.rotate(modelIterator->quadRotation);

				// add offset to indices, no need to do it for the first one (maybe it's very big)
				if (vertexOffset != 0)
					for (auto& it : segmentIterator->indices)
						it += vertexOffset;

				// save data
				vertexData += segmentIterator->vertices;
				indexData += segmentIterator->indices;

				if (segmentIterator->textureIndex < (unsigned) m_materials->size() && m_materials->at(segmentIterator->textureIndex).transparent)
					m_drawList.push_back(new_info);
				else
					m_drawList.push_front(new_info);

				// update offset
				indexOffset += new_info.size;
				vertexOffset += segmentIterator->vertices.size();
			}
		}

		// Transfer vertex data to VBO 0
		m_arrayBuf.bind();
		m_arrayBuf.allocate(vertexData.data(), vertexData.size() * sizeof(VertexData));

		// Transfer index data to VBO 1
		m_indexBuf.bind();
		m_indexBuf.allocate(indexData.data(), indexData.size() * sizeof(GLuint));

		emit requestUpdate();
		OutputDevice::getInstance()->print("done..", 0);
		OutputDevice::getInstance()->setFileInfo(filePath.right(filePath.size() - filePath.lastIndexOf(QRegExp("/|\\\\")) - 1), m_materials, vertexData.size(), indexData.size() / 3);
	}
	catch (std::invalid_argument e)
	{
		clearData();
		OutputDevice::getInstance()->print(QString(e.what()), 2);
	}
}

