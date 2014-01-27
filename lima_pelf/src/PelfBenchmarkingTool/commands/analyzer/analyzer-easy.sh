##!bin/sh
# The script needs variables to be defined : LEFFE_RESOURCES

# command line parameters
if [ $1 ]; then
  TEXT_FILE=`readlink -f $1`
fi
if [ ! $1 ] || [ ! -f $TEXT_FILE ]; then
  echo "Usage: analyzer.sh analysis-file"
  echo "  analysis-file : analysis file to analyze"
  exit
fi
readlink -f $1
echo $? $TEXT_FILE

# command configuration
LIC2M_OUTPUT_DTD=$LEFFE_RESOURCES/lic2m-xml-output.dtd

# creates output dir if not exists
TEXT_FILE_DIR=`dirname $TEXT_FILE`
TEXT_FILE_BASENAME=`basename $TEXT_FILE`
OUTPUT_DIR=`echo $TEXT_FILE_DIR/../output | xargs readlink -f`
OUTPUT_FILE=$OUTPUT_DIR/$TEXT_FILE_BASENAME.easy.xml
echo "Creating output path: "$OUTPUT_DIR
mkdir -p $OUTPUT_DIR

echo "---------"
echo "ANALYSIS"
echo "---------"

echo "Analyzing $TEXT_FILE"

# Analysis with Lima EasyXmlDumper
analyzeXmlDocuments --language=fre --pipeline=easy $TEXT_FILE

# Analysis with easyprocessor
# ANALYSIS_DIR=`dirname $TEXT_FILE`
# pushd $ANALYSIS_DIR
# cp $LIC2M_OUTPUT_DTD .
# easyprocessor -p easy $TEXT_FILE
# LIC2M_OUTPUT_DTD_BASENAME=`basename LIC2M_OUTPUT_DTD`
# rm -f $LIC2M_OUTPUT_DTD_BASENAME beginStatus-fre.log
# popd

mv $TEXT_FILE.easy.xml $OUTPUT_FILE
echo "Done, output should be in $OUTPUT_FILE"
