#!/bin/bash
KIBOT_DIR=$(git rev-parse --show-toplevel)
RES=$?
if [ $RES -eq 0 ] 
then
    KIBOT_DIR=$KIBOT_DIR/kibot
else
    KIBOT_DIR=/home/$USER/workdir/kibot
fi    
echo $KIBOT_DIR
rm -rf doc
CURRENT=`pwd`

#Check current DirName
BOARD=`basename $CURRENT`
if [ ! -f  $BOARD.kicad_sch ]
then
    BOARD=$(dirname "$CURRENT")
    BOARD=`basename $BOARD`
fi
#If not found, check with parent Dir name
if [ ! -f  $BOARD.kicad_sch ]
then   
    echo Expected file not found: $BOARD.kicad_sch
    #exit 0
    #If not found change manually.
    BOARD=LocoBuffer
fi
FILES="00_erc_drc.kibot.yaml 01_pdf_doc.kibot.yaml 02_build_doc.kibot.yaml 03_graphic_doc.kibot.yaml 04_fab_board.kibot.yaml 05_fab_pcba.kibot.yaml"  

#FILES="02_build_doc.kibot.yaml"  


for i in $FILES 
do
    echo "Running $i"
    kibot -e $BOARD.kicad_sch -b $BOARD.kicad_pcb -d doc -c $KIBOT_DIR/$i $@
done 

rm -f doc/*.ogv

exit 0




 

