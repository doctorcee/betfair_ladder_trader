#include "mainwindow.h"
#include <QJsonObject>
#include <QApplication>

int main(int argc, char *argv[])
{
    int display_theme = 0;
    const QString configfile = "config.json";
    QJsonObject program_config;

    bool file_error = true;
    QFile file(configfile);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        QString file_content = in.readAll();
        if (file_content.length() > 0)
        {
            QJsonParseError jerror;
            QJsonDocument jsonResponse = QJsonDocument::fromJson(file_content.toUtf8(),&jerror);
            if (jerror.error == QJsonParseError::NoError)
            {
                program_config = jsonResponse.object();
                file_error = false;
            }
        }
        file.close();
    }

    /*
    {
        TIniFile config_ini(configfile.toStdString());
        if (config_ini.isValid())
        {
            std::string str_error = "error";

            // FOR NOW JUST USE AUTO LOGIN - WE WILL USE OTHER FIELDS FOR GUI CONFIGURATION LATER
            std::string alfile = config_ini.getValue("general","al",str_error);
            if (alfile != str_error)
            {
                al = QString::fromStdString(alfile);
            }
        }
    }
    */

    QApplication a(argc, argv);
    MainWindow w(program_config, nullptr);

    if (w.darkMode())
    {
        a.setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window, QColor(53,53,53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(35,35,35));
        palette.setColor(QPalette::Dark, QColor(35,35,35));
        palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53,53,53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);

        palette.setColor(QPalette::Highlight, QColor(42,45,200).lighter());
        palette.setColor(QPalette::HighlightedText, Qt::white);
        a.setPalette(palette);
    }

    w.show();

    return a.exec();
}
