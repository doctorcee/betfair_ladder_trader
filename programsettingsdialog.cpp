#include "programsettingsdialog.h"
#include "ui_programsettingsdialog.h"
#include "betfair/betfair_utils.h"
#include <QPushButton>
#include <QMessageBox>
#include <QColorDialog>


//====================================================================
ProgramSettingsDialog::ProgramSettingsDialog(QJsonObject& settings,
                                             QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgramSettingsDialog),
    m_program_settings(settings),
    m_wom_calculation_depth(5),
    m_bet_update_rate_milliseconds(1000),
    m_data_update_rate_milliseconds(1000),
    m_bet_update_index(2),
    m_data_update_index(2),
    m_wom_depth_index(4),
    m_ladder_hist_interval_lo(60),
    m_ladder_hist_interval_hi(120),
    m_ladder_histogram_colour_array{betfair::utils::pbar3,betfair::utils::pbar1,betfair::utils::pbar2},
    m_unconfirmed_histogram_colour_array{betfair::utils::pbar3,betfair::utils::pbar1,betfair::utils::pbar2},
    m_customer_strategy_ref(""),
    m_place_bets_with_persistence(false),
    m_gridbet_enabled(false),
    m_ladderbet_enabled(false),
    m_ipbet_enabled(false),
    m_auto_bet_offset(0)
{
    ui->setupUi(this);
    ui->cbMarketSampleRate->addItem("200 milliseconds");
    ui->cbMarketSampleRate->addItem("500 milliseconds");
    ui->cbMarketSampleRate->addItem("1 second");
    ui->cbMarketSampleRate->addItem("2 seconds");
    ui->cbMarketSampleRate->addItem("5 seconds");
    ui->cbMarketSampleRate->addItem("10 seconds");
    ui->cbMarketSampleRate->setCurrentIndex(m_data_update_index);

    ui->cbBetsSampleRate->addItem("200 milliseconds");
    ui->cbBetsSampleRate->addItem("500 milliseconds");
    ui->cbBetsSampleRate->addItem("1 second");
    ui->cbBetsSampleRate->addItem("2 seconds");
    ui->cbBetsSampleRate->addItem("5 seconds");
    ui->cbBetsSampleRate->addItem("10 seconds");
    ui->cbBetsSampleRate->setCurrentIndex(m_bet_update_index);
    ui->cbWOMDepth->setCurrentIndex(m_wom_depth_index);

    ui->sbLadderHistInterval1->setValue(m_ladder_hist_interval_lo);
    ui->sbLadderHistInterval2->setValue(m_ladder_hist_interval_hi);
    ui->chkBetPersistenceFlag->setChecked(m_place_bets_with_persistence);
    ui->chkGridViewBettingEnabled->setChecked(m_gridbet_enabled);
    ui->chkLadderBettingEnabled->setChecked(m_ladderbet_enabled);
    ui->chkInPlayViewBettingEnabled->setChecked(m_ipbet_enabled);

    ui->chkAutoPlaceOffsetBet->setChecked(false);
    connect(ui->btnSaveSettings,&QPushButton::clicked,this,&ProgramSettingsDialog::onSettingsSaveClicked);

    connect(ui->btnTotalVolHistCol,&QPushButton::clicked,this,&ProgramSettingsDialog::onSetTotalTradedHistColour);
    connect(ui->btnLowerHistCol,&QPushButton::clicked,this,&ProgramSettingsDialog::onSetLowerTradedHistColour);
    connect(ui->btnUpperHistCol,&QPushButton::clicked,this,&ProgramSettingsDialog::onSetUpperTradedHistColour);

}

