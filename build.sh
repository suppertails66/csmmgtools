###############################################################################
#
# Chou Soujuu Mecha MG translation build script
# 
# You must have the following utilities installed (i.e. in your PATH):
#   * ImageMagick (i.e. convert/mogrify)
#   * wine (used for ndstool, because running it natively produces
#     ROMs that don't work on real hardware)
#
###############################################################################

OUTROM_NAME="csmmg_en.nds"
BUILDFILES_DIR="buildfiles"
GAMEFILES_DIR="gamefiles"
IMAGES_DIR="images"
GAMEFILES_BUILD_DIR=$BUILDFILES_DIR/gamefiles
IMAGES_BUILD_DIR=$BUILDFILES_DIR/$IMAGES_DIR
BASE_PWD=$PWD

NDSTOOL="./ndstool/ndstool"
ARMIPS="./armips/build/armips"

# Add ntrtools to PATH
PATH="ntrtools/ntrtools:$PATH"

###############################################################################
# Set these to 0/1 to turn various build steps on and off for debugging
# purposes.
###############################################################################

# disable to use old buildfiles instead of cleaning at start.
# turn off if disabling any other options.
DO_COPYGAMEFILES=1

DO_BUILDFONTS=1
DO_INJECTTEXTOPBS=1
DO_INJECTGRPOPBS=1
DO_INJECTCABINARCHIVE=1
DO_STAMPROBOKARS=1
DO_BUILDKARS=1
  DO_BUILDBIGKARS=1
DO_INJECTKARS=1
DO_INSERTBVMS=1
DO_UPDATEASM=1
DO_PACKROM=1

#   cp gamefiles/arm7.bin buildfiles/gamefiles/arm7.bin

# composite a workshop logo onto a robo preview picture
#
# $1 = name of roboimage
# $2 = name of workshop logo
function stampRoboImage() {
  composite -geometry +2+20 -compose Copy "$IMAGES_BUILD_DIR/stamps/$2.png" "images/kar/img/roboimage/orig/$1.bin-grp.png" "images/kar/img/roboimage/$1.bin-grp.png"
}

function remapToPalette() {
  convert ${1}-grp.png -dither None -remap ${1}-pal.png PNG32:${1}-grp.png
}

function remapToPalette_noDither() {
  convert ${1}-grp.png -dither None -remap ${1}-pal.png PNG32:${1}-grp.png
}

function ncerConv() {
  convert ${1}-img.png -dither None -remap ${1}-pal.png PNG32:${1}-img.png
  dsimgconv p ${1} -n ${2} -t ${2}
}

function nscrConv() {
  convert ${1}-grp.png -dither None -remap ${1}-pal.png PNG32:${1}-grp.png
  dsimgconv ps ${1} -n ${2} -t ${2}
}

###############################################################################
# Build needed tools
###############################################################################

cd ntrtools/ntrtools
  make
cd $BASE_PWD

echo "*******************************************************************************"
echo "Building CUE's compression tools..."
echo "*******************************************************************************"

cd dscmprcue
  make
cd $BASE_PWD

if [ ! -e $ARMIPS ]; then
  
  echo "********************************************************************************"
  echo "Building armips..."
  echo "********************************************************************************"
  
  cd armips
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
  cd $BASE_PWD
  
fi

if [ ! -e $NDSTOOL ]; then
  
  echo "********************************************************************************"
  echo "Building ndstool..."
  echo "********************************************************************************"
  
  cd ndstool
    ./autogen.sh
    ./configure
    make
  cd $BASE_PWD
  
fi

###############################################################################
# Extract original game files
###############################################################################

if [ ! -e $GAMEFILES_DIR ]; then

  echo "*******************************************************************************"
  echo "Extracting ROM data..."
  echo "*******************************************************************************"
  
  ./extract_rom.sh "csmmg.nds" "$GAMEFILES_DIR"
  
fi

###############################################################################
# Create directories for intermediate build files
###############################################################################
mkdir -p ${BUILDFILES_DIR}
mkdir -p ${IMAGES_BUILD_DIR}

###############################################################################
# Copy all game files (so the originals aren't destroyed)
###############################################################################
if [ ! $DO_COPYGAMEFILES == 0 ]
then
  echo "Copying original game files"
  rm -r -f $GAMEFILES_BUILD_DIR
  cp -r $GAMEFILES_DIR $GAMEFILES_BUILD_DIR
fi

###############################################################################
# Build the new 12x12 font
###############################################################################

if [ ! $DO_BUILDFONTS == 0 ]
then
  FONTBUILDFILE=$GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR
  echo "Building font: ${FONTBUILDFILE}"
  nftrbuild $FONTBUILDFILE fonts/12x12/
