//====================================================================
// Filename: mainwindow.h
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   Main GUI window class
//====================================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include "logindialog.h"
#include "runnerchartsdialog.h"
#include "runneranalysischartsdialog.h"
#include "programsettingsdialog.h"
#include "eventselectiondlg.h"
#include "marketmanager.h"
#include "racingsilksimagedownloader.h"
#include "models/selectedmarketmodel.h"
#include "models/betviewmodel.h"
#include "models/unmatchedbetviewmodel.h"
#include "models/ladderviewmodel.h"
#include "models/inplayhzladderviewmodel.h"
#include "models/stakingboxmodel.h"
#include "charting/marketvolumechartwrapper.h"
#include "charting/vwapintegralchartwrapper.h"

#include "betfair/betfairmarket.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QJsonObject& json_settings,
                        const int& display_theme,
                        QWidget *parent = nullptr);
    ~MainWindow();

    bool darkMode() const {return m_display_theme == 1;}

public slots:
    void showLoginDialog();
    void showRunnerChartDialog();
    void showRunnerAnalysisDialog();
    void showEventSelectionDialog();
    void showProgramSettingsDialog();
    void onRefreshBalanceClicked();
    void onSelectedEventChange();
    void login();
    void onProgramSettingsChange();
    void selectMarket();
    void loadSilks();

    void processLogin();
    void processListMarketCatalogueResp();
    void processListMarketBookResp();
    void processPlaceBetsResp();
    void processListCurrentOrdersResp();
    void processCancelOrdersResp();
    void processMarketPAndLResp();

    void processNetworkError();
    void processGetAccountFunds();
    void refreshSelectedMarket();
    void refreshBets();

    void onGridViewClick(const QModelIndex &index);
    void onUnmatchedBetsViewClick(const QModelIndex &index);    

    void changeLadder1SelectedRunner(int index);
    void onCentreLadderOneClicked();
    void onLadderViewClick1(const QModelIndex &index);
    void onLadder1CancelBacksClicked();
    void onLadder1CancelLaysClicked();

    void changeLadder2SelectedRunner(int index);
    void onCentreLadderTwoClicked();
    void onLadderViewClick2(const QModelIndex &index);
    void onLadder2CancelBacksClicked();
    void onLadder2CancelLaysClicked();

    void changeLadder3SelectedRunner(int index);
    void onCentreLadderThreeClicked();
    void onLadderViewClick3(const QModelIndex &index);
    void onLadder3CancelBacksClicked();
    void onLadder3CancelLaysClicked();

    void changeLadder4SelectedRunner(int index);
    void onCentreLadderFourClicked();
    void onLadderViewClick4(const QModelIndex &index);
    void onLadder4CancelBacksClicked();
    void onLadder4CancelLaysClicked();


    void onClearEventLogClicked();

    void onIPViewClick(const QModelIndex &index);

    void onLadder1ShowChartClicked();
    void onLadder2ShowChartClicked();
    void onLadder3ShowChartClicked();
    void onLadder4ShowChartClicked();

    void onChangeLadder1Stake(const QModelIndex &index);
    void onChangeLadder2Stake(const QModelIndex &index);
    void onChangeLadder3Stake(const QModelIndex &index);
    void onChangeLadder4Stake(const QModelIndex &index);
    void onChangeGridViewStake(const QModelIndex &index);
    void onChangeInplayGridViewStake(const QModelIndex &index);


