//====================================================================
// Filename: logindialog.h
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   Form used to log in to betfair
//====================================================================

#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    QString GetUsername() const {return m_username;}
    QString GetPassword() const {return m_password;}
    QString GetCertFile() const {return m_cert_file;}
    QString GetKeyFile() const {return m_key_file;}
    QString GetAppKey() const {return m_app_key;}

public slots:
    void saveData();
    void loadDataFromIniFile();

signals:
    void LoginDataChanged();

private:
    Ui::LoginDialog *ui;
    QString m_username;
    QString m_password;
    QString m_cert_file;
    QString m_key_file;
    QString m_app_key;
    QString m_path_to_ini_file;


};

#endif // LOGINDIALOG_H
