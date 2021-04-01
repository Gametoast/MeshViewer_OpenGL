#include "..\Header\MshFile.h"
#include "..\Header\tga.h"
#include "..\Header\OutputDevice.h"
#include <QVector3D>


// helper function to save data from file to any variable type
#define F2V(variableName) reinterpret_cast<char*>(&variableName)


/////////////////////////////////////////////////////////////////////////
// public constructor/destructor

MshFile::MshFile(QString path)
	: FileInterface(path)
{
	import();
}

MshFile::~MshFile()
{
}


/////////////////////////////////////////////////////////////////////////
// private functions

void MshFile::import()
{
	// go to file size information
	m_file.seek(4);

	quint32 tmp_fileSize;
	QList<ChunkHeader*> tmp_mainChunks;

	// get all chunks under HEDR
	m_file.read(F2V(tmp_fileSize), sizeof(tmp_fileSize));
	loadChunks(tmp_mainChunks, m_file.pos(), tmp_fileSize);

	// evaulate HEDR subchunks (= find MSH2)
	for (ChunkHeader* it : tmp_mainChunks)
	{
		if ("MSH2" == it->name)
		{
			// get all subchunks
			QList<ChunkHeader*> tmp_msh2Chunks;
			loadChunks(tmp_msh2Chunks, it->position, it->size);

			// evaluate MSH2 subchunks
			analyseMsh2Chunks(tmp_msh2Chunks);

			// clean up
			while (!tmp_msh2Chunks.empty())
			{
				ChunkHeader* curs = tmp_msh2Chunks.front();
				tmp_msh2Chunks.pop_front();
				delete curs;
			}
		}
	}

	// clean up
	while (!tmp_mainChunks.empty())
	{
		ChunkHeader* cur = tmp_mainChunks.front();
		tmp_mainChunks.pop_front();
		delete cur;
	}
}

void MshFile::loadChunks(QList<ChunkHeader*>& destination, qint64 start, const quint32 length)
{
	// jump to first chunk
	m_file.seek(start);

	do
	{
		// out of file. Maybe a size information is corrupted
		if (m_file.atEnd() || m_file.error() != QFileDevice::NoError)
		{
			OutputDevice::getInstance()->print("WARNING: corrupted file. Trying to continue..", 1);
			m_file.unsetError();
			m_file.seek(0);
			break;
		}

		ChunkHeader* tmp_header = new ChunkHeader();

		// get information
		char tmpName[5] = { 0 };
		m_file.read(F2V(tmpName[0]), sizeof(tmpName) -1);
		tmp_header->name = QString(tmpName);
		m_file.read(F2V(tmp_header->size), sizeof(tmp_header->size));
		tmp_header->position = m_file.pos();

		// store information
		destination.push_back(tmp_header);

		// jump to next header
		m_file.seek(tmp_header->size + m_file.pos());

	} while (m_file.pos() - start != length);
}

