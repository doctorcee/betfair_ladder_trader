#include "runnermetainfo.h"


namespace betfair {


//=================================================================
TRunnerMetaInfo::TRunnerMetaInfo()
    : id(0),
      name(""),
      age(""),
      weight_units(""),
      weight_value(""),
      f_sp_num(""),
      f_sp_denom(""),
      off_rating(""),
      sex_type(""),
      days_since_last_run(""),
      wearing_type(""),
      adj_rating(""),
      form_string(""),
      stall_draw(""),
      jockey(""),
      trainer(""),
      jockey_claim(""),
      colours_filename(""),
      sort_priority(0)
{

}

//=================================================================
TRunnerMetaInfo::~TRunnerMetaInfo()
{

}

} // end of namespace betfair
