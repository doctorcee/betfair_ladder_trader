#include "runnerchartsdialog.h"
#include "ui_runnerchartsdialog.h"
#include "betfair/runner.h"
#include "betfair/betfair_utils.h"
#include <QBrush>
#include <QPen>
#include <stdexcept>

static const unsigned int colour_array_size = 14;
static const QColor colour_array[colour_array_size] =
                                        {
                                            Qt::red, Qt::blue, Qt::green, Qt::cyan, Qt::yellow,
                                            Qt::magenta, Qt::darkGreen, QColor(0xff,0x66,0x00,0xff), Qt::darkMagenta, Qt::darkBlue,
                                            Qt::darkGray,
                                            QColor(0x88,0x00,0x88,0xff),
                                            QColor(0x00,0x99,0x99,0xff),
                                            QColor(0x99,0x66,0x33,0xff)
                                        };

//================================================================
RunnerChartsDialog::RunnerChartsDialog(betfair::TBetfairMarket& mkt,
                                       const std::uint16_t& theme,
                                       QWidget *parent)
    : QDialog(parent),
      ui(new Ui::RunnerChartsDialog),
      m_display_theme(theme),
      m_market(mkt),
      m_chart(nullptr),
      m_chart_view(nullptr),
      m_sched_off_line(nullptr),
      m_inplay_start_line(nullptr),
      m_odds_tick_map(betfair::utils::generateOffsetMap(betfair::utils::generateOddsMap())),
      m_selected_runner_index(0),
      m_selected_runner_id(-1),
      m_selected_runner_name("ALL RUNNERS"),
      m_x_axis_min_display(0.0),
      m_x_axis_max_display(0.0),
      m_y_axis_min_display(0.0),
      m_y_axis_max_display(0.0),
      m_data_x_min(0.0),
      m_data_x_max(0.0),
      m_data_y_min(0.0),
      m_data_y_max(0.0),
      m_x_zoom_mode(0),  // 0 = ALL, 1 = last 60, 2 = last 120
      m_y_zoom_mode(0)  // 0 = ALL, 1 = odds on, 2 = ????
{
    ui->setupUi(this);
    m_chart = new QChart();
    {
        m_chart->setPlotAreaBackgroundBrush(QColor(0xf4,0xf4,0xff));
        m_chart->setPlotAreaBackgroundVisible(true);
        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setTickCount(5);
        axisX->setFormat("hh:mm:ss");
        axisX->setMinorGridLineVisible(true);
        axisX->setGridLineVisible(true);
        axisX->show();
        QFont lbl_font_x = axisX->labelsFont();
        lbl_font_x.setWeight(QFont::Normal);
        lbl_font_x.setPointSize(7);
        axisX->setLabelsFont(lbl_font_x);
        axisX->setTitleFont(lbl_font_x);
        axisX->setLabelsVisible(true);

        QValueAxis *axisY = new QValueAxis;
        axisY->setTickCount(5);
        axisY->setMinorTickCount(4);
        axisY->setLabelFormat("%4.0f");
        axisY->setTitleText("Odds Tick");
        axisY->setMinorGridLineVisible(true);
        axisY->setGridLineVisible(true);
        QFont lbl_font = axisY->labelsFont();
        lbl_font.setWeight(QFont::Normal);
        lbl_font.setPointSize(7);
        axisY->setLabelsFont(lbl_font);
        //lbl_font.setPointSize(8);
        axisY->setTitleFont(lbl_font);

        m_sched_off_line = new QLineSeries();
        if (1 == m_display_theme)
        {
            QBrush stbrush(QColor(0xdd,0xff,0xff,0x99));
            QPen stpen(stbrush,2);
            m_sched_off_line->setPen(stpen);
        }
        else
        {
            QBrush stbrush(Qt::darkRed);
            QPen stpen(stbrush,2);
            m_sched_off_line->setPen(stpen);
        }


        m_sched_off_line->setName("Scheduled Start Time");
        m_chart->addSeries(m_sched_off_line);
        m_inplay_start_line = new QLineSeries();
        if (1 == m_display_theme)
        {
            QBrush ipbrush(QColor(0xff,0x44,0x00,0x99));
            QPen ippen(ipbrush,2);
            m_inplay_start_line->setPen(ippen);
        }
        else
        {
            QBrush ipbrush(QColor(0xcc,0xff,0x33,0xaa));
            QPen ippen(ipbrush,2);
            m_inplay_start_line->setPen(ippen);
        }
        m_inplay_start_line->setName("In-play Start Time");
        m_chart->addSeries(m_inplay_start_line);

        m_chart->addAxis(axisY, Qt::AlignLeft);
        m_chart->addAxis(axisX, Qt::AlignBottom);

        m_sched_off_line->attachAxis(axisX);
        m_sched_off_line->attachAxis(axisY);
        m_inplay_start_line->attachAxis(axisX);
        m_inplay_start_line->attachAxis(axisY);

        QFont lf = m_chart->legend()->font();
        lf.setPointSize(8);
        m_chart->legend()->setFont(lf);
    }

    setWindowTitle("NO MARKET SELECTED");

    m_chart_view = new QChartView(m_chart);
    m_chart_view->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(m_chart_view,3);
    setWindowTitle(m_selected_runner_name);
    connect(ui->cbChartRunnerSelect,SIGNAL(currentIndexChanged(int)),this,SLOT(changeSelectedRunner(int)));

    ui->cbYAxisDisplay->addItem("Show Auto-range");
    ui->cbYAxisDisplay->addItem("Show Odds-on range");
    ui->cbYAxisDisplay->addItem("Show 1.01 to 3.0");
    ui->cbYAxisDisplay->addItem("Show 1.01 to 5.0");
    ui->cbYAxisDisplay->addItem("Show 1.01 to 10.0");
    ui->cbYAxisDisplay->addItem("Show 1.01 to 20.0");
    ui->cbYAxisDisplay->addItem("Show 1.01 to 100.0");
    ui->cbYAxisDisplay->setCurrentIndex(0);
    connect(ui->cbYAxisDisplay,SIGNAL(currentIndexChanged(int)),this,SLOT(changeYAxisDisplayOptions(int)));

    ui->cbXAxisDisplay->addItem("Show All Data");
    ui->cbXAxisDisplay->addItem("Show Last 30 Seconds");
    ui->cbXAxisDisplay->addItem("Show Last 60 Seconds");
    ui->cbXAxisDisplay->addItem("Show Last 120 Seconds");
    connect(ui->cbXAxisDisplay,SIGNAL(currentIndexChanged(int)),this,SLOT(changeXAxisDisplayOptions(int)));
    ui->cbXAxisDisplay->setCurrentIndex(0);

    const int mg = 5;
    QMargins margins = QMargins(mg,mg,mg,mg);
    m_chart->setMargins(margins);
    m_chart->layout()->setContentsMargins(0, 0, 0, 0);
    m_chart->setBackgroundRoundness(0);
    m_chart->legend()->setAlignment(Qt::AlignRight);

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
        m_chart->legend()->setLabelColor(Qt::white);
        m_chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    }

}

