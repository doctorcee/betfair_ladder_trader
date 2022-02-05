#include "abstractchartwrapper.h"

namespace charting {

//==========================================================================
TAbstractChartWrapper::TAbstractChartWrapper(QWidget* parent,
                                             const std::uint16_t& theme)
    : m_chart(nullptr),
      m_chart_view(nullptr),
      m_x_axis_min_display(0.0),
      m_x_axis_max_display(0.0),
      m_y_axis_min_display(0.0),
      m_y_axis_max_display(0.0),
      m_data_x_min(0.0),
      m_data_x_max(0.0),
      m_data_y_min(0.0),
      m_data_y_max(0.0),
      m_default_title_font_pointsize(9),
      m_default_xaxis_title_font_pointsize(8),
      m_default_yaxis_title_font_pointsize(8),
      m_default_xaxis_label_font_pointsize(7),
      m_default_yaxis_label_font_pointsize(7),
      m_display_theme(theme)
{
    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setTitle("");
    m_chart_view = new QChartView(m_chart,parent);
    m_chart_view->setRenderHint(QPainter::Antialiasing);
}

//==========================================================================
TAbstractChartWrapper::~TAbstractChartWrapper()
{

}

//====================================================================
void TAbstractChartWrapper::applyCurrentTheme()
{
    if (1 == m_display_theme)
    {
        m_chart->setPlotAreaBackgroundBrush(QColor(53,53,53));
        m_chart->setBackgroundBrush(QColor(53,53,53));
        m_chart->setPlotAreaBackgroundVisible(true);
        m_chart->axisX()->setLinePenColor(QColor(88,88,88));
        m_chart->axisY()->setLinePenColor(QColor(88,88,88));
        m_chart->axisX()->setLabelsBrush(Qt::white);
        m_chart->axisY()->setLabelsBrush(Qt::white);
        m_chart->axisX()->setTitleBrush(Qt::white);
        m_chart->axisY()->setTitleBrush(Qt::white);
        m_chart->axisX()->setGridLineColor(QColor(88,88,88));
        m_chart->axisY()->setGridLineColor(QColor(88,88,88));
        m_chart->axisX()->setMinorGridLineColor(QColor(66,66,66));
        m_chart->axisY()->setMinorGridLineColor(QColor(66,66,66));

    }
}

//====================================================================
void TAbstractChartWrapper::setTitle(const QString& title)
{
    if (m_chart != nullptr)
    {
        m_chart->setTitle(title);
    }
}

//====================================================================
void TAbstractChartWrapper::zoomToDefaultXRange(double hist)
{
    setXAxesDisplayRange(m_data_x_min,m_data_x_max,hist);
}

//====================================================================
void TAbstractChartWrapper::zoomToDefaultYRange(double hist)
{
    setYAxesDisplayRange(m_data_y_min,m_data_y_max,hist);
}

//==========================================================================
void TAbstractChartWrapper::setYAxisTitle(const QString& title)
{
    if (m_chart != nullptr)
    {
        m_chart->axisY()->setTitleText(title);
    }
}

//==========================================================================
void TAbstractChartWrapper::setXAxisTitle(const QString& title)
{
    if (m_chart != nullptr)
    {
        m_chart->axisX()->setTitleText(title);
    }
}

//==========================================================================
void TAbstractChartWrapper::showXAxisLabels(bool show)
{
    if (m_chart != nullptr)
    {
        m_chart->axisX()->setLabelsVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::showYAxisLabels(bool show)
{
    if (m_chart != nullptr)
    {
        m_chart->axisY()->setLabelsVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::showXAxisTitle(bool show)
{
    if (m_chart != nullptr)
    {
        m_chart->axisX()->setTitleVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::showYAxisTitle(bool show)
{
    if (m_chart != nullptr)
    {
        m_chart->axisY()->setTitleVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::showGrid(bool show)
{
    if (m_chart != nullptr)
    {
        m_chart->axisX()->setGridLineVisible(show);
        m_chart->axisY()->setGridLineVisible(show);
        m_chart->axisY()->setMinorGridLineVisible(show);
        m_chart->axisX()->setMinorGridLineVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::showMinorGrid(bool show)
{
    // make sure major grid ALSO shown
    if (m_chart != nullptr)
    {
        if (show)
        {
            // if enabling display it makes no sense to show minor grid
            // without major grid.
            m_chart->axisX()->setGridLineVisible(show);
            m_chart->axisY()->setGridLineVisible(show);
        }
        m_chart->axisY()->setMinorGridLineVisible(show);
        m_chart->axisX()->setMinorGridLineVisible(show);
    }
}

//==========================================================================
void TAbstractChartWrapper::setXAxisTickCount(int ticks)
{
    if (m_chart != nullptr)
    {
        QDateTimeAxis* axis = dynamic_cast<QDateTimeAxis*>(m_chart->axisX());
        if (axis != nullptr)
        {
            axis->setTickCount(ticks);
        }
    }
}

//==========================================================================
void TAbstractChartWrapper::setYAxisTickCount(int ticks)
{
    if (m_chart != nullptr)
    {
        QValueAxis* axis = dynamic_cast<QValueAxis*>(m_chart->axisY());
        if (axis != nullptr)
        {
            axis->setTickCount(ticks);
        }
    }
}

//==========================================================================
void TAbstractChartWrapper::setXAxisMinorTickCount(int ticks)
{
    if (m_chart != nullptr)
    {
        QValueAxis* axis = dynamic_cast<QValueAxis*>(m_chart->axisX());
        if (axis != nullptr)
        {
            axis->setMinorTickCount(ticks);
        }
    }
}

//==========================================================================
void TAbstractChartWrapper::setYAxisMinorTickCount(int ticks)
{
    if (m_chart != nullptr)
    {
        QValueAxis* axis = dynamic_cast<QValueAxis*>(m_chart->axisY());
        if (axis != nullptr)
        {
            axis->setTickCount(ticks);
        }
    }
}

//==========================================================================
void TAbstractChartWrapper::changeTitleFontPointSize(int size)
{
    if ((m_chart != nullptr) && (size > 0))
    {
        m_default_title_font_pointsize = size;
        QFont font = m_chart->font();
        font.setPointSize(m_default_title_font_pointsize);
        m_chart->setTitleFont(font);
    }
}
//==========================================================================
void TAbstractChartWrapper::changeXAxisTitleFontPointSize(int size)
{
    if ((m_chart != nullptr) && (size > 0))
    {
        m_default_xaxis_title_font_pointsize = size;
        QFont font = m_chart->axisX()->titleFont();
        font.setPointSize(m_default_xaxis_title_font_pointsize);
        m_chart->axisX()->setTitleFont(font);
    }
}
//==========================================================================
void TAbstractChartWrapper::changeYAxisTitleFontPointSize(int size)
{
    if ((m_chart != nullptr) && (size > 0))
    {
        m_default_yaxis_title_font_pointsize = size;
        QFont font = m_chart->axisY()->titleFont();
        font.setPointSize(m_default_yaxis_title_font_pointsize);
        m_chart->axisY()->setTitleFont(font);
    }
}
//==========================================================================
void TAbstractChartWrapper::changeXAxisLabelFontPointSize(int size)
{
    if ((m_chart != nullptr) && (size > 0))
    {
        m_default_xaxis_label_font_pointsize = size;
        QFont font = m_chart->axisX()->labelsFont();
        font.setPointSize(m_default_xaxis_label_font_pointsize);
        m_chart->axisX()->setLabelsFont(font);
    }
}
//==========================================================================
void TAbstractChartWrapper::changeYAxisLabelFontPointSize(int size)
{
    if ((m_chart != nullptr) && (size > 0))
    {
        m_default_xaxis_label_font_pointsize = size;
        QFont font = m_chart->axisX()->labelsFont();
        font.setPointSize(m_default_xaxis_label_font_pointsize);
        m_chart->axisY()->setLabelsFont(font);
    }
}


//==========================================================================
void TAbstractChartWrapper::setMargins(const QMargins& mg)
{
    m_chart->setMargins(mg);
}

//==========================================================================
QMargins TAbstractChartWrapper::getMargins() const
{
    return m_chart->margins();
}

//====================================================================
void TAbstractChartWrapper::setMargins(int mg)
{
    if (mg >= 0)
    {
        QMargins margins = QMargins(mg,mg,mg,mg);
        m_chart->setMargins(margins);
    }
}

} // end of namespace charting
