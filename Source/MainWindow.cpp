#include "..\Header\MainWindow.h"
#include "..\Header\OglViewerWidget.h"
#include "..\Header\FileInterface.h"
#include "..\Header\OutputDevice.h"
#include "..\Header\SettingsManager.h"
#include <QSurfaceFormat>
#include <QSignalMapper>
#include <QToolButton>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPalette>
#include <QResizeEvent>
#include <QDirIterator>
 #include <QFileInfo>


#define WINDOW_NAME "Mesh Viewer"


/////////////////////////////////////////////////////////////////////////
// constructor/destructor

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass)
	, m_output(new QLabel(this))
	, m_curSeverity(0)
	, m_infoWindow(new FileInfoWindow(this))
{
	// setup window
	ui->setupUi(this);

	setWindowTitle(WINDOW_NAME);
	setWindowIcon(QIcon(":/images/icon.ico"));

	connect(OutputDevice::getInstance(this), &OutputDevice::sendMessage, this, &MainWindow::printMessage);
	connect(OutputDevice::getInstance(this), &OutputDevice::sendFileInfo, this, &MainWindow::setFileInfo);

	// setup opengl things
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	QSurfaceFormat::setDefaultFormat(format);

	// set default text to file info
	m_fileInfo = "Filename: -\nMaterials: -\nVertices: -\nTriangle: -<detail>No file is open";

	ui->dirDropDownList->addItem("BF1_ModTools", "C:/BF1_ModTools/Assets/Shipped Worlds/");
	ui->dirDropDownList->addItem("BF2_ModTools", "C:/BF2_ModTools/assets/Sides/");

	// add widgets to the window
	setupWidgets();

	// load stylesheet
	QFile styleSheet(":/files/StyleSheet.txt");
	styleSheet.open(QIODevice::ReadOnly);
	this->setStyleSheet(styleSheet.readAll());

	// setup dropdown
	setupAssetLibrary();
	connect(SettingsManager::getInstance(), &SettingsManager::dirsChanged, this, &MainWindow::setupAssetLibrary);

	printMessage("MeshViewer by Anakin", 0);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_output;
	delete m_infoWindow;
}

/////////////////////////////////////////////////////////////////////////
// functions
void MainWindow::setupWidgets()
{
	// Ogl Viewer
	OglViewerWidget* viewer = new OglViewerWidget(this);
	setCentralWidget(viewer);
	connect(this, &MainWindow::loadFile, viewer, &OglViewerWidget::loadFile);

	// open file
	QToolButton *openFileDialog = new QToolButton(this);
	openFileDialog->setObjectName("openFile");
	openFileDialog->setToolTip("open file");
	connect(openFileDialog, &QToolButton::pressed, this, &MainWindow::openFileDialog);
	ui->mainToolBar->addWidget(openFileDialog);

	// screenshot
	QToolButton *screenshot = new QToolButton(this);
	screenshot->setObjectName("screenshot");
	screenshot->setToolTip("take screenshot");
	connect(screenshot, &QToolButton::pressed, this, &MainWindow::takeScreenShot);
	ui->mainToolBar->addWidget(screenshot);

	//////////////////////////////////////////////////
	ui->mainToolBar->addSeparator();

	// Free Camera
	QToolButton *freeCamera = new QToolButton(this);
	freeCamera->setObjectName("freeCamera");
	freeCamera->setToolTip("free camera");
	connect(freeCamera, &QToolButton::pressed, viewer, &OglViewerWidget::useFreeCamera);
	ui->mainToolBar->addWidget(freeCamera);

	// Orbital Camera
	QToolButton *orbitCamera = new QToolButton(this);
	orbitCamera->setObjectName("orbitalCamera");
	orbitCamera->setToolTip("orbital camera");
	connect(orbitCamera, &QToolButton::pressed, viewer, &OglViewerWidget::useOrbitCamera);
	ui->mainToolBar->addWidget(orbitCamera);

	// Move Camera
	QToolButton *walkCamera = new QToolButton(this);
	walkCamera->setObjectName("walkCamera");
	walkCamera->setToolTip("walk camera");
	connect(walkCamera, &QToolButton::pressed, viewer, &OglViewerWidget::useMoveCamera);
	ui->mainToolBar->addWidget(walkCamera);

	//////////////////////////////////////////////////
	ui->mainToolBar->addSeparator();

	// wireframe
	QToolButton *wireframe = new QToolButton(this);
	wireframe->setObjectName("wireframe");
	wireframe->setToolTip("wireframe");
	wireframe->setCheckable(true);
	wireframe->setChecked(false);
	connect(wireframe, &QToolButton::pressed, viewer, &OglViewerWidget::toggleWireframe);
	ui->mainToolBar->addWidget(wireframe);

	// light
	QToolButton *light = new QToolButton(this);
	light->setObjectName("light");
	light->setToolTip("toggle light");
	light->setCheckable(true);
	light->setChecked(false);
	connect(light, &QToolButton::pressed, viewer, &OglViewerWidget::toggleLight);
	ui->mainToolBar->addWidget(light);

	// settings
	QToolButton *settings = new QToolButton(this);
	settings->setObjectName("settings");
	settings->setToolTip("settings");
	connect(settings, &QToolButton::pressed, viewer, &OglViewerWidget::showSettings);
	ui->mainToolBar->addWidget(settings);

	//////////////////////////////////////////////////
	ui->mainToolBar->addSeparator();

	// fileinfo
	QToolButton *fileInfo = new QToolButton(this);
	fileInfo->setObjectName("fileInfo");
	fileInfo->setToolTip("file info");
	connect(fileInfo, &QToolButton::pressed, m_infoWindow, &FileInfoWindow::show);
	ui->mainToolBar->addWidget(fileInfo);

	// help
	QToolButton *help = new QToolButton(this);
	help->setObjectName("help");
	help->setToolTip("help");
	connect(help, &QToolButton::pressed, this, &MainWindow::aboutTool);
	ui->mainToolBar->addWidget(help);

	// output on screen
	m_output->setObjectName("output");
	m_output->setAlignment(Qt::AlignTop);
	m_output->setText(m_fileInfo.left(m_fileInfo.indexOf("<detail>")));
	m_output->raise();
}

