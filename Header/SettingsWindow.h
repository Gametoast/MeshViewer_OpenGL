#pragma once
#include <QWidget>
#include "ui_SettingsWindow.h"
#include <QVector3D>


class SettingsWindow : public QWidget
{
	Q_OBJECT

public:
	SettingsWindow(QWidget * parent = Q_NULLPTR);
	~SettingsWindow();

private:
	Ui::SettingsWindow* ui;

	void setupConnections();

private slots:
	void autoColorToggled();
	void radioToggled();
	void backgroundColorOffChanged();
	void backgroundColorOnChanged();
	void lightColorChanged();

	void on_addItem_clicked();
	void on_removeItem_clicked();

signals:
	void updateBGColorOff(QVector3D value);
	void updateBGColorOn(QVector3D value);
	void updateLightColor(QVector3D value);
	void updateAttFac(double value);
	void updateAmbCoef(double value);
	void sendHeadlight(bool value);
	void sendBackfaceCulling(bool value);
	void sendZommSpeed(int percent);
	void changeLightType(int value);
	void pathChanged(QString path);
	
};
