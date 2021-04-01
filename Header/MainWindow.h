#pragma once
#include <QtWidgets/QMainWindow>
#include <QWidget>
#include "ui_MainWindow.h"
#include "FileInfoWindow.h"
#include <QByteArray>
#include <QLabel>

struct Material;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

// attributes
private:
	Ui::MainWindowClass* ui;
	
	QByteArray m_fileInfo;
	QLabel* m_output;
	int m_curSeverity;

	FileInfoWindow* m_infoWindow;
	QStringList m_filters;

// functions
private:
	void setupWidgets();
	void updateAssetTree(QString);
	void openFileDialog();
	void openFile(QString);
	void takeScreenShot();
	void aboutTool();

protected:
	virtual void resizeEvent(QResizeEvent * e) Q_DECL_OVERRIDE;

// slots
public slots:
	void printMessage(QString message, int severity);
	void setFileInfo(QString name, QVector<Material>* materials, int vertices, int triangle);

// private slots
private slots:
	void setupAssetLibrary();
	void on_fileTreeWidget_doubleClicked();
	void on_dirDropDownList_currentTextChanged(const QString &arg1);

// signals
signals:
	void loadFile(QString);
};