void MainWindow::updateAssetTree(QString path)
{

	ui->fileTreeWidget->clear();
	QDirIterator itterator(path, QStringList() << "*.msh" << "*.MSH" << "*.mesh" << "*.MESH",
												  QDir::Files, QDirIterator::Subdirectories);

	while (itterator.hasNext()) {
		QTreeWidgetItem* sub = new QTreeWidgetItem(ui->fileTreeWidget);
		sub->setData(0, Qt::DisplayRole, itterator.fileName());
		sub->setData(1, Qt::DisplayRole, itterator.fileInfo().absoluteFilePath());
		itterator.next();
	}
}

void MainWindow::openFileDialog()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Mesh (*.msh)");
	openFile(fileName);
}

void MainWindow::openFile(QString fileName)
{
	if(!fileName.isEmpty())
		emit loadFile(fileName);
}

void MainWindow::takeScreenShot()
{
	QString destination = QFileDialog::getSaveFileName(this, "Save as...", "", "PNG (*.png);; BMP (*.bmp);;TIFF (*.tiff, *.tif);;JPEG (*.jpg *jpeg)");

	OglViewerWidget* viewer = dynamic_cast<OglViewerWidget*>(centralWidget());
	if (!destination.isEmpty() && viewer != NULL)
		viewer->grab().save(destination);
}

