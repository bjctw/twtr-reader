/*
 * twtr_reader.cpp
 *
 *  Created on: Nov 4, 2011
 *      Author: ben
 */
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextBrowser>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "jsoncpp/json/reader.h"  //jsoncpp

#include "twtrReader.h"

//#define DEFAULT_URL "http://search.twitter.com/search.json?q=%23ps3&rpp=30"
#define DEFAULT_URL "http://search.twitter.com/search.json?q=%23agqr&rpp="
#define DEFAULT_TWEET (20)

twtrReader::twtrReader()
{
	setWindowFlags(Qt::Window);
	setGeometry(0,0,1280,1080);

	m_pManager = new QNetworkAccessManager();

	m_pVLayout = new QVBoxLayout(this);
	m_pHLayout1 = new QHBoxLayout();
	m_pHLayout2 = new QHBoxLayout();

	m_pUpdateButton = new QPushButton(this);
	m_pUpdateButton->setText("Update");

	m_pLineEditUrl = new QLineEdit(this);
	m_pLineEditTweet = new QLineEdit(this);
	QString strUrl;
	strUrl.setNum(DEFAULT_TWEET);
	m_pLineEditTweet->setText(strUrl);
	strUrl.setNum(DEFAULT_TWEET*3);
	strUrl = DEFAULT_URL + strUrl;
	m_pLineEditUrl->setText(strUrl);


	m_pSpinBox  = new QSpinBox(this);
	m_pSpinBox->setMaximum(9999);
	m_pSpinBox->setValue(0);

	QLabel *pLabelAutoUpdate = new QLabel(this);
	pLabelAutoUpdate->setText("Auto Update:");
	QLabel *pLabelTweets = new QLabel(this);
	pLabelTweets->setText("Tweets in row:");
	QLabel *pLabelSec = new QLabel(this);
	pLabelSec->setText("sec.");

	m_pHLayout1->addWidget(m_pUpdateButton);
	m_pHLayout1->addWidget(m_pLineEditUrl);
	m_pHLayout1->addWidget(pLabelTweets);
	m_pHLayout1->addWidget(m_pLineEditTweet);
	m_pHLayout1->addWidget(pLabelAutoUpdate);
	m_pHLayout1->addWidget(m_pSpinBox);
	m_pHLayout1->addWidget(pLabelSec);

	for(int i=0;i<3;i++)
	{
		m_textEditList.append(new QTextBrowser(this));

		////QTextBrowser attributes
		m_textEditList[i]->setOpenExternalLinks(true);
		QString sheet;
		sheet.append("a { text-decoration: none; }");
		m_textEditList[i]->document()->setDefaultStyleSheet(sheet);
		////
		m_pHLayout2->addWidget(m_textEditList[i]);
	}

	m_pVLayout->addLayout(m_pHLayout1,1);
	m_pVLayout->addLayout(m_pHLayout2,100);

	setLayout(m_pVLayout);

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateData()));
    //timer->start(60000);

    connect(m_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changInterval(int)));
	connect(m_pUpdateButton, SIGNAL(clicked()), this, SLOT(updateData()));

	connect(m_pManager, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replyFinished(QNetworkReply*)));
}

twtrReader::~twtrReader()
{

}

void twtrReader::updateData()
{
	/*
	QString str = "Test words RT @user http://google.com #agqr #2h";
	str = toHTML(str);
	m_textEditList[0]->setHtml(str);
	return;
	*/

	QNetworkRequest request;

	// get URL from widget
	QString urlStr = m_pLineEditUrl->text();
	if(m_lastUrl != urlStr)
	{
		m_lastUrl = urlStr;
		m_maxIdstr.clear();
	}
	else if(!m_maxIdstr.isEmpty())
		urlStr += "&since_id=" + m_maxIdstr;

	QUrl url;
	url.setEncodedUrl(urlStr.toUtf8());
	//request.setUrl(QUrl("http://search.twitter.com/search.json?q=%23sato_8&rpp=3&show_user=1"));
	request.setUrl(url);
	qDebug() << "Request URL:" << request.url() << "\n";

	//QNetworkReply *reply = m_pManager->get(request);
	m_pManager->get(request);
}