private:
    Ui::MainWindow *ui;
    const QString m_program_settings_file_path;
    QJsonObject m_program_settings;
    std::uint16_t m_display_theme;
    betfair::TBetfairMarket m_market;
    charting::TMarketVolumeChartWrapper my_market_vol_chart_wrapper;

    charting::TVWAPIntegralChartWrapper my_vwap_xo_1;
    charting::TVWAPIntegralChartWrapper my_vwap_xo_2;
    charting::TVWAPIntegralChartWrapper my_vwap_xo_3;
    charting::TVWAPIntegralChartWrapper my_vwap_xo_4;

    charting::TVWAPIntegralChartWrapper* my_vwap_xo_array[4];
    QHBoxLayout* my_ladder_util_layouts[4];
    LoginDialog my_login;
    RunnerChartsDialog my_runner_charts;
    TRunnerAnalysisChartsDialog my_runner_analysis_chart;
    TEventSelectionDlg my_event_selector;
    ProgramSettingsDialog my_program_settings;
    std::map<std::uint32_t, QString> m_price_offset_indices;
    std::map<QString, std::uint32_t> m_index_offset_by_price;
    std::vector<QString> m_v_odds_tick_vector;
    QString m_images_path;
    MarketManager bf_man;
    TRacingSilksImageDownloader m_silks_downloader;

    SelectedMarketModel my_market_model;
    InPlayHZLadderModel my_inplay_model;
    BetViewModel my_market_placed_bets_model;
    UnmatchedBetViewModel my_market_unmatched_bets_model;
    LadderViewModel m_ladder_views_one;
    LadderViewModel m_ladder_views_two;
    LadderViewModel m_ladder_views_three;
    LadderViewModel m_ladder_views_four;

    StakingBoxModel my_ladder_1_stake_model;
    StakingBoxModel my_ladder_2_stake_model;
    StakingBoxModel my_ladder_3_stake_model;
    StakingBoxModel my_ladder_4_stake_model;

    StakingBoxModel my_gridview_stake_model;
    StakingBoxModel my_inplay_gridview_stake_model;

    std::map<int, QString> m_market_list_index_map;

    QTimer* m_selected_market_update_timer;
    QTimer* m_update_bets_timer;

    QString m_bet_strategy_ref;

    std::uint32_t m_timer_counter;
    int m_wom_calculation_depth;
    bool m_bet_persistence_flag;
    std::uint16_t m_auto_offset_bet_placement_ticks;

    bool m_log_gui_update_times_to_file;
    std::vector<QString> m_gui_message_buffer;
    const QString m_software_version_num;

    double m_ladder_stake1;
    double m_ladder_stake2;
    double m_ladder_stake3;
    double m_ladder_stake4;
    double m_gridview_stake;
    double m_inplay_gridview_stake;

    bool m_inplay_flash_flag;

    bool m_gridview_betting_enabled;
    bool m_ladderview_betting_enabled;
    bool m_inplayview_betting_enabled;

    QString mapOddsBySignedOffset(const QString& str_odds, int tick_offset);

    void updateTimeToMarketStart();
    void updateMarketDetailsGroupBox();
    void updateSelectionComboBox(QComboBox* selbox);

    void updateLadder1();
    void updateLadder2();
    void updateLadder3();
    void updateLadder4();

    void updateLadderModels();
    void layAllRunnersAtFixedOdds(const double& odds,
                                  const double& stake);

    void initialiseLadderUtilityCharts();

    void updateChartDialogs();
    void resetChartDialogs();

    void updateModels();
    void updateMarketModel();
    void updatePlacesdBetModels();
    void updateInPlayModel();
    void resetModels();

    void cancelLadderBets(LadderViewModel& ladder,
                          bool lays);
    void cancelLadderBetsAtPrice(LadderViewModel& ladder,
                                 const double& price_to_cancel,
                                 bool lays);
    void ladderViewClicked(LadderViewModel& ladded, const double& stake, const QModelIndex& index);
    void changeLadderSelectedRunner(QComboBox* cb, LadderViewModel& ladder, QTableView* lv, int index);
    void zoomLadderToLPM(LadderViewModel& ladder, QTableView* lv);
    void updateLadder(QTableView* lv, LadderViewModel& ladder,
                      QLabel* pandl, QLabel* bwom_label,
                      QLabel* lwom_label, QLabel* total_vol_label,
                      charting::TVWAPIntegralChartWrapper* vwap_chart);


    void initialiseLadderStakeComboBox(QComboBox* selbox);

    void logEvent(const QString& msg);


    void logGUIUpdateMsg(const QString& msg);
    void writeGUIUpdateMsgBufferToFile();
    void saveProgramSettings();
    void updateProgramSettings();

    void setComboBoxIcon(QComboBox* combo,
                         const QString& runnername,
                         const QPixmap& ico);


};

#endif // MAINWINDOW_H