fi

###############################################################################
# Insert translated strings into OPB files
###############################################################################

if [ ! $DO_INJECTTEXTOPBS == 0 ]
then
  echo "Injecting translations into OPB files"

  # kcom

  opbinsr $GAMEFILES_BUILD_DIR/data/kcom/Phrase.opb text/Phrase.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/kcom/Phrase.opb

  # scene

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/BottegaDesc.opb text/BottegaDesc.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/BottegaDesc.opb

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/MissionInfo.opb text/MissionInfo.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/MissionInfo.opb

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/MissionSelect.opb text/MissionSelect.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/MissionSelect.opb

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/PartsDesc.opb text/PartsDesc.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/PartsDesc.opb

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/PersonDesc.opb text/PersonDesc.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/PersonDesc.opb

  opbinsr $GAMEFILES_BUILD_DIR/data/scene/RoboDesc.opb text/RoboDesc.csv $GAMEFILES_BUILD_DIR/data/kcom/fontjp12x12.NFTR $GAMEFILES_BUILD_DIR/data/scene/RoboDesc.opb
fi

###############################################################################
# Insert translated graphics into OPB files
###############################################################################

if [ ! $DO_INJECTGRPOPBS == 0 ]
then
  echo "Preparing and inserting OPB images"

  # The general procedure for building the OPBs is as follows:
  # 1. Copy the raw OPB contents to the images build directory.
  # 2. Run images through ImageMagick, using remap to ensure they fit the
  #    palette.
  # 3. Run dsimgconv in patch mode on each image file (using NCER/NSCR modes as
  #    needed). Overwrite the copied OPB NCGR.
  # 4. Run opbfileinsr to bundle the OPB and place it in the gamefiles build
  #    directory.
  
  ####################
  # Album
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Album
  cp -r opb/Album $IMAGES_BUILD_DIR/opb/Album
  
  ncerConv images/opb/Album/012/0 $IMAGES_BUILD_DIR/opb/Album/0-0.bin
#  convert images/opb/Album/012/0-img.png -dither None -remap images/opb/Album/012/0-pal.png PNG32:images/opb/Album/012/0-img.png
#  dsimgconv p images/opb/Album/012/0 -n $IMAGES_BUILD_DIR/opb/Album/0-0.bin -t $IMAGES_BUILD_DIR/opb/Album/0-0.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Album.opb $IMAGES_BUILD_DIR/opb/Album/
  
  ####################
  # GalouyeBottega
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/GalouyeBottega
  cp -r opb/GalouyeBottega $IMAGES_BUILD_DIR/opb/GalouyeBottega
  
  for i in $(seq 3 17); do
    ncerConv images/opb/GalouyeBottega/012/$i $IMAGES_BUILD_DIR/opb/GalouyeBottega/0-0.bin
  done
  
  convert images/opb/GalouyeBottega/4/0-4.png -dither None -remap images/opb/GalouyeBottega/4/0-4-pal.png PNG32:images/opb/GalouyeBottega/4/0-4.png
  bminsr images/opb/GalouyeBottega/4/0-4.png $IMAGES_BUILD_DIR/opb/GalouyeBottega/0-4.bin $IMAGES_BUILD_DIR/opb/GalouyeBottega/0-4.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/GalouyeBottega.opb $IMAGES_BUILD_DIR/opb/GalouyeBottega/
  
  ####################
  # Hakubutukan
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Hakubutukan
  cp -r opb/Hakubutukan $IMAGES_BUILD_DIR/opb/Hakubutukan
  
  nscrConv images/opb/Hakubutukan/456/nscr $IMAGES_BUILD_DIR/opb/Hakubutukan/0-4.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Hakubutukan.opb $IMAGES_BUILD_DIR/opb/Hakubutukan/
  
  ####################
  # ItemExp
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/ItemExp
  cp -r opb/ItemExp $IMAGES_BUILD_DIR/opb/ItemExp
  
