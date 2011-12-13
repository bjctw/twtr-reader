/*
 * twtrReader.h
 *
 *  Created on: Nov 4, 2011
 *      Author: ben
 */

#ifndef TWTRREADER_H_
#define TWTRREADER_H_

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QUrl>

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QNetworkAccessManager;
class QNetworkReply;
class QTextEdit;
class QLineEdit;
class QSpinBox;
class QTimer;

class twtrReader: public QWidget
{
Q_OBJECT

public:
	twtrReader();
	~twtrReader();

private slots:
	void updateData();
	void replyFinished(QNetworkReply * reply);
	void changInterval(int i);

private:
	QVBoxLayout *m_pVLayout;
	QHBoxLayout *m_pHLayout1;
	QHBoxLayout *m_pHLayout2;
	QList<QTextEdit*> m_textEditList;
	QPushButton *m_pUpdateButton;
	QNetworkAccessManager *m_pManager;
	QUrl _urlRedirectedTo;
	QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;
	void processData(QByteArray& data);
	QString m_maxIdstr;
	QString m_lastUrl;
	QLineEdit *m_pLineEditUrl;
	QLineEdit *m_pLineEditTweet;
	QSpinBox *m_pSpinBox;
	QTimer *m_pTimer;
};

#endif /* TWTRREADER_H_ */