void twtrReader::replyFinished(QNetworkReply * reply)
{
	QString str;

	qDebug() << __FUNCTION__;
	qDebug() << "Error:" << reply->error();
	//qDebug() << reinterpret_cast<const char*> (reply->readAll());
	//qDebug() << reply->url();
	QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	qDebug() << "Redirect URL:" << possibleRedirectUrl.toUrl();
	qDebug() << "Head:" << (reply->header(QNetworkRequest::LocationHeader)).toString();
	QByteArray data = reply->readAll();
	//qDebug() << "Data:" << data;
	/* We'll deduct if the redirection is valid in the redirectUrl function */
	_urlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl(), _urlRedirectedTo);

	/* If the URL is not empty, we're being redirected. */
	if(!_urlRedirectedTo.isEmpty())
	{
		/* We'll do another request to the redirection url. */
		m_pManager->get(QNetworkRequest(_urlRedirectedTo));
	}
	else
	{
		/*
		 * We weren't redirected anymore
		 * so we arrived to the final destination...
		 */
		/* ...so this can be cleared. */
		_urlRedirectedTo.clear();

		processData(data);
	}

	//qDebug() << (reply->readAll()).data();
	//qDebug() << "\n\n";
	reply->deleteLater();

	if(reply->error() != QNetworkReply::NoError)
	{
		//emit transferComplete(SNS_ERR_NETWORK);
		qDebug() << "REPLY ERROR!!!!";
		return;
	}
}

QUrl twtrReader::redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const
{
	QUrl redirectUrl;
	/*
	 * Check if the URL is empty and
	 * that we aren't being fooled into a infinite redirect loop.
	 * We could also keep track of how many redirects we have been to
	 * and set a limit to it, but we'll leave that to you.
	 */
	if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl)
	{
		redirectUrl = possibleRedirectUrl;
	}
	return redirectUrl;
}

void twtrReader::processData(QByteArray& data)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	//QString str, temp;
	QString str;

	/*
	temp = dataOrigin;
	QByteArray data = temp.toUtf8();
	qDebug() <<"UTF8 >>" << data;
	*/

	//m_textEditList[0]->at(0)->setText(data);

	bool parsingSuccessful = reader.parse( data.constData(), root );
	if ( !parsingSuccessful )
	{
	    // report to the user the failure and their locations in the document.
	    str = reader.getFormatedErrorMessages().c_str();
	    qDebug() << "Failed to parse configuration\n" << str;
	    return;
	}

	m_maxIdstr = root.get("max_id_str", "").asString().c_str();
	unsigned int nResultSize = root["results"].size();

	qDebug() << "RESULT SIZE:" << nResultSize;

	if(nResultSize == 0)
	{
		qDebug() << "No update";
		return;
	}

	//get tweet number in each row from widget
	int nTweetNum = m_pLineEditTweet->text().toInt();
	if(nTweetNum == 0)
	{
		//convert failed
		qDebug() << "Error when get tweet number!";
		nTweetNum = DEFAULT_TWEET;
	}

	//reserve old text
	int nKeepBlocks = (int)(nTweetNum*3 - nResultSize)/nTweetNum + 1;
	QString strKeep;

	if(nKeepBlocks<0)
		nKeepBlocks = 0;

	for(int iter = 0; iter < nKeepBlocks; iter++)
		strKeep += m_textEditList[iter]->toPlainText();

	QStringList splitList = strKeep.split("\n", QString::SkipEmptyParts);


