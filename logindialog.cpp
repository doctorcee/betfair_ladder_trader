//====================================================================
// Filename: logindialog.cpp
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   Form used to log in to betfair
//====================================================================

#include "logindialog.h"
#include "ui_logindialog.h"
#include "utils/inifile.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

//====================================================================
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(saveData()));
    connect(ui->loadLoginFromFileButton,SIGNAL(clicked()),this,SLOT(loadDataFromIniFile()));
    ui->loadLoginFromFileButton->setFocus();

}

//====================================================================
LoginDialog::~LoginDialog()
{
    delete ui;
}

//====================================================================
void LoginDialog::loadDataFromIniFile()
{
    QString selected_file = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*)");
    if (selected_file.isEmpty() == false)
    {
        TIniFile login_ini(selected_file.toStdString());
        std::string str_error = "error";
        if (login_ini.isValid())
        {
            std::string uname = login_ini.getValue("general","un",str_error);
            if (uname != str_error)
            {
                ui->unameEdit->setText(QString::fromStdString(uname));
            }
            std::string pword = login_ini.getValue("general","pd",str_error);
            if (pword != str_error)
            {
                ui->passEdit->setText(QString::fromStdString(pword));
            }
            std::string certpath = login_ini.getValue("general","certpath",str_error);
            if (certpath != str_error)
            {
                ui->certEdit->setText(QString::fromStdString(certpath));
            }
            std::string keypath = login_ini.getValue("general","keypath",str_error);
            if (keypath != str_error)
            {
                ui->keyEdit->setText(QString::fromStdString(keypath));
            }
            std::string appkey = login_ini.getValue("general","appkey",str_error);
            if (appkey != str_error)
            {
                ui->appKeyEdit->setText(QString::fromStdString(appkey));
            }
        }
        else
        {
            QMessageBox Msgbox;
            Msgbox.setText("ERROR: The file " + selected_file + " is invalid!!!");
            Msgbox.exec();
        }
    }
}

//====================================================================
void LoginDialog::saveData()
{
    if (false == ui->unameEdit->text().isEmpty()
     && false == ui->passEdit->text().isEmpty()
     && false == ui->certEdit->text().isEmpty()
     && false == ui->keyEdit->text().isEmpty()
     && false == ui->appKeyEdit->text().isEmpty())
    {
        m_username = ui->unameEdit->text();
        m_password = ui->passEdit->text();
        m_cert_file = ui->certEdit->text();
        m_key_file = ui->keyEdit->text();
        m_app_key = ui->appKeyEdit->text();
        close();
        emit LoginDataChanged();
    }
}

