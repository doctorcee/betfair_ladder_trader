#include "eventselectiondlg.h"
#include "ui_eventselectiondlg.h"
#include <QDateTime>
#include "betfair/betfair_utils.h"



//====================================================================
TEventSelectionDlg::TEventSelectionDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TEventSelectionDlg),
    m_current_loaded_event(""),
    m_market_filter("")
{
    ui->setupUi(this);
    ui->btnTodaysUKHorseWinMarkets->setFocus();
    connect(ui->btnLoadDogMarkets,SIGNAL(clicked()),this,SLOT(onLoadDogMarkets()));
    connect(ui->btnTodaysUKHorseWinMarkets,SIGNAL(clicked()),this,SLOT(onLoadTodaysUKHorsesWIN()));
    connect(ui->btnTodaysUKIREHorseWinMarkets,SIGNAL(clicked()),this,SLOT(onLoadTodaysUKIREHorsesWIN()));    
    connect(ui->btnLoadCustomFootballMarkets,&QPushButton::clicked,this,&TEventSelectionDlg::onLoadCustomFootballMarkets);

    connect(ui->btnNextXHorsesByCountry,SIGNAL(clicked()),this,SLOT(onLoadCustomHorseMarkets()));
}

//====================================================================
TEventSelectionDlg::~TEventSelectionDlg()
{
    delete ui;
}

//====================================================================
void TEventSelectionDlg::onLoadCustomHorseMarkets()
{
    // create market filter according to the current GUI state customised horse market options

    std::vector<QString> v_countries;
    if (ui->cbSelectUKHorses->isChecked())
    {
        v_countries.push_back("GB");
    }
    if (ui->cbSelectIREHorses->isChecked())
    {
        v_countries.push_back("IE");
    }
    if (ui->cbSelectUSAHorses->isChecked())
    {
        v_countries.push_back("US");
    }
    if (ui->cbSelectAUSHorses->isChecked())
    {
        v_countries.push_back("AU");
    }
    if (ui->cbSelectRSAHorses->isChecked())
    {
        v_countries.push_back("ZA");
    }
    if (ui->cbSelectFRAHorses->isChecked())
    {
        v_countries.push_back("FR");
    }

    std::vector<QString> v_types;
    if (ui->cbSelectHorsesWINType->isChecked())
    {
        v_types.push_back("WIN");
    }
    if (ui->cbSelectHorsesPLACEType->isChecked())
    {
        v_types.push_back("PLACE");
    }
    int market_limit = ui->sbHorseMarketCount->value();    
    m_market_filter = betfair::utils::generateGenericLMCatFilter(v_countries,
                                                                 v_types,
                                                                 betfair::utils::event_type_horse_racing,
                                                                 market_limit,
                                                                 false,
                                                                 false);

    m_current_loaded_event = "CUSTOM SELECTION HORSE RACING MARKETS";
    emit selectedEventChanged();

}

//====================================================================
void TEventSelectionDlg::onLoadTodaysUKHorsesWIN()
{
    std::vector<QString> v_types;
    v_types.push_back("WIN");
    std::vector<QString> v_countries;
    v_countries.push_back("GB");    
    m_market_filter = betfair::utils::generateGenericLMCatFilter(v_countries,
                                                                 v_types,
                                                                 betfair::utils::event_type_horse_racing,
                                                                 100,
                                                                 false,
                                                                 true);
    m_current_loaded_event = "TODAYS UK HORSE RACING MARKETS";
    emit selectedEventChanged();
}

//====================================================================
void TEventSelectionDlg::onLoadTodaysUKIREHorsesWIN()
{
    std::vector<QString> v_types;
    v_types.push_back("WIN");
    std::vector<QString> v_countries;
    v_countries.push_back("GB");
    v_countries.push_back("IE");
    m_market_filter = betfair::utils::generateGenericLMCatFilter(v_countries,
                                                                 v_types,
                                                                 betfair::utils::event_type_horse_racing,
                                                                 100,
                                                                 false,
                                                                 true);

    m_current_loaded_event = "TODAYS UK AND IRELAND HORSE RACING MARKETS";
    emit selectedEventChanged();
}

//====================================================================
void TEventSelectionDlg::onLoadDogMarkets()
{
    std::vector<QString> v_countries;
    v_countries.push_back("GB");

    std::vector<QString> v_types;
    if (ui->cbSelectDogsWINType->isChecked())
    {
        v_types.push_back("WIN");
    }
    if (ui->cbSelectDogsPLACEType->isChecked())
    {
        v_types.push_back("PLACE");
    }
    if (ui->cbSelectDogsFCType->isChecked())
    {
        v_types.push_back("FORECAST");
    }    
    int market_limit = ui->sbDogsMarketCount->value();
    m_market_filter = betfair::utils::generateGenericLMCatFilter(v_countries,
                                                                 v_types,
                                                                 betfair::utils::event_type_dog_racing,
                                                                 market_limit,
                                                                 false,
                                                                 false);

    m_current_loaded_event = "GREYHOUND RACING MARKETS";
    emit selectedEventChanged();
}


