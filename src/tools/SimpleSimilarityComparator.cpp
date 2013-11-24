/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const char* AVI_EXTENSION = "avi";
const float DIFF_THRESHOLD = 0.01;

void printUsage(string &programName);
bool isImageSequenceExtension(string &extension);

int main(int argc, char ** argv) {
    string programName(argv[0]);
    int s = programName.find_last_of("\\/");
    programName = programName.substr(s + 1, programName.length() - s - 1);

    if(argc != 2) {
        printUsage(programName);
        return 1;
    }

    string fileName(argv[1]);
    s = fileName.find_last_of("\\.");
    string extension = fileName.substr(s + 1, fileName.length() - s - 1);

    if (isImageSequenceExtension(extension)) {
        CvCapture *capture;

        capture = cvCaptureFromAVI(fileName.c_str());

        int frameH     = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
        int frameW     = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
        int numFrames  = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
        int fps        = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
        fps = 10;

        IplImage *prevImage = cvCreateImage(cvSize(frameW, frameH), IPL_DEPTH_8U, 3);
        IplImage *helperImage = cvCreateImage(cvSize(frameW, frameH), IPL_DEPTH_8U, 3);

        cvNamedWindow("Simple similarity comparator", CV_WINDOW_AUTOSIZE);

        CvFont font;
        char textBuff[80];
        double hScale=0.5;
        double vScale=0.5;
        int    lineWidth=1;
        cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

        int frame = 0;
        while(++frame <= numFrames) {
            IplImage *frameImg = cvQueryFrame(capture);

            cvCvtColor(frameImg, helperImage, CV_RGB2YCrCb);

            if (frame > 1) {
                float totalDiff = 0.0;
                for(int x = 0; x < frameW; ++x) {
                    for(int y = 0; y < frameH; ++y) {
                        unsigned char value1 = helperImage->imageData[y*helperImage->widthStep + x*helperImage->nChannels];
                        unsigned char value2 = prevImage->imageData[y*prevImage->widthStep + x*prevImage->nChannels];

                        float diff = ((float) value2 - value1) / 255;
                        if (diff > DIFF_THRESHOLD) {
                            totalDiff += 1.0;
                        }
                    }
                }

                totalDiff = totalDiff / (frameW * frameH);
                totalDiff *= 100;
                cout << frame << " : " << totalDiff << std::endl;

                sprintf(textBuff, "%f", totalDiff);
                cvPutText (frameImg, textBuff, cvPoint(5,20), &font, cvScalar(0,0,255));
                cvShowImage("Simple similarity comparator", frameImg);

                cvWaitKey(1000 / fps);
            }

            cvCopy(helperImage, prevImage);
        }

        cvReleaseImage(&prevImage);
        cvReleaseCapture(&capture);
        cvDestroyWindow("Simple similarity comparator");
    } else {
        printUsage(programName);
        return 1;
    }

    return 0;
}

void printUsage(string &programName) {
    cerr << "Usage: " << programName << " <input file name>" << endl << endl;
}

bool isImageSequenceExtension(string &extension) {
    return extension == AVI_EXTENSION;
}