#  convert images/opb/ItemExp/0/0-0.png -dither None -remap images/opb/ItemExp/0/0-0-pal.png PNG32:images/opb/ItemExp/0/0-0.png
#  bminsr images/opb/ItemExp/0/0-0.png $IMAGES_BUILD_DIR/opb/ItemExp/0-0.bin $IMAGES_BUILD_DIR/opb/ItemExp/0-0.bin
  
  convert images/opb/ItemExp/0/0-1.png -dither None -remap images/opb/ItemExp/0/0-1-pal.png PNG32:images/opb/ItemExp/0/0-1.png
  bminsr images/opb/ItemExp/0/0-1.png $IMAGES_BUILD_DIR/opb/ItemExp/0-1.bin $IMAGES_BUILD_DIR/opb/ItemExp/0-1.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kcom/ItemExp.opb $IMAGES_BUILD_DIR/opb/ItemExp/
  
  ####################
  # KoubouStart
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/KoubouStart
  cp -r opb/KoubouStart $IMAGES_BUILD_DIR/opb/KoubouStart
  
  for i in {$(seq 3 5),$(seq 15 17)}; do
    ncerConv images/opb/KoubouStart/012/$i $IMAGES_BUILD_DIR/opb/KoubouStart/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/KoubouStart.opb $IMAGES_BUILD_DIR/opb/KoubouStart/
  
  ####################
  # Load
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Load
  cp -r opb/Load $IMAGES_BUILD_DIR/opb/Load
  
  for i in {$(seq 0 1),$(seq 5 21)}; do
    ncerConv images/opb/Load/012/$i $IMAGES_BUILD_DIR/opb/Load/0-0.bin
  done
  
  for i in {0,$(seq 13 15)}; do
    ncerConv images/opb/Load/456/$i $IMAGES_BUILD_DIR/opb/Load/0-4.bin
  done
  
  # Deal with layered OAMs
  for i in {$(seq 1 12),$(seq 16 23)}; do
    ncerConv images/opb/Load/456/multi/$i-a $IMAGES_BUILD_DIR/opb/Load/0-4.bin
    ncerConv images/opb/Load/456/multi/$i-b $IMAGES_BUILD_DIR/opb/Load/0-4.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Load.opb $IMAGES_BUILD_DIR/opb/Load/
  
  ####################
  # MissionSelect_A
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/MissionSelect_A
  cp -r opb/MissionSelect_A $IMAGES_BUILD_DIR/opb/MissionSelect_A
  
  for i in {1,$(seq 32 42)}; do
    ncerConv images/opb/MissionSelect_A/012/$i $IMAGES_BUILD_DIR/opb/MissionSelect_A/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_A.opb $IMAGES_BUILD_DIR/opb/MissionSelect_A/
  
  # This file is present twice in the game's data. The version produced by the
  # code above appears to be unused. The second version is RLE8 compressed, and
  # is what actually gets used.
  
  csmmgrle8_encode $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_A.opb $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_A_RL.bin
  
  ####################
  # MissionSelect_U
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/MissionSelect_U
  cp -r opb/MissionSelect_U $IMAGES_BUILD_DIR/opb/MissionSelect_U
  
  for i in {$(seq 9 23),$(seq 49 50),$(seq 66 67),118,120}; do
    ncerConv images/opb/MissionSelect_U/012/$i $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-0.bin
  done
  
#   nscrConv images/opb/MissionSelect_U/456/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-4.bin
#   nscrConv images/opb/MissionSelect_U/457/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-4.bin
#   nscrConv images/opb/MissionSelect_U/458/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-4.bin
  
