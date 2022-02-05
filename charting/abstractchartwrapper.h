#ifndef ABSTRACTCHARTWRAPPER_H
#define ABSTRACTCHARTWRAPPER_H

#include <QtCharts>
#include <QtCharts/QChartView>
#include "betfair/betfairmarket.h"

namespace charting {

class TAbstractChartWrapper
{
private:
    TAbstractChartWrapper();
    TAbstractChartWrapper(const TAbstractChartWrapper& cp);
    TAbstractChartWrapper& operator=(const TAbstractChartWrapper& cp);

protected:

    QChart* m_chart;
    QChartView* m_chart_view;

    double m_x_axis_min_display;
    double m_x_axis_max_display;
    double m_y_axis_min_display;
    double m_y_axis_max_display;

    double m_data_x_min;
    double m_data_x_max;
    double m_data_y_min;
    double m_data_y_max;

    int m_default_title_font_pointsize;
    int m_default_xaxis_title_font_pointsize;
    int m_default_yaxis_title_font_pointsize;
    int m_default_xaxis_label_font_pointsize;
    int m_default_yaxis_label_font_pointsize;

    std::uint16_t m_display_theme;

public:
    TAbstractChartWrapper(QWidget *parent,
                          const std::uint16_t& theme);
    virtual ~TAbstractChartWrapper();

    QChartView* getChartView() const {return m_chart_view;}

    virtual void clear() = 0;
    virtual int getLabelWidth() = 0;

    void setMargins(int mg);
    void setMargins(const QMargins& mg);
    QMargins getMargins() const;

    void zoomToDefaultXRange(double hist = 0.0);
    void zoomToDefaultYRange(double hist = 0.0);

    double getDisplayYMax() const {return m_y_axis_max_display;}
    double getDisplayYMin() const {return m_y_axis_min_display;}

    double getDisplayXMax() const {return m_x_axis_max_display;}
    double getDisplayXMin() const {return m_x_axis_min_display;}

    void setTitle(const QString& title);

    virtual void setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst = 0.0) = 0;
    virtual void setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst = 0.0) = 0;

    virtual void showXAxisLabels(bool show);
    virtual void showYAxisLabels(bool show);
    virtual void showXAxisTitle(bool show);
    virtual void showYAxisTitle(bool show);
    virtual void setXAxisTickCount(int ticks);
    virtual void setXAxisMinorTickCount(int ticks);
    virtual void setYAxisTickCount(int ticks);
    virtual void setYAxisMinorTickCount(int ticks);
    virtual void setYAxisTitle(const QString& title);
    virtual void setXAxisTitle(const QString& title);
    virtual void showGrid(bool show);
    virtual void showMinorGrid(bool show);

    virtual void changeTitleFontPointSize(int size);
    virtual void changeXAxisTitleFontPointSize(int size);
    virtual void changeYAxisTitleFontPointSize(int size);
    virtual void changeXAxisLabelFontPointSize(int size);
    virtual void changeYAxisLabelFontPointSize(int size);

    void setTheme(const std::uint16_t& theme) {m_display_theme = theme;}
    std::uint16_t getTheme() const {return m_display_theme;}

    void applyCurrentTheme();


};

} // end of namespace charting

#endif // ABSTRACTCHARTWRAPPER_H
