/*
    Copyright 2002-2013 CEA LIST

    This file is part of LIMA.

    LIMA is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIMA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/
/************************************************************************
 *
 * @file       NormalizeDateTime.cpp
 * @author     Besancon Romaric (romaric.besancon@cea.fr)
 * @date       Tue Jun 13 2006
 * copyright   Copyright (C) 2006-2012 by CEA LIST
 * 
 ***********************************************************************/

#include "NormalizeDateTime.h"
#include "NormalizationUtils.h"
#include "MicrosForNormalization.h"
#include "common/Data/strwstrtools.h"
#include "linguisticProcessing/core/Automaton/automatonCommon.h"
#include "linguisticProcessing/core/Automaton/constraintFunctionFactory.h"
#include "linguisticProcessing/core/LinguisticProcessors/LinguisticMetaData.h"
#include "linguisticProcessing/core/LinguisticResources/LinguisticResources.h"

#include <limits>
#ifdef WIN32
#undef min  
#undef max 
#endif

using namespace Lima::Common::MediaticData;
using namespace Lima::LinguisticProcessing::LinguisticAnalysisStructure;
using namespace Lima::LinguisticProcessing::Automaton;

// using namespace boost;
// using namespace boost::local_time;
// using namespace boost::posix_time;


using namespace std;

