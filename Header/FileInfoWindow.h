#pragma once
#include <QWidget>
#include "ui_FileInfoWindow.h"

class FileInfoWindow : public QWidget
{
	Q_OBJECT

public:
	FileInfoWindow(QWidget *parent = Q_NULLPTR)
		: QWidget(parent)
		, ui(new Ui::FileInfoWindow)
	{
		ui->setupUi(this);
		setWindowFlags(Qt::Tool | Qt::NoDropShadowWindowHint);
		ui->scrollArea->widget()->setStyleSheet("background-color: #ffffff");
	};

	~FileInfoWindow() { delete ui; };

private:
	Ui::FileInfoWindow* ui;

public:
	void setBasicText(QString text) { ui->basic->setText(text); };
	void setDetailText(QString text) { ui->detail->setText(text); };
};