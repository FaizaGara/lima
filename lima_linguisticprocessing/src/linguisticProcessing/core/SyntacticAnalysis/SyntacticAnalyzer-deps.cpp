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
/**
  *
  * @file        SyntacticAnalyzer-deps.cpp
  * @author      Gael de Chalendar (Gael.de-Chalendar@cea.fr) 

  *              Copyright (c) 2003 by CEA
  * @date        Created on Aug, 31 2004
  *
  */

#include "SyntacticAnalyzer-deps.h"

#include "PhoenixGraphHomoDepsVisitor.h"
#include "common/AbstractFactoryPattern/SimpleFactory.h"
#include "linguisticProcessing/core/LinguisticAnalysisStructure/LinguisticGraph.h"
#include "linguisticProcessing/core/LinguisticAnalysisStructure/ChainIdStruct.h"
#include "linguisticProcessing/core/LinguisticResources/LinguisticResources.h"
#include "linguisticProcessing/core/LinguisticAnalysisStructure/AnalysisGraph.h"
#include "linguisticProcessing/core/TextSegmentation/SegmentationData.h"
#include "linguisticProcessing/core/Automaton/recognizerData.h"
#include "common/time/timeUtilsController.h"

#include <boost/graph/reverse_graph.hpp>

using namespace std;
using namespace boost;
using namespace Lima::Common::MediaticData;
using namespace Lima::LinguisticProcessing::Automaton;
using namespace Lima::LinguisticProcessing::LinguisticAnalysisStructure;
using namespace Lima::LinguisticProcessing::ApplyRecognizer;

namespace Lima
{
namespace LinguisticProcessing
{
namespace SyntacticAnalysis
{

SimpleFactory<MediaProcessUnit,SyntacticAnalyzerDeps> syntacticAnalyzerDepsFactory(SYNTACTICANALYZERDEPS_CLASSID);

SyntacticAnalyzerDeps::SyntacticAnalyzerDeps() :
    m_language(),
    m_recognizers(),
    m_actions(),
    m_applySameRuleWhileSuccess(false)
{}

void SyntacticAnalyzerDeps::init(
  Common::XMLConfigurationFiles::GroupConfigurationStructure& unitConfiguration,
  Manager* manager)

{
  SAPLOGINIT;
  m_language=manager->getInitializationParameters().media;
  try
  {
    m_actions = unitConfiguration.getListsValueAtKey("actions");
  }
  catch (Common::XMLConfigurationFiles::NoSuchParam& )
  {
    LERROR << "no parameter 'actions' in SyntacticAnalyzerDeps group for language " << (int) m_language << LENDL;
    throw InvalidConfiguration();
  }
  std::deque< std::string >::const_iterator actionsit, actionsit_end;
  actionsit = m_actions.begin(); actionsit_end = m_actions.end();
  for (; actionsit != actionsit_end; actionsit++)
  {
      std::string action = *actionsit;
      if ( (action != "setl2r") &&  (action != "setr2l") )
      {
          m_recognizers[action] = static_cast<Automaton::Recognizer*>(LinguisticResources::single().getResource(m_language,action));
      }
      else
      {
        LWARN << "SyntacticAnalyzerDeps actions setl2r and setr2l are deprecated" << LENDL;
      }
}

  try {
    std::string val=unitConfiguration.getParamsValueAtKey("applySameRuleWhileSuccess");
    if (val == "true" || val == "yes") {
      m_applySameRuleWhileSuccess=true;
    }
    else {
      m_applySameRuleWhileSuccess=false;
    }
  }
  catch (Common::XMLConfigurationFiles::NoSuchParam& ) {} // keep default value


}

LimaStatusCode SyntacticAnalyzerDeps::process(
  AnalysisContent& analysis) const
{
  Lima::TimeUtilsController timer("SyntacticAnalysis");
  SAPLOGINIT;
  LINFO << "start syntactic analysis - dependence relations search" << LENDL;

  AnalysisGraph* anagraph=static_cast<AnalysisGraph*>(analysis.getData("PosGraph"));
  if (anagraph==0)
  {
    LERROR << "no AnalysisGraph ! abort" << LENDL;
    return MISSING_DATA;
  }
  SegmentationData* sb=static_cast<SegmentationData*>(analysis.getData("SentenceBoundaries"));
  if (sb==0)
  {
    LERROR << "no sentence bounds ! abort" << LENDL;
    return MISSING_DATA;
  }
  if (sb->getGraphId() != "PosGraph") {
    LERROR << "SentenceBounds have been computed on " << sb->getGraphId() << " !" << LENDL;
    LERROR << "SyntacticAnalyzer-deps needs SentenceBounds on PosGraph" << LENDL;
    return INVALID_CONFIGURATION;
  }

  if (analysis.getData("SyntacticData")==0)
  {
    SyntacticData* syntacticData=new SyntacticData(anagraph,0);
    syntacticData->setupDependencyGraph();
    analysis.setData("SyntacticData",syntacticData);
  }

  RecognizerData* recoData=static_cast<RecognizerData*>(analysis.getData("RecognizerData"));
  if (recoData == 0)
  {
    recoData = new RecognizerData();
    analysis.setData("RecognizerData", recoData);
  }
  
  // ??OME2 for (SegmentationData::const_iterator boundItr=sb->begin();
  //     boundItr!=sb->end();
  for (std::vector<Segment>::const_iterator boundItr=(sb->getSegments()).begin();
       boundItr!=(sb->getSegments()).end();
       boundItr++)
  {
    LinguisticGraphVertex beginSentence=boundItr->getFirstVertex();
    LinguisticGraphVertex endSentence=boundItr->getLastVertex();
//     LDEBUG << "analyze sentence from vertex " << beginSentence
//            << " to vertex " << endSentence << LENDL;

    std::deque< std::string >::const_iterator actionsit, actionsit_end;
    actionsit = m_actions.begin(); actionsit_end = m_actions.end();
    for (; actionsit != actionsit_end; actionsit++)
    {
      std::string action = *actionsit;
      if (action == "setl2r" || action == "setr2l")
      {
        LWARN << "SyntacticAnalyzerDeps actions setl2r and setr2l are deprecated" << LENDL;
      }
      else
      {
//         LDEBUG << "Geting automaton" << LENDL;
        Automaton::Recognizer* recognizer = const_cast< Automaton::Recognizer*  >((*(m_recognizers.find(action))).second);
        std::vector<Automaton::RecognizerMatch> result;
//         LDEBUG << "Applying automaton for action " << action << " on sentence from " << beginSentence << " to " << endSentence << LENDL;
        try
        {
          recognizer->apply(*anagraph,
                            beginSentence,
                            endSentence,
                            analysis,
                            result,
                            true, // test all vertices=true
                            false,// stop at first success=false
                            false,  // only one success per type=true
                            false, // return at first success=false
                            m_applySameRuleWhileSuccess // depends on config file
                            );
        }
        catch (const PhoenixGraphHomoDepsVisitor::StartFinishedException& e) {}
      }
    }

  }

  LINFO << "end syntactic analysis - dependence relations search" << LENDL;
  return SUCCESS_ID;
}


} // closing namespace SyntacticAnalysis
} // closing namespace LinguisticProcessing
} // closing namespace Lima
