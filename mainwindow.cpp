//====================================================================
// Filename: mainwindow.cpp
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   Main GUI window class
//====================================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <QDebug>
#include <QProgressBar>

#include "delegates/progressbardelegate.h"
#include "delegates/iphzladdermatchdelegate.h"
#include "utils/inifile.h"


static const int betviews_row_height = 18;
static const QString gui_update_log = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_gui_update_timing_log.txt";
static const QString betfair_url_base = "https://content-cache.cdnbf.net/feeds_images/Horses/SilkColours/";

/*
//====================================================================
static QString scramble(int x, int y, QString target)
{
    QString rval = target;
    for (int i = 0; i < rval.length(); ++i)
    {
        char csz = rval.at(i).toLatin1();
        if (i%x == 0)
        {
            for (int j = 0; j < y; ++j)
            {
                csz++;
            }
        }
        rval.replace(i,1,QChar(csz));
    }
    return rval;
}
*/

//====================================================================
static qint64 secondsUntil(const QDateTime& target)
{
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.offsetFromUtc());
    return now.secsTo(target);
}

//====================================================================
static QString timeToStart(const qint64& secs_to_go)
{
    qint64 abs_secs_to_go = secs_to_go < 0 ? -secs_to_go : secs_to_go;
    const qint64 hours_to_go = abs_secs_to_go / 3600;
    const qint64 mins_to_go = (abs_secs_to_go - (3600 * hours_to_go))/60;
    const qint64 secs_left = abs_secs_to_go - ((3600 * hours_to_go) + (60 * mins_to_go));
    QString time_to_go = (secs_to_go < 0) ? "  -" : "  ";
    if (hours_to_go < 10)
    {
        time_to_go += "0";
    }
    time_to_go += (QString::number(hours_to_go) + ":");
    if (mins_to_go < 10)
    {
        time_to_go += "0";
    }
    time_to_go += (QString::number(mins_to_go) + ":");
    if (secs_left < 10)
    {
        time_to_go += "0";
    }
    time_to_go += QString::number(secs_left) + "  ";
    return time_to_go;
}



