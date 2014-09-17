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


#ifndef LIMACONLLTOKENIDMAPPING_H
#define LIMACONLLTOKENIDMAPPING_H

#include "common/ProcessUnitFramework/AnalysisContent.h"
#include "linguisticProcessing/core/LinguisticAnalysisStructure/LinguisticGraph.h"
#include <map>

namespace Lima {
namespace LinguisticProcessing {
namespace SemanticAnalysis {


class LimaConllTokenIdMapping : public std::map<int, std::map<int,LinguisticGraphVertex>>, public AnalysisData
{

public:
  LimaConllTokenIdMapping();
  ~LimaConllTokenIdMapping();

};


}
} // LinguisticProcessing
} // Lima

#endif
