#include "PLSRtmpChannelView.h"
#include "ui_PLSRtmpChannelView.h"
#include <QComboBox>
#include "ChannelConst.h"
#include "ChannelCommonFunctions.h"
#include <QListView>
#include "ChannelConst.h"
#include <QUrl>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "LogPredefine.h"
#include "NetBaseInfo.h"
#include "pls-app.hpp"
#include <QRegularExpression>
#include <QValidator>
#include "combobox.hpp"
#include "pls-net-url.hpp"

using namespace ChannelData;

PLSRtmpChannelView::PLSRtmpChannelView(QVariantMap &oldData, QWidget *parent) : QDialog(parent), ui(new Ui::RtmpChannelView), mOldData(oldData), isEdit(false)
{
	initUi();
	setStyleSheetFromFile(":/Style/PLSRTMPChannelView.css", this);
	loadFromData(oldData);
}

PLSRtmpChannelView::~PLSRtmpChannelView()
{
	delete ui;
}

void PLSRtmpChannelView::initUi()
{
	ui->setupUi(this);
	setWindowFlag(Qt::FramelessWindowHint);
	languageChange();
	updateRtmpInfos();
	initCommbox();

	ui->UserPasswordEdit->installEventFilter(this);
	ui->StreamKeyEdit->installEventFilter(this);

	connect(ui->NameEdit, &QLineEdit::textEdited, this, &PLSRtmpChannelView::updateSaveBtnAvailable, Qt::QueuedConnection);
	connect(ui->StreamKeyEdit, &QLineEdit::textEdited, this, &PLSRtmpChannelView::updateSaveBtnAvailable, Qt::QueuedConnection);
	connect(ui->RTMPUrlEdit, &QLineEdit::textEdited, this, &PLSRtmpChannelView::updateSaveBtnAvailable, Qt::QueuedConnection);
	connect(ui->UserIDEdit, &QLineEdit::textEdited, this, &PLSRtmpChannelView::updateSaveBtnAvailable, Qt::QueuedConnection);
	connect(ui->UserPasswordEdit, &QLineEdit::textEdited, this, &PLSRtmpChannelView::updateSaveBtnAvailable, Qt::QueuedConnection);
}

QVariantMap PLSRtmpChannelView::SaveResult()
{
	auto tmpData = mOldData;
	tmpData[g_nickName] = ui->NameEdit->text();
	tmpData[g_channelRtmpUrl] = ui->RTMPUrlEdit->text();
	tmpData[g_streamKey] = ui->StreamKeyEdit->text();
	QString platfromName;

	if (ui->PlatformCombbox->currentIndex() == 0 || ui->PlatformCombbox->currentIndex() == ui->PlatformCombbox->count() - 1) {
		platfromName = CUSTOM_RTMP;
	} else {
		platfromName = ui->PlatformCombbox->currentText();
	}

	tmpData[g_channelName] = platfromName;

	QString userID = ui->UserIDEdit->text();
	tmpData[g_userID] = userID;
	QString password = ui->UserPasswordEdit->text();
	tmpData[g_password] = password;

	return tmpData;
}

void PLSRtmpChannelView::loadFromData(const QVariantMap &oldData)
{
	QString displayName = getInfo(oldData, g_nickName);
	if (displayName == SELECT) {

		return;
	}
	isEdit = true;
	ui->TitleLabel->setText(CHANNELS_TR(RTMPEdit));
	ui->NameEdit->setText(displayName);
	QString platform = getInfo(oldData, g_channelName);
	int index = ui->PlatformCombbox->findText(platform, Qt::MatchContains);
	ui->PlatformCombbox->setDisabled(true);

	if (index != -1) {
		QSignalBlocker blocker(ui->PlatformCombbox);
		ui->PlatformCombbox->setCurrentIndex(index);
	} else {
		QSignalBlocker blocker(ui->PlatformCombbox);
		ui->PlatformCombbox->setCurrentIndex(ui->PlatformCombbox->count() - 1);
	}
	ui->RTMPUrlEdit->setEnabled(false);
	QString rtmpUrl = getInfo(oldData, g_channelRtmpUrl);
	ui->RTMPUrlEdit->setText(rtmpUrl);
	QString streamKey = getInfo(oldData, g_streamKey);
	ui->StreamKeyEdit->setText(streamKey);

	QString userID = getInfo(oldData, g_userID);
	ui->UserIDEdit->setText(userID);
	QString password = getInfo(oldData, g_password);
	ui->UserPasswordEdit->setText(password);
}

void PLSRtmpChannelView::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		languageChange();
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
bool PLSRtmpChannelView::eventFilter(QObject *watched, QEvent *event)
{
	auto parent = dynamic_cast<QWidget *>(watched)->parentWidget();
	switch (event->type()) {
	case QEvent::FocusIn:
		parent->setProperty("isFocus", true);
		refreshStyle(parent);
		break;
	case QEvent::FocusOut:
		parent->setProperty("isFocus", false);
		refreshStyle(parent);
		break;
	default:
		break;
	}
	return false;
}
void PLSRtmpChannelView::on_SaveBtn_clicked()
{
	QSignalBlocker blocker(ui->NameEdit);
	verify();
	this->accept();
}
void PLSRtmpChannelView::on_CancelBtn_clicked()
{
	this->reject();
}
void PLSRtmpChannelView::on_StreamKeyVisible_toggled(bool isCheck)
{
	ui->StreamKeyEdit->setEchoMode(isCheck ? QLineEdit::Password : QLineEdit::Normal);
}
void PLSRtmpChannelView::on_PasswordVisible_toggled(bool isCheck)
{
	ui->UserPasswordEdit->setEchoMode(isCheck ? QLineEdit::Password : QLineEdit::Normal);
}