void MshFile::analyseMsh2Chunks(QList<ChunkHeader*>& chunkList)
{
	for (auto& it : chunkList)
	{
		// scene information
		if ("SINF" == it->name)
		{
			// get SINF subchunks
			QList<ChunkHeader*> tmp_sinfChunks;
			loadChunks(tmp_sinfChunks, it->position, it->size);

			// evaluate SINF subchunks
			for (auto& it : tmp_sinfChunks)
			{
				if ("BBOX" == it->name)
				{
					m_file.seek(it->position);

					// read in the quaternion
					float tmp_quat[4];
					for (int i = 0; i < 4; i++)
						m_file.read(F2V(tmp_quat[i]), sizeof(float));

					m_sceneBbox.rotation.setX(tmp_quat[0]);
					m_sceneBbox.rotation.setY(tmp_quat[1]);
					m_sceneBbox.rotation.setZ(tmp_quat[2]);
					m_sceneBbox.rotation.setScalar(tmp_quat[3]);

					//read in the center
					for (int i = 0; i < 3; i++)
						m_file.read(F2V(m_sceneBbox.center[i]), sizeof(float));

					//read in the extents
					for (int i = 0; i < 3; i++)
						m_file.read(F2V(m_sceneBbox.extents[i]), sizeof(float));
				}
			}

			// clean up SINF subchunks
			for (ChunkHeader* it : tmp_sinfChunks)
				delete it;
		}

		// material list
		else if ("MATL" == it->name)
		{
			OutputDevice::getInstance()->print("loading materials..", 0);
			// "useless" information how many MATD follow, jump over it
			m_file.seek(it->position);
			m_file.seek(sizeof(quint32) + m_file.pos());

			// get all MATL subchunk
			QList<ChunkHeader*> tmp_matlChunks;
			loadChunks(tmp_matlChunks, m_file.pos(), it->size - 4);

			// evaluate MATL subchunks
			for (auto& it : tmp_matlChunks)
			{
				// This shouldn't be anything else than MATD
				if ("MATD" == it->name)
				{
					// get all subchunks from MATD
					QList<ChunkHeader*> tmp_matdChunks;
					loadChunks(tmp_matdChunks, it->position, it->size);

					m_materials->push_back(Material());

					// analyse MATD subchunks
					analyseMatdChunks(tmp_matdChunks);

					// clean up MATD subchunks
					while (!tmp_matdChunks.empty())
					{
						ChunkHeader* cur = tmp_matdChunks.front();
						tmp_matdChunks.pop_front();
						delete cur;
					}
				}
			}

			// clean up MATL subchunks
			while (!tmp_matlChunks.empty())
			{
				ChunkHeader* cur = tmp_matlChunks.front();
				tmp_matlChunks.pop_front();
				delete cur;
			}
		}

		// model
		else if ("MODL" == it->name)
		{
			OutputDevice::getInstance()->print("loading model..", 0);
			Model* new_model = new Model;
			m_currentType = ModelTyp::null;
			m_currentRenderFlag = -1;

			// get all MODL subchunks
			QList<ChunkHeader*> tmp_chunks;
			loadChunks(tmp_chunks, it->position, it->size);

			// evaluate MODL subchunks
			analyseModlChunks(new_model, tmp_chunks);

			//clean up MODL subchunks
			while (!tmp_chunks.empty())
			{
				ChunkHeader* cur = tmp_chunks.front();
				tmp_chunks.pop_front();
				delete cur;
			}

			// save Model data
			m_models->push_back(new_model);
		}
	}
}

void MshFile::analyseMatdChunks(QList<ChunkHeader*>& chunkList)
{
	for (auto& it : chunkList)
	{
		// name
		if ("NAME" == it->name)
		{
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			m_materials->back().name = buffer;
			delete[] buffer;
		}

		// data
		else if("DATA" == it->name)
		{
			m_file.seek(it->position);

			// diffuse
			for (unsigned int i = 0; i < 4; i++)
				m_file.read(F2V(m_materials->back().diffuseColor[i]), sizeof(float));

			// specular
			for (unsigned int i = 0; i < 4; i++)
				m_file.read(F2V(m_materials->back().specularColor[i]), sizeof(float));

			// ambient
			for (unsigned int i = 0; i < 4; i++)
				m_file.read(F2V(m_materials->back().ambientColor[i]), sizeof(float));

			// shininess
			m_file.read(F2V(m_materials->back().shininess), sizeof(float));
		}

		// attributes
		else if ("ATRB" == it->name)
		{
			// get pointer to current material
			Material* curMat = &m_materials->back();

			// read the attributes
			m_file.seek(it->position);
			quint8 flag;
			m_file.read(F2V(flag), sizeof(flag));
			m_file.read(F2V(curMat->rendertype), sizeof(quint8));
			m_file.read(F2V(curMat->dataValues[0]), sizeof(quint8));
			m_file.read(F2V(curMat->dataValues[1]), sizeof(quint8));

			// flags
			// 0: emissive
			// 1: glow
			// 2: single-sided transparency
			// 3: double-sided transparency
			// 4: hard-edged transparency
			// 5: per-pixel lighting
			// 6: additive transparency
			// 7: specular

			for (unsigned int i = 0; i < 8; i++)
				curMat->flags[i] = (quint8)(flag << (7 - i)) >> 7;

			curMat->transparent = curMat->flags[2] || curMat->flags[3] || curMat->flags[4] || curMat->flags[6] || curMat->rendertype == 4;
		}

		// texture 0
		else if ("TX0D" == it->name)
		{
			// get the texture name
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			m_materials->back().tx0d = buffer;
			delete[] buffer;

			// load the texture if the name is not empty
			if (!m_materials->back().tx0d.isEmpty())
				loadTexture(m_materials->back().texture0, m_filepath, m_materials->back().tx0d);
		}

		// texture 1
		else if ("TX1D" == it->name)
		{
			// get the texture name
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			m_materials->back().tx1d = buffer;
			delete[] buffer;

			if (!m_materials->back().tx1d.isEmpty())
				loadTexture(m_materials->back().texture1, m_filepath, m_materials->back().tx1d);
		}

		// texture 2
		else if ("TX2D" == it->name)
		{
			// get the texture name
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			m_materials->back().tx2d = buffer;
			delete[] buffer;
		}

		// texture 3
		else if ("TX3D" == it->name)
		{
			// get the texture name
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			m_materials->back().tx3d = buffer;
			delete[] buffer;
		}
	}
}