//====================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_display_theme(0),
    my_market_vol_chart_wrapper(this,m_display_theme),
    my_vwap_xo_1(this,m_display_theme),
    my_vwap_xo_2(this,m_display_theme),
    my_vwap_xo_3(this,m_display_theme),
    my_vwap_xo_4(this,m_display_theme),
    my_vwap_xo_array{&my_vwap_xo_1,&my_vwap_xo_2,&my_vwap_xo_3,&my_vwap_xo_4},
    my_ladder_util_layouts{nullptr,nullptr,nullptr,nullptr},
    my_runner_charts(m_market,m_display_theme),
    my_runner_analysis_chart(m_market,m_display_theme),
    my_program_settings(),
    m_price_offset_indices(betfair::utils::generateOddsMap()),
    m_index_offset_by_price(betfair::utils::generateOffsetMap(m_price_offset_indices)),
    m_v_odds_tick_vector(betfair::utils::generateOddsTickVector()),
    m_images_path("silks_images"),
    bf_man(m_market),
    my_market_model(nullptr,m_market,m_images_path,m_display_theme),
    my_inplay_model(nullptr,m_market,m_images_path,m_display_theme),
    my_market_placed_bets_model(nullptr,m_market,m_display_theme),
    my_market_unmatched_bets_model(nullptr,m_market,m_display_theme),
    m_ladder_views_one(nullptr,m_market,m_display_theme),
    m_ladder_views_two(nullptr,m_market,m_display_theme),
    m_ladder_views_three(nullptr,m_market,m_display_theme),
    m_ladder_views_four(nullptr,m_market,m_display_theme),
    my_ladder_1_stake_model(nullptr,8,m_display_theme),
    my_ladder_2_stake_model(nullptr,8,m_display_theme),
    my_ladder_3_stake_model(nullptr,8,m_display_theme),
    my_ladder_4_stake_model(nullptr,8,m_display_theme),
    my_gridview_stake_model(nullptr,8,m_display_theme),
    my_inplay_gridview_stake_model(nullptr,8,m_display_theme),
    m_selected_market_update_timer(nullptr),
    m_update_bets_timer(nullptr),
    m_timer_counter(0),
    m_wom_calculation_depth(5),
    m_bet_persistence_flag(false),
    m_auto_offset_bet_placement_ticks(0),
    m_log_gui_update_times_to_file(false),
    m_software_version_num("1.0.1"),
    m_ladder_stake1(betfair::utils::min_betting_stake),
    m_ladder_stake2(betfair::utils::min_betting_stake),
    m_ladder_stake3(betfair::utils::min_betting_stake),
    m_ladder_stake4(betfair::utils::min_betting_stake),
    m_gridview_stake(betfair::utils::min_betting_stake),
    m_inplay_gridview_stake(betfair::utils::min_betting_stake),
    m_inplay_flash_flag(false),
    m_gridview_betting_enabled(false),
    m_ladderview_betting_enabled(false),
    m_inplayview_betting_enabled(false)
{
    ui->setupUi(this);
    // Read info in from config file (if there is one - if not maintain defaults)
    const QString configfile = "config.dat";
    QString al = "";

    if (!QDir(m_images_path).exists())
    {
        QDir().mkpath(m_images_path);
    }

    my_ladder_1_stake_model.changeBaseStake(betfair::utils::min_betting_stake);
    my_ladder_2_stake_model.changeBaseStake(betfair::utils::min_betting_stake);
    my_ladder_3_stake_model.changeBaseStake(betfair::utils::min_betting_stake);
    my_ladder_4_stake_model.changeBaseStake(betfair::utils::min_betting_stake);
    my_gridview_stake_model.changeBaseStake(betfair::utils::min_betting_stake);
    my_inplay_gridview_stake_model.changeBaseStake(betfair::utils::min_betting_stake);

    QFileInfo configinfo(configfile);
    if (configinfo.exists())
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



    if (m_display_theme == 1)
    {
        my_program_settings.applyDefaultDarkTheme();
    }


    m_selected_market_update_timer = new QTimer(this);
    m_update_bets_timer = new QTimer(this);
    ui->mainTabWidget->setCurrentIndex(0);

    my_ladder_util_layouts[0] = ui->ladder1UtilChartLayout;
    my_ladder_util_layouts[1] = ui->ladder2UtilChartLayout;
    my_ladder_util_layouts[2] = ui->ladder3UtilChartLayout;
    my_ladder_util_layouts[3] = ui->ladder4UtilChartLayout;

    QDoubleValidator* lay_odds_validator = new QDoubleValidator(1.01,2.00, 2, this);
    lay_odds_validator->setNotation(QDoubleValidator::StandardNotation);
    ui->marketCrossMatchedLabel->setVisible(false);

    connect(m_selected_market_update_timer, SIGNAL(timeout()), this, SLOT(refreshSelectedMarket()));
    connect(m_update_bets_timer, SIGNAL(timeout()), this, SLOT(refreshBets()));

    connect(ui->actionLogin,SIGNAL(triggered()),this,SLOT(showLoginDialog()));
    connect(ui->actionProgram_Settings,SIGNAL(triggered()),this,SLOT(showProgramSettingsDialog()));
    connect(ui->actionPopulate_Market_List,SIGNAL(triggered()),this,SLOT(showEventSelectionDialog()));
    connect(ui->btnShowRunnerCharts,SIGNAL(clicked()),this,SLOT(showRunnerChartDialog()));
    connect(ui->btnRefreshBalance,SIGNAL(clicked()),this,SLOT(onRefreshBalanceClicked()));

    connect(&my_login,SIGNAL(LoginDataChanged()),this,SLOT(login()));
    connect(&my_program_settings,SIGNAL(settingsDataChanged()),this,SLOT(onProgramSettingsChange()));

    connect(&my_event_selector,SIGNAL(selectedEventChanged()),this,SLOT(onSelectedEventChange()));
    connect(&bf_man,SIGNAL(receiveLoginResponse()),this,SLOT(processLogin()));
    //connect(&bf_man,SIGNAL(receiveListEventTypesResponse()),this,SLOT(processListEventTypesResp()));
    //connect(&bf_man,SIGNAL(receiveListEventsResponse()),this,SLOT(processListEventsResp()));
    connect(&bf_man,SIGNAL(receivePlaceBetsResponse()),this,SLOT(processPlaceBetsResp()));
    connect(&bf_man,SIGNAL(receiveListCurrentOrdersResponse()),this,SLOT(processListCurrentOrdersResp()));
    connect(&bf_man,SIGNAL(receiveCancelOrdersResponse()),this,SLOT(processCancelOrdersResp()));
    connect(&bf_man,SIGNAL(receiveListMarketPAndLResponse()),this,SLOT(processMarketPAndLResp()));

    connect(&bf_man,SIGNAL(receiveListMarketCatalogueResponse()),this,SLOT(processListMarketCatalogueResp()));
    connect(&bf_man,SIGNAL(receiveListMarketBookResponse()),this,SLOT(processListMarketBookResp()));
    connect(&bf_man,SIGNAL(receiveBalanceResponse()),this,SLOT(processGetAccountFunds()));

    connect(&bf_man,SIGNAL(receiveNetworkError()),this,SLOT(processNetworkError()));

    connect(&m_silks_downloader,SIGNAL(silksDownloaded()),this,SLOT(loadSilks()));


    connect(ui->selectMarketButton,SIGNAL(clicked()),this,SLOT(selectMarket()));    

    // MARKET STATUS GROUP BOX
    ui->selectedMarketLabel->setText("NO MARKET SELECTED");

    ui->loginStatusLabel->setText("Not Logged In");
    ui->loginStatusLabel->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");
    ui->selectMarketButton->setEnabled(false);

    ui->marketInPlayLabel->setText("");
    ui->marketInPlayLabel->setEnabled(false);

    ui->lblAccountBalance->setStyleSheet("QLabel { background-color : #33CC33; color : white; font-weight: bold;}");

    // GRID VIEW TAB

    ui->selectecMarketView->setModel(&my_market_model);
    //ui->selectecMarketView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->selectecMarketView->setColumnWidth(gridview::IMAGE,0);
    ui->selectecMarketView->setColumnWidth(gridview::NAME,200);
    ui->selectecMarketView->setColumnWidth(gridview::PROFIT,70);
    ui->selectecMarketView->setColumnWidth(gridview::BACK3,70);
    ui->selectecMarketView->setColumnWidth(gridview::BACK2,70);
    ui->selectecMarketView->setColumnWidth(gridview::BACK1,70);
    ui->selectecMarketView->setColumnWidth(gridview::LAY1,70);
    ui->selectecMarketView->setColumnWidth(gridview::LAY2,70);
    ui->selectecMarketView->setColumnWidth(gridview::LAY3,70);
    ui->selectecMarketView->setColumnWidth(gridview::HEDGE,90);
    ui->selectecMarketView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->selectecMarketView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onGridViewClick(const QModelIndex&)));


    //initialiseLadderStakeComboBox(ui->cbGridStake);
    //ui->cbGridStake->setCurrentIndex(0);

    ui->gridUnmatchedBetView->setModel(&my_market_unmatched_bets_model);
    ui->gridUnmatchedBetView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->gridUnmatchedBetView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->gridUnmatchedBetView->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->gridUnmatchedBetView->show();

    ui->gridMatchedBetView->setModel(&my_market_placed_bets_model);
    ui->gridMatchedBetView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->gridMatchedBetView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->gridMatchedBetView->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->gridMatchedBetView->show();

    connect(ui->gridUnmatchedBetView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onUnmatchedBetsViewClick(const QModelIndex&)));


    // BET VIEW TAB
    ui->betviewMatchedBetView->setModel(&my_market_placed_bets_model);
    ui->betviewMatchedBetView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->betviewMatchedBetView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->betviewMatchedBetView->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->betviewMatchedBetView->show();

    ui->betviewUnmatchedBetView->setModel(&my_market_unmatched_bets_model);
    ui->betviewUnmatchedBetView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->betviewUnmatchedBetView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->betviewUnmatchedBetView->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->betviewUnmatchedBetView->show();

    connect(ui->betviewUnmatchedBetView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onUnmatchedBetsViewClick(const QModelIndex&)));


    ProgressBarDelegate* pbar_del1 = new ProgressBarDelegate(this);
    if (1 == m_display_theme)
    {
        pbar_del1->setColourText(Qt::white);
    }

    // LADDER TAB
    ui->ladderView1->setModel(&m_ladder_views_one);
    ui->ladderView1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ladderView1->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ladderView1->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->ladderView1->show();
    ui->ladderView1->setItemDelegateForColumn(6, pbar_del1);

    ProgressBarDelegate* pbar_del2 = new ProgressBarDelegate(this);
    if (1 == m_display_theme)
    {
        pbar_del2->setColourText(Qt::white);
    }

    ui->ladderView2->setModel(&m_ladder_views_two);
    ui->ladderView2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ladderView2->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ladderView2->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->ladderView2->show();
    ui->ladderView2->setItemDelegateForColumn(6, pbar_del2);

    ProgressBarDelegate* pbar_del3 = new ProgressBarDelegate(this);
    if (1 == m_display_theme)
    {
        pbar_del3->setColourText(Qt::white);
    }

    ui->ladderView3->setModel(&m_ladder_views_three);
    ui->ladderView3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ladderView3->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ladderView3->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->ladderView3->show();
    ui->ladderView3->setItemDelegateForColumn(6, pbar_del3);

    ProgressBarDelegate* pbar_del4 = new ProgressBarDelegate(this);
    if (1 == m_display_theme)
    {
        pbar_del4->setColourText(Qt::white);
    }

    ui->ladderView4->setModel(&m_ladder_views_four);
    ui->ladderView4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ladderView4->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ladderView4->verticalHeader()->setDefaultSectionSize(betviews_row_height);
    ui->ladderView4->show();
    ui->ladderView4->setItemDelegateForColumn(6, pbar_del4);

    ui->lblLadder1TotalTraded->setStyleSheet("QLabel { background-color : #ffff00; color: black}");
    ui->lblLadder2TotalTraded->setStyleSheet("QLabel { background-color : #ffff00; color: black}");
    ui->lblLadder3TotalTraded->setStyleSheet("QLabel { background-color : #ffff00; color: black}");
    ui->lblLadder4TotalTraded->setStyleSheet("QLabel { background-color : #ffff00; color: black}");

    connect(ui->ladderView1,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onLadderViewClick1(const QModelIndex&)));
    connect(ui->ladderView2,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onLadderViewClick2(const QModelIndex&)));
    connect(ui->ladderView3,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onLadderViewClick3(const QModelIndex&)));
    connect(ui->ladderView4,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onLadderViewClick4(const QModelIndex&)));

    connect(ui->btnBringUpChartLadder1,SIGNAL(clicked()),this,SLOT(onLadder1ShowChartClicked()));
    connect(ui->btnBringUpChartLadder2,SIGNAL(clicked()),this,SLOT(onLadder2ShowChartClicked()));
    connect(ui->btnBringUpChartLadder3,SIGNAL(clicked()),this,SLOT(onLadder3ShowChartClicked()));
    connect(ui->btnBringUpChartLadder4,SIGNAL(clicked()),this,SLOT(onLadder4ShowChartClicked()));


    connect(ui->btnCentreLadderOne,SIGNAL(clicked()),this,SLOT(onCentreLadderOneClicked()));
    connect(ui->btnLadder1CancelBacks,SIGNAL(clicked()),this,SLOT(onLadder1CancelBacksClicked()));
    connect(ui->btnLadder1CancelLays,SIGNAL(clicked()),this,SLOT(onLadder1CancelLaysClicked()));
    connect(ui->cbLadder1Select,SIGNAL(currentIndexChanged(int)),this,SLOT(changeLadder1SelectedRunner(int)));

    connect(ui->btnCentreLadderTwo,SIGNAL(clicked()),this,SLOT(onCentreLadderTwoClicked()));
    connect(ui->btnLadder2CancelBacks,SIGNAL(clicked()),this,SLOT(onLadder2CancelBacksClicked()));
    connect(ui->btnLadder2CancelLays,SIGNAL(clicked()),this,SLOT(onLadder2CancelLaysClicked()));
    connect(ui->cbLadder2Select,SIGNAL(currentIndexChanged(int)),this,SLOT(changeLadder2SelectedRunner(int)));

    connect(ui->btnCentreLadderThree,SIGNAL(clicked()),this,SLOT(onCentreLadderThreeClicked()));
    connect(ui->btnLadder3CancelBacks,SIGNAL(clicked()),this,SLOT(onLadder3CancelBacksClicked()));
    connect(ui->btnLadder3CancelLays,SIGNAL(clicked()),this,SLOT(onLadder3CancelLaysClicked()));
    connect(ui->cbLadder3Select,SIGNAL(currentIndexChanged(int)),this,SLOT(changeLadder3SelectedRunner(int)));

    connect(ui->btnCentreLadderFour,&QPushButton::clicked,this,&MainWindow::onCentreLadderFourClicked);
    connect(ui->btnLadder4CancelBacks,&QPushButton::clicked,this,&MainWindow::onLadder4CancelBacksClicked);
    connect(ui->btnLadder4CancelLays,&QPushButton::clicked,this,&MainWindow::onLadder4CancelLaysClicked);    
    connect(ui->cbLadder4Select,SIGNAL(currentIndexChanged(int)),this,SLOT(changeLadder4SelectedRunner(int)));


    // STAKE TABLES
    ui->tvLadder1Stake->setModel(&my_ladder_1_stake_model);
    ui->tvLadder1Stake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvLadder1Stake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvLadder1Stake->horizontalHeader()->setVisible(false);
    ui->tvLadder1Stake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvLadder1Stake->verticalHeader()->setVisible(false);
    ui->tvLadder1Stake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvLadder1Stake->show();
    connect(ui->tvLadder1Stake,&QTableView::clicked,this,&MainWindow::onChangeLadder1Stake);

    ui->tvLadder2Stake->setModel(&my_ladder_2_stake_model);
    ui->tvLadder2Stake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvLadder2Stake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvLadder2Stake->horizontalHeader()->setVisible(false);
    ui->tvLadder2Stake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvLadder2Stake->verticalHeader()->setVisible(false);
    ui->tvLadder2Stake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvLadder2Stake->show();
    connect(ui->tvLadder2Stake,&QTableView::clicked,this,&MainWindow::onChangeLadder2Stake);

    ui->tvLadder3Stake->setModel(&my_ladder_3_stake_model);
    ui->tvLadder3Stake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvLadder3Stake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvLadder3Stake->horizontalHeader()->setVisible(false);
    ui->tvLadder3Stake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvLadder3Stake->verticalHeader()->setVisible(false);
    ui->tvLadder3Stake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvLadder3Stake->show();
    connect(ui->tvLadder3Stake,&QTableView::clicked,this,&MainWindow::onChangeLadder3Stake);

    ui->tvLadder4Stake->setModel(&my_ladder_4_stake_model);
    ui->tvLadder4Stake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvLadder4Stake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvLadder4Stake->horizontalHeader()->setVisible(false);
    ui->tvLadder4Stake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvLadder4Stake->verticalHeader()->setVisible(false);
    ui->tvLadder4Stake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvLadder4Stake->show();
    connect(ui->tvLadder4Stake,&QTableView::clicked,this,&MainWindow::onChangeLadder4Stake);

    ui->tvGridStake->setModel(&my_gridview_stake_model);
    ui->tvGridStake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvGridStake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvGridStake->horizontalHeader()->setVisible(false);
    ui->tvGridStake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvGridStake->verticalHeader()->setVisible(false);
    ui->tvGridStake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvGridStake->show();
    connect(ui->tvGridStake,&QTableView::clicked,this,&MainWindow::onChangeGridViewStake);


    ui->tvInplayGridStake->setModel(&my_inplay_gridview_stake_model);
    ui->tvInplayGridStake->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tvInplayGridStake->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tvInplayGridStake->horizontalHeader()->setVisible(false);
    ui->tvInplayGridStake->verticalHeader()->setDefaultSectionSize(20);
    ui->tvInplayGridStake->verticalHeader()->setVisible(false);
    ui->tvInplayGridStake->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tvInplayGridStake->show();
    connect(ui->tvInplayGridStake,&QTableView::clicked,this,&MainWindow::onChangeInplayGridViewStake);



    // IN PLAY HORIZONTAL LADDER TAB
    ui->inPlayGridBetView->setModel(&my_inplay_model);    
    ui->inPlayGridBetView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->inPlayGridBetView->horizontalHeader()->setStretchLastSection(true);
    ui->inPlayGridBetView->verticalHeader()->setDefaultSectionSize(25);
    ui->inPlayGridBetView->show();

    ui->sbIPBetTickOffset->setValue(0);    
    connect(ui->inPlayGridBetView,SIGNAL(clicked(const QModelIndex&)),this,SLOT(onIPViewClick(const QModelIndex&)));

    // EVENT LOG TAB
    connect(ui->btnClearLog,SIGNAL(clicked()),this,SLOT(onClearEventLogClicked()));

    ui->selectecMarketView->show();
    my_runner_charts.reset();

    // START TIMERS
    m_selected_market_update_timer->start(1000);
    m_update_bets_timer->start(1000);

    QIcon chart_icon(":/images/chart_icon1.png");
    ui->btnBringUpChartLadder1->setIcon(chart_icon);
    ui->btnBringUpChartLadder2->setIcon(chart_icon);
    ui->btnBringUpChartLadder3->setIcon(chart_icon);
    ui->btnBringUpChartLadder4->setIcon(chart_icon);
    ui->btnShowRunnerCharts->setIcon(chart_icon);

    ui->lblMarketLayPct->setStyleSheet("QLabel { color : red;}");
    ui->lblMarketBackPct->setStyleSheet("QLabel { color : red;}");
    ui->lblMarketLayPct->setText("");
    ui->lblMarketBackPct->setText("");
    m_ladder_stake1 = static_cast<double>(my_ladder_1_stake_model.getSelectedStake());
    m_ladder_stake2 = static_cast<double>(my_ladder_2_stake_model.getSelectedStake());
    m_ladder_stake3 = static_cast<double>(my_ladder_3_stake_model.getSelectedStake());
    m_ladder_stake4 = static_cast<double>(my_ladder_4_stake_model.getSelectedStake());


    // NEW market vol chart
    QChartView* cv_mkvol = my_market_vol_chart_wrapper.getChartView();    
    my_market_vol_chart_wrapper.showXAxisLabels(false);
    my_market_vol_chart_wrapper.showXAxisTitle(false);
    my_market_vol_chart_wrapper.showYAxisLabels(false);
    my_market_vol_chart_wrapper.showYAxisTitle(false);
    my_market_vol_chart_wrapper.showGrid(true);
    my_market_vol_chart_wrapper.showMinorGrid(false);
    my_market_vol_chart_wrapper.setXAxisTickCount(5);
    my_market_vol_chart_wrapper.setYAxisTickCount(3);
    my_market_vol_chart_wrapper.changeXAxisLabelFontPointSize(1);
    my_market_vol_chart_wrapper.changeYAxisLabelFontPointSize(1);
    my_market_vol_chart_wrapper.changeXAxisTitleFontPointSize(1);
    my_market_vol_chart_wrapper.changeYAxisTitleFontPointSize(1);

    ui->marketVolChartLayout->addWidget(cv_mkvol);
    ui->marketVolChartLayout->setContentsMargins(0,2,0,0);



    my_market_vol_chart_wrapper.setMargins(0);

    initialiseLadderUtilityCharts();

    onProgramSettingsChange();

    if (1 == m_display_theme)
    { 
        ui->gbLadder1Controls->setStyleSheet("QGroupBox {background-color : #353535; color: white; border: none}");
        ui->gbLadder2Controls->setStyleSheet("QGroupBox {background-color : #353535; color: white; border: none}");
        ui->gbLadder3Controls->setStyleSheet("QGroupBox {background-color : #353535; color: white; border: none}");
        ui->gbLadder4Controls->setStyleSheet("QGroupBox {background-color : #353535; color: white; border: none}");
        ui->cbLadder1Select->setStyleSheet("QComboBox { background-color : #353535; color : white;}");
        ui->cbLadder2Select->setStyleSheet("QComboBox { background-color : #353535; color : white;}");
        ui->cbLadder3Select->setStyleSheet("QComboBox { background-color : #353535; color : white;}");
        ui->cbLadder4Select->setStyleSheet("QComboBox { background-color : #353535; color : white;}");
        ui->btnBringUpChartLadder1->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnBringUpChartLadder2->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnBringUpChartLadder3->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnBringUpChartLadder4->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder1CancelLays->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder2CancelLays->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder3CancelLays->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder4CancelLays->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder1CancelBacks->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder2CancelBacks->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder3CancelBacks->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnLadder4CancelBacks->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnCentreLadderOne->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnCentreLadderTwo->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnCentreLadderThree->setStyleSheet("QPushButton { background-color : #353535; color : white;}");
        ui->btnCentreLadderFour->setStyleSheet("QPushButton { background-color : #353535; color : white;}");

        ui->lblHedgeLadder1->setStyleSheet("QLabel {color : white;}");
        ui->lblHedgeLadder2->setStyleSheet("QLabel {color : white;}");
        ui->lblHedgeLadder3->setStyleSheet("QLabel {color : white;}");
        ui->lblHedgeLadder4->setStyleSheet("QLabel {color : white;}");
        ui->lblChooseStakeLadder1->setStyleSheet("QLabel {color : white;}");
        ui->lblChooseStakeLadder2->setStyleSheet("QLabel {color : white;}");
        ui->lblChooseStakeLadder3->setStyleSheet("QLabel {color : white;}");
        ui->lblChooseStakeLadder4->setStyleSheet("QLabel {color : white;}");

        ui->lblLadder1_BackWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder2_BackWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder3_BackWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder4_BackWOM->setStyleSheet("QLabel {color : white;}");

        ui->lblLadder1_LayWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder2_LayWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder3_LayWOM->setStyleSheet("QLabel {color : white;}");
        ui->lblLadder4_LayWOM->setStyleSheet("QLabel {color : white;}");
    }
    // perform autologin if appropriate to do so

    /*
    if (!al.isEmpty())
    {
        QFileInfo alinfo(al);
        if (alinfo.exists())
        {
            TIniFile login_ini(al.toStdString());
            std::string str_error = "error";
            bool autologinvalid = false;
            if (login_ini.isValid())
            {
                std::string uname = login_ini.getValue("general","un",str_error);
                if (uname != str_error)
                {
                    std::string pword = login_ini.getValue("general","pd",str_error);
                    if (pword != str_error)
                    {
                        std::string certpath = login_ini.getValue("general","certpath",str_error);
                        if (certpath != str_error)
                        {
                            std::string keypath = login_ini.getValue("general","keypath",str_error);
                            if (keypath != str_error)
                            {
                                std::string appkey = login_ini.getValue("general","appkey",str_error);
                                if (appkey != str_error)
                                {
                                    // try auto login
                                    autologinvalid = true;
                                    bf_man.setLoginDetails(scramble(1,2,QString::fromStdString(uname)),
                                                           scramble(2,1,QString::fromStdString(pword)),
                                                           QString::fromStdString(certpath),
                                                           QString::fromStdString(keypath),
                                                           QString::fromStdString(appkey));
                                    if (false == bf_man.attemptLogin())
                                    {
                                        // flag this error!
                                        logEvent("Unable to request login - supplied data is invalid!");
                                        QMessageBox Msgbox;
                                        Msgbox.setText("ERROR: autologin failed!!");
                                        Msgbox.exec();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (!autologinvalid)
            {
                QMessageBox Msgbox;
                Msgbox.setText("ERROR: autologin file is not valid!!");
                Msgbox.exec();
            }
        }
        else
        {
            QMessageBox Msgbox;
            Msgbox.setText("ERROR: autologin file does not exist!!!");
            Msgbox.exec();
        }
    }
    */
}

