#!/bin/bash

FFMPEG_DIR=/home/stanislaw/private/msc/ffmpeg
RESULTS_DIR=results

COLOR_RED=$(tput setaf 1)
COLOR_GREEN=$(tput setaf 2)
COLOR_NORMAL=$(tput sgr0)

test_single_file() {
	FILE_NAME=$1

	printf "\n-----------------------------------------------------------\n"
	printf "Test compressing file: %s\n\n" ${FILE_NAME}

	printf "\\\\begin{table}[h]\small: \n\\centering \n\\\\begin{tabular}{cl|cccc} \nqscale & kodek & bitrate & PSNR Y & PSNR \$C_{b}\$ & PSNR \$C_{r}\$ & PSNR \\\\\\\\\n"
	for QSCALE in 2 4 8 16
	do
		echo \\hline
		printf "\\multirow{3}{*}{%d}\n" ${QSCALE}
		for CODEC_NAME in 'mjpeg' 'msc' 'mpeg4'
		do
			ENCODED_FILE_NAME=results/${FILE_NAME%.*}_${CODEC_NAME}_${QSCALE}.mkv
			DECODED_FILE_NAME=results/${FILE_NAME%.*}_${CODEC_NAME}_${QSCALE}_decoded.avi

			# encode video
			if [[ ${CODEC_NAME} == 'mjpeg' ]]; then
				$FFMPEG_DIR/ffmpeg -y -i resources/${FILE_NAME} -qmin ${QSCALE} -qmax ${QSCALE} -vcodec ${CODEC_NAME} ${ENCODED_FILE_NAME} 2> /dev/null
			else
				$FFMPEG_DIR/ffmpeg -y -i resources/${FILE_NAME} -qscale ${QSCALE} -vcodec ${CODEC_NAME} ${ENCODED_FILE_NAME} 2> /dev/null
			fi

			# extract bitrate
			BITRATE=`$FFMPEG_DIR/ffmpeg -i ${ENCODED_FILE_NAME} 2>&1 | grep bitrate | awk '{ print $6; }'`
			
			# decode video
			$FFMPEG_DIR/ffmpeg -y -i ${ENCODED_FILE_NAME} -vcodec rawvideo -pix_fmt yuv420p ${DECODED_FILE_NAME} 2> /dev/null

			# calculate PSNR
			PSNR_ROW=`./diff_viewer -calc resources/${FILE_NAME} ${DECODED_FILE_NAME}`

			# output as latex table row
			printf " & %5s & %10s & " ${CODEC_NAME} ${BITRATE}
			echo ${PSNR_ROW}
		done
	done
	printf "\\\\end{tabular} \n\\caption{Wyniki testów kompresji dla sekwencji ${FILE_NAME}.} \n\\label{resultTab${FILE_NAME}} \n\\\\end{table}\n\n"
	printf "\0-----------------------------------------------------------\n"
}

print_result() {
	if [ $? -eq 0 ]; then
		printf "%s\n" "${COLOR_GREEN}${1}${COLOR_NORMAL}"
		return 0;
	else
		printf "%s\n" "${COLOR_RED}${2}${COLOR_NORMAL}"
		return 1;
	fi
}

if [ ! -d ${RESULTS_DIR} ]; then
	echo "Creating output directory"
	mkdir -p ${RESULTS_DIR}
fi

echo "Compressing files using ${CODEC_NAME} coder in mkv container"
for FILE_NAME in `ls resources/*.avi`
do	
	FILE_NAME=`basename ${FILE_NAME}`
	test_single_file $FILE_NAME
done

