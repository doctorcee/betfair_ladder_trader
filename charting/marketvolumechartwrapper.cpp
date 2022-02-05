#include "marketvolumechartwrapper.h"

namespace charting {

//====================================================================
TMarketVolumeChartWrapper::TMarketVolumeChartWrapper(QWidget *parent,
                                                     const std::uint16_t& theme)
    : TAbstractChartWrapper (parent, theme),
      m_volume(nullptr),
      m_volume_bar(nullptr),
      m_selection_id(0),
      chart_margins(5)
{
    m_volume = new QLineSeries();
    m_volume->setColor(QColor(242, 123, 147, 0xb0));
    m_volume->setVisible(false);

    m_volume_bar = new QAreaSeries(m_volume);
    m_volume_bar->setColor(betfair::utils::backVWAPXOTransparent);
    m_volume_bar->setBorderColor(Qt::red);
    m_volume_bar->setVisible(true);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("hh:mm:ss");
    axisX->setMinorGridLineVisible(false);
    axisX->setGridLineVisible(false);
    QFont lbl_font_x = axisX->labelsFont();
    lbl_font_x.setWeight(QFont::Normal);
    lbl_font_x.setPointSize(m_default_xaxis_label_font_pointsize);
    axisX->setLabelsFont(lbl_font_x);
    lbl_font_x.setPointSize(m_default_xaxis_title_font_pointsize);
    axisX->setTitleFont(lbl_font_x);
    axisX->show();
    //axisX->setLineVisible(false);
    axisX->setTitleVisible(false);
    axisX->setLabelsVisible(false);

    QValueAxis *axisY = new QValueAxis;

    axisY->setMinorTickCount(0);
    axisY->setLabelFormat("%5.0f");
    axisY->setMinorGridLineVisible(false);
    axisY->setGridLineVisible(false);
    QFont lbl_font = axisY->labelsFont();
    lbl_font.setWeight(QFont::Normal);
    lbl_font.setPointSize(m_default_yaxis_label_font_pointsize);
    axisY->setLabelsFont(lbl_font);
    lbl_font.setPointSize(m_default_yaxis_title_font_pointsize);
    axisY->setTitleFont(lbl_font);
    //axisY->setLineVisible(false);
    axisY->setLabelsVisible(false);
    axisY->setTitleVisible(false);

    m_chart->addSeries(m_volume);
    m_chart->addSeries(m_volume_bar);

    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    m_volume->attachAxis(axisX);
    m_volume->attachAxis(axisY);
    m_volume_bar->attachAxis(axisX);
    m_volume_bar->attachAxis(axisY);

    m_chart->layout()->setContentsMargins(1, 1, 1, 1);
    setMargins(chart_margins);
    m_chart->setBackgroundRoundness(0);
    applyCurrentTheme();
}




//====================================================================
TMarketVolumeChartWrapper::~TMarketVolumeChartWrapper()
{
    m_chart->removeAllSeries();
}

//====================================================================
int TMarketVolumeChartWrapper::getLabelWidth()
{
    QFont labelFont = m_chart->axisY()->labelsFont();
    QString volmax = QString::number(m_y_axis_max_display,'f',2);
    QFontMetrics fm(labelFont);

    return fm.boundingRect(volmax).width();
}

//====================================================================
void TMarketVolumeChartWrapper::clear()
{    
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
void TMarketVolumeChartWrapper::populateSeries(const betfair::TBetfairMarket& market)
{
    QVector<QPointF> v_traded;
    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = 0.0;
    m_data_y_max = 0.0;
    if (market.valid())
    {
        const std::map<double,double> data = market.getTotalMarketVolumeSeries();
        if (data.empty() == false)
        {
            m_data_x_min = data.begin()->first;
            m_data_x_max = data.rbegin()->first;
            m_data_y_min = data.begin()->second/1000.0;
            m_data_y_max = data.rbegin()->second/1000.0;
            for (auto pit = data.begin(); pit != data.end(); ++pit)
            {
                v_traded.append(QPointF(pit->first,pit->second/1000.0));
            }
        }
    }
    else
    {
        clear();
    }
    m_volume->replace(v_traded);

    zoomToDefaultXRange();
    zoomToDefaultYRange(0.005);
}



//====================================================================
void TMarketVolumeChartWrapper::populateSeries(const betfair::TBetfairMarket& market,
                                                const std::int64_t& selection_id)
{

}

//====================================================================
void TMarketVolumeChartWrapper::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
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
void TMarketVolumeChartWrapper::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
{
    m_y_axis_min_display = y_min;
    m_y_axis_max_display = y_max;
    if (m_y_axis_max_display < m_y_axis_min_display)
    {
        std::swap(m_y_axis_max_display, m_y_axis_min_display);
    }
    m_chart->axisY()->setRange(m_y_axis_min_display-hyst, m_y_axis_max_display+hyst);
}



} // end of namespace charting
