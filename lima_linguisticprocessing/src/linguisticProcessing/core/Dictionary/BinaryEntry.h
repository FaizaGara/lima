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

// NAUTITIA
//
// jys 2-OCT-2002
//
// Binary memory explorer.
// BinaryEntry is the base class of other dictionnary structure
// management class (DictionaryEntry, LingInfoEntry, etc.)
// BinaryEntry has methods to explore dictionary in memory.
// Dictionary memory strcture is described in NAU2002.r2128 JYS

#ifndef BinaryEntry_H
#define BinaryEntry_H

#include "DictionaryExport.h"
#include "common/Data/LimaString.h"
#include "common/Data/strwstrtools.h"

namespace Lima {
namespace LinguisticProcessing {
namespace Dictionary {

class LIMA_DICTIONARY_EXPORT BinaryEntry {

public:
    BinaryEntry();
    BinaryEntry(const BinaryEntry&);
    virtual ~BinaryEntry();

    BinaryEntry& operator=(const BinaryEntry& entry);

    // gets the pointed encoded number (presented in big endian)
    // advances pointer to point after the number
    uint64_t getEncodedNumber(unsigned char*& ptr) const;

    // gets the pointed encoded number (presented in big endian)
    // pointer is not moved
    uint64_t getEncodedNumberNoMove(unsigned char* ptr) const;

    // gets the next number of specified number of bytes (presented in big endian)
    // advances pointer to point after the number
    uint64_t getNumber(unsigned char*& ptr, const uint64_t count) const;
    
    // gets the next number of specified number of bytes (presented in big endian)
    // pointer is not moved
    uint64_t getNumberNoMove(unsigned char* ptr,const uint64_t count) const;
    
    // gets the pointed dictionary string as a LimaString
    // advances pointer to point after the string
    Lima::LimaString getString(unsigned char*& ptr) const;

    // gets the pointed dictionary string as a LimaString
    // pointer is not moved
    Lima::LimaString getStringNoMove(unsigned char* ptr) const;

    // gets the pointed dictionary string as a LimaString
    // advances pointer to point after the string
    Lima::LimaString getUtf8String(unsigned char*& ptr) const;

    // gets the pointed dictionary string as a LimaString
    // pointer is not moved
    Lima::LimaString getUtf8StringNoMove(unsigned char* ptr) const;

    // advances pointer to the next dictionary field
    unsigned char *nextField(unsigned char*& ptr) const;

protected:

private:
    Lima::LimaString _dummyString;

};

} // closing namespace Dictionary
} // closing namespace LinguisticProcessing
} // closing namespace Lima

#endif  
