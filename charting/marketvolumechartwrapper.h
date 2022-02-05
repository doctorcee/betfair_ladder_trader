#ifndef MARKETVOLUMECHARTWRAPPER_H
#define MARKETVOLUMECHARTWRAPPER_H

#include "abstractchartwrapper.h"
#include "betfair/betfairmarket.h"

namespace charting {

class TMarketVolumeChartWrapper : public TAbstractChartWrapper
{
private:
    QLineSeries* m_volume;
    QAreaSeries* m_volume_bar;
    std::int64_t m_selection_id;

    const int chart_margins;
public:
    TMarketVolumeChartWrapper(QWidget *parent,
                              const std::uint16_t& theme);
    virtual ~TMarketVolumeChartWrapper();

    void populateSeries(const betfair::TBetfairMarket& market);

    void populateSeries(const betfair::TBetfairMarket& market,
                        const std::int64_t& selection_id);

    virtual void clear();
    virtual int getLabelWidth();

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0);
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0);


};


} // end of namespace charting

#endif // MARKETVOLUMECHARTWRAPPER_H
