#ifndef MACDCHARTWRAPPER_H
#define MACDCHARTWRAPPER_H

#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include "individualselectionchartwrapperbase.h"
#include "betfair/betfairadvancedcandle.h"

namespace charting {

class TMACDChartWrapper : public TIndividualSelectionChartWrapperBase
{
private:

    QLineSeries* m_macd;
    QLineSeries* m_macd_signal;

    QAreaSeries* m_macd_pos;
    QAreaSeries* m_macd_neg;
    QLineSeries* m_macd_hist_pos;
    QLineSeries* m_macd_hist_neg;

    QLineSeries* m_zero_line;

    std::uint16_t m_macd_long_width;
    std::uint16_t m_macd_short_width;
    std::uint16_t m_macd_signal_width;

    std::int64_t m_selection_id;
    std::uint16_t m_candle_time_width_milliseconds;

    const int chart_margins;
public:
    TMACDChartWrapper(QWidget* parent,
                      const std::uint16_t& theme);
    virtual ~TMACDChartWrapper();

    bool setNewMAWidths(const std::uint16_t& long_ema,
                        const std::uint16_t& short_ema,
                        const std::uint16_t& signal);

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);



    void populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data,
                        const std::int64_t& sel_id);

    virtual void clear();
    virtual int getLabelWidth();

    void setCandleTimeWidthMilliseconds(const std::uint16_t& new_val) {m_candle_time_width_milliseconds = new_val;}

};

} // end of namespace charting

#endif // MACDCHARTWRAPPER_H