//============================================================
RunnerChartsDialog::~RunnerChartsDialog()
{
    delete ui;
}

//============================================================
std::uint32_t RunnerChartsDialog::getTickFromOdds(double odds)
{
    std::uint32_t tick = 0;
    QString strodds = QString::number(odds,'f',2);
    auto it = m_odds_tick_map.find(strodds);
    if (it != m_odds_tick_map.end())
    {
        tick = it->second;
    }
    return tick;
}


//============================================================
void RunnerChartsDialog::clearChart()
{    
    for (auto it = m_m_traces.begin(); it != m_m_traces.end(); ++it)
    {
        m_chart->removeSeries(it->second);
    }
    m_m_traces.clear();
    m_sched_off_line->clear();
    m_inplay_start_line->clear();
    m_chart->legend()->hide();
    setWindowTitle("NO MARKET SELECTED");
    m_x_axis_min_display = 0.0;
    m_x_axis_max_display = 0.0;
    m_y_axis_min_display = 0.0;
    m_y_axis_max_display = 0.0;
    m_data_x_min = 0.0;
    m_data_x_max = 0.0;
    m_data_y_min = 0.0;
    m_data_y_max = 0.0;
}

//============================================================
void RunnerChartsDialog::setVisibleRunner(const std::int64_t& id)
{
    // look up this ID in the selection map
    int new_index = 0;
    for (auto it = m_m_selection_index_map.begin(); it != m_m_selection_index_map.end(); ++it)
    {
        if (it->second == id)
        {
            // This is it!
            new_index = it->first;
            break;
        }
    }
    if (new_index > static_cast<int>(m_market.getNumRunners()))
    {
        throw std::runtime_error("Invalid index to chart!");
    }
    ui->cbChartRunnerSelect->setCurrentIndex(new_index);
    changeSelectedRunner(new_index);
    setCurrentYAxisLimits();
    setYAxesDisplayRange(m_y_axis_min_display,m_y_axis_max_display);
}

