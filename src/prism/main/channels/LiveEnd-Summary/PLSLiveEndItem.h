#ifndef PLSLIVEENDITEM_H
#define PLSLIVEENDITEM_H

#include <QFrame>

namespace Ui {
class PLSLiveEndItem;
}

class PLSLiveEndItem : public QFrame {
	Q_OBJECT

public:
	explicit PLSLiveEndItem(const QString &uuid, QWidget *parent = nullptr);
	~PLSLiveEndItem();

	void combineTwoImage();
	void setupStatusWidget();

private:
	Ui::PLSLiveEndItem *ui;
	const QVariantMap &mSourceData;
	void setupData();
	QString PLSLiveEndItem::toThousandsNum(QString numString);
};

#endif // PLSLIVEENDITEM_H
