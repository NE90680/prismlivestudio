#include "about-view.hpp"
#include "ui_PLSAboutView.h"
#include "ui-config.h"

PLSAboutView::PLSAboutView(QWidget *parent) : PLSDialogView(parent), ui(new Ui::PLSAboutView)
{
	ui->setupUi(this->content());
	setFixedSize(342, 360);
	QString version = QString::asprintf("Current Version %s", PLS_VERSION);
	ui->versionLabel->setText(version);
	bool disabled = pls_is_living_or_recording();
	ui->checkUpdateButton->setDisabled(disabled);
	initConnect();
}

PLSAboutView::~PLSAboutView()
{
	delete ui;
}

void PLSAboutView::initConnect()
{
	connect(ui->checkUpdateButton, SIGNAL(clicked()), this, SLOT(on_checkUpdateButton_clicked()));
}

void PLSAboutView::on_checkUpdateButton_clicked()
{
	this->accept();
}
