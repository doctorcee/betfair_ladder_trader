#ifndef RUNNERCHARTSDIALOG_H
#define RUNNERCHARTSDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <map>

#include "betfair/betfairmarket.h"


namespace Ui {
class RunnerChartsDialog;
}

class RunnerChartsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunnerChartsDialog(betfair::TBetfairMarket& mkt,
                                const std::uint16_t& theme,
                                QWidget *parent = nullptr);
    ~RunnerChartsDialog();
    void updateChart();
    void reset();
    void setVisibleRunner(const std::int64_t& id);
    void setDisplayTheme(const std::uint16_t& theme) {m_display_theme = theme;}

public slots:

    void changeSelectedRunner(int index);
    void changeYAxisDisplayOptions(int index);
    void changeXAxisDisplayOptions(int index);

private:
    Ui::RunnerChartsDialog *ui;
    std::uint16_t m_display_theme;
    betfair::TBetfairMarket& m_market;
    QChart* m_chart;
    QChartView* m_chart_view;
    QLineSeries* m_sched_off_line;
    QLineSeries* m_inplay_start_line;
    //charting::TMarketVolumeChartWrapper my_volume_wrapper;
    std::map<QString,std::uint32_t> m_odds_tick_map;
    std::map<std::int64_t, QLineSeries*> m_m_traces;
    std::map<std::int64_t, std::pair<double,double> > m_m_runner_y_bounds;

    std::map<int, std::int64_t> m_m_selection_index_map;

    int m_selected_runner_index;
    std::int64_t m_selected_runner_id;
    QString m_selected_runner_name;

    double m_x_axis_min_display;
    double m_x_axis_max_display;
    double m_y_axis_min_display;
    double m_y_axis_max_display;

    double m_data_x_min;
    double m_data_x_max;
    double m_data_y_min;
    double m_data_y_max;

    int m_x_zoom_mode;  // 0 = ALL, 1 = last 60, 2 = last 120
    int m_y_zoom_mode;  // 0 = ALL, 1 = odds on, 2 = ????

    void setCurrentYAxisLimits();
    void clearChart();

    void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);

    std::uint32_t getTickFromOdds(double odds);
};

#endif // RUNNERCHARTSDIALOG_H
