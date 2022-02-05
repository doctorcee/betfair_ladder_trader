#ifndef VWAPINTEGRALCHARTWRAPPER_H
#define VWAPINTEGRALCHARTWRAPPER_H

#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include "individualselectionchartwrapperbase.h"
#include "betfair/betfairadvancedcandle.h"

namespace charting {


class TVWAPIntegralChartWrapper : public TIndividualSelectionChartWrapperBase
{
private:

    QLineSeries* m_candle_vwap_trace_above;
    QLineSeries* m_candle_vwap_trace_below;
    QLineSeries* m_candle_vwap_trace_zero;
    QAreaSeries* m_area_above;
    QAreaSeries* m_area_below;

    std::int64_t m_selection_id;
    const int chart_margins;

public:

    TVWAPIntegralChartWrapper(QWidget *parent,
                              const std::uint16_t& theme);

    virtual ~TVWAPIntegralChartWrapper();

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);

    virtual void populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data,
                                const std::int64_t& sel_id);

    virtual void clear();
    virtual int getLabelWidth();

};


} // end of namespace charting
#endif // VWAPINTEGRALCHARTWRAPPER_H
