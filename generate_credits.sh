
#**********************************************************************
# Script to generate the credits text using ImageMagick
#**********************************************************************

if [ "$BUILDFILES_DIR" == "" ]; then
  BUILDFILES_DIR="buildfiles"
fi

# clear any existing contents of target directory
rm $BUILDFILES_DIR/credits_generated/*
mkdir -p $BUILDFILES_DIR/credits_generated

function drawTextImg() {
  file=$BUILDFILES_DIR/credits_generated/$1.bin-grp.png
  fontname=$2
  xoffset=$3
  yoffset=$4
  text=$5
  # fontsize is optional and defaults to 26
  fontsize=$6
  if [ "$fontsize" == "" ]; then
    fontsize=26
  fi

  if [ ! -f $file ]; then
#    mkdir -p $(dirname $file)
    convert -size 256x192 xc: -alpha transparent "$file"
  fi
  
  convert $file -font $fontname -pointsize $fontsize -fill white -gravity North -draw "text $xoffset,$yoffset '$text'" $file
}

# generate black outline around non-transparent part of input image,
# for e.g. credits text
#
# $1 = input image
# $2 = output image
function outlineSolidPixels() {
  convert "$1" \( +clone -channel A -morphology EdgeOut Diamond -negate -threshold 0 -negate +channel +level-colors black \) -compose DstOver -composite "$2"
}

#convert -size 256x192 xc: -alpha transparent -font Teko-SemiBold -pointsize 26 -fill white -gravity North -draw "text 0,0 'Test'" -draw "text 0,25 'Test2'" test_out3.png

##########################
# Generate staff roll text
##########################

echo "Generating staff roll"

#note: subtract 6 from base y-position of original text

drawTextImg  0 Teko-SemiBold   0  38  "Producer"
drawTextImg  1 Teko-Regular    0  78  "Hitoshi Yamagami"

drawTextImg  2 Teko-SemiBold   0  38  "Director"
drawTextImg  3 Teko-Regular    0  78  "Azusa Tajima"

drawTextImg  4 Teko-SemiBold   0  38  "Character Designer"
drawTextImg  5 Teko-Regular    0  78  "Chiharu Sakiyama"

drawTextImg  6 Teko-SemiBold   0  38  "Director/Scenario"
drawTextImg  7 Teko-Regular    0  78  "Takehiro Honma"

drawTextImg  8 Teko-SemiBold   0  38  "Main Programming"
drawTextImg  9 Teko-Regular    0  78  "Yoshihiro Ito"

drawTextImg 10 Teko-SemiBold   0  38  "Art Director/Puppet Design"
drawTextImg 11 Teko-Regular    0  78  "Masatsugu Igarashi"

drawTextImg 12 Teko-SemiBold   0  19  "Programming"
drawTextImg 13 Teko-Regular    0  52  "Toshiro Iwasaki"
drawTextImg 13 Teko-Regular    0  78  "Shinichi Okada"
drawTextImg 13 Teko-Regular    0  104 "Kanji Tsukahara"

drawTextImg 14 Teko-SemiBold   0  38  "Puppet Modelling"
drawTextImg 15 Teko-Regular    0  78  "Hirofumi Morimoto"

drawTextImg 16 Teko-SemiBold   0  38  "Motion Design"
drawTextImg 17 Teko-Regular    0  78  "Yumiko Toyama"

drawTextImg 18 Teko-SemiBold   0  38  "Cockpit Design"
drawTextImg 19 Teko-Regular    0  78  "Masato Yamashita"

# ??? 21 林　英悟 Hayashi EiSatoru
drawTextImg 20 Teko-SemiBold   0  38  "Planning"
drawTextImg 21 Teko-Regular    0  78  "Hayashi EiSatoru"

drawTextImg 22 Teko-SemiBold   0  38  "Map Modelling"
drawTextImg 23 Teko-Regular    0  68  "Kazuhiro Ohkawa"
drawTextImg 23 Teko-Regular    0  94  "Kenji Sato"

drawTextImg 24 Teko-SemiBold   0  38  "Script"
drawTextImg 25 Teko-Regular    0  78  "Atsushi Takeba"

drawTextImg 26 Teko-SemiBold   0  38  "Network Manager"
drawTextImg 27 Teko-Regular    0  78  "Nobuo Fujii"

drawTextImg 28 Teko-SemiBold   0  38  "Sound Design"
drawTextImg 29 Teko-Regular    0  68  "Masafumi Takada/ghm"
drawTextImg 29 Teko-Regular    0  94  "Atsushi Fukuda/ghm"

drawTextImg 30 Teko-SemiBold   0  38  "Puppet Design"
drawTextImg 31 Teko-Regular    0  78  "Kei Hagiwara"

drawTextImg 32 Teko-SemiBold   0  38  "Design Producer"
drawTextImg 33 Teko-Regular    0  68  "Yuko Yamamoto"
drawTextImg 33 Teko-Regular    0  94  "Kohei Takeshige"

drawTextImg 34 Teko-SemiBold   0  38  "Animation Chief"
drawTextImg 35 Teko-Regular    0  78  "Kazunori Ariga"

drawTextImg 36 Teko-SemiBold   0  38  "Animation"
drawTextImg 37 Teko-Regular    0  68  "Masako Tanaka"
drawTextImg 37 Teko-Regular    0  94  "Akihisa Sasaki"

drawTextImg 38 Teko-SemiBold   0  38  "Sub Character Design"
drawTextImg 39 Teko-Regular    0  78  "Masako Tanaka"

drawTextImg 40 Teko-SemiBold   0  19  "2D Design"
drawTextImg 41 Teko-Regular    0  50  "Miki Miura"
drawTextImg 41 Teko-Regular    0  74  "Megumi Aikawa"
drawTextImg 41 Teko-Regular    0  98 "Sho Saito"
drawTextImg 41 Teko-Regular    0  122 "Jun Iwai"

drawTextImg 42 Teko-SemiBold   0  38  "2D Producer"
drawTextImg 43 Teko-Regular    0  78  "Toshihiro Honda"

drawTextImg 44 Teko-SemiBold   0  38  "Producer/Sandlot"
drawTextImg 45 Teko-Regular    0  78  "Shiro Kuroda"

drawTextImg 46 Teko-SemiBold   0  19  "Special Thanks"
drawTextImg 47 Teko-Regular    0  50  "Grasshopper Manufacture"
drawTextImg 47 Teko-Regular    0  74  "Studio A-CAT"
drawTextImg 47 Teko-Regular    0  98  "Jams Works"
drawTextImg 47 Teko-Regular    0  122 "Cyclone Entertainment"

############ ORIGINAL CREDITS #############
# drawTextImg 48 Teko-SemiBold   0  38  "Debug Support"
# drawTextImg 49 Teko-Regular    0  68  "Shuichi Narisawa"
# drawTextImg 49 Teko-Regular    0  94  "Shintaro Jikumaru"
# 
# drawTextImg 50 Teko-SemiBold   0  38  "Debug"
# drawTextImg 51 Teko-Regular    0  78  "Super Mario Club"
# 
# drawTextImg 52 Teko-SemiBold   0  38  "Planning and Development"
# drawTextImg 53 Teko-Regular    0  78  "Sandlot"
# 
# drawTextImg 54 Teko-SemiBold   0  38  "Executive Producer"
# drawTextImg 55 Teko-Regular    0  78  "Satoru Iwata"
# 
# drawTextImg 56 Teko-SemiBold   0  38  "Production Copyright"
# drawTextImg 57 Teko-Regular    0  78  "Nintendo Co., Ltd."

############ NEW CREDITS #############
# concatenate the two debugging-related slides so we can add one for
# the translation credits.
# cruder than i'd like, but good enough.

drawTextImg 48 Teko-SemiBold   0  3  "Debug Support"
drawTextImg 49 Teko-Regular    0  34  "Shuichi Narisawa"
drawTextImg 49 Teko-Regular    0  62  "Shintaro Jikumaru"
drawTextImg 48 Teko-SemiBold   0  100  "Debug"
drawTextImg 49 Teko-Regular    0  132  "Super Mario Club"

drawTextImg 50 Teko-SemiBold   0  38  "Planning and Development"
drawTextImg 51 Teko-Regular    0  78  "Sandlot"

drawTextImg 52 Teko-SemiBold   0  38  "Executive Producer"
drawTextImg 53 Teko-Regular    0  78  "Satoru Iwata"

drawTextImg 54 Teko-SemiBold   0  38  "Production Copyright"
drawTextImg 55 Teko-Regular    0  78  "Nintendo Co., Ltd."

#drawTextImg 56 Teko-SemiBold   0  13  "Hacking"
#drawTextImg 57 Teko-Regular    0  44  "Supper"
#drawTextImg 56 Teko-SemiBold   0  75  "Translation and Art"
#drawTextImg 57 Teko-Regular    0  107  "Phantom"
drawTextImg 56 Teko-SemiBold   0  3  "Hacking"
drawTextImg 57 Teko-Regular    0  34  "Supper"
drawTextImg 56 Teko-SemiBold   0  65  "Translation and Art"
drawTextImg 57 Teko-Regular    0  96  "Phantom"
drawTextImg 56 Teko-SemiBold   0  127  "Testing"
drawTextImg 57 Teko-Regular    0  158  "cccmar"

##########################
# Generate outline around raw credits text images
##########################

echo "Generating staff roll text outline"

mkdir -p $BUILDFILES_DIR/credits_outlined
cp images/kar/img/StaffText/* -t $BUILDFILES_DIR/credits_outlined
cp $BUILDFILES_DIR/credits_generated/* -t $BUILDFILES_DIR/credits_outlined
for file in $BUILDFILES_DIR/credits_outlined/*.bin-grp.png; do
  filebase=$(basename $file)
#  echo $filebase
#  outfile=$BUILDFILES_DIR/credits_outlined/$filebase
  
  echo "  Outlining image: $file"
  outlineSolidPixels $file $file
done


# images 58/59 are not supposed to be outlined;
# overwrite with original files
cp images/kar/img/StaffText/58.bin-grp.png $BUILDFILES_DIR/credits_outlined
cp images/kar/img/StaffText/59.bin-grp.png $BUILDFILES_DIR/credits_outlined
  