#   remapToPalette images/opb/MissionSelect_U/456/nscr
#   remapToPalette images/opb/MissionSelect_U/457/nscr
#   remapToPalette images/opb/MissionSelect_U/458/nscr
#   nscr_gen opb/MissionSelect_U/0-4.bin $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-4.bin -p opb/MissionSelect_U/0-5.bin -pn 12 -nscr images/opb/MissionSelect_U/456/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-6.bin -nscr images/opb/MissionSelect_U/457/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-7.bin -nscr images/opb/MissionSelect_U/458/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-8.bin

  nscr_gen opb/MissionSelect_U/0-4.bin $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-4.bin -nscr images/opb/MissionSelect_U/456/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-6.bin -nscr images/opb/MissionSelect_U/457/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-7.bin -nscr images/opb/MissionSelect_U/458/nscr $IMAGES_BUILD_DIR/opb/MissionSelect_U/0-8.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_U.opb $IMAGES_BUILD_DIR/opb/MissionSelect_U/
  
  # This file is present twice in the game's data. The version produced by the
  # code above appears to be unused. The second version is RLE8 compressed, and
  # is what actually gets used.
  
  csmmgrle8_encode $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_U.opb $GAMEFILES_BUILD_DIR/data/kScene/MissionSelect_U_RL.bin
  
  ####################
  # NameInput
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/NameInput
  cp -r opb/NameInput $IMAGES_BUILD_DIR/opb/NameInput
  
  for i in {$(seq 1 18),$(seq 32 39)}; do
    ncerConv images/opb/NameInput/012/$i $IMAGES_BUILD_DIR/opb/NameInput/0-0.bin
  done
  
  for i in {$(seq 0 1),$(seq 34 36)}; do
    ncerConv images/opb/NameInput/456/$i $IMAGES_BUILD_DIR/opb/NameInput/0-4.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/NameInput.opb $IMAGES_BUILD_DIR/opb/NameInput/
  
  ####################
  # NetworkCenter
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/NetworkCenter
  cp -r opb/NetworkCenter $IMAGES_BUILD_DIR/opb/NetworkCenter
  
  for i in {0,$(seq 10 15)}; do
    ncerConv images/opb/NetworkCenter/012/$i $IMAGES_BUILD_DIR/opb/NetworkCenter/0-0.bin
  done
  
  for i in {$(seq 1 6),$(seq 24 25)}; do
    ncerConv images/opb/NetworkCenter/456/$i $IMAGES_BUILD_DIR/opb/NetworkCenter/0-4.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/NetworkCenter.opb $IMAGES_BUILD_DIR/opb/NetworkCenter/
  
  ####################
  # NetworkStart
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/NetworkStart
  cp -r opb/NetworkStart $IMAGES_BUILD_DIR/opb/NetworkStart
  
  for i in {$(seq 3 5),$(seq 15 17)}; do
    ncerConv images/opb/NetworkStart/012/$i $IMAGES_BUILD_DIR/opb/NetworkStart/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/NetworkStart.opb $IMAGES_BUILD_DIR/opb/NetworkStart/
  
  ####################
  # PartsStart
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/PartsStart
  cp -r opb/PartsStart $IMAGES_BUILD_DIR/opb/PartsStart
  
  for i in {$(seq 3 5),$(seq 15 17)}; do
    ncerConv images/opb/PartsStart/012/$i $IMAGES_BUILD_DIR/opb/PartsStart/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/PartsStart.opb $IMAGES_BUILD_DIR/opb/PartsStart/
  
  ####################
  # SelectMario
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/SelectMario
  cp -r opb/SelectMario $IMAGES_BUILD_DIR/opb/SelectMario
  
  for i in {$(seq 1 10),$(seq 13 15),$(seq 30 32),34}; do
    ncerConv images/opb/SelectMario/012/$i $IMAGES_BUILD_DIR/opb/SelectMario/0-0.bin
  done
  
  nscrConv images/opb/SelectMario/456/nscr $IMAGES_BUILD_DIR/opb/SelectMario/0-4.bin
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/SelectMario.opb $IMAGES_BUILD_DIR/opb/SelectMario/
  
  ####################
  # SelectParts
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/SelectParts
  cp -r opb/SelectParts $IMAGES_BUILD_DIR/opb/SelectParts
  
  for i in {$(seq 1 15),$(seq 30 32)}; do
    ncerConv images/opb/SelectParts/012/$i $IMAGES_BUILD_DIR/opb/SelectParts/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/SelectParts.opb $IMAGES_BUILD_DIR/opb/SelectParts/
  
  ####################
  # Settei
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Settei
  cp -r opb/Settei $IMAGES_BUILD_DIR/opb/Settei
  
  for i in {0,$(seq 8 20),$(seq 27 28)}; do
    ncerConv images/opb/Settei/012/$i $IMAGES_BUILD_DIR/opb/Settei/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Settei.opb $IMAGES_BUILD_DIR/opb/Settei/
  
  ####################
  # Title
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Title
  cp -r opb/Title $IMAGES_BUILD_DIR/opb/Title
  
#  for i in $(seq 2 4); do
#    ncerConv images/opb/Title/012/$i $IMAGES_BUILD_DIR/opb/Title/0-0.bin
#  done

  # the gradient on these looks like crap without dithering
