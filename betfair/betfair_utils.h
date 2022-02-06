#ifndef BETFAIR_UTILS_H
#define BETFAIR_UTILS_H


#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <QString>
#include <QColor>

class TMarketSnapshot;


namespace betfair {
namespace utils {

    class TBetfairAdvancedCandle;

    static const QColor pbar1(QColor(0,70,250));
    static const QColor pbar2(QColor(50, 155, 255));
    static const QColor pbar3(QColor(153, 235, 255));

    static const QColor pbar1dark(QColor(0x88,0x00,0x88));
    static const QColor pbar2dark(QColor(0xbb,0x50,0xff));
    static const QColor pbar3dark(QColor(0x33, 0x33, 0xcc));

    static const QColor green1(QColor(0x33,0xcc,0x33));
    static const QColor greenOne(QColor(0x99,0xff,0x33));
    static const QColor greenTwo(QColor(0x80,0xff,0x00));
    static const QColor backOne(QColor(0x66,0xe0,0xff));
    static const QColor backTwo(QColor(0x99,0xeb,0xff));
    static const QColor backThree(QColor(0xb3,0xf0,0xff));
    static const QColor layOne(QColor(0xff, 0x99, 0xcc));
    static const QColor layTwo(QColor(0xff,0xb3,0xd9));
    static const QColor layThree(QColor(0xff,0xcc,0xe6));

    static const QColor layLadderFlash(QColor(0xee,0x80,0xaa));
    static const QColor backLadderFlash(QColor(0x33,0xcc,0xee));

    static const QColor layVWAPXOTransparent(QColor(0x80,0xff,0x00,0xaa));
    static const QColor backVWAPXOTransparent(QColor(0xee,0x1a,0x1a,0xaa));

    static const QColor red1(QColor(0xff,0x1a,0x1a));
    static const QColor redTwo(QColor(0xff,0x26,0x00));
    static const QColor darkgrey1(QColor(0x22,0x22,0x33));
    static const QColor gold1(QColor(0xff,0xc0,0x00));
    static const QColor midgrey1(QColor(0x77,0x77,0x77));
    static const QColor veryLightGrey1(QColor(0xee,0xee,0xee));
    static const QColor susp(QColor(0xff,0x92,0x33));
    static const double min_betting_stake = 1.0;

    static const QString event_type_horse_racing("7");
    static const QString event_type_dog_racing("4339");

    struct betInstruction {
        std::int64_t selection;
        QString market_id;
        QString strategy_string;
        bool b_lay_type;
        bool b_persistence_flag;
        double odds;
        double stake;

        betInstruction()
            : selection(0),
              market_id(""),
              strategy_string(""),
              b_lay_type(false),
              b_persistence_flag(false),
              odds(0.0),
              stake(0.0) {}

        betInstruction(const std::int64_t& sel,
                       const QString& mkid,
                       const QString& strat_id,
                       bool lay,
                       const double& price,
                       const double& dstake,
                       bool b_pers_flag = false)
            : selection(sel),
              market_id(mkid),
              strategy_string(strat_id),
              b_lay_type(lay),
              b_persistence_flag(b_pers_flag),
              odds(price),
              stake(dstake) {}
    };

    struct betInstructionBasic {
        std::int64_t selection;
        QString market_id;
        bool b_lay_type;
        double odds;
        double stake;
    };

    struct betfairInterval {
        std::map<double,double> m_m_matched_price_volume;
        double lpm;
        std::int64_t selection_id;
        std::uint16_t count_id;
        QString timestamp;
    };

    bool calculateTradedIntervalData(const std::map<double,double>& a_hist,
                                     const std::map<double,double>& b_hist,
                                     std::map<double,double>& interval_data,
                                     QString& error_string);

    TBetfairAdvancedCandle generateAdvancedCandle(const std::int64_t& selection_id,
                                                  const std::shared_ptr<TMarketSnapshot>& a,
                                                  const std::shared_ptr<TMarketSnapshot>& b);

    TBetfairAdvancedCandle createMergedCandle(const std::vector<TBetfairAdvancedCandle>& data);
    std::vector<TBetfairAdvancedCandle> mergeCandles(const std::vector<TBetfairAdvancedCandle>& data,
                                                     const std::uint16_t& width);
    std::vector<TBetfairAdvancedCandle> mergeCandlesByMinimumTimeInterval(const std::vector<TBetfairAdvancedCandle>& data,
                                                                          const double& millisecond_interval);


    void calculateMACD(const std::uint32_t& short_width,
                       const std::uint32_t& long_width,
                       const std::vector<TBetfairAdvancedCandle>& candle_data,
                       std::vector<std::pair<double,double> >& macd_vector);
    void calculateEMA(std::uint32_t width,
                      const std::vector<TBetfairAdvancedCandle>& candle_data,
                      std::vector<std::pair<double,double> >& ema_vector);

    void calculateEMA(std::uint32_t width,
                      const std::vector<std::pair<double,double> >& t_y_data,
                      std::vector<std::pair<double,double> >& ema_vector);

    std::map<QString,std::uint32_t> generateOffsetMap(const std::map<std::uint32_t,QString>& mp);
    std::map<std::uint32_t,QString> generateOddsMap();
    std::vector<QString> generateOddsTickVector();
    std::map<double,std::uint32_t> generatePriceDoubleTickMap();


    // JSON generation for common requests
    QString generateGenericLMCatFilter(const std::vector<QString>& v_countries,
                                       const std::vector<QString>& v_market_types,
                                       const QString& evtypeid,
                                       int market_count_limit,
                                       bool list_orders = false,
                                       bool today_only = false);

    void logMessage(const QString& filename, const QString& msg);

} // end of namespace utils
} // end of namespace betfair

#endif // BETFAIR_UTILS_H
