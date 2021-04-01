#pragma once
#include <QObject>

struct Material;

class OutputDevice : public QObject
{
	Q_OBJECT

private:
	OutputDevice(QObject *parent = Q_NULLPTR) : QObject(parent) {};

public:
	OutputDevice(OutputDevice const&) = delete;
	void operator=(OutputDevice const&) = delete;

	~OutputDevice() {};

	static OutputDevice* getInstance(QObject *parent = Q_NULLPTR) {
		static OutputDevice* instance = new OutputDevice(parent);
		return instance;
	};

	void print(QString message, int severity) { emit sendMessage(message, severity); };
	void setFileInfo(QString name, QVector<Material>* materials, int vertices, int triangle) {
		emit sendFileInfo(name, materials, vertices, triangle);
	};

signals:
	void sendMessage(QString message, int severity);
	void sendFileInfo(QString name, QVector<Material>* materials, int vertices, int triangle);
};