//====================================================================
void TEventSelectionDlg::onLoadCustomFootballMarkets()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QDate sel_day = now.date();

    std::vector<QString> v_mktypes;
    std::vector<QString> v_competitions;
    std::vector<QString> v_countries;

    // Market type selections
    if (ui->chkMatchOdds->isChecked())      {v_mktypes.push_back("MATCH_ODDS");}
    if (ui->chkCorrectScore->isChecked())   {v_mktypes.push_back("CORRECT_SCORE");}
    if (ui->chkOver0_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_05");}
    if (ui->chkOver1_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_15");}
    if (ui->chkOver2_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_25");}
    if (ui->chkOver3_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_35");}
    if (ui->chkOver4_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_45");}
    if (ui->chkOver5_5->isChecked())        {v_mktypes.push_back("OVER_UNDER_55");}
    if (ui->chkHTFT->isChecked())           {v_mktypes.push_back("HALF_TIME_FULL_TIME");}

    // League/competition selections
    if (ui->chkEngPrem->isChecked())        {v_competitions.push_back("10932509");}
    if (ui->chkEngChamp->isChecked())       {v_competitions.push_back("7129730");}
    if (ui->chkEngLeague1->isChecked())     {v_competitions.push_back("35");}
    if (ui->chkEngLeague2->isChecked())     {v_competitions.push_back("37");}
    if (ui->chkEngFACup->isChecked())       {v_competitions.push_back("30558");}
    if (ui->chkChampionsLeague->isChecked()){v_competitions.push_back("228");}
    if (ui->chkEuropaLeague->isChecked())   {v_competitions.push_back("205");}
    if (ui->chkSpanishLaLiga->isChecked())  {v_competitions.push_back("117");}
    if (ui->chkSpanishSegunda->isChecked()) {v_competitions.push_back("119");}
    if (ui->chkGermanBundesliga1->isChecked()) {v_competitions.push_back("59");}
    if (ui->chkGermanBundesliga2->isChecked()) {v_competitions.push_back("61");}
    if (ui->chkItalySerieA->isChecked())    {v_competitions.push_back("81");}
    if (ui->chkItalySerieB->isChecked())    {v_competitions.push_back("83");}
    if (ui->chkFrenchLigue1->isChecked())    {v_competitions.push_back("55");}
    if (ui->chkFrenchLigue2->isChecked())    {v_competitions.push_back("57");}
    if (ui->chkPortPrem->isChecked())       {v_competitions.push_back("99");}
    if (ui->chkPortSegunda->isChecked())    {v_competitions.push_back("9513");}

    if (!v_competitions.empty() && !v_mktypes.empty())
    {
        createUKFootballMarketFilter(v_competitions,v_mktypes,sel_day);
    }
}

//====================================================================
void TEventSelectionDlg::createUKFootballMarketFilter(const std::vector<QString>& competition_ids,
                                                      const std::vector<QString>& market_types,
                                                      const QDate& sel_day)
{
    if (competition_ids.empty() && market_types.empty())
    {
        return;
    }

    QString qpayload("{\"filter\":{");
    QString comp_ids = "";
    QString mktypes = "";
    if (!competition_ids.empty())
    {
        comp_ids = "\"competitionIds\": [";
        for (auto cid : competition_ids)
        {
            comp_ids.append("\"" + cid + "\",");
        }
        comp_ids.chop(1);
        comp_ids.append("]");
    }
    if (!market_types.empty())
    {
        mktypes = "\"marketTypeCodes\": [";
        for (auto mt : market_types)
        {
            mktypes.append("\"" + mt + "\",");
        }
        mktypes.chop(1);
        mktypes.append("]");
    }

    QString start_time = sel_day.toString(QString("yyyy-MM-dd")) + QString("T00:01:01Z");
    QString end_time = sel_day.toString(QString("yyyy-MM-dd")) + QString("T23:59:00Z");
    qpayload.append(mktypes);
    if (!mktypes.isEmpty())
    {
        qpayload.append(",");
    }
    qpayload.append(comp_ids);
    if (!comp_ids.isEmpty())
    {
        qpayload.append(",");
    }
    qpayload.append(QString("\"marketStartTime\":{\"from\":\""));
    qpayload.append(start_time);
    qpayload.append(QString("\",\"to\":\""));
    qpayload.append(end_time);
    qpayload.append(QString("\"}}"));
    qpayload.append(QString(",\"sort\":\"FIRST_TO_START\",\"maxResults\":\"100\""));
    qpayload.append(QString(",\"marketProjection\":[\"MARKET_DESCRIPTION\",\"EVENT_TYPE\",\"RUNNER_METADATA\",\"MARKET_START_TIME\",\"EVENT\",\"COMPETITION\"]}"));

    m_market_filter = qpayload;

    emit selectedEventChanged();
}
