#include "macdchartwrapper.h"
#include "betfair/betfair_utils.h"
#include <cassert>

namespace charting {

//==============================================================
TMACDChartWrapper::TMACDChartWrapper(QWidget* parent,
                                     const std::uint16_t& theme)
    : TIndividualSelectionChartWrapperBase(parent,theme),
      m_macd(nullptr),
      m_macd_signal(nullptr),
      m_macd_pos(nullptr),
      m_macd_neg(nullptr),
      m_macd_hist_pos(nullptr),
      m_macd_hist_neg(nullptr),
      m_zero_line(nullptr),
      m_macd_long_width(26),
      m_macd_short_width(12),
      m_macd_signal_width(9),
      m_selection_id(0),
      m_candle_time_width_milliseconds(0),
      chart_margins(5)
{
    m_zero_line = new QLineSeries();
    m_zero_line->setColor(Qt::darkBlue);

    m_macd = new QLineSeries();
    m_macd->setColor(Qt::magenta);

    m_macd_signal = new QLineSeries();
    m_macd_signal->setColor(Qt::cyan);

    m_macd_hist_pos = new QLineSeries();
    m_macd_hist_pos->setColor(Qt::green);
    m_macd_hist_pos->setVisible(false);

    m_macd_hist_neg = new QLineSeries();
    m_macd_hist_neg->setColor(Qt::red);
    m_macd_hist_neg->setVisible(false);

    m_macd_pos = new QAreaSeries(m_macd_hist_pos,m_zero_line);
    m_macd_pos->setColor(betfair::utils::layOne);
    m_macd_pos->setBorderColor(QColor(200,70,90,0xee));

    m_macd_neg = new QAreaSeries(m_zero_line,m_macd_hist_neg);
    m_macd_neg->setColor(betfair::utils::backOne);
    m_macd_neg->setBorderColor(QColor(80, 120, 220, 0xcc));

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
    //axisX->setLabelsVisible(false);
    axisX->show();

    QValueAxis *axisY = new QValueAxis;
    axisY->setTickCount(3);
    axisY->setMinorTickCount(1);
    axisY->setLabelFormat("%4.2f");
    axisY->setTitleText("MACD");
    axisY->setMinorGridLineVisible(true);
    axisY->setGridLineVisible(true);
    QFont lbl_font = axisY->labelsFont();
    lbl_font.setWeight(QFont::Normal);
    lbl_font.setPointSize(m_default_yaxis_label_font_pointsize);
    axisY->setLabelsFont(lbl_font);
    lbl_font.setPointSize(m_default_yaxis_title_font_pointsize);
    //lbl_font.setStyle(QFont::StyleItalic);
    axisY->setTitleFont(lbl_font);

    m_chart->addSeries(m_zero_line);
    m_chart->addSeries(m_macd);
    m_chart->addSeries(m_macd_signal);
    m_chart->addSeries(m_macd_pos);
    m_chart->addSeries(m_macd_neg);
    m_chart->addSeries(m_macd_hist_neg);
    m_chart->addSeries(m_macd_hist_pos);

    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    m_zero_line->attachAxis(axisX);
    m_zero_line->attachAxis(axisY);
    m_macd->attachAxis(axisX);
    m_macd->attachAxis(axisY);
    m_macd_signal->attachAxis(axisX);
    m_macd_signal->attachAxis(axisY);
    m_macd_pos->attachAxis(axisX);
    m_macd_pos->attachAxis(axisY);
    m_macd_neg->attachAxis(axisX);
    m_macd_neg->attachAxis(axisY);
    m_macd_hist_neg->attachAxis(axisX);
    m_macd_hist_neg->attachAxis(axisY);
    m_macd_hist_pos->attachAxis(axisX);
    m_macd_hist_pos->attachAxis(axisY);

    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    setMargins(chart_margins);
    m_chart->setBackgroundRoundness(0);

    applyCurrentTheme();
}

//====================================================================
TMACDChartWrapper::~TMACDChartWrapper()
{
}

//====================================================================
int TMACDChartWrapper::getLabelWidth()
{
    QFont labelFont = m_chart->axisY()->labelsFont();
    QValueAxis* ax = dynamic_cast<QValueAxis*>(m_chart->axisY());
    QString ymax = QString::number(ax->max(),'f',2);
    QString ymin = QString::number(ax->min(),'f',2);
    QFontMetrics fm(labelFont);
    return std::max(fm.boundingRect(ymax).width(),fm.boundingRect(ymin).width());
}

//====================================================================
void TMACDChartWrapper::clear()
{
    m_zero_line->clear();
    m_macd->clear();
    m_macd_signal->clear();
    m_macd_hist_neg->clear();
    m_macd_hist_pos->clear();
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
bool TMACDChartWrapper::setNewMAWidths(const std::uint16_t& long_ema,
                                       const std::uint16_t& short_ema,
                                       const std::uint16_t& signal)
{
    bool ok = false;
    if  (long_ema > 0 && short_ema > 0 && signal > 0 && long_ema > short_ema)
    {
        ok = true;
        m_macd_long_width = long_ema;
        m_macd_short_width = short_ema;
        m_macd_signal_width = signal;
    }
    return ok;
}


//====================================================================
void TMACDChartWrapper::populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data_vector,
                                       const std::int64_t& sel_id)
{
    QVector<QPointF> qmacd;
    QVector<QPointF> qmacd_signal;
    QVector<QPointF> zeroline;
    QVector<QPointF> macd_hist_upper;
    QVector<QPointF> macd_hist_lower;

    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = -0.01;
    m_data_y_max = 0.01;
    m_selection_id = sel_id;
    if (!data_vector.empty())
    {
        m_data_x_min = std::numeric_limits<double>::max();
        m_data_x_max = -m_data_x_min;
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
            last_ts = ts;
        }
        // time order validity checked and x limits set
        const std::size_t ct = data_vector.size();
        zeroline.append(QPointF(m_data_x_min,0.0));
        if (ct > m_macd_long_width)
        {
            // Calculate MACD and the signal line now
            std::vector<std::pair<double,double> > macd;
            std::vector<std::pair<double,double> > macd_signal;            
            betfair::utils::calculateMACD(m_macd_short_width,
                                          m_macd_long_width,
                                          data_vector,
                                          macd);

            if (macd.size() > m_macd_signal_width)
            {
                betfair::utils::calculateEMA(m_macd_signal_width, macd, macd_signal);
                assert(macd_signal.size() + (m_macd_signal_width-1) == macd.size());
                assert(macd.size() + (m_macd_long_width-1) == ct);

                double last_ts = -1;
                for (auto it = macd.begin(); it != macd.end(); ++it)
                {
                    const double ts = it->first;
                    if (ts <= last_ts)
                    {
                        throw std::runtime_error("TMACDChartWrapper::populateSeries() ERROR - Data points in MACD not sorted chronologically!");
                    }
                    qmacd.append(QPointF(ts,it->second));

                }
                last_ts = -1.0;

                for (auto it = macd_signal.begin(); it != macd_signal.end(); ++it)
                {
                    const double ts = it->first;
                    if (ts <= last_ts)
                    {
                        throw std::runtime_error("TMACDChartWrapper::populateSeries() ERROR - Data points in signal not sorted chronologically!");
                    }
                    qmacd_signal.append(QPointF(ts,it->second));
                }
                zeroline.append(QPointF(m_data_x_min,0.0));
                for (auto it = macd_signal.begin(); it != macd_signal.end(); ++it)
                {
                    for (auto mcdit = macd.begin(); mcdit != macd.end(); ++mcdit)
                    {
                        if (mcdit->first == it->first)
                        {
                            double hist_val = mcdit->second - it->second;
                            if (hist_val < m_data_y_min)
                            {
                                m_data_y_min = hist_val;
                            }
                            if (hist_val > m_data_y_max)
                            {
                                m_data_y_max = hist_val;
                            }
                            const double st = it->first - (0.4*m_candle_time_width_milliseconds);
                            const double en = it->first + (0.4*m_candle_time_width_milliseconds);
                            if (hist_val > 0.0)
                            {
                                macd_hist_upper.append(QPointF(st,0.0));
                                macd_hist_upper.append(QPointF(st,hist_val));
                                macd_hist_upper.append(QPointF(en,hist_val));
                                macd_hist_upper.append(QPointF(en,0.0));
                                zeroline.append(QPointF(st,0.0));
                                zeroline.append(QPointF(en,0.0));
                            }
                            else if (hist_val < 0.0)
                            {
                                macd_hist_lower.append(QPointF(st,0.0));
                                macd_hist_lower.append(QPointF(st,hist_val));
                                macd_hist_lower.append(QPointF(en,hist_val));
                                macd_hist_lower.append(QPointF(en,0.0));
                                zeroline.append(QPointF(st,0.0));
                                zeroline.append(QPointF(en,0.0));
                            }
                            break;
                        }
                    }
                }
            }
        }
        zeroline.append(QPointF(m_data_x_max,0.0));
    }
    m_x_axis_min_display = m_data_x_min;
    m_x_axis_max_display = m_data_x_max;
    m_y_axis_min_display = 2.0*m_data_y_min;
    m_y_axis_max_display = 2.0*m_data_y_max;

    m_macd->replace(qmacd);
    m_macd_signal->replace(qmacd_signal);
    m_macd_hist_neg->replace(macd_hist_lower);
    m_macd_hist_pos->replace(macd_hist_upper);
    m_zero_line->replace(zeroline);

    zoomToDefaultXRange();
    zoomToDefaultYRange();
}

//====================================================================
void TMACDChartWrapper::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
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
void TMACDChartWrapper::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
{
    m_y_axis_min_display = y_min;
    m_y_axis_max_display = y_max;
    if (m_y_axis_max_display < m_y_axis_min_display)
    {
        std::swap(m_y_axis_max_display, m_y_axis_min_display);
    }
    m_chart->axisY()->setRange(m_y_axis_min_display-hyst, m_y_axis_max_display+hyst);
}

} // endof namespace charting