//============================================================
void RunnerChartsDialog::changeSelectedRunner(int i)
{
    m_m_runner_y_bounds.clear();
    m_chart->legend()->hide();
    QString mlabel = m_market.getLabel();
    if (i > 0)
    {
        m_selected_runner_index = i;
        m_selected_runner_id = m_m_selection_index_map[m_selected_runner_index];
        m_selected_runner_name = ui->cbChartRunnerSelect->currentText();
    }
    else
    {
        m_selected_runner_id = -1;
        m_selected_runner_index = 0;
        m_selected_runner_name = "ALL RUNNERS";       
    }
    mlabel.append(" : ");
    mlabel.append(m_selected_runner_name);
    setWindowTitle(mlabel);
    if (m_m_traces.empty() == false)
    {
        updateChart();
    }
}

//============================================================
void RunnerChartsDialog::reset()
{
    if (m_market.valid())
    {
        clearChart();        
        m_m_runner_y_bounds.clear();
        const std::vector<std::shared_ptr<betfair::TRunner> >& runners = m_market.getRunners();
        std::size_t ct = runners.size();
        ui->cbChartRunnerSelect->clear();
        m_selected_runner_index = 0;
        m_selected_runner_id = -1;
        m_selected_runner_name = "ALL RUNNERS";
        ui->cbChartRunnerSelect->addItem(m_selected_runner_name);
        m_m_selection_index_map.clear();
        for (std::size_t i = 0; i < ct; ++i)
        {
            QLineSeries *series = new QLineSeries();
            if (i < colour_array_size)
            {
                series->setColor(colour_array[i]);
            }
            series->setPointsVisible(false);
            m_chart->addSeries(series);
            series->attachAxis(dynamic_cast<QDateTimeAxis*>(m_chart->axisX()));
            series->attachAxis(dynamic_cast<QValueAxis*>(m_chart->axisY()));
            const std::int64_t sel_id = runners[i]->getID();
            const QString selection_name = runners[i]->getName();
            series->setName(selection_name);
            ui->cbChartRunnerSelect->addItem(selection_name);
            m_m_selection_index_map.insert(std::pair<int,std::int64_t>(i+1,sel_id));
            m_m_runner_y_bounds.insert(std::pair<std::int64_t,std::pair<double,double> >(sel_id,std::pair<double,double>(0.0,0.0)));
            auto it = m_m_traces.find(sel_id);
            if (it == m_m_traces.end())
            {
                m_m_traces[sel_id] = series;
            }
            else
            {
                throw std::runtime_error("ERROR!! RunnerChartsDialog::setMarket() encountered duplicate selection ID");
            }
        }
        ui->cbChartRunnerSelect->setCurrentIndex(0);

        QString mlabel = m_market.getLabel();
        mlabel.append(" : ");
        mlabel.append(m_selected_runner_name);
        setWindowTitle(mlabel);
        m_chart->legend()->hide();


    }
    else
    {
        clearChart();
    }
}

//====================================================================
void RunnerChartsDialog::setCurrentYAxisLimits()
{
    double ymin = 1.0;
    double ymax = 0.0;
    if (0 == m_y_zoom_mode)
    {
        if (m_selected_runner_id > 0)
        {
            // find bounds of selected runner
            auto x_it = m_m_runner_y_bounds.find(m_selected_runner_id);
            if (x_it != m_m_runner_y_bounds.end())
            {
                ymin = x_it->second.first;
                ymax = x_it->second.second;
            }
        }
    }
    else if (1 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(2.0));
    }
    else if (2 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(3.0));
    }
    else if (3 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(5.0));
    }
    else if (4 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(10.0));
    }
    else if (5 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(20.0));
    }
    else if (6 == m_y_zoom_mode)
    {
        ymax = static_cast<double>(getTickFromOdds(100.0));
    }
    if (ymin < ymax)
    {
        m_y_axis_max_display = ymax;
        m_y_axis_min_display = ymin;
    }
    else
    {
        m_y_axis_max_display = m_data_y_max;
        m_y_axis_min_display = m_data_y_min;
    }
}

//====================================================================
void RunnerChartsDialog::changeYAxisDisplayOptions(int index)
{
    m_y_zoom_mode = index;
    setCurrentYAxisLimits();
}

//====================================================================
void RunnerChartsDialog::changeXAxisDisplayOptions(int index)
{
    m_x_zoom_mode = index;
}


