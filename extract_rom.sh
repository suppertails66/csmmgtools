#./ndstool -v -x cmmg.nds -9 cmmg_re/arm9.bin -7 cmmg_re/arm7.bin -y9 cmmg_re/y9.bin -y7 cmmg_re/y7.bin -d cmmg_re/data -y cmmg_re/overlay -t cmmg_re/banner.bin -h cmmg_re/header.bin -o cmmg_re/logo.bin

set -o errexit

FILENAME=$1
FILEBASE=$(basename "$FILENAME" .nds)
FOLDERBASE="$2"

#echo $FILENAME
#echo $FILEBASE
#echo $FOLDERBASE

mkdir -p "$FOLDERBASE"
#mkdir -p "$FOLDERBASE/data"
#mkdir -p "$FOLDERBASE/overlay"

#./ndstool -v -x "cmmg.nds" -9 cmmg_re/arm9.bin -7 cmmg_re/arm7.bin -y9 cmmg_re/y9.bin -y7 cmmg_re/y7.bin -d cmmg_re/data -y cmmg_re/overlay -t cmmg_re/banner.bin -h cmmg_re/header.bin -o cmmg_re/logo.bin
./ndstool/ndstool -v -x "$FILENAME" -9 "$FOLDERBASE/arm9.bin" -7 "$FOLDERBASE/arm7.bin" -y9 "$FOLDERBASE/y9.bin" -y7 "$FOLDERBASE/y7.bin" -d "$FOLDERBASE/data" -y "$FOLDERBASE/overlay" -t "$FOLDERBASE/banner.bin" -h "$FOLDERBASE/header.bin" -o "$FOLDERBASE/logo.bin"

