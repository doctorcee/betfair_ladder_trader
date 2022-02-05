#include "runneranalysischartsdialog.h"
#include "ui_runneranalysischartsdialog.h"


//=========================================================================================
TRunnerAnalysisChartsDialog::TRunnerAnalysisChartsDialog(betfair::TBetfairMarket& mkt,
                                                         const std::uint16_t& theme,
                                                         QWidget *parent)
    : QDialog(parent),
      ui(new Ui::TRunnerAnalysisChartsDialog),
      m_display_theme(theme),
      m_market(mkt),
      m_candles_wrapper(this,m_display_theme),
      m_volume_wrapper(this,m_display_theme),
      m_macd_wrapper(this,m_display_theme),      
      m_selected_runner_id(-1),
      m_selected_runner_index(-1),
      m_selected_runner_name("NONE"),
      m_x_zoom_mode(0),
      m_y_zoom_mode(0),
      m_display_interval_milliseconds(0)
{
    ui->setupUi(this);
    {
        QChartView* cv_candle = m_candles_wrapper.getChartView();
        ui->chartVLayout->addWidget(cv_candle,2);
    }
    {
        QChartView* cv_volume = m_volume_wrapper.getChartView();
        ui->chartVLayout->addWidget(cv_volume,1);
    }

    QChartView* cv_macd = m_macd_wrapper.getChartView();
    ui->chartVLayout->addWidget(cv_macd,1);


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
    ui->cbXAxisDisplay->addItem("Show Last 300 Seconds");
    connect(ui->cbXAxisDisplay,SIGNAL(currentIndexChanged(int)),this,SLOT(changeXAxisDisplayOptions(int)));
    ui->cbXAxisDisplay->setCurrentIndex(0);

    ui->cbChartDIsplayInterval->addItem("Auto (sample rate)");
    ui->cbChartDIsplayInterval->addItem("1 second");
    ui->cbChartDIsplayInterval->addItem("2 seconds");
    ui->cbChartDIsplayInterval->addItem("5 seconds");
    ui->cbChartDIsplayInterval->addItem("10 seconds");
    connect(ui->cbChartDIsplayInterval,SIGNAL(currentIndexChanged(int)),this,SLOT(onChangeDisplayInterval(int)));
    ui->cbXAxisDisplay->setCurrentIndex(0);

    m_macd_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);
    m_candles_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);
    m_volume_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);

    connect(ui->chkHeikinAshi,&QCheckBox::stateChanged,this,&TRunnerAnalysisChartsDialog::onChangeHeikinAshiCandleMode);
}

//=========================================================================================
TRunnerAnalysisChartsDialog::~TRunnerAnalysisChartsDialog()
{
    delete ui;
}

//====================================================================
void TRunnerAnalysisChartsDialog::setCurrentYAxisLimits()
{
    double ymin = 1.0;
    double ymax = 0.0;
    if (1 == m_y_zoom_mode)
    {
        ymax = 2.0;
    }
    else if (2 == m_y_zoom_mode)
    {
        ymax = 3.0;
    }
    else if (3 == m_y_zoom_mode)
    {
        ymax = 5.0;
    }
    else if (4 == m_y_zoom_mode)
    {
        ymax = 10.0;
    }
    else if (5 == m_y_zoom_mode)
    {
        ymax = 20.0;
    }
    else if (6 == m_y_zoom_mode)
    {
        ymax = 100.0;
    }
    if (ymin < ymax)
    {
        m_candles_wrapper.setYAxesDisplayRange(ymin,ymax);
    }
    else
    {
        m_candles_wrapper.zoomToDefaultYRange();
    }
    m_volume_wrapper.zoomToDefaultYRange();
}

//====================================================================
void TRunnerAnalysisChartsDialog::onChangeHeikinAshiCandleMode(int state)
{
    m_candles_wrapper.setHeikinAshiMode(Qt::Checked == state);
    updateChart();
}