/*
	//shift old text
	if(nSize < 20 && nSize > 9)
	{
		//1->3
		m_textEditList[2]->setHtml(m_textEditList[0]->toHtml());
	}
	else if(nSize < 10)
	{
		//2->3
		m_textEditList[2]->setHtml(m_textEditList[1]->toHtml());
		//1->2
		m_textEditList[1]->setHtml(m_textEditList[0]->toHtml());
	}
*/

	int nUpdate = 0;
	unsigned int i;
	QString strName;

	for(i=0; i< nResultSize; i++)
	{
		//name
		str += "<b><font color=blue>";
		strName = root["results"][i].get("from_user","").asString().c_str();
		str += "<a href=\"http://twitter.com/";
		str += strName;
		str += "\">";
		str += strName;
		str += "</a>";
		str += "> ";
		str += "</font></b>";
		//text
		//str += QString::fromUtf8(root["results"][i].get("text","").asString().c_str());
		str += toHTML(QString::fromUtf8(root["results"][i].get("text","").asString().c_str()));
		str += "<hr>";
		str += "\n";	//for debug output

		if((i+1)%nTweetNum == 0)
		{
			//qDebug() << str;
			m_textEditList[nUpdate]->setHtml(str);
			nUpdate++;
			str.clear();
			if(nUpdate == 3)	//all rows are filled
			{
				i++;	//correct number
				break;
			}
		}
	}
/*
	if(i%10 != 0)
	{
		//remains
		qDebug() << str;
		m_textEditList[nUpdate]->setHtml(str);
	}
*/

	unsigned int nSplitSize = splitList.size();
	qDebug() << "SPLIT SIZE:" << nSplitSize;
	if((int)i < (nTweetNum*3))
	{
		str += "<font color=gray>";
		//add remains
		for(unsigned int j=0; j<(nTweetNum*3)-i; j++)
		{
			if(j < nSplitSize)
			{
				str += splitList.at(j);
				str += "<hr>";
				str += "\n";	//for debug output
			}

			if((i+j+1)%nTweetNum == 0)
			{
				str += "</font>";//"</color>";
				//qDebug() << str;
				m_textEditList[nUpdate]->setHtml(str);
				nUpdate++;
				str.clear();
				str += "<font color=gray>";
				//str += "<color=red>";
			}
		}
		//str += "</color>";
	}

	//if(!str.isEmpty())
	//	m_textEditList[nUpdate]->setHtml(str);

	//qDebug() << "PLAIN TEXT:" << m_textEditList[0]->toPlainText();

}

void twtrReader::changInterval(int i)
{
	if(i==0)
		m_pTimer->stop();
	else
		m_pTimer->start(i*1000);
}

QString twtrReader::toHTML(QString str)
{
	QString resultStr, tempStr;

	QStringList splitList = str.split(" ", QString::SkipEmptyParts);
	for(int i=0; i<splitList.size(); i++)
	{
		tempStr = splitList.at(i);
		if(tempStr.startsWith('@'))
		{
			//user name
			resultStr += "<font color=blue>";
			resultStr += "<a href=\"http://twitter.com/";
			resultStr += tempStr;
			resultStr += "\">";
			resultStr += tempStr;
			resultStr += "</a>";
			resultStr += "</font>";
			resultStr += " ";
		}
		else if(tempStr.startsWith('#'))
		{
			//sharp tag
			resultStr += "<font color=LightBlue>";
			resultStr += tempStr;
			resultStr += "</font>";
			resultStr += " ";
		}
		else if(tempStr.startsWith("http://"))
		{
			//TRL
			resultStr += "<font color=blue>";
			resultStr += "<a href=\"";
			resultStr += tempStr;
			resultStr += "\">";
			resultStr += tempStr;
			resultStr += "</a>";
			resultStr += "</font>";
			resultStr += " ";
		}
		else
		{
			resultStr += tempStr;
			resultStr += " ";
		}
	}

	//qDebug() << resultStr;

	return resultStr;
}

int main(int argc, char *argv[])
{
	int nRet;

	QApplication app(argc, argv);
	twtrReader *pReaderApp;
    pReaderApp = new twtrReader();
    pReaderApp->show();

	nRet = app.exec();

    //qDebug() << nRet;

	delete pReaderApp;

    return nRet;
}