//====================================================================
void MainWindow::initialiseLadderUtilityCharts()
{
    for (int i = 0; i < 4; ++i)
    {
        QChartView* cview = my_vwap_xo_array[i]->getChartView();
        my_vwap_xo_array[i]->showXAxisLabels(false);
        my_vwap_xo_array[i]->showXAxisTitle(false);
        my_vwap_xo_array[i]->showYAxisLabels(true);
        my_vwap_xo_array[i]->showYAxisTitle(false);
        my_vwap_xo_array[i]->showGrid(true);
        my_vwap_xo_array[i]->showMinorGrid(false);
        my_vwap_xo_array[i]->setXAxisTickCount(5);
        my_vwap_xo_array[i]->setYAxisTickCount(3);

        my_vwap_xo_array[i]->changeXAxisLabelFontPointSize(1);
        my_vwap_xo_array[i]->changeYAxisLabelFontPointSize(5);
        my_vwap_xo_array[i]->changeXAxisTitleFontPointSize(1);
        my_vwap_xo_array[i]->changeYAxisTitleFontPointSize(1);

        my_ladder_util_layouts[i]->addWidget(cview);
        my_ladder_util_layouts[i]->setContentsMargins(0,0,0,0);
        my_vwap_xo_array[i]->setMargins(1);
    }
}

//====================================================================
void MainWindow::logEvent(const QString& msg)
{
    QString tnow = QDateTime::currentDateTimeUtc().toString("[yyyyMMMdd HH:mm:ss] > ");
    QString line = tnow + msg;
    ui->debugInfo->addItem(line);
}

//====================================================================
QString MainWindow::mapOddsBySignedOffset(const QString& str_odds, int tick_offset)
{
    QString ret_price = "0.0";
    std::map<QString,std::uint32_t>::iterator it = m_index_offset_by_price.find(str_odds);
    if (it != m_index_offset_by_price.end())
    {
        std::uint32_t index = it->second;
        if (tick_offset < 0)
        {
            std::uint32_t abs_offset = std::abs(tick_offset);
            if (abs_offset >= index)
            {
                index = 0;
            }
            else
            {
                index -= abs_offset;
            }
        }
        else if (tick_offset > 0)
        {
            index += static_cast<std::uint32_t>(tick_offset);
        }
        std::map<std::uint32_t,QString>::iterator it2 = m_price_offset_indices.find(index);
        if (it2 != m_price_offset_indices.end())
        {
            ret_price = it2->second;
        }
        else
        {
            if (index >= m_price_offset_indices.size())
            {
                ret_price = "1000.00";
            }
        }
    }
    return ret_price;
}

//====================================================================
MainWindow::~MainWindow()
{
    writeGUIUpdateMsgBufferToFile();
    delete ui;
    QApplication::exit();
}


//====================================================================
void MainWindow::onLadder1CancelBacksClicked()
{
    cancelLadderBets(m_ladder_views_one,false);
}

//====================================================================
void MainWindow::onLadder1CancelLaysClicked()
{
    cancelLadderBets(m_ladder_views_one,true);
}

//====================================================================
void MainWindow::onLadder2CancelBacksClicked()
{
    cancelLadderBets(m_ladder_views_two,false);
}

//====================================================================
void MainWindow::onLadder2CancelLaysClicked()
{
    cancelLadderBets(m_ladder_views_two,true);
}

//====================================================================
void MainWindow::onLadder3CancelBacksClicked()
{
    cancelLadderBets(m_ladder_views_three,false);
}

//====================================================================
void MainWindow::onLadder3CancelLaysClicked()
{
    cancelLadderBets(m_ladder_views_three,true);
}

//====================================================================
void MainWindow::onLadder4CancelBacksClicked()
{
    cancelLadderBets(m_ladder_views_four,false);
}

//====================================================================
void MainWindow::onLadder4CancelLaysClicked()
{
    cancelLadderBets(m_ladder_views_four,true);
}

