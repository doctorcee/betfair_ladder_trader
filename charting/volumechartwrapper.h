#ifndef VOLUMECHARTWRAPPER_H
#define VOLUMECHARTWRAPPER_H

#include <QtCharts/QLineSeries>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QAreaSeries>
#include "individualselectionchartwrapperbase.h"
#include "betfair/betfairadvancedcandle.h"

// Used just for volume
namespace charting {

class TVolumeChartWrapper : public TIndividualSelectionChartWrapperBase
{
private:

    QLineSeries* m_volume;
    QAreaSeries* m_volume_bar;

    std::int64_t m_selection_id;
    std::uint16_t m_candle_time_width_milliseconds;

    const int chart_margins;

public:
    TVolumeChartWrapper(QWidget *parent,
                        const std::uint16_t& theme);
    virtual ~TVolumeChartWrapper();

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);

    virtual void populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data,
                                const std::int64_t& sel_id);

    virtual void clear();
    virtual int getLabelWidth();

    void setCandleTimeWidthMilliseconds(const std::uint16_t& new_val) {m_candle_time_width_milliseconds = new_val;}

};

} // end of namespace charting

#endif // VOLUMECHARTWRAPPER_H
