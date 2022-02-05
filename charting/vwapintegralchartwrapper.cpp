#include "vwapintegralchartwrapper.h"

namespace charting {

//=====================================================================
TVWAPIntegralChartWrapper::TVWAPIntegralChartWrapper(QWidget *parent,
                                                     const std::uint16_t& theme)
    : TIndividualSelectionChartWrapperBase (parent,theme),
      m_candle_vwap_trace_above(nullptr),
      m_candle_vwap_trace_below(nullptr),
      m_candle_vwap_trace_zero(nullptr),
      m_area_above(nullptr),
      m_area_below(nullptr),
      chart_margins(5)
{

    m_candle_vwap_trace_above = new QLineSeries();
    m_candle_vwap_trace_above->setColor(Qt::black);
    m_candle_vwap_trace_above->setVisible(false);

    m_candle_vwap_trace_below = new QLineSeries();
    m_candle_vwap_trace_below->setColor(Qt::black);
    m_candle_vwap_trace_below->setVisible(false);

    m_candle_vwap_trace_zero = new QLineSeries();
    m_candle_vwap_trace_zero->setColor(Qt::black);
    m_candle_vwap_trace_zero->setVisible(false);

    m_area_above = new QAreaSeries(m_candle_vwap_trace_above,m_candle_vwap_trace_zero);
    m_area_above->setColor(betfair::utils::layVWAPXOTransparent);
    m_area_above->setBorderColor(Qt::green);

    m_area_below = new QAreaSeries(m_candle_vwap_trace_zero,m_candle_vwap_trace_below);
    m_area_below->setColor(betfair::utils::backVWAPXOTransparent);
    m_area_below->setBorderColor(Qt::red);


    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(5);
    axisX->setFormat("hh:mm:ss");
    //axisX->setMinorGridLineVisible(true);
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
    axisY->setTickCount(3);
    axisY->setMinorTickCount(2);
    axisY->setLabelFormat("%3.1f");
    axisY->setTitleText("VWAPXO");
    //axisY->setMinorGridLineVisible(true);
    axisY->setGridLineVisible(true);
    QFont lbl_font = axisY->labelsFont();
    lbl_font.setWeight(QFont::Normal);
    lbl_font.setPointSize(m_default_yaxis_label_font_pointsize);
    axisY->setLabelsFont(lbl_font);
    lbl_font.setPointSize(m_default_yaxis_title_font_pointsize);
    //lbl_font.setStyle(QFont::StyleItalic);
    axisY->setTitleFont(lbl_font);

    m_chart->addSeries(m_candle_vwap_trace_zero);
    m_chart->addSeries(m_candle_vwap_trace_above);
    m_chart->addSeries(m_candle_vwap_trace_below);
    m_chart->addSeries(m_area_above);
    m_chart->addSeries(m_area_below);

    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    m_candle_vwap_trace_zero->attachAxis(axisX);
    m_candle_vwap_trace_zero->attachAxis(axisY);
    m_candle_vwap_trace_above->attachAxis(axisX);
    m_candle_vwap_trace_above->attachAxis(axisY);
    m_candle_vwap_trace_below->attachAxis(axisX);
    m_candle_vwap_trace_below->attachAxis(axisY);
    m_area_above->attachAxis(axisX);
    m_area_above->attachAxis(axisY);
    m_area_below->attachAxis(axisX);
    m_area_below->attachAxis(axisY);

    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    setMargins(chart_margins);
    m_chart->setBackgroundRoundness(0);
    applyCurrentTheme();
}

//=====================================================================
TVWAPIntegralChartWrapper::~TVWAPIntegralChartWrapper()
{
    m_chart->removeAllSeries();
}

//====================================================================
void TVWAPIntegralChartWrapper::clear()
{
    m_chart->setTitle("");
    m_candle_vwap_trace_zero->clear();
    m_candle_vwap_trace_above->clear();
    m_candle_vwap_trace_below->clear();
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
void TVWAPIntegralChartWrapper::populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data_vector,
                                               const std::int64_t& sel_id)
{
    QVector<QPointF> v_zeros;
    QVector<QPointF> v_upper;
    QVector<QPointF> v_lower;

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
        double last_data = 0.0;
        std::uint32_t ct = 0;
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

            double new_data = (data_it->getCandleVol() > 0.0) ? (data_it->getCandleVWAPAsTick() - data_it->getCumulativeVWAPAsTick()) : last_data;

            if (new_data > 0.0)
            {
                if (ct > 0)
                {
                    // Is previous point LESS than 0.0?
                    if (last_data < 0.0)
                    {
                        // crosses the x-axis here - calculate timestamp for this
                        double diff = new_data - last_data;
                        double ratio = new_data / diff;
                        double ts_cross = last_ts + (ts - last_ts)*ratio;

                        // add the LOWER points
                        v_lower.append(QPointF(ts_cross,0.0));
                        v_upper.append(QPointF(ts_cross,0.0));
                        v_zeros.append(QPointF(ts_cross,0.0));
                    }

                }
                v_upper.append(QPointF(ts,new_data));
                v_zeros.append(QPointF(ts,0.0));
            }
            else if (new_data < 0.0)
            {
                if (ct > 0)
                {
                    // Is previous point GREATER than 0.0?
                    if (last_data > 0.0)
                    {
                        // crosses the x-axis here - calculate timestamp for this
                        double diff = -new_data + last_data;
                        double ratio = -new_data / diff;
                        double ts_cross = last_ts + (ts - last_ts)*ratio;

                        // add the LOWER points
                        v_upper.append(QPointF(ts_cross,0.0));
                        v_lower.append(QPointF(ts_cross,0.0));
                        v_zeros.append(QPointF(ts_cross,0.0));
                    }

                }
                v_lower.append(QPointF(ts,new_data));
                v_zeros.append(QPointF(ts,0.0));
            }
            else
            {
                v_lower.append(QPointF(ts,new_data));
                v_upper.append(QPointF(ts,new_data));
                v_zeros.append(QPointF(ts,0.0));
            }

            if (new_data > m_data_y_max)
            {
                m_data_y_max = new_data;
            }
            if (new_data < m_data_y_min)
            {
                m_data_y_min = new_data;
            }

            last_ts = ts;
            last_data = new_data;
            ++ct;
        }
    }
    m_x_axis_min_display = m_data_x_min;
    m_x_axis_max_display = m_data_x_max;
    m_y_axis_min_display = m_data_y_min;
    m_y_axis_max_display = m_data_y_max;


    m_candle_vwap_trace_below->replace(v_lower);
    m_candle_vwap_trace_above->replace(v_upper);
    m_candle_vwap_trace_zero->replace(v_zeros);


    zoomToDefaultXRange();
    zoomToDefaultYRange();
}

//====================================================================
int TVWAPIntegralChartWrapper::getLabelWidth()
{
    QFont labelFont = m_chart->axisY()->labelsFont();
    QString volmax = QString::number(m_y_axis_max_display,'f',2);
    QFontMetrics fm(labelFont);

    return fm.boundingRect(volmax).width();
}

//====================================================================
void TVWAPIntegralChartWrapper::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
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
void TVWAPIntegralChartWrapper::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
{
    if (fabs(y_min) >= fabs(y_max))
    {
        m_y_axis_min_display = y_min;
        m_y_axis_max_display = - y_min;;
    }
    else
    {
        m_y_axis_min_display = -y_max;
        m_y_axis_max_display = y_max;
    }
    if (m_y_axis_max_display < m_y_axis_min_display)
    {
        std::swap(m_y_axis_max_display, m_y_axis_min_display);
    }
    m_chart->axisY()->setRange(m_y_axis_min_display-hyst, m_y_axis_max_display+hyst);
}



} // end of namespace charting