//====================================================================
void MainWindow::showLoginDialog()
{
    my_login.show();
    my_login.raise();
}

//====================================================================
void MainWindow::showEventSelectionDialog()
{
    my_event_selector.show();
    my_event_selector.raise();
}



//====================================================================
void MainWindow::showRunnerChartDialog()
{
    my_runner_charts.show();
    my_runner_charts.raise();
}

//====================================================================
void MainWindow::showProgramSettingsDialog()
{
    my_program_settings.show();
    my_program_settings.refresh();
    my_program_settings.raise();
}

//====================================================================
void MainWindow::showRunnerAnalysisDialog()
{
    my_runner_analysis_chart.show();
    my_runner_analysis_chart.raise();
}

//====================================================================
void MainWindow::onRefreshBalanceClicked()
{
    bf_man.getAccountFunds();
}

//====================================================================
void MainWindow::onClearEventLogClicked()
{
    ui->debugInfo->clear();
}


//====================================================================
void MainWindow::onUnmatchedBetsViewClick(const QModelIndex &index)
{
    int row = index.row();
    int col = index.column();
    if (5 == col)
    {
        if (m_market.valid() && (row >= 0))
        {
            auto unmatched_bets = m_market.getPlacedBets();
            const int num_bets = static_cast<int>(unmatched_bets.size());
            if (row < num_bets)
            {
                std::shared_ptr<betfair::TBet> this_bet = unmatched_bets[static_cast<std::size_t>(row)];
                if (false == this_bet->isBetFullyMatched())
                {                    
                    std::vector<QString> v_bets(1,this_bet->getBetID());
                    bf_man.cancelBets(m_market.getMarketID(),v_bets);
                }
            }
        }
    }
}

//====================================================================
void MainWindow::onIPViewClick(const QModelIndex &index)
{
    if (false == m_inplayview_betting_enabled)
    {
        QMessageBox::information(this, "Inplay Grid View Betting Disabled", "To place bets, enable inplay grid view betting\n from the \"Program Settings\" dialog.");
    }
    else
    {
        int row = index.row();
        int col = index.column();
        if (m_market.valid())
        {
            auto runners = m_market.getRunners();
            const int num_runners = static_cast<int>(runners.size());
            QString m_id = m_market.getMarketID();
            if (0 == col)
            {
                // This colum is runner name which should display that runners
                // chart when its cell is clicked
                if (row > 0)
                {
                    // ROW ZERO IS THE LAY ALL ROW SO THERE IS NO
                    // SINGLE RUNNER ASSOCIATED WITH IT - THEREFORE
                    // WE DECREMENT
                    --row;
                    if (row < num_runners)
                    {
                        std::shared_ptr<betfair::TRunner> this_runner = runners[static_cast<std::size_t>(row)];
                        if (this_runner->isActive())
                        {
                            std::int64_t id = this_runner->getID();
                            my_runner_analysis_chart.setSelectedRunner(id);
                            my_runner_analysis_chart.show();
                            my_runner_analysis_chart.raise();
                        }
                    }
                }
            }
            else if (col > 4)
            {
                if (m_market.marketInPlay())
                {
                    const double d_stake = m_inplay_gridview_stake;
                    if (d_stake > 0.0)
                    {
                        double odds = my_inplay_model.getColumnOdds(col);
                        // Apply tick offset
                        int tick_offset = ui->chkInplayTickOffset->isChecked() ? ui->sbIPBetTickOffset->value() : 0;
                        QString str_odds = QString::number(odds,'f',2);
                        str_odds = mapOddsBySignedOffset(str_odds, tick_offset);
                        odds = str_odds.toDouble();                     
                        if (odds > 0.0)
                        {
                            if (0 == row)
                            {
                                // Lay ALL runners at column price                                                                                               
                                bf_man.layFieldAt(odds,d_stake,false);
                                QString line = "Attempting to lay all runners @ " + QString::number(odds,'f',2) + " for £" + QString::number(d_stake,'f',2) + " (from IP grid view)";
                                logEvent(line);
                            }
                            else if (row > 0)
                            {
                                // As before, decrement row to make it match runner index. This is because
                                // row 0 is for LAY ALL!
                                --row;
                                // Lay/Back selected row runner at column proce
                                bool laytype = (false == QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier));
                                if (row < num_runners)
                                {
                                    std::shared_ptr<betfair::TRunner> this_runner = runners[static_cast<std::size_t>(row)];
                                    if (this_runner->isActive())
                                    {
                                        // Looks good to go now.
                                        QString runner_name = this_runner->getName();
                                        std::vector<std::int64_t> v_sels;
                                        v_sels.push_back(this_runner->getID());
                                        QString bettype = laytype ? "LAY " : "BACK ";
                                        bf_man.placeBets(m_id, v_sels, odds, d_stake, laytype, m_bet_persistence_flag);

                                        QString line = "Attempting to " + bettype + runner_name + " @ " + QString::number(odds,'f',2) + " for £" + QString::number(d_stake,'f',2) + " (from IP grid view)";
                                        logEvent(line);
                                    }
                                }
                            }
                        }
                        else
                        {
                            QString err = "MainWindow::onIPViewClick() : Error obtaining valid odds from column index!";
                            logEvent(err);
                        }
                    }
                    else
                    {
                        QString err = "MainWindow::onIPViewClick() : Error converting selected stake to double!";
                        logEvent(err);
                        // TODO: popup here to alert user to make sure stake is selected?
                    }
                }
                else
                {
                    // TODO: Dialog to tell user no bets can be placed until market in play
                }
            }
            else
            {
                // TODO: Dialog to tell user one click betting not enabled
            }
        }
    }
}

//====================================================================
void MainWindow::onChangeGridViewStake(const QModelIndex &index)
{
    int col = index.column();
    my_gridview_stake_model.setSelectedColumn(col);
    my_gridview_stake_model.updateAllData();
    m_gridview_stake = static_cast<double>(my_gridview_stake_model.getSelectedStake());
}

//====================================================================
void MainWindow::onChangeInplayGridViewStake(const QModelIndex &index)
{
    int col = index.column();
    my_inplay_gridview_stake_model.setSelectedColumn(col);
    my_inplay_gridview_stake_model.updateAllData();
    m_inplay_gridview_stake = static_cast<double>(my_inplay_gridview_stake_model.getSelectedStake());
}

//====================================================================
void MainWindow::onChangeLadder1Stake(const QModelIndex &index)
{
    int col = index.column();
    my_ladder_1_stake_model.setSelectedColumn(col);
    my_ladder_1_stake_model.updateAllData();
    m_ladder_stake1 = static_cast<double>(my_ladder_1_stake_model.getSelectedStake());

}

//====================================================================
void MainWindow::onChangeLadder2Stake(const QModelIndex &index)
{
    int col = index.column();
    my_ladder_2_stake_model.setSelectedColumn(col);
    my_ladder_2_stake_model.updateAllData();
    m_ladder_stake2 = static_cast<double>(my_ladder_2_stake_model.getSelectedStake());

}

//====================================================================
void MainWindow::onChangeLadder3Stake(const QModelIndex &index)
{
    int col = index.column();
    my_ladder_3_stake_model.setSelectedColumn(col);
    my_ladder_3_stake_model.updateAllData();
    m_ladder_stake3 = static_cast<double>(my_ladder_3_stake_model.getSelectedStake());

}

//====================================================================
void MainWindow::onChangeLadder4Stake(const QModelIndex &index)
{
    int col = index.column();
    my_ladder_4_stake_model.setSelectedColumn(col);
    my_ladder_4_stake_model.updateAllData();
    m_ladder_stake4 = static_cast<double>(my_ladder_4_stake_model.getSelectedStake());

}

//====================================================================
void MainWindow::ladderViewClicked(LadderViewModel& ladder,
                                   const double& stake,
                                   const QModelIndex& index)
{
    int row = index.row();
    int col = index.column();
    const std::int64_t sel_id = ladder.getSelectionID();
    if (m_market.valid() && (row >= 0) && (sel_id > 0))
    {
        const QString odds = m_v_odds_tick_vector[static_cast<std::size_t>(row)];
        const double d_odds = odds.toDouble();
        QString m_id = m_market.getMarketID();
        if (0 == col)
        {
            // hedge at row odds
            std::shared_ptr<betfair::TRunner> selection = m_market.getSelectionByID(sel_id);
            if (selection)
            {
                QString info;
                betfair::utils::betInstruction bet = m_market.hedgeRunnerAtFixedOdds(sel_id, m_id, d_odds, info);
                if (bet.selection == sel_id && bet.market_id == m_id)
                {
                    if (bet.stake > 0.0 && bet.odds > 1.0)
                    {
                        std::vector<std::int64_t> v_sels;
                        v_sels.push_back(sel_id);
                        bf_man.placeBets(m_id, v_sels, bet.odds, bet.stake, bet.b_lay_type, m_bet_persistence_flag);
                        QString type = bet.b_lay_type ? "LAY " : "BACK ";
                        QString line = "Attempting to " + type + selection->getName() + " @ " + QString::number(bet.odds,'f',2) + " for £" + QString::number(bet.stake,'f',2) + " (Hedge action from ladder view)";
                    }
                }
                logEvent(info);
            }
        }
        else if (1 == col)
        {
            const bool cancel_lays_at_price = QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
            if (cancel_lays_at_price)
            {
                cancelLadderBetsAtPrice(ladder,d_odds,true);
            }
            else
            {
                // lay at row odds
                std::shared_ptr<betfair::TRunner> selection = m_market.getSelectionByID(sel_id);
                if (selection && selection->isActive())
                {
                    if (stake > 0.0)
                    {
                        if (m_auto_offset_bet_placement_ticks > 0)
                        {
                            std::vector<betfair::utils::betInstruction> v_bets(1,betfair::utils::betInstruction(sel_id,m_id,"",true,d_odds,stake,m_bet_persistence_flag));
                            // Get opposing bet now
                            int offset = row - m_auto_offset_bet_placement_ticks;
                            if (offset < 0)
                            {
                                offset = 0;
                            }
                            else if (offset >= static_cast<int>(m_v_odds_tick_vector.size()))
                            {
                                offset = static_cast<int>(m_v_odds_tick_vector.size()) - 1;
                            }
                            if (offset != row)
                            {
                                double offset_odds = m_v_odds_tick_vector[static_cast<std::size_t>(offset)].toDouble();
                                v_bets.push_back(betfair::utils::betInstruction(sel_id,m_id,"",false,offset_odds,stake,m_bet_persistence_flag));
                            }
                            bf_man.placeBets(m_id, v_bets, "");
                            QString line = "Attempting to LAY " + selection->getName() + " @ " + odds + " for £" +  QString::number(stake,'f',2) + " (from ladder view)";
                            line.append(" with " + QString::number(offset) + " tick opposing automatic offset bet.");
                            logEvent(line);
                        }
                        else
                        {
                            std::vector<std::int64_t> v_sels(1,sel_id);
                            bf_man.placeBets(m_id, v_sels, d_odds, stake, true, m_bet_persistence_flag);
                            QString line = "Attempting to LAY " + selection->getName() + " @ " + odds + " for £" +  QString::number(stake,'f',2) + " (from ladder view)";
                            logEvent(line);
                        }
                    }
                }
            }
        }
        else if (5 == col)
        {
            // back at row odds
            const bool cancel_backs_at_price = QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
            if (cancel_backs_at_price)
            {
                cancelLadderBetsAtPrice(ladder,d_odds,false);
            }
            else
            {
                std::shared_ptr<betfair::TRunner> selection = m_market.getSelectionByID(sel_id);
                if (selection && selection->isActive())
                {
                    if (stake > 0.0)
                    {
                        if (m_auto_offset_bet_placement_ticks > 0)
                        {
                            std::vector<betfair::utils::betInstruction> v_bets(1,betfair::utils::betInstruction(sel_id,m_id,"",false,d_odds,stake,m_bet_persistence_flag));
                            // Get opposing bet now
                            int offset = row + m_auto_offset_bet_placement_ticks;
                            if (offset < 0)
                            {
                                offset = 0;
                            }
                            else if (offset >= static_cast<int>(m_v_odds_tick_vector.size()))
                            {
                                offset = static_cast<int>(m_v_odds_tick_vector.size()) - 1;
                            }
                            if (offset != row)
                            {
                                double offset_odds = m_v_odds_tick_vector[static_cast<std::size_t>(offset)].toDouble();
                                v_bets.push_back(betfair::utils::betInstruction(sel_id,m_id,"",true,offset_odds,stake,m_bet_persistence_flag));
                            }
                            bf_man.placeBets(m_id, v_bets, "");
                            QString line = "Attempting to BACK " + selection->getName() + " @ " + odds + " for £" + QString::number(stake,'f',2) + " (from ladder view)";;
                            line.append(" with " + QString::number(offset) + " tick opposing automatic offset bet.");
                            logEvent(line);
                        }
                        else
                        {
                            std::vector<std::int64_t> v_sels(1,sel_id);
                            bf_man.placeBets(m_id, v_sels, d_odds, stake, false, m_bet_persistence_flag);
                            QString line = "Attempting to BACK " + selection->getName() + " @ " + odds + " for £" + QString::number(stake,'f',2) + " (from ladder view)";;
                            logEvent(line);
                        }
                    }
                }
            }
        }
    }
}


