
#include <stdint.h>
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


int find_video_stream(AVFormatContext * formatCtx);
int read_packet(AVFormatContext *formatCtx, AVCodecContext *codecCtx, AVFrame *frame, int videoStream);
void calc_mse(AVFrame *frame1, AVFrame *frame2, double mse[3]);
void printPSNR(double mse[4]);
void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

int main(int argc, char ** argv) {
	char * path1, * path2;
	int videoStream1, videoStream2;
	int frameRead1, frameRead2, frameIndex;
	int width, height, numBytes;
	AVFormatContext *formatCtx1, *formatCtx2;
	AVCodecContext *codecCtx1, *codecCtx2;
	AVCodec *codec1, *codec2;
	AVFrame *frame1, *frame2, *frameTmp;
	struct SwsContext *swsCtx1, *swsCtx2;
	double mse[4], totalMse[4];
	unsigned char *buffer1, *buffer2;

	if (argc != 3) {
		printf("Usage:\ndiff_video file1 file2\n");
		return EXIT_FAILURE;
	}

	path1 = argv[1];
	path2 = argv[2];

	av_register_all();

	formatCtx1 = avformat_alloc_context();
	formatCtx2 = avformat_alloc_context();

	if (avformat_open_input(&formatCtx1, path1, NULL, NULL)) {
		printf("Could not open file %s\n", path1);
		return EXIT_FAILURE;
	}
	if (avformat_open_input(&formatCtx2, path2, NULL, NULL)) {
		printf("Could not open file %s\n", path2);
		return EXIT_FAILURE;
	}

	av_dump_format(formatCtx1, 0, path1, 0);
	av_dump_format(formatCtx2, 0, path2, 0);

	if ((videoStream1 = find_video_stream(formatCtx1)) < 0) {
		printf("Could not video stream in file %s\n", path1);
		return EXIT_FAILURE;
	}
	if ((videoStream2 = find_video_stream(formatCtx2)) < 0) {
		printf("Could not video stream in file %s\n", path2);
		return EXIT_FAILURE;
	}

	codecCtx1 = formatCtx1->streams[videoStream1]->codec;
	codecCtx2 = formatCtx2->streams[videoStream2]->codec;

	codec1 = avcodec_find_decoder(codecCtx1->codec_id);
	codec2 = avcodec_find_decoder(codecCtx2->codec_id);

	if (avcodec_open2(codecCtx1, codec1, NULL) < 0) {
		printf("Could not video open codec for file %s\n", path1);
		return EXIT_FAILURE;
	}
	if (avcodec_open2(codecCtx2, codec2, NULL) < 0) {
		printf("Could not video open codec for file %s\n", path2);
		return EXIT_FAILURE;
	}
	
	if (codecCtx1->width != codecCtx2->width || codecCtx1->height != codecCtx2->height) {
		printf("Video sequence dimensions non equal\n");
		return EXIT_FAILURE;
	}
	width = codecCtx1->width;
	height = codecCtx1->height;

	if (!(frame1 = avcodec_alloc_frame())) {
		printf("Error while allocating frame for file %s\n", path1);
		return EXIT_FAILURE;
	}
	if (!(frame2 = avcodec_alloc_frame())) {
		printf("Error while allocating frame for file %s\n", path2);
		return EXIT_FAILURE;
	}
	if (!(frameTmp = avcodec_alloc_frame())) {
		printf("Error while allocating temp frame\n");
		return EXIT_FAILURE;
	}

	numBytes = avpicture_get_size(PIX_FMT_YUV420P, width, height);
	buffer1 = malloc(numBytes);
	buffer2 = malloc(numBytes);

	avpicture_fill((AVPicture *) frame1, buffer1, PIX_FMT_YUV420P, width, height);
	avpicture_fill((AVPicture *) frame2, buffer2, PIX_FMT_YUV420P, width, height);

	frame1->width = frame2->width = width;
	frame1->height = frame2->height = height;

	swsCtx1 = sws_getContext(width, height, codecCtx1->pix_fmt, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	swsCtx2 = sws_getContext(width, height, codecCtx2->pix_fmt, width, height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	frameIndex = 0;
	for (int i = 0; i < 4; ++i) {
		totalMse[i] = 0.0;
	}

	while (1) {
		frameRead1 = read_packet(formatCtx1, codecCtx1, frameTmp, videoStream1);

		if (frameRead1 > 0) {
			sws_scale(swsCtx1, frameTmp->data, frameTmp->linesize, 0, height, frame1->data, frame1->linesize);
		}

		frameRead2 = read_packet(formatCtx2, codecCtx2, frameTmp, videoStream2);

		if (frameRead2 > 0) {
			sws_scale(swsCtx2, frameTmp->data, frameTmp->linesize, 0, height, frame2->data, frame2->linesize);
		}

		if (frameRead1 < 0 || frameRead2 < 0) {
			printf("Error while reading packet\n");
			return EXIT_FAILURE;
		}

		if (!frameRead1 || !frameRead2) {
			break;
		}

//		if (frameIndex == 0) {
//			pgm_save(frame1->data[0], frame1->linesize[0], frame1->width, frame1->height, "/tmp/1.pgm");
//			pgm_save(frame2->data[0], frame2->linesize[0], frame2->width, frame2->height, "/tmp/2.pgm");
//		}

		calc_mse(frame1, frame2, mse);
		printPSNR(mse);
		for (int i = 0; i < 4; ++i) {
			totalMse[i] += mse[i];
		}
	}
	printPSNR(totalMse);

	if (frameRead1 || frameRead2) {
		printf("One video finished before another!\n");
		return EXIT_FAILURE;
	}

	return 0;
}

int find_video_stream(AVFormatContext * formatCtx) {
	for (int i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			return i;
		}
	}
	return -1;
}

int read_packet(AVFormatContext *formatCtx, AVCodecContext *codecCtx, AVFrame *frame, int videoStream) {
	AVPacket packet;
	av_init_packet(&packet);
	while(av_read_frame(formatCtx, &packet) >= 0) {
		if (packet.stream_index == videoStream) {
			int frameFinished = 0;

			avcodec_get_frame_defaults(frame);

			if (avcodec_decode_video2(codecCtx, frame, &frameFinished, &packet) < 0) {
				return -1;
			}

			if (frameFinished) {
				av_free_packet(&packet);
				return 1;
			}
		}
	}
	return 0;
}

void calc_mse(AVFrame *frame1, AVFrame *frame2, double mse[4]) {
	int diffV, diffU, diffY1, diffY2, diffY3, diffY4;
	double errorY, errorU, errorV;
	unsigned char *y1, *y2, *u1, *u2, *v1, *v2;

	errorY = errorU = errorV = 0;

	y1 = frame1->data[0];
	u1 = frame1->data[1];
	v1 = frame1->data[2];
	y2 = frame2->data[0];
	u2 = frame2->data[1];
	v2 = frame2->data[2];

	for (int i = 0; i < frame1->height + 1 >> 1; i++) {
		for (int j = 0; j < frame1->width + 1 >> 1; j++) {
			diffU = u1[j] - u2[j];
			diffV = v1[j] - v2[j];
			diffY1 = y1[                      2 * j    ] - y2[                      2 * j    ];
			diffY2 = y1[                      2 * j + 1] - y2[                      2 * j + 1];
			diffY3 = y1[frame1->linesize[0] + 2 * j    ] - y2[frame2->linesize[0] + 2 * j    ];
			diffY4 = y1[frame1->linesize[0] + 2 * j + 1] - y2[frame2->linesize[0] + 2 * j + 1];

			errorU += diffU * diffU;
			errorV += diffV * diffV;
			errorY += diffY1 * diffY1;
			errorY += diffY2 * diffY2;
			errorY += diffY3 * diffY3;
			errorY += diffY4 * diffY4;
		}
		y1 += 2 * frame1->linesize[0];
		u1 +=     frame1->linesize[1];
		v1 +=     frame1->linesize[2];
		y2 += 2 * frame2->linesize[0];
		u2 +=     frame2->linesize[1];
		v2 +=     frame2->linesize[2];
	}

	mse[0] = errorY / (frame1->height * frame1->width);
	mse[1] = errorU / ((frame1->height * frame1->width) / 4);
	mse[2] = errorV / ((frame1->height * frame1->width) / 4);
	mse[3] = (errorY + errorV + errorU) / (frame1->height * frame1->width * 1.5);
}

void printPSNR(double mse[4]) {
	double totalMse = mse[3];
	for(int i = 0; i < 3; ++i) {
		totalMse += mse[i];
		double psnr = 20.0 * log(255.0/sqrt(mse[i]))/log(10.0);
		printf("PSNR[%d] = %lf\n", i, psnr);
	}
	double totalPSNR = 20.0 * log(255.0/sqrt(totalMse))/log(10.0);
	printf("Total PSNR = %lf\n", totalPSNR);
}

void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename) {
    FILE *f;
    int i;

    f=fopen(filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}