#  convert images/opb/Title/012/multi/3-a-img.png -dither Riemersma -remap images/opb/Title/012/multi/3-a-pal.png PNG32:images/opb/Title/012/multi/3-a-img.png
#  convert images/opb/Title/012/multi/3-b-img.png -dither Riemersma -remap images/opb/Title/012/multi/3-b-pal.png PNG32:images/opb/Title/012/multi/3-b-img.png
  convert images/opb/Title/012/multi/3-a-img.png -dither None -remap images/opb/Title/012/multi/3-a-pal.png PNG32:images/opb/Title/012/multi/3-a-img.png
  convert images/opb/Title/012/multi/3-b-img.png -dither None -remap images/opb/Title/012/multi/3-b-pal.png PNG32:images/opb/Title/012/multi/3-b-img.png
  
  # The title screen uses layered OAMs combined with 16-color mode, so they're
  # split into two parts corresponding to the two layers.
  for i in $(seq 2 3); do
    ncerConv images/opb/Title/012/multi/$i-a $IMAGES_BUILD_DIR/opb/Title/0-0.bin
    ncerConv images/opb/Title/012/multi/$i-b $IMAGES_BUILD_DIR/opb/Title/0-0.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Title.opb $IMAGES_BUILD_DIR/opb/Title/
  
  ####################
  # Tuduki
  ####################
  
  rm -r -f $IMAGES_BUILD_DIR/opb/Tuduki
  cp -r opb/Tuduki $IMAGES_BUILD_DIR/opb/Tuduki
  
  for i in {$(seq 0 1),$(seq 5 21)}; do
    ncerConv images/opb/Tuduki/012/$i $IMAGES_BUILD_DIR/opb/Tuduki/0-0.bin
  done
  
  for i in {0,$(seq 13 15)}; do
    ncerConv images/opb/Tuduki/456/$i $IMAGES_BUILD_DIR/opb/Tuduki/0-4.bin
  done
  
  # Deal with layered OAMs
  for i in {$(seq 1 12),$(seq 16 23)}; do
    ncerConv images/opb/Tuduki/456/multi/$i-a $IMAGES_BUILD_DIR/opb/Tuduki/0-4.bin
    ncerConv images/opb/Tuduki/456/multi/$i-b $IMAGES_BUILD_DIR/opb/Tuduki/0-4.bin
  done
  
  opbfileinsr $GAMEFILES_BUILD_DIR/data/kScene/Tuduki.opb $IMAGES_BUILD_DIR/opb/Tuduki/
  
  ####################
  # mission1205
  # (not actually OPB)
  ####################
  
  for i in $(seq 0 1); do
    ncerConv images/mission1205/Character2Cell2/full/$i $GAMEFILES_BUILD_DIR/data/decor/mission1205/Character2.NCGR
  done
  
  for i in $(seq 0 9); do
    ncerConv images/mission1205/txt00Cell4/full/$i $GAMEFILES_BUILD_DIR/data/decor/mission1205/txt00.NCGR
  done
  
fi

###############################################################################
# Insert translated images into ca.binarchive
###############################################################################

if [ ! $DO_INJECTCABINARCHIVE == 0 ]
then
  
  echo "Preparing and inserting ca.binarchive images"
  
  rm -r -f $IMAGES_BUILD_DIR/cabinarchive
  cp -r cabinarchive $IMAGES_BUILD_DIR/cabinarchive
  
  # Deal with layered OAMs
  for i in $(seq 44 51); do
    ncerConv images/cabinarchive/163/multi/$i-a $IMAGES_BUILD_DIR/cabinarchive/163/main.NCGR
    ncerConv images/cabinarchive/163/multi/$i-b $IMAGES_BUILD_DIR/cabinarchive/163/main.NCGR
  done
  
  cabinarc_insr $GAMEFILES_BUILD_DIR/data/ba/ca.binarchive $IMAGES_BUILD_DIR/cabinarchive/
  
fi

