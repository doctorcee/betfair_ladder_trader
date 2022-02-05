#-------------------------------------------------
#
# Project created by QtCreator 2017-01-08T17:03:16
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ladder_trader
TEMPLATE = app

SOURCES += main.cpp\
    utils/inifile.cpp \
    charting/individualselectionchartwrapperbase.cpp \
    charting/marketvolumechartwrapper.cpp \
    charting/vwapintegralchartwrapper.cpp \
    delegates/iphzladdermatchdelegate.cpp \
    mainwindow.cpp \
    marketmanager.cpp \
    logindialog.cpp \
    delegates/progressbardelegate.cpp \
    models/selectedmarketmodel.cpp \
    programsettingsdialog.cpp \
    racingsilksimagedownloader.cpp \
    runnerchartsdialog.cpp \
    models/betviewmodel.cpp \
    betfair/marketinfo.cpp \
    betfair/runnermetainfo.cpp \
    betfair/json_utils/qt/json_qt_utils.cpp \
    betfair/betfairmarket.cpp \
    betfair/marketsnapshot.cpp \
    betfair/runner.cpp \
    betfair/betfair_utils.cpp \
    betfair/bet.cpp \
    models/unmatchedbetviewmodel.cpp \
    charting/singlerunnerchartwrapper.cpp \
    runneranalysischartsdialog.cpp \
    models/ladderviewmodel.cpp \
    charting/volumechartwrapper.cpp \
    charting/abstractchartwrapper.cpp \
    models/inplayhzladderviewmodel.cpp \
    eventselectiondlg.cpp \
    charting/macdchartwrapper.cpp \
    betfair/betfairadvancedcandle.cpp \
    models/stakingboxmodel.cpp

HEADERS  += mainwindow.h \
    utils/inifile.h \
    charting/individualselectionchartwrapperbase.h \
    charting/marketvolumechartwrapper.h \
    charting/vwapintegralchartwrapper.h \
    delegates/iphzladdermatchdelegate.h \
    marketmanager.h \
    logindialog.h \
    delegates/progressbardelegate.h \
    models/selectedmarketmodel.h \
    programsettingsdialog.h \
    racingsilksimagedownloader.h \
    runnerchartsdialog.h \
    models/betviewmodel.h \
    betfair/marketinfo.h \
    betfair/runnermetainfo.h \
    betfair/json_utils/qt/json_qt_utils.h \
    betfair/betfairmarket.h \
    betfair/betfair_utils.h \
    betfair/marketsnapshot.h \
    betfair/runner.h \
    betfair/bet.h \
    models/unmatchedbetviewmodel.h \
    charting/singlerunnerchartwrapper.h \
    runneranalysischartsdialog.h \
    models/ladderviewmodel.h \
    charting/volumechartwrapper.h \
    charting/abstractchartwrapper.h \
    models/inplayhzladderviewmodel.h \
    eventselectiondlg.h \
    charting/macdchartwrapper.h \
    betfair/betfairadvancedcandle.h \
    models/stakingboxmodel.h

FORMS    += mainwindow.ui \
    logindialog.ui \
    programsettingsdialog.ui \
    runnerchartsdialog.ui \
    runneranalysischartsdialog.ui \
    eventselectiondlg.ui


CONFIG += c++11

RESOURCES += \
    application.qrc