void MshFile::analyseModlChunks(Model * dataDestination, QList<ChunkHeader*>& chunkList)
{
	for (auto& it : chunkList)
	{
		// model type
		if ("MTYP" == it->name)
		{
			m_file.seek(it->position);
			quint32 tmp_type;
			m_file.read(F2V(tmp_type), sizeof(tmp_type));
			m_currentType = (ModelTyp)tmp_type;
		}

		// parent name
		else if ("PRNT" == it->name)
		{
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			dataDestination->parent = buffer;
			delete[] buffer;
		}

		// model name
		else if ("NAME" == it->name)
		{
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);
			dataDestination->name = buffer;
			delete[] buffer;
		}

		// render flags
		else if ("FLGS" == it->name)
		{
			m_file.seek(it->position);
			m_file.read(F2V(m_currentRenderFlag), sizeof(m_currentRenderFlag));
		}

		// translation
		else if ("TRAN" == it->name)
		{
			float tmp_scale[3];
			float tmp_rotation[4];
			float tmp_trans[3];

			m_file.seek(it->position);

			// read in the data
			for (int i = 0; i < 3; i++)
				m_file.read(F2V(tmp_scale[i]), sizeof(float));

			for (int i = 0; i < 4; i++)
				m_file.read(F2V(tmp_rotation[i]), sizeof(float));

			for (int i = 0; i < 3; i++)
				m_file.read(F2V(tmp_trans[i]), sizeof(float));

			// modify the matrix and quaternion
			dataDestination->m4x4Translation.scale(tmp_scale[0], tmp_scale[1], tmp_scale[2]);
			dataDestination->m4x4Translation.translate(tmp_trans[0], tmp_trans[1], tmp_trans[2]);
			dataDestination->quadRotation.setVector(QVector3D(tmp_rotation[0], tmp_rotation[1], tmp_rotation[2]));
			dataDestination->quadRotation.setScalar(tmp_rotation[3]);

			dataDestination->m4x4Translation = getParentMatrix(dataDestination->parent) * dataDestination->m4x4Translation;
			dataDestination->quadRotation = getParentRotation(dataDestination->parent) * dataDestination->quadRotation;
		}

		// geometry data
		else if ("GEOM" == it->name)
		{
			// don't get null, bone, shadowMesh and hidden mesh indices
			if (m_currentType == null || m_currentType == bone || m_currentType == shadowMesh || m_currentRenderFlag == 1)
				continue;

			// get all GEOM subchunks
			QList<ChunkHeader*> tmp_geomChunks;
			loadChunks(tmp_geomChunks, it->position, it->size);

			// evaluate GEOM subchunks
			analyseGeomChunks(dataDestination, tmp_geomChunks);

			// clean up GEOM subchunks
			while (!tmp_geomChunks.empty())
			{
				ChunkHeader* cur = tmp_geomChunks.front();
				tmp_geomChunks.pop_front();
				delete cur;
			}
		}
	}
}