//====================================================================
void MainWindow::onLadderViewClick4(const QModelIndex &index)
{
    if (m_ladderview_betting_enabled)
    {
        ladderViewClicked(m_ladder_views_four,m_ladder_stake4,index);
    }
    else
    {
        QMessageBox::information(this, "Ladder View Betting Disabled", "To place bets, enable ladder view betting from\nthe \"Program Settings\" dialog.");
    }
}

//====================================================================
void MainWindow::onLadderViewClick3(const QModelIndex &index)
{
    if (m_ladderview_betting_enabled)
    {
        ladderViewClicked(m_ladder_views_three,m_ladder_stake3,index);
    }
    else
    {
        QMessageBox::information(this, "Ladder View Betting Disabled", "To place bets, enable ladder view betting from\nthe \"Program Settings\" dialog.");
    }
}

//====================================================================
void MainWindow::onLadderViewClick2(const QModelIndex &index)
{
    if (m_ladderview_betting_enabled)
    {
        ladderViewClicked(m_ladder_views_two,m_ladder_stake2,index);
    }
    else
    {
        QMessageBox::information(this, "Ladder View Betting Disabled", "To place bets, enable ladder view betting from\nthe \"Program Settings\" dialog.");
    }
}

//====================================================================
void MainWindow::onLadderViewClick1(const QModelIndex &index)
{
    if (m_ladderview_betting_enabled)
    {
        ladderViewClicked(m_ladder_views_one,m_ladder_stake1,index);
    }
    else
    {
        QMessageBox::information(this, "Ladder View Betting Disabled", "To place bets, enable ladder view betting from\nthe \"Program Settings\" dialog.");
    }
}

//====================================================================
void MainWindow::onGridViewClick(const QModelIndex &index)
{
    const double d_stake = m_gridview_stake;
    int row = index.row();
    int col = index.column();
    if (gridview::IMAGE == col || gridview::NAME == col)
    {
        if (m_market.valid() && (row >= 0))
        {
            auto runners = m_market.getRunners();
            const int num_runners = static_cast<int>(runners.size());
            if (row < num_runners)
            {
                std::shared_ptr<betfair::TRunner> this_runner = runners[static_cast<std::size_t>(row)];
                if (this_runner->isActive())
                {
                    std::int64_t id = this_runner->getID();
                    my_runner_analysis_chart.setSelectedRunner(id);
                    my_runner_analysis_chart.show();
                    my_runner_analysis_chart.raise();
                }
            }
        }
    }
    else
    {
        if ((row >= 0) && m_market.valid())
        {
            if (m_gridview_betting_enabled)
            {
                if (gridview::BACK1 == col || gridview::BACK2 == col || gridview::BACK3 == col || gridview::LAY1 == col || gridview::LAY2 == col || gridview::LAY3 == col)
                {
                    const bool backing_action = (gridview::BACK1 == col || gridview::BACK2 == col || gridview::BACK3 == col);
                    int lprice_index = 0;
                    if (gridview::LAY2 == col)
                    {
                        lprice_index = 1;
                    }
                    else if (gridview::LAY3 == col)
                    {
                        lprice_index = 2;
                    }
                    int bprice_index = 0;
                    if (gridview::BACK2 == col)
                    {
                        bprice_index = 1;
                    }
                    else if (gridview::BACK3 == col)
                    {
                        bprice_index = 2;
                    }
                    auto runners = m_market.getRunners();
                    const int num_runners = static_cast<int>(runners.size());
                    const QString m_id = m_market.getMarketID();
                    if (d_stake > 0.0)
                    {
                        if (row < num_runners)
                        {
                            std::shared_ptr<betfair::TRunner> this_runner = runners[static_cast<std::size_t>(row)];
                            if (this_runner->isActive())
                            {
                                QString runner_name = this_runner->getName();
                                double d_actual_odds = 0.0;
                                QString str_odds = "0.0";

                                std::vector<std::int64_t> v_sels;
                                v_sels.push_back(this_runner->getID());

                                int tick_offset = ui->chkTickOffset->isChecked() ? ui->sbTickOffset->value() : 0;
                                if (backing_action)
                                {
                                    std::pair<double,double> bp = this_runner->getOrderedBackPrice(bprice_index);
                                    double d_odds = bp.first;

                                    if (d_odds > 1.0)
                                    {
                                        // Available reported odds plus an offset
                                        str_odds = QString::number(d_odds,'f',2);
                                        if (tick_offset != 0)
                                        {
                                            str_odds = mapOddsBySignedOffset(str_odds, tick_offset);
                                        }
                                        d_actual_odds = str_odds.toDouble();
                                        bf_man.placeBets(m_id, v_sels, d_actual_odds, d_stake, false, m_bet_persistence_flag);
                                        QString line = "Attempting to BACK " + runner_name + " @ " + str_odds + " for £" + QString::number(d_stake,'f',2) + " (from grid view)";
                                        logEvent(line);
                                    }
                                    else
                                    {
                                        QMessageBox::information(this, "Grid View Error", "Invalid back odds of " + QString::number(d_odds,'f',2));
                                    }
                                }
                                else
                                {
                                    std::pair<double,double> lp = this_runner->getOrderedLayPrice(bprice_index);
                                    double d_odds = lp.first;
                                    if (d_odds > 1.0)
                                    {

                                        str_odds = QString::number(d_odds,'f',2);
                                        if (tick_offset != 0)
                                        {
                                            str_odds = mapOddsBySignedOffset(str_odds, tick_offset);
                                        }
                                        d_actual_odds = str_odds.toDouble();
                                        bf_man.placeBets(m_id, v_sels, d_actual_odds, d_stake, true, m_bet_persistence_flag);
                                        QString line = "Attempting to LAY " + runner_name + " @ " + str_odds + " for £" + QString::number(d_stake,'f',2) + " (from grid view)";
                                        logEvent(line);
                                    }
                                    else
                                    {
                                        QMessageBox::information(this, "Grid View Error", "Invalid lay odds of " + QString::number(d_odds,'f',2));
                                    }
                                }
                            }
                        }
                    }
                }
                else if (gridview::HEDGE == col)
                {
                    // Hedge runner position here at whatever the available back or lay is
                    const QString m_id = m_market.getMarketID();
                    auto runners = m_market.getRunners();
                    const int num_runners = static_cast<int>(runners.size());
                    if (row < num_runners)
                    {
                        std::shared_ptr<betfair::TRunner> this_runner = runners[static_cast<std::size_t>(row)];
                        const std::int64_t sel_id = this_runner->getID();
                        QString info;
                        betfair::utils::betInstruction bet = m_market.hedgeRunnerAtAvailableOdds(sel_id, m_id, info);
                        if (bet.selection == sel_id && bet.market_id == m_id)
                        {
                            if (bet.stake > 0.0 && bet.odds > 1.0)
                            {
                                std::vector<std::int64_t> v_sels;
                                v_sels.push_back(sel_id);
                                bf_man.placeBets(m_id, v_sels, bet.odds, bet.stake, bet.b_lay_type, m_bet_persistence_flag);
                                QString type = bet.b_lay_type ? "LAY " : "BACK ";
                                QString line = "Attempting to " + type + (this_runner->getName()) + " @ " + QString::number(bet.odds,'f',2) + " for £" + QString::number(bet.stake,'f',2) + " (Hedge action from grid view)";
                            }
                        }
                        logEvent(info);
                    }
                }
            }
            else
            {
                QMessageBox::information(this, "Grid View Betting Disabled", "To place bets, enable grid view betting from\nthe \"Program Settings\" dialog.");
            }
        }
    }
}