###############################################################################
# Stamp roboimage KARs with new workshop logos
###############################################################################
if [ ! $DO_STAMPROBOKARS == 0 ]
then
  echo "Stamping roboimage KARs"
  
  # Stamp workshop logos onto outline
  mkdir -p $IMAGES_BUILD_DIR/stamps
  for file in images/workshop/32x32/*.png; do
#    imgstamp $file images/workshop/_outline.png $IMAGES_BUILD_DIR/stamps/$(basename $file) -b -x 0 -y 0
    composite -compose DstOver -gravity center $file images/workshop/_outline.png $IMAGES_BUILD_DIR/stamps/$(basename $file)
  done
  
  # Composite every roboimage with its workshop logo (if it has one)
  
#  composite -geometry +0+20 $IMAGES_BUILD_DIR/stamps/tree.png images/kar/img/roboimage/orig/0.bin-grp.png images/kar/img/roboimage/0.bin-grp.png
  stampRoboImage 0 tree
  stampRoboImage 4 circle
  stampRoboImage 5 circle
  stampRoboImage 6 circle
  stampRoboImage 13 bullet
  stampRoboImage 14 bullet
  stampRoboImage 15 bullet
  stampRoboImage 16 wheel
  stampRoboImage 17 wheel
  stampRoboImage 18 wheel
  stampRoboImage 19 bow
  stampRoboImage 20 bow
  stampRoboImage 21 bow
  stampRoboImage 22 heavy
  stampRoboImage 23 heavy
  stampRoboImage 24 heavy
  stampRoboImage 25 bullet
  stampRoboImage 26 bullet
  stampRoboImage 27 bullet
  stampRoboImage 28 wheel
  stampRoboImage 29 wheel
  stampRoboImage 30 wheel
  stampRoboImage 31 wheel
  stampRoboImage 32 wheel
  stampRoboImage 33 wheel
  stampRoboImage 34 wheel
  stampRoboImage 35 wheel
  stampRoboImage 36 wheel
  stampRoboImage 37 fang
  stampRoboImage 38 fang
  stampRoboImage 39 fang
  stampRoboImage 40 bow
  stampRoboImage 41 bow
  stampRoboImage 42 bow
  stampRoboImage 44 special
  stampRoboImage 45 special
  stampRoboImage 46 special
  stampRoboImage 50 special
  stampRoboImage 51 special
  stampRoboImage 52 special
  stampRoboImage 53 heavy
  stampRoboImage 54 heavy
  stampRoboImage 55 heavy
  stampRoboImage 56 bullet
  stampRoboImage 57 bullet
  stampRoboImage 58 tree
  stampRoboImage 59 tree
  stampRoboImage 60 tree
  stampRoboImage 61 tree
  stampRoboImage 62 tree
  stampRoboImage 65 heavy
  stampRoboImage 66 heavy
  stampRoboImage 67 heavy
  stampRoboImage 70 special
  stampRoboImage 71 special
  stampRoboImage 72 special
  stampRoboImage 73 bomb
  stampRoboImage 74 bomb
  stampRoboImage 75 bomb
  stampRoboImage 76 tree
  stampRoboImage 77 tree
  stampRoboImage 78 tree
  stampRoboImage 79 bullet
  stampRoboImage 80 bullet
  stampRoboImage 81 fang
  stampRoboImage 82 fang
  stampRoboImage 83 fang
  stampRoboImage 84 bomb
  stampRoboImage 85 bomb
  stampRoboImage 88 circle
  stampRoboImage 89 heavy
  stampRoboImage 90 heavy
  stampRoboImage 91 heavy
  stampRoboImage 92 bullet
  stampRoboImage 93 bullet
  stampRoboImage 94 bullet
  stampRoboImage 97 bullet
  stampRoboImage 98 bullet
  stampRoboImage 99 bullet
  stampRoboImage 102 ninja
  stampRoboImage 103 ninja
  stampRoboImage 104 ninja
  stampRoboImage 109 fang
  stampRoboImage 110 fang
  stampRoboImage 111 circle
  stampRoboImage 112 circle
  stampRoboImage 113 circle
  stampRoboImage 114 bow
  stampRoboImage 115 bow
  stampRoboImage 116 bow
  stampRoboImage 117 circle
  stampRoboImage 118 circle
  stampRoboImage 119 circle
  stampRoboImage 120 wheel
  stampRoboImage 121 wheel
  stampRoboImage 122 wheel
  stampRoboImage 123 ninja
  stampRoboImage 124 ninja
  stampRoboImage 125 ninja
  stampRoboImage 126 bomb
  stampRoboImage 129 heavy
  stampRoboImage 130 heavy
  stampRoboImage 131 heavy
  stampRoboImage 132 bomb
  stampRoboImage 133 bomb
  stampRoboImage 134 bomb
  stampRoboImage 135 bow
  stampRoboImage 136 bow
  stampRoboImage 137 bow
  stampRoboImage 138 circle
  stampRoboImage 139 circle
  stampRoboImage 140 circle
  
fi

###############################################################################
# Build edited KAR images
###############################################################################

if [ ! $DO_BUILDKARS == 0 ]
then

  #############################################################################
  # convert KARs to binary
  #############################################################################
  
  echo "Converting KAR images to binary format"

  if [ ! $DO_BUILDBIGKARS == 0 ]
  then

    ##########################
    # do roboimages
    ##########################
    mkdir -p $BUILDFILES_DIR/images/kar/roboimage
    cp images/kar/raw/roboimage/index.bin $BUILDFILES_DIR/images/kar/roboimage
    for file in images/kar/img/roboimage/*.bin-dat.bin; do
      IMGBASENAME=$(basename $file .bin)
      IMGFULLBASENAME=$(basename $file .bin-dat.bin)
      IMGPREFIX=$(dirname $file)/$IMGFULLBASENAME.bin
      DSTFILENAME=$BUILDFILES_DIR/images/kar/roboimage/$IMGFULLBASENAME.bin
      
      convert ${IMGPREFIX}-grp.png -dither None -remap ${IMGPREFIX}-pal.png PNG32:${IMGPREFIX}-grp.png
      karimginsr $DSTFILENAME b $IMGPREFIX
      csmmgrle8_encode $DSTFILENAME $DSTFILENAME

    done
    
    ##########################
    # generate and do staff roll
    ##########################
    BUILDFILES_DIR=$BUILDFILES_DIR ./generate_credits.sh
    mkdir -p $BUILDFILES_DIR/images/kar/StaffText
    cp images/kar/raw/StaffText/index.bin $BUILDFILES_DIR/images/kar/StaffText
#    for file in images/kar/img/StaffText/*.bin-dat.bin; do
    for file in $BUILDFILES_DIR/credits_outlined/*.bin-dat.bin; do
      IMGBASENAME=$(basename $file .bin)
      IMGFULLBASENAME=$(basename $file .bin-dat.bin)
      IMGPREFIX=$(dirname $file)/$IMGFULLBASENAME.bin
      DSTFILENAME=$BUILDFILES_DIR/images/kar/StaffText/$IMGFULLBASENAME.bin
      
      convert ${IMGPREFIX}-grp.png -dither None -remap ${IMGPREFIX}-pal.png PNG32:${IMGPREFIX}-grp.png
      karimginsr $DSTFILENAME b $IMGPREFIX
      csmmgrle8_encode $DSTFILENAME $DSTFILENAME

    done
  fi

  mkdir -p $BUILDFILES_DIR/images/kar/bg
  cp images/kar/raw/bg/index.bin $BUILDFILES_DIR/images/kar/bg
  for file in images/kar/img/bg/*.bin-dat.bin; do
    IMGBASENAME=$(basename $file .bin)
    IMGFULLBASENAME=$(basename $file .bin-dat.bin)
    IMGPREFIX=$(dirname $file)/$IMGFULLBASENAME.bin
    DSTFILENAME=$BUILDFILES_DIR/images/kar/bg/$IMGFULLBASENAME.bin
    
    convert ${IMGPREFIX}-grp.png -dither None -remap ${IMGPREFIX}-pal.png PNG32:${IMGPREFIX}-grp.png
    karimginsr $DSTFILENAME 2 $IMGPREFIX
    csmmgrle8_encode $DSTFILENAME $DSTFILENAME

  done

  mkdir -p $BUILDFILES_DIR/images/kar/ShopStartTop
  cp images/kar/raw/ShopStartTop/index.bin $BUILDFILES_DIR/images/kar/ShopStartTop
  for file in images/kar/img/ShopStartTop/*.bin-dat.bin; do
    IMGBASENAME=$(basename $file .bin)
    IMGFULLBASENAME=$(basename $file .bin-dat.bin)
    IMGPREFIX=$(dirname $file)/$IMGFULLBASENAME.bin
    DSTFILENAME=$BUILDFILES_DIR/images/kar/ShopStartTop/$IMGFULLBASENAME.bin
    
    convert ${IMGPREFIX}-grp.png -dither None -remap ${IMGPREFIX}-pal.png PNG32:${IMGPREFIX}-grp.png
    karimginsr $DSTFILENAME b $IMGPREFIX

  done
fi

###############################################################################
# Insert edited KAR images into original KARs and put in build directory
###############################################################################

if [ ! $DO_INJECTKARS == 0 ]
then
  echo "Injecting edited KARs into original files"

  karinsr $GAMEFILES_DIR/data/kcom/roboimage.kar $GAMEFILES_BUILD_DIR/data/kcom/roboimage.kar $BUILDFILES_DIR/images/kar/roboimage/
  karinsr $GAMEFILES_DIR/data/kScene/StaffText.kar $GAMEFILES_BUILD_DIR/data/kScene/StaffText.kar $BUILDFILES_DIR/images/kar/StaffText/
  karinsr $GAMEFILES_DIR/data/kadv/bg.kar $GAMEFILES_BUILD_DIR/data/kadv/bg.kar $BUILDFILES_DIR/images/kar/bg/
  karinsr $GAMEFILES_DIR/data/kScene/ShopStartTop.kar $GAMEFILES_BUILD_DIR/data/kScene/ShopStartTop.kar $BUILDFILES_DIR/images/kar/ShopStartTop/
fi

###############################################################################
# Insert translated strings into BVM files
###############################################################################

if [ ! $DO_INSERTBVMS == 0 ]
then
  mkdir -p ${BUILDFILES_DIR}/bvm
  for file in {bvm/decompressed/map,bvm/decompressed/main}/*; do
    BVMNAME=$(basename $file)
    BVMNAMEBASE=$(basename $file .bvmc)
    BVMNAMEBASE=$(basename $BVMNAMEBASE .bvm)
    FOLDERNAME=$(basename $(dirname $file))
    INRAWFILENAME=bvm/decompressed/$FOLDERNAME/$BVMNAME
    OUTFOLDERNAME=${BUILDFILES_DIR}/bvm/$FOLDERNAME
    OUTFILENAME=$OUTFOLDERNAME/$BVMNAME
    INTEXTFILENAME=bvm/extracted/$FOLDERNAME/$BVMNAMEBASE
    
    echo "Building BVM: ${OUTFILENAME}"
    mkdir -p $OUTFOLDERNAME
    bvmtxt_insr $INRAWFILENAME $OUTFILENAME $INTEXTFILENAME fonts/12x12-ascii/
  done

  for file in bvm/decompressed/m/*/*; do
    BVMNAME=$(basename $file)
    BVMNAMEBASE=$(basename $file .bvmc)
    BVMNAMEBASE=$(basename $BVMNAMEBASE .bvm)
    FOLDERNAME=$(basename $(dirname $(dirname $file)))/$(basename $(dirname $file))
    INRAWFILENAME=bvm/decompressed/$FOLDERNAME/$BVMNAME
    OUTFOLDERNAME=${BUILDFILES_DIR}/bvm/$FOLDERNAME
    OUTFILENAME=$OUTFOLDERNAME/$BVMNAME
    INTEXTFILENAME=bvm/extracted/$FOLDERNAME/$BVMNAMEBASE
    
    echo "Building BVM: ${OUTFILENAME}"
    mkdir -p $OUTFOLDERNAME
    bvmtxt_insr $INRAWFILENAME $OUTFILENAME $INTEXTFILENAME fonts/12x12-ascii/
  done

  ###############################################################################
  # Compress the built BVMs.
  # Note that the BVMs in the "main" subfolder are not compressed.
  ###############################################################################

  for file in $BUILDFILES_DIR/bvm/map/*; do
    echo "Compressing: ${file}"
#    wine dscmprcue/lzss.exe -ewo $file
    dscmprcue/lzss -ewo $file
  done

  for file in $BUILDFILES_DIR/bvm/m/*/*; do
    echo "Compressing: ${file}"
