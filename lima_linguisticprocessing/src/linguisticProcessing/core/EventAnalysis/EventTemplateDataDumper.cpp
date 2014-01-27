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
 * @file       EventTemplateDataDumper.cpp
 * @author     Romaric Besancon (romaric.besancon@cea.fr)
 * @date       Mon Sep 19 2011
 * copyright   Copyright (C) 2011 by CEA - LIST
 * 
 ***********************************************************************/

#include "EventTemplateDataDumper.h"
#include "common/time/traceUtils.h"
#include "common/Data/strwstrtools.h"
#include "common/AbstractFactoryPattern/SimpleFactory.h"
#include "common/MediaticData/mediaticData.h"
#include "linguisticProcessing/common/annotationGraph/AnnotationData.h"
#include "linguisticProcessing/core/Automaton/SpecificEntityAnnotation.h"
#include "Events.h"
#include "Entity.h"

using namespace std;
using namespace Lima::Common;
using namespace Lima::Common::AnnotationGraphs;
using namespace Lima::LinguisticProcessing::SpecificEntities;
using namespace boost;

namespace Lima {
namespace LinguisticProcessing {
namespace EventAnalysis {

//***********************************************************************
SimpleFactory<MediaProcessUnit,EventTemplateDataDumper> 
EventTemplateDataDumperFactory(EVENTTEMPLATEDATADUMPER_CLASSID);

EventTemplateDataDumper::EventTemplateDataDumper() : 
AbstractTextualAnalysisDumper(),
m_eventData("EventTemplateData")
{
}

EventTemplateDataDumper::~EventTemplateDataDumper() {
}

//***********************************************************************
void EventTemplateDataDumper::
init(Common::XMLConfigurationFiles::GroupConfigurationStructure& unitConfiguration,
     Manager* manager)
  
{
  LOGINIT("LP::EventAnalysis");
  LDEBUG << "EventTemplateDataDumper::init" << LENDL;

  AbstractTextualAnalysisDumper::init(unitConfiguration,manager);

  try {
    m_eventData=unitConfiguration.getParamsValueAtKey("eventTemplateData");
  }
  catch (Common::XMLConfigurationFiles::NoSuchParam& ) {
    LDEBUG << "EventTemplateDataDumper: no parameter 'eventTemplateData', use default ('"<<m_eventData << "')" << LENDL;
    // not an error, keep default
  }
}
  
LimaStatusCode EventTemplateDataDumper::process(AnalysisContent& analysis) const
{
  LOGINIT("LP::EventAnalysis");
  LDEBUG << "EventTemplateDataDumper::process" << LENDL;
  TimeUtils::updateCurrentTime();

  // initialize output
  DumperStream* dstream=AbstractTextualAnalysisDumper::initialize(analysis);
  ostream& out=dstream->out();

  const AnnotationData* annotationData = static_cast< AnnotationData* >(analysis.getData("AnnotationData"));
  if (annotationData==0)
  {
    LERROR << "no annotation graph available !" << LENDL;
    return MISSING_DATA;
  }
  
  if (! m_eventData.empty()) {
    const AnalysisData* data =analysis.getData(m_eventData);
    if (data!=0) {
      // see if the data is of type Events
      const EventTemplateData* eventData=dynamic_cast<const EventTemplateData*>(data);
      if (eventData==0) {
        LOGINIT("LP::EventAnalysis");
        LERROR << "data '" << m_eventData << "' is neither of type EventData nor Events" << LENDL;
        return MISSING_DATA;
      }
      else {
        Events *events=eventData->convertToEvents(annotationData);
        events->write(out);
      }
    }
    else {
      LOGINIT("LP::EventAnalysis");
      LERROR << "no data of name " << m_eventData << LENDL;
    }
  }
  
  delete dstream;
  TimeUtils::logElapsedTime("EventTemplateDataDumper");
  return SUCCESS_ID;
}

} // end namespace
} // end namespace
} // end namespace