void MainWindow::aboutTool()
{
	QFile file(":/files/about.txt");
	file.open(QIODevice::ReadOnly);
	QMessageBox* dialog = new QMessageBox(
		QMessageBox::Question,
		WINDOW_NAME,
		QString(file.readAll()),
		QMessageBox::StandardButton::Close,
		this,
		Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

	file.close();

	dialog->exec();

	delete dialog;
}

void MainWindow::resizeEvent(QResizeEvent * e)
{
	m_output->move(40, e->size().height() - 80);
}


/////////////////////////////////////////////////////////////////////////
// slots

void MainWindow::setupAssetLibrary()
{
	// get all directories and put them in the dropdown.
	while (ui->dirDropDownList->count() != 0)
		ui->dirDropDownList->removeItem(0);

	QStringList tmp_list = SettingsManager::getInstance()->getListOfDirs();
	for (QString &it : tmp_list)
		ui->dirDropDownList->addItem(it, it);

	// choose the current path and display it.
	if (ui->dirDropDownList->currentData().isValid())
		updateAssetTree(ui->dirDropDownList->currentData().toString());
}

void MainWindow::on_fileTreeWidget_doubleClicked()
{
	QString clickedFile = ui->fileTreeWidget->currentItem()->text(1);
	openFile(clickedFile);
}

void MainWindow::on_dirDropDownList_currentTextChanged(const QString &arg1)
{
	QString selectedDir;
	selectedDir = ui->dirDropDownList->itemData(ui->dirDropDownList->currentIndex()).toString();
	printMessage(arg1 + " : " + selectedDir, 0);
	updateAssetTree(selectedDir);
}

void MainWindow::printMessage(QString message, int severity)
{
	if (!ui->statusBar->currentMessage().isEmpty() && severity < m_curSeverity)
		return;

	m_curSeverity = severity;
	int time(0);
	QPalette palette;

	switch (severity)
	{
	case 1:
		time = 3000;
		palette.setColor(QPalette::WindowText, Qt::darkYellow);
		break;
	case 2:
		time = 3000;
		palette.setColor(QPalette::WindowText, Qt::red);
		break;
	case 0:
	default:
		time = 2000;
		palette.setColor(QPalette::WindowText, Qt::black);
		break;
	}

	ui->statusBar->setPalette(palette);
	ui->statusBar->showMessage(message, time);
}

void MainWindow::setFileInfo(QString name, QVector<Material>* materials, int vertices, int triangle)
{
	// save basic file information
	m_fileInfo = QByteArray("Filename: ");
	m_fileInfo += name;
	m_fileInfo += "\nMaterials: ";
	m_fileInfo += QByteArray::number(materials->size());
	m_fileInfo += "\nVertices: ";
	m_fileInfo += QByteArray::number(vertices);
	m_fileInfo += "\nTriangle: ";
	m_fileInfo += QByteArray::number(triangle);
	m_fileInfo += "<detail>";

	// add detailed information
	for (auto& it : *materials)
	{
		m_fileInfo += it.name;
		m_fileInfo += "\n";

		m_fileInfo += "TX0D:\t\t";
		if (it.tx0d.isEmpty())
			m_fileInfo += "-";
		else
			m_fileInfo += it.tx0d;
		m_fileInfo += "\n";

		m_fileInfo += "TX1D:\t\t";
		if (it.tx1d.isEmpty())
			m_fileInfo += "-";
		else
			m_fileInfo += it.tx1d;
		m_fileInfo += "\n";

		m_fileInfo += "TX2D:\t\t";
		if (it.tx2d.isEmpty())
			m_fileInfo += "-";
		else
			m_fileInfo += it.tx2d;
		m_fileInfo += "\n";

		m_fileInfo += "TX3D:\t\t";
		if (it.tx3d.isEmpty())
			m_fileInfo += "-";
		else
			m_fileInfo += it.tx3d;
		m_fileInfo += "\n";

		m_fileInfo += "Flags:\t\t";
		for (int i = 0; i < 8; i++)
		{
			if (it.flags[i])
				m_fileInfo += "1";
			else
				m_fileInfo += "0";
		}
		m_fileInfo += "\n";

		m_fileInfo += "Rendertype:\t";
		m_fileInfo += QByteArray::number(it.rendertype);
		m_fileInfo += "\n";

		m_fileInfo += "Gloss:\t";
		m_fileInfo += QByteArray::number(it.shininess);
		m_fileInfo += "\tData0:\t";
		m_fileInfo += QByteArray::number(it.dataValues[0]);
		m_fileInfo += "\tData1:\t";
		m_fileInfo += QByteArray::number(it.dataValues[1]);
		m_fileInfo += "\n";

		m_fileInfo += "Diffusecolor:\tR: ";
		m_fileInfo += QByteArray::number(it.diffuseColor.x());
		m_fileInfo += "\tG: ";
		m_fileInfo += QByteArray::number(it.diffuseColor.y());
		m_fileInfo += "\tB: ";
		m_fileInfo += QByteArray::number(it.diffuseColor.z());
		m_fileInfo += "\tA: ";
		m_fileInfo += QByteArray::number(it.diffuseColor.w());
		m_fileInfo += "\n";

		m_fileInfo += "Ambientcolor:\tR: ";
		m_fileInfo += QByteArray::number(it.ambientColor.x());
		m_fileInfo += "\tG: ";
		m_fileInfo += QByteArray::number(it.ambientColor.y());
		m_fileInfo += "\tB: ";
		m_fileInfo += QByteArray::number(it.ambientColor.z());
		m_fileInfo += "\tA: ";
		m_fileInfo += QByteArray::number(it.ambientColor.w());
		m_fileInfo += "\n";

		m_fileInfo += "Specularcolor:\tR: ";
		m_fileInfo += QByteArray::number(it.specularColor.x());
		m_fileInfo += "\tG: ";
		m_fileInfo += QByteArray::number(it.specularColor.y());
		m_fileInfo += " \tB: ";
		m_fileInfo += QByteArray::number(it.specularColor.z());
		m_fileInfo += " \tA: ";
		m_fileInfo += QByteArray::number(it.specularColor.w());
		m_fileInfo += "\n";

		m_fileInfo += "-----------------------------------------------------------------\n";
	}

	// print basic information on screen
	m_output->setText(m_fileInfo.left(m_fileInfo.indexOf("<detail>")));

	// print basic and detailed information on info window
	m_infoWindow->setBasicText(QString(m_fileInfo.left(m_fileInfo.indexOf("<detail>"))));
	m_infoWindow->setDetailText(QString(m_fileInfo.right(m_fileInfo.size() - m_fileInfo.indexOf("<detail>") - 8)));

}
