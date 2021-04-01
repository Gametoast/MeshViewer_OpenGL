#include "..\Header\SettingsWindow.h"
#include "..\Header\SettingsManager.h"
#include <QFileDialog>
#include <QListWidgetItem>

/////////////////////////////////////////////////////////////////////////
// constructor/destructor

SettingsWindow::SettingsWindow(QWidget * parent)
	: QWidget(parent)
	, ui(new Ui::SettingsWindow)
{
	ui->setupUi(this);

	setWindowFlags(Qt::Tool | Qt::NoDropShadowWindowHint);

	setupConnections();

	// set default values
	SettingsManager* sm = SettingsManager::getInstance(this);

	// set dirList for passing to Main and then fill settings manager dir list window
	QStringList tmp_directories = sm->getListOfDirs();
	for (auto &it : tmp_directories)
		ui->dirListWidget->insertItem(0, it);

	ui->lightOff_R_SB->setValue((int)(sm->getBgColorOff()[0]));
	ui->lightOff_G_SB->setValue((int)(sm->getBgColorOff()[1]));
	ui->lightOff_B_SB->setValue((int)(sm->getBgColorOff()[2]));

	ui->lightOn_R_SB->setValue((int)(sm->getBgColorOn()[0]));
	ui->lightOn_G_SB->setValue((int)(sm->getBgColorOn()[1]));
	ui->lightOn_B_SB->setValue((int)(sm->getBgColorOn()[2]));

	ui->light_R_SB->setValue((int)(sm->getLightColor()[0]));
	ui->light_G_SB->setValue((int)(sm->getLightColor()[1]));
	ui->light_B_SB->setValue((int)(sm->getLightColor()[2]));

	ui->ambCoef->setValue(sm->getAmbient());
	ui->attFac->setValue(sm->getAttenuation());

	ui->checkBackfaceCulling->setChecked(sm->isBfCulling());
	ui->checkAutoColor->setChecked(sm->isAutoColor());
	ui->checkHeadlight->setChecked(sm->isHeadlight());

	if (sm->getLightType() == 1)
		ui->radioDirectLight->setChecked(true);
	else
		ui->radioPointLight->setChecked(true);

	connect(this, &SettingsWindow::updateBGColorOff, sm, &SettingsManager::setBgColorOff);
	connect(this, &SettingsWindow::updateBGColorOn, sm, &SettingsManager::setBgColorOn);
	connect(this, &SettingsWindow::updateLightColor, sm, &SettingsManager::setLightColor);
	connect(this, &SettingsWindow::updateAttFac, sm, &SettingsManager::setAttenuation);
	connect(this, &SettingsWindow::updateAmbCoef, sm, &SettingsManager::setAmbient);
	connect(this, &SettingsWindow::sendHeadlight, sm, &SettingsManager::setHeadlight);
	connect(this, &SettingsWindow::sendBackfaceCulling, sm, &SettingsManager::setBfCulling);
	connect(ui->checkAutoColor, &QCheckBox::stateChanged, sm, &SettingsManager::setAutoColor);
	connect(this, &SettingsWindow::changeLightType, sm, &SettingsManager::setLightType);
	connect(this, &SettingsWindow::pathChanged, sm, &SettingsManager::updateDirectories);
}

SettingsWindow::~SettingsWindow()
{
	delete ui;
}


/////////////////////////////////////////////////////////////////////////
// functions

