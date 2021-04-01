#pragma once
#include <QObject>
#include <QFile>
#include <QVector3D>
#include <QVector4D>


class SettingsManager : public QObject
{
	Q_OBJECT

private:
	SettingsManager(QObject *parent = Q_NULLPTR);

public:
	SettingsManager(SettingsManager const&) = delete;
	void operator=(SettingsManager const&) = delete;

	~SettingsManager();

	static SettingsManager* getInstance(QObject *parent = Q_NULLPTR);

// attributes
private:
	QFile m_file;
	QStringList m_listOfDirs;

	QVector3D m_bgColorOn = { 5, 5, 5 };
	QVector3D m_bgColorOff = { 128, 204, 255 };
	QVector3D m_lightColor = { 255,255,255 };
	bool m_bfCulling = false;
	bool m_light = false;
	bool m_headlight = false;
	bool m_autoColor = true;
	float m_attenuation = 0.0f;
	float m_ambient = 0.005f;
	int m_lightType = 1;	// 1 = direct, 2 = point

// functions
private:
	void readFromFile();
	void writeToFile();

public:
	QVector3D getBgColorOn() const;
	QVector3D getBgColorOff() const;
	bool isBfCulling() const;
	bool isLight() const;

	QStringList getListOfDirs();

	int getLightType() const;
	QVector3D getLightColor() const;
	float getAttenuation() const;
	float getAmbient() const;
	bool isHeadlight() const;
	bool isAutoColor() const;


// slots
public:
	void setBgColorOn(QVector3D value);
	void setBgColorOff(QVector3D value);
	void setBfCulling(bool value);
	void setLight(bool value);

	void setLightType(int value);
	void setLightColor(QVector3D value);
	void setAttenuation(double value);
	void setAmbient(double value);
	void setHeadlight(bool value);
	void setAutoColor(int value);

	void updateDirectories(QString path);

// signals
signals:
	void dirsChanged();
};