void MshFile::analyseGeomChunks(Model * dataDestination, QList<ChunkHeader*>& chunkList)
{
	for (auto& it : chunkList)
	{
		// segment
		if ("SEGM" == it->name)
		{
			// get all SEGM subchunks
			QList<ChunkHeader*> tmp_segmChunks;
			loadChunks(tmp_segmChunks, it->position, it->size);

			// evaluate SEGM subchunks
			analyseSegmChunks(dataDestination, tmp_segmChunks);

			// clean up SEGM subchunk
			while (!tmp_segmChunks.empty())
			{
				ChunkHeader* cur = tmp_segmChunks.front();
				tmp_segmChunks.pop_front();
				delete cur;
			}
		}

		// cloth
		else if ("CLTH" == it->name)
		{
			// get all CLTH subchunks
			QList<ChunkHeader*> tmp_clthChunks;
			loadChunks(tmp_clthChunks, it->position, it->size);

			// evaluate CLTH subchunks
			analyseClthChunks(dataDestination, tmp_clthChunks);

			// clean up CLTH subchunks
			while (!tmp_clthChunks.empty())
			{
				ChunkHeader* cur = tmp_clthChunks.front();
				tmp_clthChunks.pop_front();
				delete cur;
			}
		}
	}
}

void MshFile::analyseSegmChunks(Model * dataDestination, QList<ChunkHeader*>& chunkList)
{
	Segment* new_segment = new Segment;

	for (auto& it : chunkList)
	{
		// material index
		if ("MATI" == it->name)
		{
			m_file.seek(it->position);
			m_file.read(F2V(new_segment->textureIndex), sizeof(new_segment->textureIndex));
		}

		// position list (vertex)
		else if ("POSL" == it->name)
		{
			readVertex(new_segment, it->position);
		}

		// normals
		else if ("NRML" == it->name)
		{
			quint32 tmp_size;
			m_file.seek(it->position);
			m_file.read(F2V(tmp_size), sizeof(tmp_size));

			if (tmp_size < (unsigned) new_segment->vertices.size())
			{
				OutputDevice::getInstance()->print("WARNING: too less normals " + QString::number(tmp_size) + " < " + QString::number(new_segment->vertices.size()), 1);

				for (unsigned int i = new_segment->vertices.size(); i != tmp_size; i--)
					for (unsigned int j = 0; j < 3; j++)
						new_segment->vertices[i - 1].vertexNormal[j] = 0;
			}
			else if (tmp_size > (unsigned) new_segment->vertices.size())
			{
				OutputDevice::getInstance()->print("WARNING: too many normals " + QString::number(tmp_size) + " > " + QString::number(new_segment->vertices.size()), 1);
				tmp_size = new_segment->vertices.size();
			}

			for (unsigned int i = 0; i < tmp_size; i++)
				for (unsigned int j = 0; j < 3; j++)
					m_file.read(F2V(new_segment->vertices[i].vertexNormal[j]), sizeof(float));

		}

		// uv
		else if ("UV0L" == it->name)
		{
			readUV(new_segment, it->position);
		}

		// polygons (indices into vertex/uv list)
		else if ("STRP" == it->name)
		{

			// jump to the data section and read the size;
			quint32 tmp_size;
			m_file.seek(it->position);
			m_file.read(F2V(tmp_size), sizeof(tmp_size));

			int highBitCount(0);
			QVector<GLuint> tmp_buffer;

			for (unsigned int i = 0; i < tmp_size; i++)
			{
				// ReadData
				quint16 tmp_value;
				m_file.read(F2V(tmp_value), sizeof(tmp_value));

				// Check if highbit is set
				if (tmp_value >> 15)
				{
					highBitCount++;
					// remove the high bit, to get the actually value
					tmp_value = (quint16(tmp_value << 1) >> 1);
				}

				// save data
				tmp_buffer.push_back((GLuint)tmp_value);

				// if the last 2 highBits are set, it was a new poly
				if (highBitCount == 2)
				{
					// reset highBitCount
					highBitCount = 0;

					if (tmp_buffer.size() == 5)
					{
						// calculate poylgon normal, tangent and bitangent
						QVector3D vec1, vec2, norm, tan, bi;
						QVector2D uv1, uv2;
						float f;

						vec1 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[1]].position;
						vec2 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[2]].position;
						uv1 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[1]].texCoord;
						uv2 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[2]].texCoord;
						f = 1.0f / (uv1.x() * uv2.y() - uv2.x() * uv1.y());

						norm = QVector3D::crossProduct(vec1, vec2).normalized();

						tan.setX(f * (uv2.y() * vec1.x() - uv1.y() * vec2.x()));
						tan.setY(f * (uv2.y() * vec1.y() - uv1.y() * vec2.y()));
						tan.setZ(f * (uv2.y() * vec1.z() - uv1.y() * vec2.z()));
						tan.normalize();

						bi.setX(f * (-uv2.x() * vec1.x() + uv1.x() * vec2.x()));
						bi.setY(f * (-uv2.x() * vec1.y() + uv1.x() * vec2.y()));
						bi.setZ(f * (-uv2.x() * vec1.z() + uv1.x() * vec2.z()));
						bi.normalize();

						for (int k = 0; k < 3; k++)
						{
							// polygon normal wasn't calculated before
							if (new_segment->vertices[tmp_buffer[k]].polygonNormal == QVector3D(0, 0, 0))
							{
								new_segment->vertices[tmp_buffer[k]].polygonNormal = norm;
								new_segment->vertices[tmp_buffer[k]].tangent = tan;
								new_segment->vertices[tmp_buffer[k]].bitangent = bi;

								new_segment->indices.push_back(tmp_buffer[k]);
							}
							// polygon normal already calculated so duplicate the vertex
							else 
							{
								new_segment->vertices.push_back(new_segment->vertices[tmp_buffer[k]]);
								new_segment->vertices.back().polygonNormal = norm;
								new_segment->vertices.back().tangent = tan;
								new_segment->vertices.back().bitangent = bi;
								new_segment->indices.push_back(new_segment->vertices.size() - 1);
							}
						}
						tmp_buffer.remove(0, 3);
					}
					else if (tmp_buffer.size() > 5)
					{
						unsigned int tmp_multiPolySize = tmp_buffer.size() - 2;

						// calculate poylgon normal, tangent and bitangent
						QVector3D vec1, vec2, norm, tan, bi;
						QVector2D uv1, uv2;
						float f;

						vec1 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[1]].position;
						vec2 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[2]].position;
						uv1 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[1]].texCoord;
						uv2 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[2]].texCoord;
						f = 1.0f / (uv1.x() * uv2.y() - uv2.x() * uv1.y());

						norm = QVector3D::crossProduct(vec1, vec2).normalized();

						tan.setX(f * (uv2.y() * vec1.x() - uv1.y() * vec2.x()));
						tan.setY(f * (uv2.y() * vec1.y() - uv1.y() * vec2.y()));
						tan.setZ(f * (uv2.y() * vec1.z() - uv1.y() * vec2.z()));
						tan.normalize();

						bi.setX(f * (-uv2.x() * vec1.x() + uv1.x() * vec2.x()));
						bi.setY(f * (-uv2.x() * vec1.y() + uv1.x() * vec2.y()));
						bi.setZ(f * (-uv2.x() * vec1.z() + uv1.x() * vec2.z()));
						bi.normalize();

						// for every triangle of the multi polygon..
						for (unsigned int tri = 0; tri < tmp_multiPolySize - 2; tri++)
						{
							// ..calculate the edge indices
							for (int triEdge = 0; triEdge < 3; triEdge++)
							{
								int curIndi = tmp_buffer[(tri + triEdge - ((tri % 2) * (triEdge - 1) * 2))];

								// polygon normal wasn't calculated before
								if (new_segment->vertices[curIndi].polygonNormal == QVector3D(0, 0, 0))
								{
									new_segment->vertices[curIndi].polygonNormal = norm;
									new_segment->vertices[curIndi].tangent = tan;
									new_segment->vertices[curIndi].bitangent = bi;
									new_segment->indices.push_back(curIndi);
								}
								// polygon normal already calculated so duplicate the vertex
								else
								{
									new_segment->vertices.push_back(new_segment->vertices[curIndi]);
									new_segment->vertices.back().polygonNormal = norm;
									new_segment->vertices.back().tangent = tan;
									new_segment->vertices.back().bitangent = bi;
									new_segment->indices.push_back(new_segment->vertices.size() - 1);
								}
							}
						}
						tmp_buffer.remove(0, tmp_multiPolySize);
					}

				}	// if 2 high bits are set

			}	// for all values

			// save the last polygon (no 2 high bit followed)
			if (tmp_buffer.size() == 3)
			{
				// calculate poylgon normal, tangent and bitangent
				QVector3D vec1, vec2, norm, tan, bi;
				QVector2D uv1, uv2;
				float f;

				vec1 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[1]].position;
				vec2 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[2]].position;
				uv1 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[1]].texCoord;
				uv2 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[2]].texCoord;
				f = 1.0f / (uv1.x() * uv2.y() - uv2.x() * uv1.y());

				norm = QVector3D::crossProduct(vec1, vec2).normalized();

				tan.setX(f * (uv2.y() * vec1.x() - uv1.y() * vec2.x()));
				tan.setY(f * (uv2.y() * vec1.y() - uv1.y() * vec2.y()));
				tan.setZ(f * (uv2.y() * vec1.z() - uv1.y() * vec2.z()));
				tan.normalize();

				bi.setX(f * (-uv2.x() * vec1.x() + uv1.x() * vec2.x()));
				bi.setY(f * (-uv2.x() * vec1.y() + uv1.x() * vec2.y()));
				bi.setZ(f * (-uv2.x() * vec1.z() + uv1.x() * vec2.z()));
				bi.normalize();

				for (int k = 0; k < 3; k++)
				{
					// polygon normal wasn't calculated before
					if (new_segment->vertices[tmp_buffer[k]].polygonNormal == QVector3D(0, 0, 0))
					{
						new_segment->vertices[tmp_buffer[k]].polygonNormal = norm;
						new_segment->vertices[tmp_buffer[k]].tangent = tan;
						new_segment->vertices[tmp_buffer[k]].bitangent = bi;

						new_segment->indices.push_back(tmp_buffer[k]);
					}
					// polygon normal already calculated so duplicate the vertex
					else
					{
						new_segment->vertices.push_back(new_segment->vertices[tmp_buffer[k]]);
						new_segment->vertices.back().polygonNormal = norm;
						new_segment->vertices.back().tangent = tan;
						new_segment->vertices.back().bitangent = bi;
						new_segment->indices.push_back(new_segment->vertices.size() - 1);
					}
				}
				tmp_buffer.remove(0, 3);
			}
			else if (tmp_buffer.size() > 3)
			{
				unsigned int tmp_multiPolySize = tmp_buffer.size();

				// calculate poylgon normal, tangent and bitangent
				QVector3D vec1, vec2, norm, tan, bi;
				QVector2D uv1, uv2;
				float f;

				vec1 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[1]].position;
				vec2 = new_segment->vertices[tmp_buffer[0]].position - new_segment->vertices[tmp_buffer[2]].position;
				uv1 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[1]].texCoord;
				uv2 = new_segment->vertices[tmp_buffer[0]].texCoord - new_segment->vertices[tmp_buffer[2]].texCoord;
				f = 1.0f / (uv1.x() * uv2.y() - uv2.x() * uv1.y());

				norm = QVector3D::crossProduct(vec1, vec2).normalized();

				tan.setX(f * (uv2.y() * vec1.x() - uv1.y() * vec2.x()));
				tan.setY(f * (uv2.y() * vec1.y() - uv1.y() * vec2.y()));
				tan.setZ(f * (uv2.y() * vec1.z() - uv1.y() * vec2.z()));
				tan.normalize();

				bi.setX(f * (-uv2.x() * vec1.x() + uv1.x() * vec2.x()));
				bi.setY(f * (-uv2.x() * vec1.y() + uv1.x() * vec2.y()));
				bi.setZ(f * (-uv2.x() * vec1.z() + uv1.x() * vec2.z()));
				bi.normalize();

				// for every triangle of the multi polygon..
				for (unsigned int tri = 0; tri < tmp_multiPolySize - 2; tri++)
				{
					// ..calculate the edge indices
					for (int triEdge = 0; triEdge < 3; triEdge++)
					{
						int curIndi = tmp_buffer[(tri + triEdge - ((tri % 2) * (triEdge - 1) * 2))];

						// polygon normal wasn't calculated before
						if (new_segment->vertices[curIndi].polygonNormal == QVector3D(0, 0, 0))
						{
							new_segment->vertices[curIndi].polygonNormal = norm;
							new_segment->vertices[curIndi].tangent = tan;
							new_segment->vertices[curIndi].bitangent = bi;
							new_segment->indices.push_back(curIndi);
						}
						// polygon normal already calculated so duplicate the vertex
						else
						{
							new_segment->vertices.push_back(new_segment->vertices[curIndi]);
							new_segment->vertices.back().polygonNormal = norm;
							new_segment->vertices.back().tangent = tan;
							new_segment->vertices.back().bitangent = bi;
							new_segment->indices.push_back(new_segment->vertices.size() - 1);
						}
					}
				}
			}
		}
	}

	dataDestination->segmList.push_back(new_segment);
}

