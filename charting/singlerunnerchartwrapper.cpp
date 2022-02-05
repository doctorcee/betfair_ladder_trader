#include "singlerunnerchartwrapper.h"


namespace charting {


//==========================================================================
TSingleRunnerChartWrapper::TSingleRunnerChartWrapper(QWidget *parent,
                                                     const std::uint16_t& theme)
    : TIndividualSelectionChartWrapperBase(parent,theme),
      m_candle_vwap(nullptr),
      m_global_vwap(nullptr),
      m_candlesticks(nullptr),
      m_selection_id(0),
      m_candle_time_width_milliseconds(0),
      chart_margins(5),
      m_heikin_ashi_candles(false),
      m_show_ema_1(false),
      m_show_ema_2(false),
      m_show_cwap(false),
      m_show_vwap(true)
{
    m_candlesticks = new QCandlestickSeries();
    m_candlesticks->setIncreasingColor(QColor(20, 242, 20, 0xaa));
    m_candlesticks->setDecreasingColor(QColor(242, 20, 20, 0xaa));
    m_candlesticks->setBodyWidth(0.8);
    m_candlesticks->setBodyOutlineVisible(false);
    m_candlesticks->setCapsVisible(true);
    m_candlesticks->setVisible(true);

    m_candle_vwap = new QLineSeries();
    m_candle_vwap->setColor(Qt::darkBlue);
    m_candle_vwap->setVisible(m_show_cwap);

    m_global_vwap = new QLineSeries();
    m_global_vwap->setColor(Qt::red);
    m_global_vwap->setVisible(m_show_vwap);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(5);
    axisX->setFormat("hh:mm:ss");
    axisX->setMinorGridLineVisible(true);
    axisX->setGridLineVisible(true);
    QFont lbl_font_x = axisX->labelsFont();
    lbl_font_x.setWeight(QFont::Normal);
    lbl_font_x.setPointSize(m_default_xaxis_label_font_pointsize);
    axisX->setLabelsFont(lbl_font_x);
    lbl_font_x.setPointSize(m_default_xaxis_title_font_pointsize);
    axisX->setTitleFont(lbl_font_x);

    // Dont need labels and the chart underneath will display
    axisX->setLabelsVisible(false);
    axisX->show();

    QValueAxis *axisY = new QValueAxis;
    axisY->setTickCount(5);
    axisY->setMinorTickCount(4);
    axisY->setLabelFormat("%4.2f");
    axisY->setTitleText("Market Odds");
    axisY->setMinorGridLineVisible(true);
    axisY->setGridLineVisible(true);
    QFont lbl_font = axisY->labelsFont();
    lbl_font.setWeight(QFont::Normal);
    lbl_font.setPointSize(m_default_yaxis_label_font_pointsize);
    axisY->setLabelsFont(lbl_font);
    lbl_font.setPointSize(m_default_yaxis_title_font_pointsize);
    //lbl_font.setStyle(QFont::StyleItalic);
    axisY->setTitleFont(lbl_font);

    m_chart->addSeries(m_candle_vwap);
    m_chart->addSeries(m_global_vwap);
    m_chart->addSeries(m_candlesticks);

    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    m_candlesticks->attachAxis(axisX);
    m_candlesticks->attachAxis(axisY);
    m_candle_vwap->attachAxis(axisX);
    m_candle_vwap->attachAxis(axisY);
    m_global_vwap->attachAxis(axisX);
    m_global_vwap->attachAxis(axisY);

    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    setMargins(chart_margins);
    m_chart->setBackgroundRoundness(0);
    applyCurrentTheme();
}


//====================================================================
TSingleRunnerChartWrapper::~TSingleRunnerChartWrapper()
{
    m_chart->removeAllSeries();
}

//====================================================================
void TSingleRunnerChartWrapper::clear()
{
    //m_chart->setTitle("");
    m_candlesticks->clear();
    m_global_vwap->clear();
    m_candle_vwap->clear();
    m_x_axis_min_display = 0.0;
    m_x_axis_max_display = 0.0;
    m_y_axis_min_display = 0.0;
    m_y_axis_max_display = 0.0;
    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = 0.0;
    m_data_y_max = 0.0;
    m_selection_id = 0;
    zoomToDefaultXRange();
    zoomToDefaultYRange();
}


//====================================================================
void TSingleRunnerChartWrapper::populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data_vector,
                                               const std::int64_t& sel_id)
{
    QVector<QPointF> v_avg;
    QVector<QPointF> v_cum_avg;
    QList<QCandlestickSet*> new_points;
    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = 0.0;
    m_data_y_max = 0.0;
    m_selection_id = sel_id;
    if (!data_vector.empty())
    {
        m_data_x_min = std::numeric_limits<double>::max();
        m_data_x_max = -m_data_x_min;
        m_data_y_min = m_data_x_min;
        m_data_y_max = m_data_x_max;

        double last_ts = -1.0;
        for (auto data_it = data_vector.begin(); data_it != data_vector.end(); ++data_it)
        {            
            const double ts = data_it->end_timestamp;
            if (ts <= last_ts)
            {
                throw std::runtime_error("ERROR: Candlestick array not sorted chronologically!");
            }
            if (data_it->start_timestamp < m_data_x_min)
            {
                m_data_x_min = data_it->start_timestamp;
            }
            if (data_it->end_timestamp > m_data_x_max)
            {
                m_data_x_max = data_it->end_timestamp;
            }
            if (data_it->getCandleVol() > 0.0)
            {
                QCandlestickSet* new_item = new QCandlestickSet(ts);
                const double hi = data_it->getHigh();
                const double lo = data_it->getLow();
                const double open = data_it->getOpen();
                const double close = data_it->getClose();
                const double cum_vwap = data_it->getCumulativeVWAP();
                if (m_heikin_ashi_candles)
                {
                    double close_price = (close + lo + hi + open)/4.0;
                    double open_price = open > 0.0 ? open : close_price;
                    if (new_points.size() > 0)
                    {
                        QCandlestickSet* prev = new_points.at(new_points.size()-1);
                        open_price = (prev->open() + prev->close())/2.0;
                    }
                    double high = std::max(std::max(open_price,close_price),hi);
                    double low = std::min(std::min(open_price,close_price),lo);
                    new_item->setHigh(high);
                    new_item->setLow(low);
                    new_item->setClose(close_price);
                    new_item->setOpen(open_price);
                    if (high > m_data_y_max)
                    {
                        m_data_y_max = high;
                    }
                    if (low < m_data_y_min)
                    {
                        m_data_y_min = low;
                    }
                }
                else
                {
                    new_item->setHigh(hi);
                    new_item->setLow(lo);
                    // It is possible that on the first candle we have NO prior LPM on the API call.
                    // To avoid plotting a candle with zero open price, set open to close
                    if (open > 0.0)
                    {
                        new_item->setOpen(open);
                    }
                    else
                    {
                        new_item->setOpen(close);
                    }
                    new_item->setClose(data_it->getClose());
                    v_avg.append(QPointF(ts,data_it->getCandleVWAP()));
                    if (hi > m_data_y_max)
                    {
                        m_data_y_max = hi;
                    }
                    if (lo < m_data_y_min)
                    {
                        m_data_y_min = lo;
                    }

                    v_cum_avg.append(QPointF(ts,cum_vwap));
                    if (cum_vwap > m_data_y_max)
                    {
                        m_data_y_max = cum_vwap;
                    }
                    if (cum_vwap < m_data_y_min)
                    {
                        m_data_y_min = cum_vwap;
                    }
                }
                new_points.append(new_item);        
            }
            last_ts = ts;
        }
    }
    m_x_axis_min_display = m_data_x_min;
    m_x_axis_max_display = m_data_x_max;
    m_y_axis_min_display = m_data_y_min;
    m_y_axis_max_display = m_data_y_max;
    m_candlesticks->clear();

    if (!m_candlesticks->append(new_points))
    {
        throw std::runtime_error("ERROR in TSelectionChartWrapper::populateSeries(): appending QCandlestickSet to series failed.");
    }

    m_candle_vwap->replace(v_avg);
    m_global_vwap->replace(v_cum_avg);

    zoomToDefaultXRange();
    zoomToDefaultYRange();
}