//====================================================================
void ProgramSettingsDialog::importStartUpJSONSettings()
{
    if (m_program_settings.contains("customerStrategyRefefence"))
    {
        QString cref = m_program_settings.value("customerStrategyRefefence").toString();
        if ((cref.isNull() == false) && (cref.length() > 0) && (cref.length() <= 15))
        {
            m_customer_strategy_ref = cref;
        }
    }
    if (m_program_settings.contains("betUpdateRateMilliseconds"))
    {
        const int betrate = m_program_settings.value("betUpdateRateMilliseconds").toInt(-1);
        if (betrate == 200)
        {
            m_bet_update_index = 0;
        }
        else if (betrate == 500)
        {
            m_bet_update_index = 1;
        }
        else if (betrate == 1000)
        {
            m_bet_update_index = 2;
        }
        else if (betrate == 2000)
        {
            m_bet_update_index = 3;
        }
        else if (betrate == 5000)
        {
            m_bet_update_index = 4;
        }
        else if (betrate == 10000)
        {
            m_bet_update_index = 5;
        }
        else
        {
            m_bet_update_index = 2;
        }
        m_bet_update_rate_milliseconds = m_preset_sample_rates_millisec[m_bet_update_index];
        ui->cbBetsSampleRate->setCurrentIndex(m_bet_update_index);
    }
    if (m_program_settings.contains("priceUpdateRateMilliseconds"))
    {
        const int prate = m_program_settings.value("priceUpdateRateMilliseconds").toInt(-1);
        if (prate == 200)
        {
            m_data_update_index = 0;
        }
        else if (prate == 500)
        {
            m_data_update_index = 1;
        }
        else if (prate == 1000)
        {
            m_data_update_index = 2;
        }
        else if (prate == 2000)
        {
            m_data_update_index = 3;
        }
        else if (prate == 5000)
        {
            m_data_update_index = 4;
        }
        else if (prate == 10000)
        {
            m_data_update_index = 5;
        }
        else
        {
            m_data_update_index = 2;
        }
        m_data_update_rate_milliseconds = m_preset_sample_rates_millisec[m_data_update_index];
        ui->cbMarketSampleRate->setCurrentIndex(m_data_update_index);
    }

    m_gridbet_enabled = m_program_settings.contains("enableGridViewBetting") && m_program_settings.value("enableGridViewBetting").isBool() && m_program_settings.value("enableGridViewBetting").toBool();
    m_ladderbet_enabled = m_program_settings.contains("enableLadderViewBetting") && m_program_settings.value("enableLadderViewBetting").isBool() && m_program_settings.value("enableLadderViewBetting").toBool();
    m_ipbet_enabled = m_program_settings.contains("enableIPViewBetting") && m_program_settings.value("enableIPViewBetting").isBool() && m_program_settings.value("enableIPViewBetting").toBool();

    refresh();
}


//====================================================================
ProgramSettingsDialog::~ProgramSettingsDialog()
{
    delete ui;
}

//====================================================================
void ProgramSettingsDialog::refresh()
{
    ui->cbBetsSampleRate->setCurrentIndex(m_bet_update_index);
    ui->cbMarketSampleRate->setCurrentIndex(m_data_update_index);
    ui->cbWOMDepth->setCurrentIndex(m_wom_depth_index);
    ui->chkBetPersistenceFlag->setChecked(m_place_bets_with_persistence);
    ui->chkGridViewBettingEnabled->setChecked(m_gridbet_enabled);
    ui->chkLadderBettingEnabled->setChecked(m_ladderbet_enabled);
    ui->chkInPlayViewBettingEnabled->setChecked(m_ipbet_enabled);
    ui->sbLadderHistInterval1->setValue(m_ladder_hist_interval_lo);
    ui->sbLadderHistInterval2->setValue(m_ladder_hist_interval_hi);
    if (m_auto_bet_offset > 0)
    {
        ui->chkAutoPlaceOffsetBet->setChecked(true);
        ui->sbAutoPlaceOffsetBet->setValue(m_auto_bet_offset);
    }
    else
    {
        ui->sbAutoPlaceOffsetBet->setValue(1);
        ui->chkAutoPlaceOffsetBet->setChecked(false);
    }
    if (m_customer_strategy_ref.length() > 0)
    {
        ui->leCustomerStratRef->setText(m_customer_strategy_ref);
    }
}

//====================================================================
void ProgramSettingsDialog::onSetTotalTradedHistColour()
{
    setColour(0);
}

//====================================================================
void ProgramSettingsDialog::onSetLowerTradedHistColour()
{
    setColour(1);
}

//====================================================================
void ProgramSettingsDialog::onSetUpperTradedHistColour()
{
    setColour(2);
}

//====================================================================
void ProgramSettingsDialog::setColour(const unsigned char index)
{
    if (index < 3)
    {
        QColor new_colour = QColorDialog::getColor();
        if (new_colour.isValid())
        {
            m_unconfirmed_histogram_colour_array[index] = new_colour;
        }
    }
}

