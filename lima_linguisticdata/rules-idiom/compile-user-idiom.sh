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
#!/bin/bash
# 
# Shell script created by besancon on Tue Jul  5 2005 
# Version : $Id$ 
#

if (( $# < 2 )); then
  echo must specify language and filename
  exit 1 ;
fi

export lang=$1;
export file=$2;

# programs
export compile=compile-rules
export reformat=reformat_idioms.pl
export transcode=transcodeIdiomatic.pl

# resources
export codeDir=$LINGUISTIC_DATA_ROOT/analysisDictionary/$lang/code/
export codeFile=$codeDir/code-$lang.xml
export symbCodeFile=$codeDir/symbolicCode-$lang.xml
export catConvertFile=`dirname $file`/categories_convert

export reformatNeeded=0;
export CategoryConvertNeeded=0;

# try to guess type of file
if [[ -n `head -5 $file | grep "^[^;]*;[^;]*;[^;]*;[^;]*"` ]]; then
    echo "file is in simplified format";
    # file is in simplified format
    export reformatNeeded=1;
    # look if need transcoding of categories
    if [[ -z `head -5 $file | cut -d";" -f6 | grep "^[NDPAVRSCFIE][a-z\-]*$"` ]]; then
        echo "file needs category conversion";
        export CategoryConvertNeeded=1;
    fi;
else 
    echo "file is in rule format";
fi;

# check if all needed resources and programs are available
export neededFiles=$file
export neededPrograms=
if (( $reformatNeeded )); then
    export neededPrograms="$neededPrograms $reformat";
    if (( $CategoryConvertNeeded )); then
        export neededFiles="$neededFiles $catConvertFile";
    fi;
fi;
export neededPrograms="$neededPrograms $transcode";
export neededFiles="$neededFiles $codeFile $symbCodeFile";

for f in $neededFiles; do
    if [[ ! -r $f ]]; then
        echo "cannot find file \"$f\"";
        exit;
    fi;
done;
for f in $neededPrograms; do
    if [[ -z `which $f` ]]; then
        echo "cannot access program \"$f\"";
        exit;
    fi;
done;

# reformat
export rulesFileIsTemporary=0;
if (( $reformatNeeded )); then
    export rulesFileIsTemporary=1;
    export rulesFile=$file.rules.1;
    if (( $CategoryConvertNeeded==0 )); then
        export reformat="$reformat -noCatConvert";
    else 
        export reformat="$reformat -categoriesConvert=$catConvertFile";
    fi;
    $reformat $file > $rulesFile;
else 
    export rulesFile=$file;
fi;

# transcode
export transcodedFile=$file.rules;
echo "set defaultAction=>CreateIdiomaticAlternative()" > $transcodedFile
$transcode $codeFile $codeDir/symbolicCode*.xml $rulesFile >> $transcodedFile

# compile rules
export outputFile=`dirname $file`/`basename $file .txt`.bin
echo "compile expressions in $outputFile"
$compile --language=$lang --output=$outputFile $transcodedFile 

# clean 
if (( $rulesFileIsTemporary )); then
    /bin/rm $rulesFile;
fi;
/bin/rm $transcodedFile