//====================================================================
void TRunnerAnalysisChartsDialog::onChangeDisplayInterval(int index)
{
    if (0 == index)
    {
        m_display_interval_milliseconds = 0;
    }
    else if (1 == index)
    {
        m_display_interval_milliseconds = 1000;
    }
    else if (2 == index)
    {
        m_display_interval_milliseconds = 2000;
    }
    else if (3 == index)
    {
        m_display_interval_milliseconds = 5000;
    }
    else if (4 == index)
    {
        m_display_interval_milliseconds = 10000;
    }
    m_macd_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);
    m_candles_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);
    m_volume_wrapper.setCandleTimeWidthMilliseconds(m_display_interval_milliseconds);
}

//====================================================================
void TRunnerAnalysisChartsDialog::changeYAxisDisplayOptions(int index)
{
    m_y_zoom_mode = index;
    setCurrentYAxisLimits();
    alignChartMargins();
}

//====================================================================
void TRunnerAnalysisChartsDialog::changeXAxisDisplayOptions(int index)
{
    m_x_zoom_mode = index;
    alignChartMargins();
}

//============================================================
void TRunnerAnalysisChartsDialog::reset()
{
    m_candles_wrapper.clear();
    m_volume_wrapper.clear();
    m_macd_wrapper.clear();    
    ui->cbChartRunnerSelect->clear();
    m_selected_runner_index = -1;
    m_selected_runner_id = -1;
    m_selected_runner_name = "NO RUNNER SELECTED";
    QString mlabel = "INVALID MARKET";
    m_m_selection_index_map.clear();
    if (m_market.valid())
    {
        const std::vector<std::shared_ptr<betfair::TRunner> >& runners = m_market.getRunners();
        int counter = 0;
        for (auto it = runners.begin(); it != runners.end(); ++it)
        {
            const std::int64_t sel_id = (*it)->getID();
            const QString selection_name = (*it)->getName();
            ui->cbChartRunnerSelect->addItem(selection_name);

            m_m_selection_index_map[counter++] = sel_id;
        }
        mlabel = m_market.getLabel();
        mlabel.append(" : ");
        mlabel.append(m_selected_runner_name);

    }
    ui->cbChartRunnerSelect->setCurrentIndex(m_selected_runner_index);
    setWindowTitle(m_selected_runner_name);
    alignChartMargins();
}

//============================================================
void TRunnerAnalysisChartsDialog::updateChart()
{
    if (m_market.valid())
    {
        std::shared_ptr<betfair::TRunner> runner = m_market.getSelectionByID(m_selected_runner_id);
        if (runner)
        {

            // TEST of new advanced candles for MACD AND CANDLES
            const std::vector<betfair::utils::TBetfairAdvancedCandle>& advdata = runner->getAdvCandleData();
            if (m_display_interval_milliseconds > 0)
            {
                auto transdata = betfair::utils::mergeCandlesByMinimumTimeInterval(advdata,m_display_interval_milliseconds);
                m_macd_wrapper.populateSeries(transdata,runner->getID());
                m_candles_wrapper.populateSeries(transdata,runner->getID());
                m_volume_wrapper.populateSeries(transdata,runner->getID());                
            }
            else
            {
                m_macd_wrapper.populateSeries(advdata,runner->getID());
                m_candles_wrapper.populateSeries(advdata,runner->getID());
                m_volume_wrapper.populateSeries(advdata,runner->getID());
            }

            QString match_label = "Total Matched : £";
            double runner_matched = runner->getTotalMatched();
            if (runner_matched > 1000.0)
            {
                match_label += QString::number(runner_matched/1000.0,'f',1) + "K";
            }
            else
            {
                match_label += QString::number(runner->getTotalMatched(),'f',0);
            }
            QString window_title = m_selected_runner_name + "  £" + QString::number(runner->getTotalMatched(),'f',0);
            double timestamp_offset = -1.0;
            setWindowTitle(window_title);
            if (1 == m_x_zoom_mode)
            {
                timestamp_offset = 31000.0;
            }
            else if (2 == m_x_zoom_mode)
            {
                timestamp_offset = 61000.0;
            }
            else if (3 == m_x_zoom_mode)
            {
                timestamp_offset = 121000.0;
            }
            else if (4 == m_x_zoom_mode)
            {
                timestamp_offset = 301000.0;
            }
            else
            {
                m_candles_wrapper.zoomToDefaultXRange(1000.0);
                m_volume_wrapper.zoomToDefaultXRange(1000.0);
                // NOTE that now MACD chart can be configured to MERGE candles, we should
                // set x range to match that of main candle chart                

                m_macd_wrapper.zoomToDefaultXRange(1000.0);
            }
            if (timestamp_offset > 0.0)
            {
                if (!advdata.empty())
                {
                    auto it = advdata.rbegin();
                    double start = it->end_timestamp - timestamp_offset;
                    double end = it->end_timestamp;
                    m_candles_wrapper.setXAxesDisplayRange(start,end);
                    m_volume_wrapper.setXAxesDisplayRange(start,end);
                    m_macd_wrapper.setXAxesDisplayRange(start,end);                    
                }
            }
            setCurrentYAxisLimits();
            alignChartMargins();
        }
        else
        {
           reset();
        }
    }
    else
    {
        reset();
    }
}