//====================================================================
void MainWindow::updateMarketDetailsGroupBox()
{
    QString current_time = QDateTime::currentDateTimeUtc().toString("HH:mm:ss");
    QString window_title = "Betfair Trading Application (version " + m_software_version_num + ")";
    updateTimeToMarketStart();
    QString trade_vol = "";
    if (m_market.valid())
    {
        // update book percentages
        double backpct = m_market.getBackSideBookPercentage();
        double laypct = m_market.getLaySideBookPercentage();
        ui->lblMarketLayPct->setText(QString::number(laypct,'f',2) + "%");
        ui->lblMarketBackPct->setText(QString::number(backpct,'f',2) + "%");

        // Update market status
        QString market_status = m_market.getMarketStatus();
        if (market_status == "OPEN")
        {
            ui->selectedMarketLabel->setStyleSheet("QLabel { background-color : #111122; color : white;}");
        }
        else if (market_status == "CLOSED" || market_status == "INACTIVE")
        {
            ui->selectedMarketLabel->setStyleSheet("QLabel { background-color : none; color : #888888;}");
        }
        else if (market_status == "SUSPENDED")
        {            
            ui->selectedMarketLabel->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");

        }
        // Update IP status
        if (false == m_market.marketInPlay())
        {
            ui->marketInPlayLabel->setText("");
            ui->marketInPlayLabel->setEnabled(false);
            ui->marketInPlayLabel->setStyleSheet("QLabel { background-color : none; color : black; }");
        }
        else
        {
            ui->marketInPlayLabel->setText("IN PLAY");
            ui->marketInPlayLabel->setEnabled(true);
            ui->marketInPlayLabel->setStyleSheet("QLabel { background-color : #33CC00; color : white;}");
        }
        window_title.append("   " + m_market.getLabel() + " [" + QString::number(m_market.getMarketVersion()) + "]");
        double d_vol = m_market.getTotalMatchedVolume();
        if (d_vol > 0.0)
        {
            if (d_vol > 1000.0)
            {
                trade_vol = ("£" + QString::number(d_vol/1000.0,'f',1) + "K");
            }
            else
            {
                trade_vol = ("£" + QString::number(d_vol,'f',2));
            }
        }
        ui->marketCrossMatchedLabel->setVisible(m_market.crossMatchingEnabled());
    }
    else
    {
        ui->selectedMarketLabel->setText("");
        ui->marketInPlayLabel->setText("");
        ui->marketInPlayLabel->setEnabled(false);
        ui->marketCrossMatchedLabel->setVisible(false);
    }

    if (trade_vol.isEmpty())
    {
        ui->gbMarketVolChart->setTitle("Market Volume ");
    }
    else
    {
            ui->gbMarketVolChart->setTitle("Market Volume = " + trade_vol);
    }
    setWindowTitle(window_title);
}

//====================================================================
void MainWindow::refreshBets()
{
    if (m_market.valid())
    {
        if (false == m_market.closed())
        {
            QString id = (m_market.getMarketID());
            bf_man.retrieveAllBetsForMarket(id);
            bf_man.getMarketProfitAndLoss(id);
        }
    }
}

//====================================================================
void MainWindow::refreshSelectedMarket()
{
    if (m_market.valid())
    {
        if (false == m_market.closed())
        {
            bf_man.getMarketBook((m_market.getMarketID()));
        }
    }
    std::vector<QString> errors;
    bf_man.getGUIErrorMessages(errors);
    for (const QString& err : errors)
    {
        logEvent(err);
    }
}

//====================================================================
void MainWindow::processGetAccountFunds()
{
    ui->lblAccountBalance->setText("£ " + QString::number(bf_man.getAvailableBalance(),'f',2));
}

//====================================================================
void MainWindow::processMarketPAndLResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }
    updateMarketModel();
    updateInPlayModel();
    updateLadderModels();
}

//====================================================================
void MainWindow::processListMarketBookResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }
    updateMarketModel();
    updateInPlayModel();
    updateLadderModels();
    updateChartDialogs();

    // Update chart
    if (m_market.valid())
    {
        if (ui->cbLadder1Select->count() == 0)
        {
            m_ladder_views_one.updateAllData();
            m_ladder_views_two.updateAllData();
            m_ladder_views_three.updateAllData();
            m_ladder_views_four.updateAllData();
            // When combo box lists are empty, populate and
            // auto set to top 3 priced runners
            updateSelectionComboBox(ui->cbLadder1Select);
            updateSelectionComboBox(ui->cbLadder2Select);
            updateSelectionComboBox(ui->cbLadder3Select);
            updateSelectionComboBox(ui->cbLadder4Select);
            auto runners = m_market.getRunners();
            int ct = 0;
            for (auto it = runners.begin(); it != runners.end(); ++it)
            {
                if (*it)
                {
                    if (0 == ct)
                    {
                        ui->cbLadder1Select->setCurrentIndex(0);                       
                    }
                    else if (1 == ct)
                    {
                        ui->cbLadder2Select->setCurrentIndex(1);                       
                    }
                    else if (2 == ct)
                    {
                        ui->cbLadder3Select->setCurrentIndex(2);                       
                    }
                    else if (3 == ct)
                    {
                        ui->cbLadder4Select->setCurrentIndex(3);
                    }
                    ++ct;
                    if (ct > 3)
                    {
                        break;
                    }
                }
            }
            my_runner_charts.reset();
            updateMarketModel();
            updateInPlayModel();
            updateLadderModels();
            updateChartDialogs();
        }
        loadSilks();
    }
    updateMarketDetailsGroupBox();
}

//====================================================================
void MainWindow::onCentreLadderOneClicked()
{
    zoomLadderToLPM(m_ladder_views_one,ui->ladderView1);
}

//====================================================================
void MainWindow::onCentreLadderTwoClicked()
{
    zoomLadderToLPM(m_ladder_views_two,ui->ladderView2);
}

//====================================================================
void MainWindow::onCentreLadderThreeClicked()
{
    zoomLadderToLPM(m_ladder_views_three,ui->ladderView3);
}

//====================================================================
void MainWindow::onCentreLadderFourClicked()
{
    zoomLadderToLPM(m_ladder_views_four,ui->ladderView4);
}

//====================================================================
void MainWindow::changeLadder1SelectedRunner(int index)
{
    changeLadderSelectedRunner(ui->cbLadder1Select, m_ladder_views_one, ui->ladderView1, index);
}

//====================================================================
void MainWindow::changeLadder2SelectedRunner(int index)
{
    changeLadderSelectedRunner(ui->cbLadder2Select, m_ladder_views_two, ui->ladderView2, index);
}

//====================================================================
void MainWindow::changeLadder3SelectedRunner(int index)
{
    changeLadderSelectedRunner(ui->cbLadder3Select, m_ladder_views_three, ui->ladderView3, index);
}

//====================================================================
void MainWindow::changeLadder4SelectedRunner(int index)
{
    changeLadderSelectedRunner(ui->cbLadder4Select, m_ladder_views_four, ui->ladderView4, index);
}

//====================================================================
void MainWindow::onLadder1ShowChartClicked()
{
    std::int64_t id = m_ladder_views_one.getSelectionID();
    if (id > 0 && m_market.valid())
    {
        my_runner_analysis_chart.setSelectedRunner(id);
        my_runner_analysis_chart.show();
        my_runner_analysis_chart.raise();
    }
}

//====================================================================
void MainWindow::onLadder2ShowChartClicked()
{
    std::int64_t id = m_ladder_views_two.getSelectionID();
    if (id > 0 && m_market.valid())
    {
        my_runner_analysis_chart.setSelectedRunner(id);
        my_runner_analysis_chart.show();
        my_runner_analysis_chart.raise();
    }
}

//====================================================================
void MainWindow::onLadder3ShowChartClicked()
{
    std::int64_t id = m_ladder_views_three.getSelectionID();
    if (id > 0 && m_market.valid())
    {
        my_runner_analysis_chart.setSelectedRunner(id);
        my_runner_analysis_chart.show();
        my_runner_analysis_chart.raise();
    }
}

//====================================================================
void MainWindow::onLadder4ShowChartClicked()
{
    std::int64_t id = m_ladder_views_four.getSelectionID();
    if (id > 0 && m_market.valid())
    {
        my_runner_analysis_chart.setSelectedRunner(id);
        my_runner_analysis_chart.show();
        my_runner_analysis_chart.raise();
    }
}

//====================================================================
void MainWindow::changeLadderSelectedRunner(QComboBox* cb,
                                            LadderViewModel& ladder,
                                            QTableView* lv,
                                            int index)
{
    std::int64_t new_sel = 0;
    if (index >= 0 && m_market.valid())
    {
        QString selname = cb->itemText(index);
        if (selname.isEmpty() == false)
        {
            auto runner = m_market.getSelectionByName(selname);
            if (runner)
            {
                new_sel = runner->getID();
            }
        }
    }
    if (ladder.getSelectionID() != new_sel)
    {
        if (index >= 0 && index < 4)
        {
            my_vwap_xo_array[index]->clear();
        }
        ladder.setSelectedRunner(new_sel);
        zoomLadderToLPM(ladder,lv);        
    }
}

//====================================================================
void MainWindow::zoomLadderToLPM(LadderViewModel& ladder, QTableView* lv)
{
    int lpm_index = ladder.getLastLPMRow();
    lv->scrollTo(ladder.index(lpm_index,3),QAbstractItemView::PositionAtCenter);
    ladder.updateAllData();
}

//====================================================================
void MainWindow::updateSelectionComboBox(QComboBox* selbox)
{
    if (selbox != nullptr)
    {
        selbox->clear();
        if (m_market.valid())
        {
            const std::vector<std::shared_ptr<betfair::TRunner> >& runners = m_market.getRunners();
            for (auto it = runners.begin(); it != runners.end(); ++it)
            {
                const QString selection_name = (*it)->getName();
                selbox->addItem(selection_name);
                // Add silks icon if it exists
                QString fname = m_images_path + "/" + QString::number((*it)->getID()) + ".jpg";
                QFileInfo finfo(fname);
                if (finfo.exists())
                {
                    QPixmap pixmap(fname);
                    selbox->setItemIcon(selbox->count()-1,pixmap);
                }
            }
        }
        selbox->setCurrentIndex(-1);
    }
}

//====================================================================
void MainWindow::initialiseLadderStakeComboBox(QComboBox* selbox)
{
    if (selbox != nullptr)
    {
        selbox->clear();
        selbox->addItem("2.00");
        selbox->addItem("5.00");
        selbox->addItem("10.00");
        selbox->addItem("20.00");
        selbox->addItem("50.00");
        selbox->setCurrentIndex(-1);
    }
}