void MshFile::analyseClthChunks(Model * dataDestination, QList<ChunkHeader*>& chunkList)
{
	Segment* new_segment = new Segment;

	for (auto& it : chunkList)
	{
		// texture name
		if ("CTEX" == it->name)
		{
			// read the texture name
			m_file.seek(it->position);
			char* buffer = new char[it->size + 1];
			*buffer = { 0 };
			m_file.read(buffer, it->size);

			
			m_materials->push_back(Material());
			m_materials->back().name = "Cloth Material";
			m_materials->back().tx0d = QString(buffer);

			m_materials->back().shininess = 10;

			if (!m_materials->back().tx0d.isEmpty())
				loadTexture(m_materials->back().texture0, m_filepath, m_materials->back().tx0d);

			new_segment->textureIndex = m_materials->size() - 1;

			delete[] buffer;
		}

		// position list (vertex)
		else if ("CPOS" == it->name)
		{
			readVertex(new_segment, it->position);
		}

		// uv 
		else if ("CUV0" == it->name)
		{
			readUV(new_segment, it->position);
		}

		// triangles (indices into vertex/uv list)
		else if ("CMSH" == it->name)
		{
			// jump to the data section and read the size;
			quint32 tmp_size;
			m_file.seek(it->position);
			m_file.read(F2V(tmp_size), sizeof(tmp_size));

			// for every triangle..
			for (unsigned int i = 0; i < tmp_size; i++)
			{
				quint32 tmp_value[3];
				for (unsigned int j = 0; j < 3; j++)
				{
					m_file.read(F2V(tmp_value[j]), sizeof(quint32));
					new_segment->indices.push_back((GLuint)tmp_value[j]);
				}

				QVector3D vec1, vec2, norm;

				vec1 = new_segment->vertices[new_segment->indices[i * 3]].position - new_segment->vertices[new_segment->indices[i * 3 + 1]].position;
				vec2 = new_segment->vertices[new_segment->indices[i * 3]].position - new_segment->vertices[new_segment->indices[i * 3 + 2]].position;
				norm = QVector3D::crossProduct(vec1, vec2);

				for (int k = 0; k < 3; k++)
				{
					new_segment->vertices[new_segment->indices[i * 3 + k]].vertexNormal += norm;
					new_segment->vertices[new_segment->indices[i * 3 + k]].vertexNormal.normalize();
				}
			}
		}
	}

	dataDestination->segmList.push_back(new_segment);
}

