#ifndef RUNNERMETAINFO_H
#define RUNNERMETAINFO_H

#include <QString>
#include <cstdint>


namespace betfair {

// This is very dirty to purists but this class is just a simple collection of some of the
// more useful metadata items. Yes public members are filthy, but why complicate when not needed

class TRunnerMetaInfo {
public:
    TRunnerMetaInfo();
    ~TRunnerMetaInfo();

    std::int64_t id;
    QString name;
    QString age;
    QString weight_units;
    QString weight_value;
    QString f_sp_num;
    QString f_sp_denom;
    QString off_rating;
    QString sex_type;
    QString days_since_last_run;
    QString wearing_type;
    QString adj_rating;
    QString form_string;
    QString stall_draw;
    QString jockey;
    QString trainer;
    QString jockey_claim;
    QString colours_filename;
    int sort_priority;
};


} // end of namespace betfair

#endif // RUNNERMETAINFO_H