//============================================================
void TRunnerAnalysisChartsDialog::changeChartLimits()
{

}

//============================================================
void TRunnerAnalysisChartsDialog::alignChartMargins()
{
    QMargins init(5,2,5,1);
    m_volume_wrapper.setMargins(init);
    m_candles_wrapper.setMargins(init);
    m_macd_wrapper.setMargins(5);

    int w1 = m_volume_wrapper.getLabelWidth();
    int w2 = m_candles_wrapper.getLabelWidth();
    int w3 = m_macd_wrapper.getLabelWidth();

    QMargins mg1 = m_volume_wrapper.getMargins();
    QMargins mg2 = m_candles_wrapper.getMargins();
    QMargins mg3 = m_macd_wrapper.getMargins();

    int maxw = std::max(w1,std::max(w2,w3));
    if (maxw == w1)
    {
        int newmargin = mg1.left();
        mg2.setLeft(newmargin + (w1 - w2));
        mg3.setLeft(newmargin + (w1 - w3));
    }
    else if (maxw == w2)
    {
        int newmargin = mg2.left();
        mg1.setLeft(newmargin + (w2 - w1));
        mg3.setLeft(newmargin + (w2 - w3));
    }
    else
    {
        int newmargin = mg3.left();
        mg1.setLeft(newmargin + (w3 - w1));
        mg2.setLeft(newmargin + (w3 - w2));
    }
    m_volume_wrapper.setMargins(mg1);
    m_candles_wrapper.setMargins(mg2);
    m_macd_wrapper.setMargins(mg3);
}

//============================================================
void TRunnerAnalysisChartsDialog::setSelectedRunner(const std::int64_t& runner_id)
{
    // look up this ID in the selection map
    int new_index = -1;
    for (auto it = m_m_selection_index_map.begin(); it != m_m_selection_index_map.end(); ++it)
    {
        if (it->second == runner_id)
        {
            // This is it!
            new_index = it->first;
            break;
        }
    }
    if (new_index > static_cast<int>(m_market.getNumRunners()))
    {
        throw std::runtime_error("Invalid runner index!");
    }
    ui->cbChartRunnerSelect->setCurrentIndex(new_index);
    changeSelectedRunner(new_index);
}

//============================================================
void TRunnerAnalysisChartsDialog::changeSelectedRunner(int index)
{
    m_candles_wrapper.clear();
    m_volume_wrapper.clear();
    m_macd_wrapper.clear();    
    m_selected_runner_id = -1;
    m_selected_runner_index = -1;
    m_selected_runner_name = "NO RUNNER SELECTED";   
    QString mlabel = "INVALID MARKET";
    if (index >= 0 && m_market.valid())
    {
        m_selected_runner_index = index;
        m_selected_runner_id = m_m_selection_index_map[m_selected_runner_index];
        m_selected_runner_name = ui->cbChartRunnerSelect->currentText();
        mlabel = m_market.getLabel();
        mlabel.append(" : ");
        mlabel.append(m_selected_runner_name);

    }
    setWindowTitle(m_selected_runner_name);    
}

//=========================================================================================
