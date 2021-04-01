#include "..\Header\SettingsManager.h"
#include "qdebug.h"


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

SettingsManager::SettingsManager(QObject *parent)
	: QObject(parent)
{
	m_file.setFileName("meshviewer.config");
	readFromFile();
}

SettingsManager::~SettingsManager()
{
	writeToFile();
}

SettingsManager* SettingsManager::getInstance(QObject *parent)
{
	static SettingsManager* instance = new SettingsManager(parent);
	return instance;
}


/////////////////////////////////////////////////////////////////////////
// functions

void SettingsManager::readFromFile()
{
	if (m_file.open(QIODevice::ReadOnly))
	{

		QByteArray stream = m_file.readAll();
		QList<QByteArray> lines = stream.split('\n');

		for (auto& it : lines)
		{
			if (it.startsWith("<bgOn>"))
			{
				QList<QByteArray> values = it.right(it.size() - it.indexOf('>') - 1).split(';');

				for (int i = 0; i < 3; i++)
					m_bgColorOn[i] = values[i].toFloat();
			}
			else if (it.startsWith("<bgOff>"))
			{
				QList<QByteArray> values = it.right(it.size() - it.indexOf('>') - 1).split(';');

				for (int i = 0; i < 3; i++)
					m_bgColorOff[i] = values[i].toFloat();
			}
			else if (it.startsWith("<liCo>"))
			{
				QList<QByteArray> values = it.right(it.size() - it.indexOf('>') - 1).split(';');

				for (int i = 0; i < 3; i++)
					m_lightColor[i] = values[i].toFloat();
			}
			else if (it.startsWith("<bfCul>"))
			{
				m_bfCulling = it.right(it.size() - it.indexOf('>') - 1).toInt();
			}
			else if (it.startsWith("<liOn>"))
			{
				m_light = it.right(it.size() - it.indexOf('>') - 1).toInt();
			}
			else if (it.startsWith("<heLi>"))
			{
				m_headlight = it.right(it.size() - it.indexOf('>') - 1).toInt();
			}
			else if (it.startsWith("<auCo>"))
			{
				m_autoColor = it.right(it.size() - it.indexOf('>') - 1).toInt();
			}
			else if (it.startsWith("<liTy>"))
			{
				m_lightType = it.right(it.size() - it.indexOf('>') - 1).toInt();
			}
			else if (it.startsWith("<atFa>"))
			{
				m_attenuation = it.right(it.size() - it.indexOf('>') - 1).toFloat();
			}
			else if (it.startsWith("<amCo>"))
			{
				m_ambient = it.right(it.size() - it.indexOf('>') - 1).toFloat();
			}
			else if (it.startsWith("<qkList>"))
			{
				m_listOfDirs = QString(it.right(it.size() - it.indexOf('>') - 1)).split(";");
			}

		}
		m_file.close();
	}
}

void SettingsManager::writeToFile()
{
	m_file.open(QIODevice::WriteOnly);

	m_file.write(QString("<bgOn>%1;%2;%3\n").arg(m_bgColorOn.x()).arg(m_bgColorOn.y()).arg(m_bgColorOn.z()).toUtf8());
	m_file.write(QString("<bgOff>%1;%2;%3\n").arg(m_bgColorOff.x()).arg(m_bgColorOff.y()).arg(m_bgColorOff.z()).toUtf8());
	m_file.write(QString("<liCo>%1;%2;%3\n").arg(m_lightColor.x()).arg(m_lightColor.y()).arg(m_lightColor.z()).toUtf8());

	m_file.write(QString("<bfCul>%1\n").arg(m_bfCulling).toUtf8());
	m_file.write(QString("<liOn>%1\n").arg(m_light).toUtf8());
	m_file.write(QString("<heLi>%1\n").arg(m_headlight).toUtf8());
	m_file.write(QString("<auCo>%1\n").arg(m_autoColor).toUtf8());

	m_file.write(QString("<liTy>%1\n").arg(m_lightType).toUtf8());
	m_file.write(QString("<atFa>%1\n").arg(m_attenuation).toUtf8());
	m_file.write(QString("<amCo>%1\n").arg(m_ambient).toUtf8());
	if(!m_listOfDirs.isEmpty())
		m_file.write(QString("<qkList>%1\n").arg(m_listOfDirs.join(";")).toUtf8());

	m_file.close();
}

// getter ///////////////////////////////////////////////////////////////

QStringList SettingsManager::getListOfDirs()
{
	return m_listOfDirs;
}

QVector3D SettingsManager::getBgColorOn() const
{
	return m_bgColorOn;
}

QVector3D SettingsManager::getBgColorOff() const
{
	return m_bgColorOff;
}

bool SettingsManager::isBfCulling() const
{
	return m_bfCulling;
}

bool SettingsManager::isLight() const
{
	return m_light;
}

int SettingsManager::getLightType() const
{
	return m_lightType;
}

QVector3D SettingsManager::getLightColor() const
{
	return m_lightColor;
}

float SettingsManager::getAttenuation() const
{
	return m_attenuation;
}

float SettingsManager::getAmbient() const
{
	return m_ambient;
}

bool SettingsManager::isHeadlight() const
{
	return m_headlight;
}

bool SettingsManager::isAutoColor() const
{
	return m_autoColor;
}


/////////////////////////////////////////////////////////////////////////
// slots

void SettingsManager::setBgColorOn(QVector3D value)
{
	m_bgColorOn = value;
}

void SettingsManager::setBgColorOff(QVector3D value)
{
	m_bgColorOff = value;
}

void SettingsManager::setBfCulling(bool value)
{
	m_bfCulling = value;
}

void SettingsManager::setLight(bool value)
{
	m_light = value;
}

void SettingsManager::setLightType(int value)
{
	m_lightType = value;
}

void SettingsManager::setLightColor(QVector3D value)
{
	m_lightColor = value;
}

void SettingsManager::setAttenuation(double value)
{
	m_attenuation = value;
}

void SettingsManager::setAmbient(double value)
{
	m_ambient = value;
}

void SettingsManager::setHeadlight(bool value)
{
	m_headlight = value;
}

void SettingsManager::setAutoColor(int value)
{
	if (value == 0)
		m_autoColor = false;
	else
		m_autoColor = true;
}

void SettingsManager::updateDirectories(QString path)
{
	if (m_listOfDirs.contains(path))
		m_listOfDirs.removeAll(path);
	else
		m_listOfDirs.append(path);

	emit dirsChanged();
}