#    wine dscmprcue/lzss.exe -ewo $file
    dscmprcue/lzss -ewo $file
  done

  ###############################################################################
  # Copy the built and compressed BVMs into the buildfiles directory
  ###############################################################################

  for file in $BUILDFILES_DIR/bvm/*; do
    echo "Syncing built BVM directory: ${file}"
    rsync -a $file $GAMEFILES_BUILD_DIR/data
  done
fi

###############################################################################
# Update ASM
###############################################################################

if [ ! $DO_UPDATEASM == 0 ]; then
  echo "Building ASM..."
  mkdir -p out/asm
  $ARMIPS asm/csmmg.asm -temp out/asm/temp.txt -sym out/asm/symbols.sym -sym2 out/asm/symbols.sym2
fi

###############################################################################
# Pack the ROM
###############################################################################

if [ ! $DO_PACKROM == 0 ]
then
  echo "Packing ROM: $OUTROM_NAME"

#  ndstool -c $OUTROM_NAME -9 ${GAMEFILES_BUILD_DIR}/arm9.bin -7 ${GAMEFILES_BUILD_DIR}/arm7.bin -y9 ${GAMEFILES_BUILD_DIR}/y9.bin -y7 ${GAMEFILES_BUILD_DIR}/y7.bin -d ${GAMEFILES_BUILD_DIR}/data -y ${GAMEFILES_BUILD_DIR}/overlay -t ${GAMEFILES_BUILD_DIR}/banner.bin -h ${GAMEFILES_BUILD_DIR}/header.bin

  # After testing on real hardware, I discovered that ROMs built using ndstool
  # on *nix wouldn't boot (despite working in desmume). On a hunch that the FAT
  # was somehow getting built in an invalid way due to observing *nix
  # conventions on upper/lower case, I tried building with the ancient (circa
  # 2005) Windows binary of ndstool that was bundled with dslazy, and it
  # actually worked.
  # So that's the story behind this very very stupid line in the build script.
  # Maybe if I'm ever feeling ambitious, I'll fix ndstool. Or not.
  wine ndstool.exe -c $OUTROM_NAME -9 ${GAMEFILES_BUILD_DIR}/arm9.bin -7 ${GAMEFILES_BUILD_DIR}/arm7.bin -y9 ${GAMEFILES_BUILD_DIR}/y9.bin -y7 ${GAMEFILES_BUILD_DIR}/y7.bin -d ${GAMEFILES_BUILD_DIR}/data -y ${GAMEFILES_BUILD_DIR}/overlay -t ${GAMEFILES_BUILD_DIR}/banner.bin -h ${GAMEFILES_BUILD_DIR}/header.bin
fi