//====================================================================
void MainWindow::selectMarket()
{
    ui->selectedMarketLabel->setText("");
    ui->marketInPlayLabel->setText("");
    ui->marketInPlayLabel->setEnabled(false);
    ui->marketCrossMatchedLabel->setVisible(false);
    ui->marketInPlayLabel->setStyleSheet("QLabel { background-color : none; color : black; }");
    if (m_market.valid())
    {
        m_market.logMarketBetData();
    }
    if (ui->marketsComboBox->currentIndex() >= 0)
    {
        QString current_market_id = (m_market.getMarketID());
        int market_index = ui->marketsComboBox->currentIndex();
        ui->btnShowRunnerCharts->setEnabled(true);
        std::map<int,QString>::iterator it = m_market_list_index_map.find(market_index);
        if (it != m_market_list_index_map.end())
        {            
            // market index found
            const QString selected_mkt = it->second;
            if (current_market_id != selected_mkt)
            {                
                // New market selected
                betfair::TMarketInfo inf = bf_man.getMarketInfoByID(selected_mkt);

                // Download silks
                std::map<QString,QString> silks_map;
                auto runners = inf.getAllMetaData();
                for (auto rit = runners.begin(); rit != runners.end(); ++rit)
                {
                    QString target = betfair_url_base + rit->second.colours_filename;
                    QString save_path = m_images_path + "/" +  QString::number(rit->first) + ".jpg";
                    silks_map[target] = save_path;
                }
                m_silks_downloader.setFileList(silks_map);
                m_silks_downloader.downloadCurrentFileList();

                m_market.setMarketInfo(inf);
                if (false == m_market.valid())
                {
                    throw std::runtime_error("ERROR - betfair::TBetfairMarket instance is not valid!!");
                }
                ui->marketInPlayLabel->setText("");
                ui->marketInPlayLabel->setEnabled(false);
                //ui->marketStatusLabel->setText("");
                //ui->marketStatusLabel->setEnabled(false);
                resetModels();
                resetChartDialogs();
                ui->cbLadder1Select->clear();
                ui->cbLadder2Select->clear();
                ui->cbLadder3Select->clear();
                ui->cbLadder4Select->clear();

                m_ladder_views_one.setSelectedRunner(0);
                m_ladder_views_two.setSelectedRunner(0);
                m_ladder_views_three.setSelectedRunner(0);
                m_ladder_views_four.setSelectedRunner(0);

                //setMarketSilksImages();

            }
            QString mlabel = (m_market.getLabel());
            ui->selectedMarketLabel->setText(mlabel);

            // request market list from bf_man
            bf_man.getMarketBook(selected_mkt);
            // update GUI components and models
            m_ladder_views_one.updateAllData();
            m_ladder_views_two.updateAllData();
            m_ladder_views_three.updateAllData();
            m_ladder_views_four.updateAllData();
            updateLadderModels();
            updateChartDialogs();
        }
    }
}

//====================================================================
void MainWindow::loadSilks()
{
    // Silks should be available now for our loaded market   
    if (m_market.valid())
    {
        auto runners = m_market.getRunners();
        for (auto runner : runners)
        {
            const QString runner_name = runner->getName();
            QString fname = m_images_path + QString::number(runner->getID()) + ".jpg";
            QFileInfo finfo(fname);
            if (finfo.exists())
            {
                QPixmap pixmap(fname);
                setComboBoxIcon(ui->cbLadder1Select,runner_name,pixmap);
                setComboBoxIcon(ui->cbLadder2Select,runner_name,pixmap);
                setComboBoxIcon(ui->cbLadder3Select,runner_name,pixmap);
                setComboBoxIcon(ui->cbLadder4Select,runner_name,pixmap);          
            }
        }
    }
}

//====================================================================
void MainWindow::setComboBoxIcon(QComboBox* combo,
                                 const QString& runnername,
                                 const QPixmap& ico)
{
    if ((combo != nullptr) && (!ico.isNull()) && (!runnername.isEmpty()))
    {
        const int size = combo->count();
        for (int i = 0; i < size; ++i)
        {
            if (combo->itemText(i) == runnername)
            {
                if (combo->itemIcon(i).isNull())
                {
                    combo->setItemIcon(i,ico);
                    break;
                }
            }
        }
    }
}

//====================================================================
void MainWindow::updateModels()
{
    updateMarketModel();
    updateInPlayModel();
    updatePlacesdBetModels();
}

//====================================================================
void MainWindow::updatePlacesdBetModels()
{
    if ((ui->mainTabWidget->currentIndex() == 0) || (ui->mainTabWidget->currentIndex() == 3))
    {
        QDateTime start = QDateTime::currentDateTimeUtc();
        my_market_placed_bets_model.updateData();
        my_market_unmatched_bets_model.updateData();        
        qint64 msdiff = start.msecsTo(QDateTime::currentDateTimeUtc());
        QString msg("Updating placed bet models took " + QString::number(msdiff) + " milliseconds\n");
        logGUIUpdateMsg(msg);
    }
}

//====================================================================
void MainWindow::resetModels()
{
    my_market_model.refresh();
    my_market_placed_bets_model.refresh();
    my_market_unmatched_bets_model.refresh();
    my_inplay_model.refresh();
}


//====================================================================
void MainWindow::updateChartDialogs()
{
    my_runner_charts.updateChart();
    my_runner_analysis_chart.updateChart();
    my_market_vol_chart_wrapper.populateSeries(m_market);
}

//====================================================================
void MainWindow::resetChartDialogs()
{
    my_runner_charts.reset();
    my_runner_analysis_chart.reset();
    my_market_vol_chart_wrapper.clear();
    my_vwap_xo_array[0]->clear();
    my_vwap_xo_array[1]->clear();
    my_vwap_xo_array[2]->clear();
    my_vwap_xo_array[3]->clear();
}

//====================================================================
void MainWindow::updateMarketModel()
{
    if (ui->mainTabWidget->currentIndex() == 0)
    {
        my_market_model.updateData();
    }
}

//====================================================================
void MainWindow::updateInPlayModel()
{
    if (ui->mainTabWidget->currentIndex() == 2)
    {
        my_inplay_model.updateData();
    }
}

//====================================================================
void MainWindow::updateLadderModels()
{
    if (ui->mainTabWidget->currentIndex() == 1)
    {
        updateLadder1();
        updateLadder2();
        updateLadder3();
        updateLadder4();
    }
}

//====================================================================
void MainWindow::cancelLadderBets(LadderViewModel& ladder, bool lays)
{
    // Cancel back/lay bets for this ladders selected runner.
    std::int64_t selid = ladder.getSelectionID();
    QString mid = m_market.getMarketID();
    if (m_market.valid() && (selid > 0))
    {
        std::vector<QString> bets = m_market.getSelectionUnmatchedBetIDs(selid,
                                                                         mid,
                                                                         lays);
        if (!bets.empty())
        {
            bf_man.cancelBets(mid,bets);
        }
    }
}


//====================================================================
void MainWindow::cancelLadderBetsAtPrice(LadderViewModel& ladder,
                                         const double& price_to_cancel,
                                         bool lays)
{
    // Cancel back/lay bets for this ladders selected runner AT FIXED PRICE!

    std::int64_t selid = ladder.getSelectionID();
    QString mid = m_market.getMarketID();
    if (m_market.valid() && (selid > 0))
    {

        std::vector<QString> bets = m_market.getSelectionUnmatchedBetByAskPriceIDs(selid,
                                                                                   mid,
                                                                                   price_to_cancel,
                                                                                   lays);
        if (!bets.empty())
        {
            bf_man.cancelBets(mid,bets);
        }
    }
}


//====================================================================
void MainWindow::updateLadder1()
{
    updateLadder(ui->ladderView1, m_ladder_views_one, ui->lblLadder1_PL,
                 ui->lblLadder1_BackWOM, ui->lblLadder1_LayWOM, ui->lblLadder1TotalTraded,
                 my_vwap_xo_array[0]);
}

//====================================================================
void MainWindow::updateLadder2()
{
    updateLadder(ui->ladderView2, m_ladder_views_two, ui->lblLadder2_PL,
                 ui->lblLadder2_BackWOM, ui->lblLadder2_LayWOM, ui->lblLadder2TotalTraded,
                 my_vwap_xo_array[1]);
}

//====================================================================
void MainWindow::updateLadder3()
{
    updateLadder(ui->ladderView3, m_ladder_views_three, ui->lblLadder3_PL,
                 ui->lblLadder3_BackWOM, ui->lblLadder3_LayWOM, ui->lblLadder3TotalTraded,
                 my_vwap_xo_array[2]);
}

//====================================================================
void MainWindow::updateLadder4()
{
    updateLadder(ui->ladderView4, m_ladder_views_four, ui->lblLadder4_PL,
                 ui->lblLadder4_BackWOM, ui->lblLadder4_LayWOM, ui->lblLadder4TotalTraded,
                 my_vwap_xo_array[3]);
}

//====================================================================
void MainWindow::updateLadder(QTableView* lv, LadderViewModel& ladder, QLabel* pl,
                              QLabel* bwom_label, QLabel* lwom_label, QLabel* total_vol_label,
                              charting::TVWAPIntegralChartWrapper* vwap_chart)

{
    QModelIndex topLeft = lv->indexAt(lv->rect().topLeft());
    QModelIndex bottomRight = lv->indexAt(lv->rect().bottomRight());
    ladder.updateVisibleData(topLeft,bottomRight);
    QString laywom = "";
    QString backwom = "";
    QString pandl = "£0.00";
    QString total = "£0.00";
    std::int64_t sel_id = ladder.getSelectionID();
    pl->setStyleSheet("QLabel { background-color : #222222; color : white;}");
    if (m_market.valid() && sel_id > 0)
    {
        // Calculate WOMs
        double back = 0.0;
        double lay = 0.0;

        m_market.getSelectionWOM(sel_id,back,lay,m_wom_calculation_depth);
        double womtotal = back + lay;
        if (womtotal > 0.0)
        {
            laywom = QString::number(100.0*(lay/womtotal),'f',1) + "%";
            backwom = QString::number(100.0*(back/womtotal),'f',1) + "%";
        }
        // Get P&L and determine potential hedge P&L for each ladder price
        double pft = m_market.getSelectionProfitIfWins(sel_id);
        if (pft > 0.0)
        {
            //ui->lblLadder1_PL->setStyleSheet("QLabel { color : green; }");
            pl->setStyleSheet("QLabel { background-color : #33CC33; color : white;}");
            pandl = "£" + QString::number(pft,'f',2);
        }
        else if (pft < 0.0)
        {
            //ui->lblLadder1_PL->setStyleSheet("QLabel { color : red; }");            
            pl->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");
            pandl = "-£" + QString::number(-pft,'f',2);
        }
        double market_vol = m_market.getTotalTradedVolume(sel_id);
        if (market_vol > 1000.0)
        {
            total = "£" + QString::number(market_vol/1000.0,'f',1) + "K";
        }
        else
        {
            total = "£" + QString::number(market_vol,'f',0);
        }
        if (vwap_chart != nullptr)
        {
            auto runner = m_market.getSelectionByID(sel_id);
            if (runner)
            {
                auto candles = runner->getAdvCandleData();
                vwap_chart->populateSeries(candles,sel_id);
            }
        }
    }
    lwom_label->setText(laywom);
    bwom_label->setText(backwom);
    pl->setText(pandl);
    total_vol_label->setText(total);

}