void PLSRtmpChannelView::on_RTMPUrlEdit_textChanged(const QString &rtmpUrl)
{

	if (ui->PlatformCombbox->currentIndex() == 0) {
		ui->PlatformCombbox->setCurrentText(CHANNELS_TR(UserInput));
	}
	if (ui->PlatformCombbox->currentText() == (ui->PlatformCombbox->count() - 1) && ui->NameEdit->text().isEmpty()) {
		QString nameStr = "CustomRTMP";
		int count = ui->PlatformCombbox->count();
		for (int i = 1; i < count - 1; ++i) {
			QString indexTxt = ui->PlatformCombbox->itemText(i);
			if (rtmpUrl.contains(indexTxt, Qt::CaseInsensitive)) {
				nameStr = indexTxt;
			}
		}
		ui->NameEdit->setText(nameStr);
		ui->NameEdit->setModified(false);
	}
	updateSaveBtnAvailable();
}

void PLSRtmpChannelView::on_PlatformCombbox_currentIndexChanged(const QString &platForm)
{
	if (ui->PlatformCombbox->currentIndex() == (ui->PlatformCombbox->count() - 1) || ui->PlatformCombbox->currentIndex() == 0) {

		ui->RTMPUrlEdit->setEnabled(true);

		if (!ui->NameEdit->isModified()) {
			ui->NameEdit->clear();
		}
		if (!ui->RTMPUrlEdit->isModified()) {
			ui->RTMPUrlEdit->clear();
		}
		updateSaveBtnAvailable();
		return;
	}

	ui->RTMPUrlEdit->setEnabled(false);
	if (ui->NameEdit->text().isEmpty() || !ui->NameEdit->isModified()) {
		ui->NameEdit->setText(platForm);
		ui->NameEdit->setModified(false);
	}

	auto retIte = mRtmps.find(platForm);
	if (retIte != mRtmps.end()) {
		ui->RTMPUrlEdit->setText(retIte.value());
		ui->RTMPUrlEdit->setModified(false);
	}
	updateSaveBtnAvailable();
}

void PLSRtmpChannelView::on_OpenLink_clicked()
{
	QString urlStr;
	QString langguage = App()->GetLocale();
	if (langguage.contains("en")) {
		urlStr = g_streamKeyPrismHelperEn;
	} else {
		urlStr = g_streamKeyPrismHelperKr;
	}

	if (!QDesktopServices::openUrl(urlStr)) {
		PRE_LOG(" error open url " + urlStr, ERROR);
	}
}

void PLSRtmpChannelView::updateSaveBtnAvailable()
{
	ui->SaveBtn->setEnabled(checkIsModified());
}
void PLSRtmpChannelView::languageChange()
{
	ui->UserIDEdit->setPlaceholderText(CHANNELS_TR(Optional));
	ui->UserPasswordEdit->setPlaceholderText(CHANNELS_TR(Optional));
}

void PLSRtmpChannelView::initCommbox()
{
	QStringList rtmpNames;
	rtmpNames += CHANNELS_TR(Select);
	ui->PlatformCombbox->addItems(rtmpNames << mPlatforms << CHANNELS_TR(UserInput));
	auto view = dynamic_cast<QListView *>(ui->PlatformCombbox->view());
	view->setRowHidden(0, true);
	ui->PlatformCombbox->setMaxVisibleItems(6);
	ui->PlatformCombbox->update();
}

void PLSRtmpChannelView::verify()
{
	const auto &infos = PLSCHANNELS_API->getAllChannelInfoReference();
	auto myName = ui->NameEdit->text();
	QRegularExpression regx("(-\\d+$)");
	auto isSameName = [=](const QVariantMap &info) {
		auto name = getInfo(info, g_nickName);
		if (isEdit) {
			auto uuid = getInfo(info, g_channelUUID);
			auto oldUUid = getInfo(mOldData, g_channelUUID);
			return name == myName && uuid != oldUUid;
		}
		bool ret = (name == myName);
		return ret;
	};
	static int countN = 1;
	auto ite = std::find_if(infos.cbegin(), infos.cend(), isSameName);
	if (ite != infos.end()) {
		myName.remove(regx);
		QString newName = myName + "-" + QString::number(countN) + QString("");
		if (newName.count() > ui->NameEdit->maxLength()) {
			return;
		}
		ui->NameEdit->setText(newName);
		++countN;
		verify();
	}
	countN = 1;
}

bool PLSRtmpChannelView::checkIsModified()
{
	if (ui->NameEdit->text().isEmpty() || ui->RTMPUrlEdit->text().isEmpty() || ui->StreamKeyEdit->text().isEmpty()) {
		return false;
	}
	if (!isRtmUrlRight()) {
		return false;
	}
	auto tmp = SaveResult();
	return tmp != mOldData;
}

void PLSRtmpChannelView::updateRtmpInfos()
{
	mRtmps = PLSCHANNELS_API->getRTMPInfos();
	mPlatforms = PLSCHANNELS_API->getRTMPsName();
}

bool PLSRtmpChannelView::isRtmUrlRight()
{
	QRegularExpression reg("^rtmp[s]?://\\w+");
	reg.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	auto matchRe = reg.match(ui->RTMPUrlEdit->text());
	return matchRe.hasMatch();
}
