#ifndef PROGRAMSETTINGSDIALOG_H
#define PROGRAMSETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>


namespace Ui {
class ProgramSettingsDialog;
}

class ProgramSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgramSettingsDialog(QJsonObject& settings,
                                   QWidget *parent = nullptr);
    ~ProgramSettingsDialog();

    int getWOMCalcDepth() const {return m_wom_calculation_depth;}
    int getBetUpdateRateMillisec() const {return m_bet_update_rate_milliseconds;}
    int getDataUpdateRateMillisec() const {return m_data_update_rate_milliseconds;}
    bool getBetPersistenceFlag() const {return m_place_bets_with_persistence;}

    bool getGridViewBettingEnabledFlag() const {return m_gridbet_enabled;}
    bool getLadderViewBettingEnabledFlag() const {return m_ladderbet_enabled;}
    bool getInplayViewBettingEnabledFlag() const {return m_ipbet_enabled;}

    int getLadderHistogramIntervalUpper() const {return m_ladder_hist_interval_hi;}
    int getLadderHistogramIntervalLower() const {return m_ladder_hist_interval_lo;}

    QColor getLadderHistogramColorTotal() const {return m_ladder_histogram_colour_array[0];}
    QColor getLadderHistogramColorLower() const {return m_ladder_histogram_colour_array[1];}
    QColor getLadderHistogramColorUpper() const {return m_ladder_histogram_colour_array[2];}
    std::uint16_t getAutoBetPlacementOffset() const {return m_auto_bet_offset;}

    QString getCustomStrategyRef() const {return m_customer_strategy_ref;}
    void applyDefaultDarkTheme();

    void refresh();

    void importStartUpJSONSettings();


public slots:

    void onSettingsSaveClicked();
    void onSetTotalTradedHistColour();
    void onSetLowerTradedHistColour();
    void onSetUpperTradedHistColour();

signals:
    void settingsDataChanged();

private:
    Ui::ProgramSettingsDialog *ui;

    QJsonObject& m_program_settings;
    int m_wom_calculation_depth;
    int m_bet_update_rate_milliseconds;
    int m_data_update_rate_milliseconds;
    int m_bet_update_index;
    int m_data_update_index;
    int m_wom_depth_index;
    int m_ladder_hist_interval_lo;
    int m_ladder_hist_interval_hi;

    QColor m_ladder_histogram_colour_array[3];
    QColor m_unconfirmed_histogram_colour_array[3];

    QString m_customer_strategy_ref;

    bool m_place_bets_with_persistence;

    bool m_gridbet_enabled;
    bool m_ladderbet_enabled;
    bool m_ipbet_enabled;

    std::uint16_t m_auto_bet_offset;

    const int m_preset_sample_rates_millisec[6] = {200,500,1000,2000,5000,10000};

    void setColour(const unsigned char index);



};

#endif // PROGRAMSETTINGSDIALOG_H
