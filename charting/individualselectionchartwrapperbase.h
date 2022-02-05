#ifndef INDIVIDUALSELECTIONCHARTWRAPPERBASE_H
#define INDIVIDUALSELECTIONCHARTWRAPPERBASE_H

#include "abstractchartwrapper.h"

namespace charting {

class TIndividualSelectionChartWrapperBase : public TAbstractChartWrapper
{
private:
    TIndividualSelectionChartWrapperBase();
    TIndividualSelectionChartWrapperBase(const TIndividualSelectionChartWrapperBase& cp);
    TIndividualSelectionChartWrapperBase& operator=(const TIndividualSelectionChartWrapperBase& cp);

public:

    TIndividualSelectionChartWrapperBase(QWidget *parent,
                                         const std::uint16_t& theme);
    virtual ~TIndividualSelectionChartWrapperBase();

    virtual void populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data,
                                const std::int64_t& sel_id) = 0;
};

} // end of namespace charting

#endif // INDIVIDUALSELECTIONCHARTWRAPPERBASE_H