//====================================================================
int TSingleRunnerChartWrapper::getLabelWidth()
{
    QFont labelFont = m_chart->axisY()->labelsFont();
    QString volmax = QString::number(m_y_axis_max_display,'f',2);
    QFontMetrics fm(labelFont);

    return fm.boundingRect(volmax).width();
}

//====================================================================
void TSingleRunnerChartWrapper::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
{
    m_x_axis_min_display = x_min;
    m_x_axis_max_display = x_max;
    if (m_x_axis_max_display < m_x_axis_min_display)
    {
        std::swap(m_x_axis_min_display, m_x_axis_max_display);
    }
    m_chart->axisX()->setRange(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(m_x_axis_min_display-hyst)),
                               QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(m_x_axis_max_display+hyst)));
}

//====================================================================
void TSingleRunnerChartWrapper::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
{
    m_y_axis_min_display = y_min;
    m_y_axis_max_display = y_max;
    if (m_y_axis_max_display < m_y_axis_min_display)
    {
        std::swap(m_y_axis_max_display, m_y_axis_min_display);
    }
    if (m_y_axis_min_display < 1.0)
    {
        m_y_axis_min_display = 1.0;
    }
    if (m_y_axis_max_display > 1000.0)
    {
        m_y_axis_max_display = 1000.0;
    }
    m_chart->axisY()->setRange(m_y_axis_min_display-hyst, m_y_axis_max_display+hyst);
}

} // end of namespace charting