//====================================================================
void ProgramSettingsDialog::applyDefaultDarkTheme()
{
    m_unconfirmed_histogram_colour_array[0] = betfair::utils::pbar1dark;
    m_unconfirmed_histogram_colour_array[1] = betfair::utils::pbar3dark;
    m_unconfirmed_histogram_colour_array[2] = betfair::utils::pbar2dark;
    m_ladder_histogram_colour_array[0] = m_unconfirmed_histogram_colour_array[0];
    m_ladder_histogram_colour_array[1] = m_unconfirmed_histogram_colour_array[1];
    m_ladder_histogram_colour_array[2] = m_unconfirmed_histogram_colour_array[2];
}

//====================================================================
void ProgramSettingsDialog::onSettingsSaveClicked()
{
    if (ui->sbLadderHistInterval2->value() <= ui->sbLadderHistInterval1->value())
    {
        QMessageBox msgBox;
        msgBox.setText("Error - Upper ladder histogram interval must be larger than the lower interval!     \nSettings have not been saved.");
        msgBox.exec();
        return;
    }
    if (ui->leCustomerStratRef->text().length() > 0)
    {
        QString strat_ref = ui->leCustomerStratRef->text();
        if (strat_ref.length() <= 15)
        {
            m_customer_strategy_ref = strat_ref;
            m_program_settings.insert("customerStrategyRefefence",m_customer_strategy_ref);
        }
        else
        {
            // Message box
            QMessageBox msgBox;
            msgBox.setText("Error - Customer strategy reference string cannot be more than 15 characters long.");
            msgBox.exec();
            return;
        }
    }
    else
    {
        // Clear current reference string if not empty
        if (m_customer_strategy_ref.length() > 0)
        {
            m_customer_strategy_ref = "";
        }
        m_program_settings.remove("customerStrategyRefefence");
    }
    m_bet_update_index = ui->cbBetsSampleRate->currentIndex();
    if (m_bet_update_index < 0 || m_bet_update_index > 5)
    {
        m_bet_update_index = 2;
    }
    m_bet_update_rate_milliseconds = m_preset_sample_rates_millisec[m_bet_update_index];
    m_program_settings.insert("betUpdateRateMilliseconds",m_bet_update_rate_milliseconds);

    m_data_update_index = ui->cbMarketSampleRate->currentIndex();
    if (m_data_update_index < 0 || m_data_update_index > 5)
    {
        m_data_update_index = 2;
    }
    m_data_update_rate_milliseconds = m_preset_sample_rates_millisec[m_data_update_index];
    m_program_settings.insert("priceUpdateRateMilliseconds",m_data_update_rate_milliseconds);

    m_wom_depth_index = ui->cbWOMDepth->currentIndex();
    if (m_wom_depth_index < 0 || m_wom_depth_index > 9)
    {
        m_wom_depth_index = 4;
    }
    m_wom_calculation_depth = m_wom_depth_index + 1;
    m_place_bets_with_persistence = ui->chkBetPersistenceFlag->isChecked();

    m_gridbet_enabled = ui->chkGridViewBettingEnabled->isChecked();
    m_ladderbet_enabled = ui->chkLadderBettingEnabled->isChecked();
    m_ipbet_enabled = ui->chkInPlayViewBettingEnabled->isChecked();

    m_program_settings.insert("enableGridViewBetting",m_gridbet_enabled);
    m_program_settings.insert("enableLadderViewBetting",m_ladderbet_enabled);
    m_program_settings.insert("enableIPViewBetting",m_ipbet_enabled);



    if (ui->chkAutoPlaceOffsetBet->isChecked())
    {
        m_auto_bet_offset = ui->sbAutoPlaceOffsetBet->value();
    }
    else
    {
        m_auto_bet_offset = 0;
    }
    m_ladder_hist_interval_hi = ui->sbLadderHistInterval2->value();
    m_ladder_hist_interval_lo = ui->sbLadderHistInterval1->value();

    m_ladder_histogram_colour_array[0] = m_unconfirmed_histogram_colour_array[0];
    m_ladder_histogram_colour_array[1] = m_unconfirmed_histogram_colour_array[1];
    m_ladder_histogram_colour_array[2] = m_unconfirmed_histogram_colour_array[2];

    close();
    emit settingsDataChanged();
}

