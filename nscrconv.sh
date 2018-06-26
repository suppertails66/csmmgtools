convert ${1}-grp.png -dither None -remap ${1}-pal.png PNG32:${1}-grp.png
dsimgconv ps ${1} -n ${2} -t ${2}