void MshFile::readVertex(Segment * dataDestination, qint64 position)
{
	quint32 tmp_size;
	m_file.seek(position);
	m_file.read(F2V(tmp_size), sizeof(tmp_size));

	for (unsigned int i = 0; i < tmp_size; i++)
	{
		float tmp[3];
		for (unsigned int j = 0; j < 3; j++)
			m_file.read(F2V(tmp[j]), sizeof(float));
		
		VertexData new_data;
		new_data.position = QVector3D(tmp[0], tmp[1], tmp[2]);

		dataDestination->vertices.push_back(new_data);
	}
}

void MshFile::readUV(Segment * dataDestination, qint64 position)
{
	quint32 tmp_size;
	m_file.seek(position);
	m_file.read(F2V(tmp_size), sizeof(tmp_size));

	if (tmp_size < (unsigned) dataDestination->vertices.size())
	{
		OutputDevice::getInstance()->print("WARNING: too less UVs " + QString::number(tmp_size) + " < " + QString::number(dataDestination->vertices.size()),1);

		for (unsigned int i = dataDestination->vertices.size(); i != tmp_size; i--)
			for (unsigned int j = 0; j < 2; j++)
				dataDestination->vertices[i - 1].texCoord[j] = 0;
	}
	else if (tmp_size > (unsigned) dataDestination->vertices.size())
	{
		OutputDevice::getInstance()->print("WARNING: too many UVs " + QString::number(tmp_size) + " > " + QString::number(dataDestination->vertices.size()), 1);
		tmp_size = dataDestination->vertices.size();
	}

	for (unsigned int i = 0; i < tmp_size; i++)
		for (unsigned int j = 0; j < 2; j++)
			m_file.read(F2V(dataDestination->vertices[i].texCoord[j]), sizeof(float));
}

