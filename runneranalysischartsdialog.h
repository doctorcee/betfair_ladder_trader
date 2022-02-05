#ifndef RUNNERANALYSISCHARTSDIALOG_H
#define RUNNERANALYSISCHARTSDIALOG_H

#include <QDialog>
#include "betfair/betfairmarket.h"
#include "charting/singlerunnerchartwrapper.h"
#include "charting/volumechartwrapper.h"
#include "charting/macdchartwrapper.h"
#include "charting/vwapintegralchartwrapper.h"

namespace Ui {
class TRunnerAnalysisChartsDialog;
}

class TRunnerAnalysisChartsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TRunnerAnalysisChartsDialog(betfair::TBetfairMarket& mkt,
                                         const std::uint16_t& theme,
                                         QWidget *parent = nullptr);
    ~TRunnerAnalysisChartsDialog();

    void updateChart();
    void reset();
    void setSelectedRunner(const std::int64_t& runner_id);

public slots:
    void changeChartLimits();
    void changeSelectedRunner(int index);
    void changeYAxisDisplayOptions(int index);
    void changeXAxisDisplayOptions(int index);
    void onChangeDisplayInterval(int index);
    void onChangeHeikinAshiCandleMode(int state);

private:
    Ui::TRunnerAnalysisChartsDialog *ui;
    std::uint16_t m_display_theme;
    betfair::TBetfairMarket& m_market;
    charting::TSingleRunnerChartWrapper m_candles_wrapper;
    charting::TVolumeChartWrapper m_volume_wrapper;
    charting::TMACDChartWrapper m_macd_wrapper;
    //charting::TVWAPIntegralChartWrapper m_vwap_cx_wrapper;

    std::map<int, std::int64_t> m_m_selection_index_map;

    std::int64_t m_selected_runner_id;
    int m_selected_runner_index;
    QString m_selected_runner_name;

    int m_x_zoom_mode;  // 0 = ALL, 1 = last 60, 2 = last 120
    int m_y_zoom_mode;  // 0 = ALL, 1 = odds on, 2 = ????

    std::uint16_t m_display_interval_milliseconds;


    void setCurrentYAxisLimits();

    void alignChartMargins();

    void setChartDisplayIntervalMilliseconds(const std::uint16_t& new_val) {m_display_interval_milliseconds = new_val;}

    void setDisplayTheme(const std::uint16_t& theme) {m_display_theme = theme;}


};

#endif // RUNNERANALYSISCHARTSDIALOG_H