//============================================================
void RunnerChartsDialog::updateChart()
{
    m_data_x_min = std::numeric_limits<double>::max();
    m_data_x_max = -std::numeric_limits<double>::max();
    m_data_y_min = std::numeric_limits<double>::max();
    m_data_y_max = -std::numeric_limits<double>::max();
    double min_ts = std::numeric_limits<double>::max();
    double max_ts = -std::numeric_limits<double>::max();
    //my_volume_wrapper.populateSeries(m_market);
    double ipstart = -1.0;
    if (m_market.valid())
    {        
        auto runners = m_market.getRunners();
        m_m_runner_y_bounds.clear();        

        for (auto it = runners.begin(); it != runners.end(); ++it)
        {
            if ((*it)->isActive())
            {
                //auto candle_data = (*it)->getCandleData();
                auto candle_data = (*it)->getAdvCandleData();
                const std::int64_t id = (*it)->getID();
                if (m_m_runner_y_bounds.find(id) == m_m_runner_y_bounds.end())
                {
                    m_m_runner_y_bounds[id] = std::make_pair(1000000.0,-1.0);
                }
                auto current_trace = m_m_traces.find(id);

                if (current_trace != m_m_traces.end())
                {
                    QVector<QPointF> v_trace;
                    for (auto d_it = candle_data.begin(); d_it != candle_data.end(); ++d_it)
                    {                                               
                        const double p = d_it->getLPMAsTick();                        
                        const double ts = d_it->end_timestamp;
                        if (ts > 0.0 && ts < min_ts)
                        {
                            min_ts = ts;
                        }
                        if (ts > 0.0 && ts > max_ts)
                        {
                            max_ts = ts;
                        }
                        if (p < m_data_y_min)
                        {
                            // YMIN
                            m_data_y_min = p;
                        }
                        if (p > m_data_y_max)
                        {
                            // YMAX
                            m_data_y_max = p;
                        }
                        auto x_it = m_m_runner_y_bounds.find(id);
                        if (x_it != m_m_runner_y_bounds.end())
                        {
                            if (p < x_it->second.first)
                            {
                                // YMIN
                                x_it->second.first = p;
                            }
                            if (p > x_it->second.second)
                            {
                                // YMAX
                                x_it->second.second = p;
                            }
                        }
                        v_trace.append(QPointF(ts,p));
                        if (ipstart < 0.0)
                        {
                            if (d_it->market_in_play_at_end)
                            {
                                ipstart = ts;
                            }
                        }
                    }
                    current_trace->second->replace(v_trace);
                    current_trace->second->setVisible((m_selected_runner_id < 0) || (id == m_selected_runner_id));
                }
                else
                {
                    throw std::runtime_error("ERROR!! RunnerChartsDialog::updateChart() selection ID does not exist");
                }
            }
        }
        QVector<QPointF> v_start_time_trace;
        QDateTime stt = m_market.getStartTime();
        int offset = stt.utcOffset();
        stt = stt.addSecs(offset);
        const double mkst = static_cast<double>(stt.toMSecsSinceEpoch());
        v_start_time_trace.append(QPointF(mkst,0));
        v_start_time_trace.append(QPointF(mkst,m_odds_tick_map.size()));
        m_sched_off_line->replace(v_start_time_trace);

        QVector<QPointF> v_ip_start;
        if (ipstart > 0.0)
        {
            v_ip_start.append(QPointF(ipstart,0));
            v_ip_start.append(QPointF(ipstart,m_odds_tick_map.size()));
        }
        m_inplay_start_line->replace(v_ip_start);

    }
    else
    {
        max_ts = 0;
    }
    // Set X axis limits
    if (1 == m_x_zoom_mode)
    {
        min_ts = max_ts - 31000.0;
    }
    else if (2 == m_x_zoom_mode)
    {
        min_ts = max_ts - 61000.0;
    }
    else if (3 == m_x_zoom_mode)
    {
        min_ts = max_ts  - 121000.0;
    }
    m_chart->legend()->setVisible(m_selected_runner_id < 0);
    setCurrentYAxisLimits();
    setXAxesDisplayRange(min_ts,max_ts,200);
    double yhyst = 1.0;
    if (m_selected_runner_id < 0)
    {
        yhyst = (m_y_zoom_mode > 0) ? 1.0 : 10.0;
    }
    setYAxesDisplayRange(m_y_axis_min_display,m_y_axis_max_display,yhyst);
}


//====================================================================
void RunnerChartsDialog::setXAxesDisplayRange(const double& x_min, const double& x_max, double hyst)
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
void RunnerChartsDialog::setYAxesDisplayRange(const double& y_min, const double& y_max, double hyst)
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

