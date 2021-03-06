#   Copyright 2002-2013 CEA LIST
#    
#   This file is part of LIMA.
#
#   LIMA is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   LIMA is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with LIMA.  If not, see <http://www.gnu.org/licenses/>
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/. ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/linguisticProcessing/core )


########### next target ###############

SET(apply-rules_SRCS
  lima-lp-applyrecognizer.cpp
  apply-rules.cpp
  )

add_executable(apply-rules ${apply-rules_SRCS})

target_link_libraries(apply-rules
  lima-lp-automatoncompiler
  tokenizer
  lima-lp-syntacticanalysis
  lima-lp-specificentities
  lima-lp-postagger
  lima-lp-morphologicanalysis
  lima-lp-linguisticresources
  lima-lp-linguisticprocessors
  phenixGraph
  general
  lima-lp-dictionary
  data
  lima-lp-automaton
  lima-lp-analysisdumpers
  lima-lp-linguisticprocessing-core
  lima-lp-analysishandlers
  lima-lp-client
  lp-core-annotationGraph
  lima-lp-applyrecognizer
  lima-lp-analysisdict 
  lima-lp-propertycode 
  lima-common-fsaaccess 
  lima-common-processunitframework
  lima-common-xmlconfigurationfiles
  lima-lp-linguisticdata
  lima-lp-bagofwords 
  misc
 ${QT_LIBRARIES} lima-common-qslog
 ${Boost_REGEX}
  ${Boost_LIBRARIES} #wsock32
 ${optionalLibs}
  )

install(TARGETS apply-rules DESTINATION bin)


########### install files ###############
