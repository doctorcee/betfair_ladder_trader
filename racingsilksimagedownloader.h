#ifndef RACINGSILKSIMAGEDOWNLOADER_H
#define RACINGSILKSIMAGEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QStringList>

class TRacingSilksImageDownloader : public QObject
{
    Q_OBJECT
public:
    TRacingSilksImageDownloader(QObject *parent = nullptr);
    virtual ~TRacingSilksImageDownloader();
    void setFileList(const std::map<QString,QString>& files);
    void downloadCurrentFileList();

signals:
    void silksDownloaded();

private:

    QNetworkAccessManager* m_netmanager;
    std::map<QString,QString> m_file_list;  // URL, file save name
    std::map<QString,QString>::iterator m_file_list_it;

private slots:

    void replyFinished(QNetworkReply *reply);
    void onReadyRead();
    void onReplyFinished();
};

#endif // RACINGSILKSIMAGEDOWNLOADER_H
