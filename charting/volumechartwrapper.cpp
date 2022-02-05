#include "volumechartwrapper.h"

namespace charting {

//==========================================================================
TVolumeChartWrapper::TVolumeChartWrapper(QWidget *parent,
                                         const std::uint16_t& theme)
    : TIndividualSelectionChartWrapperBase(parent,theme),
      m_volume(nullptr),
      m_volume_bar(nullptr),
      m_selection_id(0),
      m_candle_time_width_milliseconds(0),
      chart_margins(5)
{
    m_volume = new QLineSeries();
    m_volume->setColor(QColor(242, 123, 147, 0xb0));

    m_volume_bar = new QAreaSeries(m_volume);
    m_volume_bar->setColor(QColor(242, 123, 147, 0xd0));
    m_volume_bar->setBorderColor(Qt::red);
    m_volume_bar->setVisible(true);

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
    axisX->show();
    axisX->setLabelsVisible(false);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTickCount(3);
    axisY->setTitleText("Vol/£1K");
    axisY->setMinorTickCount(2);
    axisY->setLabelFormat("%4.2f");
    axisY->setMinorGridLineVisible(true);
    axisY->setGridLineVisible(true);
    QFont lbl_font = axisY->labelsFont();
    lbl_font.setWeight(QFont::Normal);
    lbl_font.setPointSize(m_default_yaxis_label_font_pointsize);
    axisY->setLabelsFont(lbl_font);
    lbl_font.setPointSize(m_default_yaxis_title_font_pointsize);
    axisY->setTitleFont(lbl_font);

    m_chart->addSeries(m_volume);
    m_chart->addSeries(m_volume_bar);

    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    m_volume->attachAxis(axisX);
    m_volume->attachAxis(axisY);
    m_volume_bar->attachAxis(axisX);
    m_volume_bar->attachAxis(axisY);

    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    setMargins(chart_margins);
    m_chart->setBackgroundRoundness(0);
    applyCurrentTheme();
}

//====================================================================
TVolumeChartWrapper::~TVolumeChartWrapper()
{
}

//====================================================================
int TVolumeChartWrapper::getLabelWidth()
{
    QFont labelFont = m_chart->axisY()->labelsFont();
    QString volmax = QString::number(m_y_axis_max_display,'f',2);
    QFontMetrics fm(labelFont);

    return fm.boundingRect(volmax).width();
}

//====================================================================
void TVolumeChartWrapper::clear()
{
    //m_chart->setTitle("");
    m_volume->clear();
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
void TVolumeChartWrapper::populateSeries(const std::vector<betfair::utils::TBetfairAdvancedCandle>& data_vector,
                                         const std::int64_t& sel_id)
{
    QVector<QPointF> v_vol;
    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = 0.0;
    m_data_y_max = 0.0;
    m_selection_id = sel_id;
    if (!data_vector.empty())
    {
        m_data_x_min = std::numeric_limits<double>::max();
        m_data_x_max = -m_data_x_min;
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
            const double v = data_it->getCandleVol()/1000.0;
            const qreal fake_bar_width = (m_candle_time_width_milliseconds > 0) ? (0.4 * m_candle_time_width_milliseconds) : 100.0;
            v_vol.append(QPointF(ts-fake_bar_width,0));
            v_vol.append(QPointF((ts-fake_bar_width),v));
            v_vol.append(QPointF((ts+fake_bar_width),v));
            v_vol.append(QPointF(ts+fake_bar_width,0));
            if (v > m_data_y_max)
            {
                m_data_y_max = v;
            }
            last_ts = ts;
        }
    }
    m_x_axis_min_display = m_data_x_min;
    m_x_axis_max_display = m_data_x_max;
    m_y_axis_min_display = m_data_y_min;
    m_y_axis_max_display = m_data_y_max;

    m_volume->replace(v_vol);

    zoomToDefaultXRange();
    zoomToDefaultYRange();
}

//====================================================================
void TVolumeChartWrapper::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
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
void TVolumeChartWrapper::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
{
    m_y_axis_min_display = 0;
    m_y_axis_max_display = y_max;
    if (m_y_axis_max_display <= 0.0)
    {
        m_y_axis_max_display = 1.0;
    }
    if (m_y_axis_max_display < m_y_axis_min_display)
    {
        std::swap(m_y_axis_max_display, m_y_axis_min_display);
    }
    m_chart->axisY()->setRange(m_y_axis_min_display-hyst, m_y_axis_max_display+hyst);
}

} // end of namespace charting

