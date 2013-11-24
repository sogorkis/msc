#!/bin/bash

RESOURCES_DIR=resources
TMP_DIR=tmp

COLOR_GREEN=$(tput setaf 2)
COLOR_NORMAL=$(tput sgr0)

DOWNLOAD_URL="http://media.xiph.org/video/derf/y4m/"
DOWNLOAD_FILES=("akiyo_qcif.y4m"
		"news_qcif.y4m"
		"bowing_qcif.y4m"
		"coastguard_qcif.y4m"
		"bridge_close_qcif.y4m"
		"mother_daughter_qcif.y4m"
		"foreman_qcif.y4m"
		"suzie_qcif.y4m"
		"silent_qcif.y4m"
		"container_qcif.y4m")

print_green() {
	printf "%s\n" "${COLOR_GREEN}${1}${COLOR_NORMAL}"
}

if [ ! -d ${RESOURCES_DIR} ]; then
	print_green "Creating resources dir"
	mkdir -p ${RESOURCES_DIR}
fi
if [ ! -d ${TMP_DIR} ]; then
	print_green "Creating tmp dir"
	mkdir -p ${TMP_DIR}
fi

print_green "Downloading resources"
for FILE_NAME in ${DOWNLOAD_FILES[@]}
do
	wget -O tmp/${FILE_NAME} ${DOWNLOAD_URL}${FILE_NAME}
	print_green "Downloaded ${FILE_NAME}"
done

print_green "Converting to raw i420 avi"
for FILE_NAME in ${DOWNLOAD_FILES[@]}
do
	ffmpeg -i tmp/${FILE_NAME} -vcodec rawvideo resources/${FILE_NAME%.*}.avi
	mv tmp/${FILE_NAME} resources/${FILE_NAME}
	print_green "Converted ${FILE_NAME}"
done

print_green "Cleanup"
rm -r ${TMP_DIR}
