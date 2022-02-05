#ifndef SINGLERUNNERCHARTWRAPPER_H
#define SINGLERUNNERCHARTWRAPPER_H

#include <QtCharts/QLineSeries>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QAreaSeries>
#include "individualselectionchartwrapperbase.h"
#include "betfair/betfairadvancedcandle.h"

namespace charting {

// Contains the following series:
// 1. Candlestick
// 2. Weighted average price match per interval
// 3. Cumulative weighted average price match overall

class TSingleRunnerChartWrapper : public TIndividualSelectionChartWrapperBase
{
private:
    QLineSeries* m_candle_vwap;
    QLineSeries* m_global_vwap;
    QLineSeries* m_ema_1;
    QLineSeries* m_ema_2;
    QCandlestickSeries* m_candlesticks;

    std::int64_t m_selection_id;
    std::uint16_t m_candle_time_width_milliseconds;

    const int chart_margins;

    bool m_heikin_ashi_candles;
    bool m_show_ema_1;
    bool m_show_ema_2;
    bool m_show_cwap;
    bool m_show_vwap;

public:
    TSingleRunnerChartWrapper(QWidget *parent,
                              const std::uint16_t& theme);
    virtual ~TSingleRunnerChartWrapper();

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);


    virtual void populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data,
                                const std::int64_t& sel_id);

    void setHeikinAshiMode(bool enable) {m_heikin_ashi_candles = enable;}



    virtual void clear();

    virtual int getLabelWidth();

    void setCandleTimeWidthMilliseconds(const std::uint16_t& new_val) {m_candle_time_width_milliseconds = new_val;}

};

} // end of namespace charting

#endif // SINGLERUNNERCHARTWRAPPER_H