namespace Lima {
namespace LinguisticProcessing {
namespace SpecificEntities {
  
#define DATESTRING_FEATURE_NAME "date_string" // date as a string
#define DATE_FEATURE_NAME "date" // date as a QDate
#define DATE_BEGIN_FEATURE_NAME "date_begin" 
#define DATE_END_FEATURE_NAME "date_end" 
#define DAY_FEATURE_NAME "day"
#define MONTH_FEATURE_NAME "month"
#define YEAR_FEATURE_NAME "year"

#define LOCALTIME_FEATURE_NAME "local_time"
#define UTCTIME_FEATURE_NAME "utc_time"
#define TIMESTRING_FEATURE_NAME "time_string"

//**********************************************************************
// factories for actions defined in this file
Automaton::ConstraintFunctionFactory<NormalizeDate>
NormalizeDateFactory(NormalizeDateId);

Automaton::ConstraintFunctionFactory<NormalizeRelativeDate>
NormalizeRelativeDateFactory(NormalizeRelativeDateId);

Automaton::ConstraintFunctionFactory<NormalizeLocalTime>
NormalizeLocalTimeFactory(NormalizeLocalTimeId);

Automaton::ConstraintFunctionFactory<NormalizeUTCTime>
NormalizeUTCTimeFactory(NormalizeUTCTimeId);


ReferenceData::ReferenceData():
m_dateRefName("document"),
m_timeRefName("document"),
m_locRefName("document")
{}

ReferenceData::~ReferenceData() {}

void ReferenceData::parseComplement(const LimaString& complement) 
{
  if (! complement.isEmpty()) {
    //uint64_t i(0),prev(0); portage 32 64
    int i(0),prev(0);
    do {
      i=complement.indexOf(LimaChar(','),prev);
      parseReference(complement.mid(prev,i-prev));
      prev=i+1;
    } while (i!=-1);
  }
}

void ReferenceData::parseReference(const LimaString& str) 
{
  static const LimaString datePrefix=Common::Misc::utf8stdstring2limastring("date_");
  static const LimaString locPrefix=Common::Misc::utf8stdstring2limastring("loc_");
  static const LimaString timePrefix=Common::Misc::utf8stdstring2limastring("time_");

  if (str.indexOf(datePrefix)==0) {
    m_dateRefName=Common::Misc::limastring2utf8stdstring(str.mid(5));
  }
  else if (str.indexOf(locPrefix)==0) {
    m_locRefName=Common::Misc::limastring2utf8stdstring(str.mid(4));
  }
  else if (str.indexOf(timePrefix)==0) {
    m_timeRefName=Common::Misc::limastring2utf8stdstring(str.mid(5));
  }
}

bool ReferenceData::
getReferenceDate(const AnalysisContent& analysis,
                 QDate& date) const
{
  // get posix time from current text metadata
  const AnalysisData* data=analysis.getData("LinguisticMetaData");
  if (data == 0) {
    SELOGINIT;
    LERROR << "missing data 'LinguisticMetaData'" << LENDL;
    return false;
  }
  const LinguisticMetaData* metadata=dynamic_cast<const LinguisticMetaData*>(data);

  date=metadata->getDate(m_dateRefName);
  SELOGINIT;
  LDEBUG << "m_dateRefName =" << m_dateRefName <<" , date = " << date.toString() << LENDL;
  if (!date.isValid()) {
    //try backoff on document date
    SELOGINIT;
    LWARN << "no reference date '"<< m_dateRefName << "'" << LENDL;
    
    date=metadata->getDate("document");
    if (!date.isValid()) {
      SELOGINIT;
      LWARN << "no reference date 'document'" << LENDL;
      return false;
    }
    return true;
  }
  return true;
}

bool ReferenceData::
getReferenceTime(const AnalysisContent& analysis,
                 QTime& time) const
{
  // get posix time from current text metadata
  const AnalysisData* data=analysis.getData("LinguisticMetaData");
  if (data == 0) {
    SELOGINIT;
    LERROR << "missing data 'LinguisticMetaData'" << LENDL;
    return false;
  }
  const LinguisticMetaData* metadata=dynamic_cast<const LinguisticMetaData*>(data);

  time=metadata->getTime(m_timeRefName);
  return (time.isValid());
}

bool ReferenceData::
getReferenceLocation(const AnalysisContent& analysis,
                     std::string& location) const
{
  // get posix time from current text metadata
  const AnalysisData* data=analysis.getData("LinguisticMetaData");
  if (data == 0) {
    SELOGINIT;
    LERROR << "missing data 'LinguisticMetaData'" << LENDL;
    return false;
  }
  const LinguisticMetaData* metadata=dynamic_cast<const LinguisticMetaData*>(data);

  location=metadata->getLocation(m_locRefName);
  return (! location.empty());
}

//**********************************************************************
NormalizeDate::
NormalizeDate(MediaId language,
              const LimaString& complement):
Automaton::ConstraintFunction(language,complement),
m_language(language),
m_resources(0),
m_referenceData(),
m_microsForMonth(0),
m_microsForDays(0),
m_microAccessor(0),
m_isInterval(false)
{
  m_referenceData.parseComplement(complement);

  m_microAccessor=&(static_cast<const Common::MediaticData::LanguageData&>(Common::MediaticData::MediaticData::single().mediaData(language)).getPropertyCodeManager().getPropertyAccessor("MICRO"));

  if (language != UNDEFLANG) {
    try {
      AbstractResource* res = LinguisticResources::single().getResource(language,"microsForDateTimeNormalization");
      MicrosForNormalization* micros=static_cast<MicrosForNormalization*>(res);
      m_microsForMonth=micros->getMicros("MonthMicros");
      m_microsForDays=micros->getMicros("DayMicros");
    }
    catch (std::exception& e) {
      SELOGINIT;
      LWARN << "Exception caught: " << e.what() << LENDL;
      LWARN << "-> micros for person name normalization are not initialized" << LENDL;
    }
  }

  try {
    AbstractResource* res=LinguisticResources::changeable().
      getResource(language,"DateTimeNormalization");
    if (res==0) {
      SELOGINIT;
      LWARN << "no resource 'DateTimeNormalization'" << LENDL;
    }
    else {
      m_resources=static_cast<const NormalizeDateTimeResources*>(res);
    }
  }
  catch (std::exception& e) {
    // resources are not loaded when compiling the rules
      SELOGINIT;
      LWARN << "Exception caught: " << e.what() << LENDL;
  }

  static const LimaString intervalString=Common::Misc::utf8stdstring2limastring("isInterval");
  if (complement.indexOf(intervalString)!=-1) {
    m_isInterval=true;
  }
}

void NormalizeDate::
updateCurrentDate(AnalysisContent& analysis,
                  const QDate& currentDate) const
{
  // get metadata to update
  AnalysisData* data=analysis.getData("LinguisticMetaData");
  if (data == 0) {
    SELOGINIT;
    LERROR << "missing data 'LinguisticMetaData'" << LENDL;
    return;
  }
  LinguisticMetaData* metadata=dynamic_cast<LinguisticMetaData*>(data);

  metadata->setDate("current",currentDate);
}

bool NormalizeDate::
operator()(RecognizerMatch& m,
           AnalysisContent& analysis) const 
{
  // assume all information for normalization is in recognized 
  // expression: do not use external information

  SELOGINIT;
  
  unsigned short day(0);
  unsigned short day_end(0);
  unsigned short month(0);
  unsigned short month_end(0);
  unsigned short year(0);

  for (RecognizerMatch::const_iterator i(m.begin()); i!=m.end(); i++) {
    if (! (*i).isKept()) {
      continue;
    }
    Token* t = m.getToken(i);
    MorphoSyntacticData* data = m.getData(i);
    if (testMicroCategory(m_microsForMonth,m_microAccessor,data)) {
      if (isInteger(t)) {
        month=LimaStringToInt(t->stringForm());
      }
      else if (m_resources) {
        unsigned short monthNum=m_resources->getMonthNumber(t->stringForm());
        if (monthNum==NormalizeDateTimeResources::no_month) {
          // failed to recognize month => no normalization
          LDEBUG << "NormalizeDate: '" << Common::Misc::limastring2utf8stdstring(t->stringForm())
          << "' not recognized as month\n" << LENDL;
          m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
        }
        else {
          if (month!=0 && m_isInterval) {
            month_end=monthNum;
          }
          else {
            month=monthNum;
          }
        }
      }
    }
    else {
      const TStatus& status=t->status();
      if (status.getNumeric()==T_FRACTION) {
        uint64_t pos(t->stringForm().indexOf(LimaChar('/')));
        uint64_t val1=t->stringForm().left(pos).toUInt();
        uint64_t val2=t->stringForm().mid(pos+1).toUInt();
        if (val1 > 31) {
          //assume year
          year=val1;
          //assume next is month
          if (val2 <= 12) {
            month=val2;
          }
          else { // should not happen => it may not be a date
          }
        }
        // otherwhise, suppose day before month, but test it
        else if (val2 > 12) {
          day=val2;
          month=val1;
        }
        else {
          day=val1;
          month=val2;
        }
      }
      else if (isInteger(t)) {
        uint64_t value=LimaStringToInt(t->stringForm());
        if (value < 31) {
          if (day==0) { // suppose day before month
            day = value;
          }
          else if (m_isInterval && day_end==0) {
            day_end=value;
          }
          else if (month==0) {
            if (value > 12) { // swap : month cant be > 12
              month = day;
              day = value;
            }
            else { // assume month
              month = value;
            }
          }
          else { // month and day are assigned -> assume year
            year = value;
          }
        }
        else {
          if (value > 1000 && value < 3000) {
            year = value;
          }
          else if (month!=0) {
            // can be a year on two digits -> year assumed if
            // day and month are already assigned
            year = value;
          }
        }
      }
    }
  }

  //ad hoc correction of year on two digits
  if (year!=0 && year<99) {
    if (year < 10) {
      year+=2000;
    }
    else {
      year+=1900;
    }
  }
  
  QDate newCurrentDate;
  try { // catch date conversion exceptions
    if (day==0 && month==0 && year==0) {
      //const FsaStringsPool& sp=Common::MediaticData::MediaticData::single().stringsPool(m_language);
      LDEBUG << "NormalizeDate: no day, month or year identified in " 
             << Common::Misc::limastring2utf8stdstring(m.getString())
             << LENDL;
      m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
    }
    else {
      if (day==0) {
        if (month==0) {
          // only year : do not set interval of dates from first to last day of year
          // set only year : cast to int in features
          m.features().addFeature(YEAR_FEATURE_NAME,static_cast<int>(year));
        }
        else {
          // set interval
          QDate firstDayOfMonth(year,month,1);
          m.features().addFeature(DATE_BEGIN_FEATURE_NAME,firstDayOfMonth);
          if (month_end==0) {
            m.features().addFeature(DATE_END_FEATURE_NAME,firstDayOfMonth.daysInMonth());
          }
          else {
            m.features().addFeature(DATE_END_FEATURE_NAME,QDate(year,month_end,1).daysInMonth());
          }
        }
      }
      else {
        if (month==0) {
          if (year==0) {
            //day only => take month and year from reference
            QDate referenceDate;
            if (! m_referenceData.getReferenceDate(analysis,referenceDate)) {
              m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
            }
            else {
              int refYear=referenceDate.year();
              int refMonth=referenceDate.month();
              newCurrentDate=QDate(refYear,refMonth,day);
              m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
            }
          }
          else {
            // day and year => should not happen: failed to normalize: set only string
            LDEBUG << "NormalizeDate: only day and year in " 
                   << Common::Misc::limastring2utf8stdstring(m.getString())
                   << LENDL;
            m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
          }
        }
        else {
          if (year==0) {
            // get year from reference
            QDate referenceDate;
            if (! m_referenceData.getReferenceDate(analysis,referenceDate)) {
              m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
            }
            else {
              int refYear=referenceDate.year();
              newCurrentDate=QDate(refYear,month,day);
              if (day_end==0) {
                m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
              }
              else {
                m.features().addFeature(DATE_BEGIN_FEATURE_NAME,QDate(refYear,month,day));
                m.features().addFeature(DATE_END_FEATURE_NAME,QDate(refYear,month,day_end));
              }
            }
          }
          else {
            // complete !!
            if (day_end==0) {
              newCurrentDate=QDate(year,month,day);
              m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
            }
            else {
              m.features().addFeature(DATE_BEGIN_FEATURE_NAME,QDate(year,month,day));
              m.features().addFeature(DATE_END_FEATURE_NAME,QDate(year,month,day_end));
            }
          }
        }
      }
    }
  }
  catch (std::exception& e) {
    SELOGINIT;
    LWARN << "Error trying to normalize date " 
          << Common::Misc::limastring2utf8stdstring(m.getString())
          << ":" << e.what()
          << LENDL;
    m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
    return true;
  }

  if (newCurrentDate.isValid()) {
    updateCurrentDate(analysis,newCurrentDate);
  }
  return true;
}

//**********************************************************************
NormalizeRelativeDate::
NormalizeRelativeDate(MediaId language,
                      const LimaString& complement):
NormalizeDate(language,complement),
m_getNext(false), // default is previous date
m_diff(0) 
{
  static const LimaString nextString=Common::Misc::utf8stdstring2limastring("next_");
  if (complement.indexOf(nextString)!=-1) {
    m_getNext=true;
  }
  static const LimaString diffString=Common::Misc::utf8stdstring2limastring("diff_");
  static const LimaString days=Common::Misc::utf8stdstring2limastring("d");
  //static const LimaString months=Common::Misc::utf8stdstring2limastring("m");
  //static const LimaString years=Common::Misc::utf8stdstring2limastring("y");
  //uint64_t i=complement.find(diffString); portage 32 64
  int i=complement.indexOf(diffString);
  if (i!=-1) {
    i+=5; // at end of diff_
    uint64_t end=complement.indexOf(days,i);
    LimaString str = complement.mid(i,end-i);
    uint64_t days=atoi(Common::Misc::limastring2utf8stdstring(str).c_str());
    m_diff=days;
  }
}

bool NormalizeRelativeDate::
operator()(RecognizerMatch& m,
           AnalysisContent& analysis) const 
{
  // use a reference to normalize the relative date

  if (m_resources == 0) {
    // no resources: cannot normalize date
    SELOGINIT;
    LDEBUG << "NormalizeRelativeDate: no resources" 
           << LENDL;
    m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
    return true;
  }

  QDate referenceDate;
  
  if (! m_referenceData.getReferenceDate(analysis,referenceDate)) {
    m.features().addFeature(DATESTRING_FEATURE_NAME,m.getString());
    return true;
  }
  
  // get day of the week
  QDate newCurrentDate;
  unsigned short dayOfMonth(0); 
  for (RecognizerMatch::const_iterator i(m.begin()); i!=m.end(); i++) {
    if (! (*i).isKept()) {
      continue;
    }
    Token* t=m.getToken(i);
    LimaString str=t->stringForm();

    // if integer, assume it's a day number (10 mai prochain)
    if (isInteger(t)) {
      dayOfMonth=str.toUShort();
      if (dayOfMonth>31) { dayOfMonth=0; }
      continue;
    }

    // test if it is a day name
    unsigned short day=m_resources->getDayNumber(str);
    if (day!=NormalizeDateTimeResources::no_day) {
      if (day == referenceDate.dayOfWeek()) {
        // same day of week as reference: assume it's the same date
        newCurrentDate=referenceDate;
        m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
      }
      else if (m_getNext) {
        // get next day according to reference
        newCurrentDate=referenceDate.addDays(1);
//         first_day_of_the_week_after nextDay(day);
//         newCurrentDate=nextDay.get_date(referenceDate);
        m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
      }
      else {
        // get previous day according to reference
        newCurrentDate=referenceDate.addDays(-1);
//         first_day_of_the_week_before prevDay(day);
//         newCurrentDate=prevDay.get_date(referenceDate);
        m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
      }
      continue;
    }

    // test if it is a month name
    unsigned short month=m_resources->getMonthNumber(str);
    if (month!=NormalizeDateTimeResources::no_month) {
      unsigned short year=referenceDate.year();
      int refmonth=referenceDate.month();
      // possible change of year
      if (m_getNext) {
        if (refmonth>month) {
          year++;
        }
      }
      else {
        if (refmonth<month) {
          year--;
        }
      }
      // if a day is specified
      if (dayOfMonth!=0) {
        newCurrentDate=QDate(year,month,dayOfMonth);
        m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
      }
      else {
        // set an interval value
        QDate firstDayOfMonth(year,month,1);
        m.features().addFeature(DATE_BEGIN_FEATURE_NAME,firstDayOfMonth);
        m.features().addFeature(DATE_END_FEATURE_NAME,firstDayOfMonth.addMonths(1).addDays(-1));
      }
      continue;
    }

    // other cases: maybe a diff (hier,ajourd'hui...)
    if (m_diff != 0) {
      newCurrentDate=referenceDate.addDays(m_diff);
      m.features().addFeature(DATE_FEATURE_NAME,newCurrentDate);
    }
  }
  if (newCurrentDate.isValid()) {
    updateCurrentDate(analysis,newCurrentDate);
  }
  return true;
}

//**********************************************************************
NormalizeTime::
NormalizeTime(MediaId language,
              const LimaString& complement):
Automaton::ConstraintFunction(language,complement),
m_language(language),
m_resources(0),
m_referenceData()
{
  // default reference date and location are the one indicated for 
  // the document, if set...

  // parse complement to find reference location and reference date,
  // if indicated
  m_referenceData.parseComplement(complement);

  try {
    AbstractResource* res=LinguisticResources::changeable().
      getResource(language,"DateTimeNormalization");
    if (res==0) {
      SELOGINIT;
      LWARN << "no resource 'DateTimeNormalization'" << LENDL;
    }
    else {
      m_resources=static_cast<const NormalizeDateTimeResources*>(res);
    }
  }
  catch (std::exception& e) {
    // resources are not loaded when compiling the rules
  }
}

QTime NormalizeTime::
getTimeDuration(const RecognizerMatch& m) const
{
  SELOGINIT;
  QTime timeDuration;
  if (m.size()==1) {
    std::string timeString=Common::Misc::
      limastring2utf8stdstring(m.getToken(m.begin())->stringForm());
    //uint64_t i=timeString.find(':'); portage 32 64
    string::size_type i=timeString.find(':');
    if (i!=string::npos) {
      try {
        // has at least one ":" sep -> guess it has form hh:mm or hh:mm:ss, 
        // try use direct construction from string
        timeDuration=QTime::fromString(QString::fromUtf8(timeString.c_str()));
      }
      catch (std::exception& e) {
        LWARN << "Error getting time duration from string '"
               << timeString << "'" << LENDL;
        return timeDuration;
      }
    }
    else {
      i=timeString.find_first_of("hH");
      if (i!=string::npos) {
        uint64_t h=atoi(string(timeString,0,i).c_str());
        uint64_t m=atoi(string(timeString,i+1).c_str());
        timeDuration=QTime(h,m);
      }
    }
  }
  else {
    // several tokens: 11 h 57
    uint64_t hou(0),min(0),sec(0);
    bool isSetH(false),isSetM(false),isSetS(false);
    for (RecognizerMatch::const_iterator i(m.begin()); i!=m.end(); i++) {
      if (! (*i).isKept()) {
        continue;
      }
      Token* t = m.getToken(i);
      const TStatus& status=t->status();
      if (status.getNumeric()==T_INTEGER) {
        int val=LimaStringToInt(t->stringForm());
        if (! isSetH) { hou=val; isSetH=true; }
        else if (! isSetM) { min=val; isSetM=true; }
        else if (! isSetS) { sec=val; isSetS=true; }
      }
      else { // not a number
        static const LimaString openPar=Common::Misc::utf8stdstring2limastring("(");
        if (t->stringForm()==openPar) {
          // probably format xx h yy (aa h bb GMT)
          // -> reinit hour decoding
          hou=0; isSetH=false; 
          min=0; isSetM=false;
          sec=0; isSetS=false;
        }
      }
    }
    timeDuration=QTime(hou,min,sec);
  }
  return timeDuration;
}


// normalizeTime helper functions
QTime NormalizeTime::
getUTCTime(const QDate& d,
           const QTime& duration/*,
           const std::string& location,
           const boost::local_time::tz_database& tz_db*/) const
{
  return QDateTime(d,duration).toUTC().time();
//   time_zone_ptr zone;
//   try {
//     zone=tz_db.time_zone_from_region(location);
//   }
//   catch (std::exception& e) {
//     cerr << "Error getting timezone:" << e.what() << endl;
//     return QTime();
//   }
//   if (zone == 0 ) {
//     cerr << "Error getting timezone for '" << location <<"'"<< endl;
//     return QTime();
//   }
// 
//   if (zone->has_dst()) {
//     ptime ptimeLocal(d,duration);
//     if (ptimeLocal > zone->dst_local_start_time(d.year()) &&
//         ptimeLocal < zone->dst_local_end_time(d.year())) {
//       return local_date_time(d, duration, zone, true).utc_time();
//     }
//     else {
//       return local_date_time(d, duration, zone, false).utc_time();
//     }
//   }
//   return local_date_time(d, duration, zone, false).utc_time();
}

NormalizeLocalTime::
NormalizeLocalTime(MediaId language,
                   const LimaString& complement):
NormalizeTime(language,complement)
{
}

bool NormalizeLocalTime::
operator()(RecognizerMatch& m,
           AnalysisContent& analysis) const 
{
  // normalize hour to UTC => needs to know the location
  // use 'DocumentLocation' if exists
  // otherwise, use adaptor from locale

  // get reference dates and location
  QDate referenceDate;
  std::string referenceLocation;
  bool hasReferenceLocation(true);
  
  if (! getReferenceData().getReferenceDate(analysis,referenceDate)) {
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
    return true;
  }                                                               

  if (! getReferenceData().getReferenceLocation(analysis,referenceLocation)) {
    SELOGINIT;
    LWARN << "no reference location '"<< getReferenceData().getLocRefName()
           << "' found" << LENDL;
    hasReferenceLocation=false;
  }                                                               

  // parse recognized expression to get duration
  QTime timeDuration=getTimeDuration(m);
  if (!timeDuration.isValid()) {
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
    return true;
  }

  try {
    QTime local(QDateTime(referenceDate,timeDuration).time());
   
    m.features().addFeature(LOCALTIME_FEATURE_NAME,local);

    if (! hasReferenceLocation) {
      return true;
    }
    
    if (hasResources() && getResources()->hasTimezoneDatabase()) {
      QTime utc=getUTCTime(referenceDate,timeDuration/*,
                                       referenceLocation,
                                       getResources()->getTimezoneDatabase()*/);
      if (!utc.isValid()) {
        SELOGINIT;
        LWARN << "failed to normalize time: " << LENDL;
        m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
      }
      else {
        m.features().addFeature(UTCTIME_FEATURE_NAME,utc);
      }
    }
  }
  catch (std::exception& e) {
    SELOGINIT;
    LWARN << "exception caught in time normalization of '"
          << Common::Misc::limastring2utf8stdstring(m.getString()) 
          << "': " << e.what() << LENDL;
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
  }
  return true;
}

//**********************************************************************
NormalizeUTCTime::
NormalizeUTCTime(MediaId language,
                 const LimaString& complement):
NormalizeTime(language,complement)
{
}

bool NormalizeUTCTime::
operator()(RecognizerMatch& m,
           AnalysisContent& analysis) const 
{
  // do not use a reference location, time is supposed to be UTC already

  // get reference date
  QDate referenceDate;
  if (! getReferenceData().getReferenceDate(analysis,referenceDate)) {
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
    return true;
  }
  
  QTime timeDuration=getTimeDuration(m);
  if (!timeDuration.isValid()) {
    SELOGINIT;
    LWARN << "cannot compute time duration for '"
          << Common::Misc::limastring2utf8stdstring(m.getString())
          << "'" << LENDL;
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
    return true;
  }
    
  try {
    QTime utcTime(QDateTime(referenceDate,timeDuration).toUTC().time());
    m.features().addFeature(UTCTIME_FEATURE_NAME,utcTime);
  }
  catch (std::exception& e) {
    SELOGINIT;
    LWARN << "exception caught in time normalization of '"
          << Common::Misc::limastring2utf8stdstring(m.getString())
          << "': " << e.what() << LENDL;
    m.features().addFeature(TIMESTRING_FEATURE_NAME,m.getString());
  }
  
  return true;
}

} // end namespace
} // end namespace
} // end namespace
