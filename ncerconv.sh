convert ${1}-img.png -dither None -remap ${1}-pal.png PNG32:${1}-img.png
dsimgconv p ${1} -n ${2} -t ${2}