//====================================================================
void MainWindow::updateTimeToMarketStart()
{
    if (m_market.valid())
    {
        const bool closed = m_market.closed();
        if (closed)
        {
            ui->lblStartTimeCountdown->setText(" MARKET CLOSED ");
            ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #666666; color : white;}");
        }
        else
        {
            QDateTime mkstarttime = m_market.getStartTime();//QDateTime::fromString((m_market.getFullStartTime()),"yyyy-MM-ddTHH:mm:ss.zzzZ");

            qint64 seconds_to_start = secondsUntil(mkstarttime);
            const QString time_to_go = timeToStart(seconds_to_start);
            if (seconds_to_start < 0)
            {
                // overdue
                if (m_market.marketInPlay())
                {
                    //ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #ff1a1a; color : white;}");
                    ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #33CC33; color : white;}");
                }
                else
                {
                    // Not in play but overdue - continue flashing
                    if (m_inplay_flash_flag)
                    {
                        ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #222222; color : white;}");
                    }
                    else
                    {
                        ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");
                    }
                    m_inplay_flash_flag = !m_inplay_flash_flag;
                }
            }
            else
            {
                if (seconds_to_start - 60 < 0)
                {
                    // less than a minute to go = start flashing!!
                    if (m_inplay_flash_flag)
                    {
                        ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #222222; color : white;}");
                    }
                    else
                    {
                        ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");
                    }
                    m_inplay_flash_flag = !m_inplay_flash_flag;
                }
                else
                {
                    ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : #222222; color : white;}");
                }

            }
            ui->lblStartTimeCountdown->setText(time_to_go);
        }
    }
    else
    {
        ui->lblStartTimeCountdown->setStyleSheet("QLabel { background-color : none; color : #222222; }");
        ui->lblStartTimeCountdown->setText("");
    }
}

//====================================================================
void MainWindow::onSelectedEventChange()
{
    // This slot is connected to the event selection change signal
    // We must get the market filter from that dialog and make a listMarketCatalogue
    // request with the market filter via the market manager
    if (bf_man.isLoggedIn())
    {
        QString market_filter = my_event_selector.getMarketFilter();
        bf_man.makeGenericListMarketCatalogueRequest(market_filter);
    }
}

//====================================================================
void MainWindow::processLogin()
{
    QString err = bf_man.getErrorString();

    QString show_info = "Login status: ";
    show_info.append(err);
    logEvent(show_info);
    if (bf_man.isLoggedIn())
    {
        logEvent("Successfully logged in.");

        ui->loginStatusLabel->setText("Logged in as ****************");
        ui->loginStatusLabel->setStyleSheet("QLabel { background-color : #33CC33; color : white;}");
        bf_man.getAccountFunds();
        bf_man.getTodaysUKHorseMarkets();
    }
    else
    {
        logEvent("Login attempt failed.");
        ui->loginStatusLabel->setStyleSheet("QLabel { background-color : #dd2222; color : white;}");
        ui->loginStatusLabel->setText("Not Logged In");
        ui->marketsComboBox->clear();
        ui->marketsComboBox->setCurrentIndex(-1);
        ui->marketsComboBox->setEnabled(false);
    }
}

//====================================================================
void MainWindow::onProgramSettingsChange()
{
    m_selected_market_update_timer->setInterval(my_program_settings.getDataUpdateRateMillisec());
    m_update_bets_timer->setInterval(my_program_settings.getBetUpdateRateMillisec());
    m_wom_calculation_depth = my_program_settings.getWOMCalcDepth();
    m_bet_persistence_flag = my_program_settings.getBetPersistenceFlag();
    m_gridview_betting_enabled = my_program_settings.getGridViewBettingEnabledFlag();
    m_ladderview_betting_enabled = my_program_settings.getLadderViewBettingEnabledFlag();

    m_inplayview_betting_enabled = my_program_settings.getInplayViewBettingEnabledFlag();

    int hi = my_program_settings.getLadderHistogramIntervalUpper();
    int lo = my_program_settings.getLadderHistogramIntervalLower();
    m_ladder_views_one.setTradeHistorySecondsHi(hi);
    m_ladder_views_one.setTradeHistorySecondsLo(lo);
    m_ladder_views_two.setTradeHistorySecondsHi(hi);
    m_ladder_views_two.setTradeHistorySecondsLo(lo);
    m_ladder_views_three.setTradeHistorySecondsHi(hi);
    m_ladder_views_three.setTradeHistorySecondsLo(lo);
    m_ladder_views_four.setTradeHistorySecondsHi(hi);
    m_ladder_views_four.setTradeHistorySecondsLo(lo);

    my_market_model.setBettingEnabled(m_gridview_betting_enabled);
    my_inplay_model.setBettingEnabled(m_inplayview_betting_enabled);
    m_ladder_views_one.setBettingEnabled(m_ladderview_betting_enabled);
    m_ladder_views_two.setBettingEnabled(m_ladderview_betting_enabled);
    m_ladder_views_three.setBettingEnabled(m_ladderview_betting_enabled);
    m_ladder_views_four.setBettingEnabled(m_ladderview_betting_enabled);



    QTableView* tvarr[4] = {ui->ladderView1,ui->ladderView2,ui->ladderView3,ui->ladderView4};

    for (unsigned int i = 0; i < 4; ++i)
    {
        ProgressBarDelegate* pbardel = dynamic_cast<ProgressBarDelegate*>(tvarr[i]->itemDelegateForColumn(6));
        if (pbardel != nullptr)
        {
            pbardel->setColourTotal(my_program_settings.getLadderHistogramColorTotal());
            pbardel->setColourUpper(my_program_settings.getLadderHistogramColorUpper());
            pbardel->setColourLower(my_program_settings.getLadderHistogramColorLower());

        }
    }

    m_auto_offset_bet_placement_ticks = my_program_settings.getAutoBetPlacementOffset();
}

//====================================================================
void MainWindow::login()
{
    // Get login data from login form, and feed to manager
    if (false == bf_man.isLoggedIn())
    {
        bf_man.setLoginDetails(my_login.GetUsername(), my_login.GetPassword(),
                               my_login.GetCertFile(), my_login.GetKeyFile(),
                               my_login.GetAppKey());
        bf_man.attemptLogin();
    }
}


//====================================================================
void MainWindow::processNetworkError()
{
    QString err = bf_man.getErrorString();
    logEvent(err);
}

//====================================================================
void MainWindow::processCancelOrdersResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }

    // We wont actually remove the bets here we will leave that to
    // the next listCurrent orders call.
}

//====================================================================
void MainWindow::processListMarketCatalogueResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }
    const std::vector<betfair::TMarketInfo>& markets = bf_man.getMarketInfoList();
    if (!markets.empty())
    {
        m_market_list_index_map.clear();
        ui->marketsComboBox->clear();
        ui->marketsComboBox->setEnabled(true);
        ui->selectMarketButton->setEnabled(true);
        int market_it = 0;
        for (auto it=markets.begin();it!=markets.end();++it)
        {
            QString market_id = (it->getMarketID());
            QString label = (it->getLabel());
            ui->marketsComboBox->addItem(label);
            m_market_list_index_map.insert(std::make_pair(market_it,market_id));
            ++market_it;                        
        }
        updateModels();
        updateLadderModels();                      
    }
}

/*
//====================================================================
void MainWindow::processListEventTypesResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {        
        logEvent(err);
    }
    const std::map<int, QString>& events = bf_man.getEventTypesList();
    if (events.empty() == false)
    {
        //std::map<int,QString>::const_iterator it;
        ui->eventTypesComboBox->clear();
        ui->eventsComboBox->clear();
        ui->eventsComboBox->setEnabled(false);
        ui->eventTypesComboBox->setEnabled(true);
        ui->selectEventButton->setEnabled(true);
        for (auto it=events.begin();it!=events.end();++it)
        {
            ui->eventTypesComboBox->addItem(it->second);
        }
        ui->selectEventTypeButton->setEnabled(true);
    }
}
*/

//====================================================================
void MainWindow::processPlaceBetsResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }
    if (m_market.getNumFailedBets() > 0)
    {
        const std::vector<std::shared_ptr<betfair::TBet> >& v_f_bets = m_market.getFailedBets();
        for (auto it = v_f_bets.begin(); it != v_f_bets.end(); ++it)
        {
            QString error = "Bet failure: ";
            error.append(((*it)->getSelectionName()));
            error.append(" £");
            error.append(QString::number((*it)->getStake(),'f',2));
            error.append(" @ ");
            error.append(QString::number((*it)->getAskPrice(),'f',2));
            error.append(" ");
            error.append(((*it)->getError()));
            logEvent(error);
        }
        m_market.clearFailedBets();
    }
    m_market.updateBetLists();
    updatePlacesdBetModels();
    updateLadderModels();
    bf_man.getAccountFunds();
}

//====================================================================
void MainWindow::processListCurrentOrdersResp()
{
    QString err = bf_man.getErrorString();
    if (err.length() > 0)
    {
        logEvent(err);
    }
    m_market.updateBetLists();
    updatePlacesdBetModels();
    updateLadderModels();
}


/*
//====================================================================
void MainWindow::processListEventsResp()
{
    QString err = bf_man.getErrorString();
    logEvent(err);
    const std::map<QString, QString>& events = bf_man.getEventsList();
    if (events.empty() == false)
    {
        ui->eventsComboBox->clear();
        ui->eventsComboBox->setEnabled(true);
        ui->selectEventButton->setEnabled(true);
        for (auto it=events.begin();it!=events.end();++it)
        {
            ui->eventsComboBox->addItem(it->second);
        }
    }
}
*/


//====================================================================
void MainWindow::logGUIUpdateMsg(const QString& msg)
{
    if (m_log_gui_update_times_to_file)
    {
        m_gui_message_buffer.push_back((QDateTime::currentDateTimeUtc().toString("[yyyyMMMdd HH:mm:ss.zzz] ") + msg + "\n"));
        if (m_gui_message_buffer.size() > 100)
        {
            writeGUIUpdateMsgBufferToFile();
        }
    }
}
//====================================================================
void MainWindow::writeGUIUpdateMsgBufferToFile()
{
    if (!m_gui_message_buffer.empty())
    {
        QFile file(gui_update_log);
        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            for (auto it = m_gui_message_buffer.begin(); it != m_gui_message_buffer.end(); ++it)
            {
                out << *it;
            }
            file.close();
            m_gui_message_buffer.clear();
            m_gui_message_buffer.reserve(100);
        }
    }
}