void MshFile::loadTexture(QOpenGLTexture *& destination, QString filepath, QString& filename)
{
	bool loadSuccess(false);

	QImage img = loadTga(filepath + "/" + filename, loadSuccess);

	if (!loadSuccess)
	{
		OutputDevice::getInstance()->print("WARNING: texture not found or corrupted: " + filename, 1);
		//TODO: cloth use the wrong diffuse color. should be null
		img = QImage(1, 1, QImage::Format_RGB32);
		img.fill(QColor(m_materials->back().diffuseColor[0] * 255, m_materials->back().diffuseColor[1] * 255, m_materials->back().diffuseColor[2] * 255));
		filename += " *";
	}
	
	// Load image to OglTexture
	QOpenGLTexture* new_texture = new QOpenGLTexture(img.mirrored());

	// Set nearest filtering mode for texture minification
	new_texture->setMinificationFilter(QOpenGLTexture::Nearest);

	// Set bilinear filtering mode for texture magnification
	new_texture->setMagnificationFilter(QOpenGLTexture::Linear);

	// Wrap texture coordinates by repeating
	// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
	new_texture->setWrapMode(QOpenGLTexture::Repeat);

	destination = new_texture;
}

QMatrix4x4 MshFile::getParentMatrix(QString parent) const
{
	QMatrix4x4 matrix;

	for (auto& it : *m_models)
	{
		if (parent == it->name)
		{
			matrix = getParentMatrix(it->parent) * it->m4x4Translation;
			break;
		}
	}

	return matrix;
}

QQuaternion MshFile::getParentRotation(QString parent) const
{
	QQuaternion rotation;

	for (auto& it : *m_models)
	{
		if (parent == it->name)
		{
			rotation = getParentRotation(it->parent) * it->quadRotation;
			break;
		}
	}

	return rotation;
}
