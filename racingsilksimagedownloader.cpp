#include "racingsilksimagedownloader.h"
#include <QDebug>
#include <QFileInfo>


//=========================================================================
TRacingSilksImageDownloader::TRacingSilksImageDownloader(QObject *parent)
    : QObject(parent),
      m_netmanager(nullptr)
{
    m_netmanager = new QNetworkAccessManager();
    QObject::connect(m_netmanager, SIGNAL(finished(QNetworkReply*)),
                        this,SLOT(replyFinished(QNetworkReply*)));
}

//====================================================================
TRacingSilksImageDownloader::~TRacingSilksImageDownloader()
{
    delete m_netmanager;
}

//====================================================================
void TRacingSilksImageDownloader::setFileList(const std::map<QString,QString>& files)
{
    m_file_list = files;
    m_file_list_it = m_file_list.begin();
}

//====================================================================
void TRacingSilksImageDownloader::downloadCurrentFileList()
{
    if (!m_file_list.empty())
    {
        m_file_list_it = m_file_list.begin();
        bool new_req = false;
        while (m_file_list_it != m_file_list.end())
        {
            QFileInfo finfo(m_file_list_it->second);
            if (finfo.exists())
            {
                ++m_file_list_it;
            }
            else
            {
                QString url = m_file_list_it->first;
                QNetworkRequest request;
                request.setUrl(url);
                m_netmanager->get(request);
                new_req = true;
                break;
            }
        }
        if (!new_req)
        {
            // files must be here
            //emit silksDownloaded();
        }
    }
}


//====================================================================
void TRacingSilksImageDownloader::onReadyRead()
{

}

//====================================================================
void TRacingSilksImageDownloader::onReplyFinished()
{

}


//====================================================================
void TRacingSilksImageDownloader::replyFinished(QNetworkReply *reply)
{
    QByteArray response_data = reply->readAll();

    QNetworkReply::NetworkError nerr = reply->error();
    if (nerr == QNetworkReply::NoError)
    {
        if (m_file_list_it != m_file_list.end())
        {
            QFile file(m_file_list_it->second);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(response_data);
                file.close();
            }           
            ++m_file_list_it;
        }
        while (m_file_list_it != m_file_list.end())
        {
            QFileInfo finfo(m_file_list_it->second);
            if (finfo.exists())
            {
                ++m_file_list_it;
            }
            else
            {
                QString url = m_file_list_it->first;
                QNetworkRequest request;
                request.setUrl(url);
                m_netmanager->get(request);
                break;
            }
        }
        if (m_file_list_it != m_file_list.end())
        {            
            emit silksDownloaded();
        }
    }
    else
    {
        qDebug() << reply->errorString();
    }
}