void SettingsWindow::setupConnections()
{
	// light off
	connect(ui->lightOff_R_SB, SIGNAL(valueChanged(int)), ui->lightOff_R_S, SLOT(setValue(int)));
	connect(ui->lightOff_R_S, SIGNAL(valueChanged(int)), ui->lightOff_R_SB, SLOT(setValue(int)));
	connect(ui->lightOff_R_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOffChanged);

	connect(ui->lightOff_G_SB, SIGNAL(valueChanged(int)), ui->lightOff_G_S, SLOT(setValue(int)));
	connect(ui->lightOff_G_S, SIGNAL(valueChanged(int)), ui->lightOff_G_SB, SLOT(setValue(int)));
	connect(ui->lightOff_G_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOffChanged);

	connect(ui->lightOff_B_SB, SIGNAL(valueChanged(int)), ui->lightOff_B_S, SLOT(setValue(int)));
	connect(ui->lightOff_B_S, SIGNAL(valueChanged(int)), ui->lightOff_B_SB, SLOT(setValue(int)));
	connect(ui->lightOff_B_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOffChanged);

	// light on
	connect(ui->lightOn_R_SB, SIGNAL(valueChanged(int)), ui->lightOn_R_S, SLOT(setValue(int)));
	connect(ui->lightOn_R_S, SIGNAL(valueChanged(int)), ui->lightOn_R_SB, SLOT(setValue(int)));
	connect(ui->lightOn_R_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOnChanged);

	connect(ui->lightOn_G_SB, SIGNAL(valueChanged(int)), ui->lightOn_G_S, SLOT(setValue(int)));
	connect(ui->lightOn_G_S, SIGNAL(valueChanged(int)), ui->lightOn_G_SB, SLOT(setValue(int)));
	connect(ui->lightOn_G_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOnChanged);

	connect(ui->lightOn_B_SB, SIGNAL(valueChanged(int)), ui->lightOn_B_S, SLOT(setValue(int)));
	connect(ui->lightOn_B_S, SIGNAL(valueChanged(int)), ui->lightOn_B_SB, SLOT(setValue(int)));
	connect(ui->lightOn_B_S, &QSlider::valueChanged, this, &SettingsWindow::backgroundColorOnChanged);

	// light
	connect(ui->light_R_SB, SIGNAL(valueChanged(int)), ui->light_R_S, SLOT(setValue(int)));
	connect(ui->light_R_S, SIGNAL(valueChanged(int)), ui->light_R_SB, SLOT(setValue(int)));
	connect(ui->light_R_S, &QSlider::valueChanged, this, &SettingsWindow::lightColorChanged);

	connect(ui->light_G_SB, SIGNAL(valueChanged(int)), ui->light_G_S, SLOT(setValue(int)));
	connect(ui->light_G_S, SIGNAL(valueChanged(int)), ui->light_G_SB, SLOT(setValue(int)));
	connect(ui->light_G_S, &QSlider::valueChanged, this, &SettingsWindow::lightColorChanged);

	connect(ui->light_B_SB, SIGNAL(valueChanged(int)), ui->light_B_S, SLOT(setValue(int)));
	connect(ui->light_B_S, SIGNAL(valueChanged(int)), ui->light_B_SB, SLOT(setValue(int)));
	connect(ui->light_B_S, &QSlider::valueChanged, this, &SettingsWindow::lightColorChanged);


	connect(ui->checkAutoColor, &QCheckBox::toggled, this, &SettingsWindow::autoColorToggled);
	connect(ui->radioDirectLight, &QRadioButton::toggled, this, &SettingsWindow::radioToggled);
	connect(ui->ambCoef, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value) {emit updateAmbCoef(value); });
	connect(ui->attFac, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value) {emit updateAttFac(value); });

	connect(ui->checkBackfaceCulling, &QCheckBox::toggled, [this]() {emit sendBackfaceCulling(ui->checkBackfaceCulling->isChecked()); });
	connect(ui->spinZSpeed, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value) {emit sendZommSpeed(value); });
	connect(ui->checkHeadlight, &QCheckBox::toggled, [this]() {emit sendHeadlight(ui->checkHeadlight->isChecked()); });
}


/////////////////////////////////////////////////////////////////////////
// slots

void SettingsWindow::autoColorToggled()
{
	if (!ui->checkAutoColor->isChecked())
	{
		ui->lightOn_R_SB->setEnabled(true);
		ui->lightOn_R_S->setEnabled(true);
		ui->lightOn_G_SB->setEnabled(true);
		ui->lightOn_G_S->setEnabled(true);
		ui->lightOn_B_SB->setEnabled(true);
		ui->lightOn_B_S->setEnabled(true);
	}
	else
	{
		ui->lightOn_R_SB->setEnabled(false);
		ui->lightOn_R_S->setEnabled(false);
		ui->lightOn_G_SB->setEnabled(false);
		ui->lightOn_G_S->setEnabled(false);
		ui->lightOn_B_SB->setEnabled(false);
		ui->lightOn_B_S->setEnabled(false);

		ui->lightOn_R_S->setValue((int)(ui->light_R_S->value() / 50));
		ui->lightOn_G_S->setValue((int)(ui->light_G_S->value() / 50));
		ui->lightOn_B_S->setValue((int)(ui->light_B_S->value() / 50));
	}
}

void SettingsWindow::radioToggled()
{
	if (ui->radioDirectLight->isChecked())
	{
		ui->attFac->setValue(0.0);
		ui->attFac->setEnabled(false);
		emit changeLightType(1);
	}
	else
	{
		ui->attFac->setEnabled(true);
		emit changeLightType(2);
	}
}

void SettingsWindow::backgroundColorOffChanged()
{
	emit updateBGColorOff(QVector3D(ui->lightOff_R_S->value(), ui->lightOff_G_S->value(), ui->lightOff_B_S->value()));
}

void SettingsWindow::backgroundColorOnChanged()
{
	emit updateBGColorOn(QVector3D(ui->lightOn_R_S->value(), ui->lightOn_G_S->value(), ui->lightOn_B_S->value()));
}

void SettingsWindow::lightColorChanged()
{
	emit updateLightColor(QVector3D(ui->light_R_S->value(), ui->light_G_S->value(), ui->light_B_S->value()));

	if (ui->checkAutoColor->isChecked())
	{
		ui->lightOn_R_S->setValue((int)(ui->light_R_S->value() / 50));
		ui->lightOn_G_S->setValue((int)(ui->light_G_S->value() / 50));
		ui->lightOn_B_S->setValue((int)(ui->light_B_S->value() / 50));
	}
}

void SettingsWindow::on_addItem_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",
														QFileDialog::ShowDirsOnly |
														QFileDialog::DontResolveSymlinks);
	if (!SettingsManager::getInstance()->getListOfDirs().contains(dirName))
	{
		ui->dirListWidget->insertItem(0, dirName);
		emit pathChanged(dirName);
	}
}

void SettingsWindow::on_removeItem_clicked()
{
	QListWidgetItem* tmp = ui->dirListWidget->takeItem(ui->dirListWidget->currentRow());
	emit pathChanged(tmp->text());
	delete tmp;
}
