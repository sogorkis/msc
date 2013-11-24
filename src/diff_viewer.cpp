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
#include <iomanip>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const int MAX_WIDTH = 700;
const char* AVI_EXTENSION = "avi";
const char* PGM_EXTENSION = "pgm";
const char* BMP_EXTENSION = "bmp";
const char* JPG_EXTENSION = "jpg";
const char* TIFF_EXTENSION = "tiff";

void printUsage(string &programName);
bool isImageExtension(string &extension);
bool isImageSequenceExtension(string &extension);
void resizeImagesIfNecessary(IplImage **img1, IplImage **img2);
void calculateMeanSquareError(IplImage *img1, IplImage *img2, double *result, IplImage *tmpImg1, IplImage *tmpImg2);
void copyImages(IplImage *srcImage1, IplImage *srcImage2, IplImage *dstImage);
void extractDiff(IplImage *srcImage1, IplImage *srcImage2);
void printPSNR(double *mse, int nChannels);

int main(int argc, char ** argv) {
    string programName(argv[0]);
    int s = programName.find_last_of("\\/");
    programName = programName.substr(s + 1, programName.length() - s - 1);
    bool diff = false, calc = false;

    if(argc == 3 || argc == 4) {
        if (argc == 4) {
            string diffStr(argv[1]);
            if (diffStr == "-diff") {
                diff = true;
            }
            else if (diffStr == "-calc") {
                calc = true;
            }
            else {
                printUsage(programName);
                return 1;
            }
        }
    } else {
        printUsage(programName);
        return 1;
    }

    string fileName1(argv[argc - 2]);
    string fileName2(argv[argc - 1]);
    int s1 = fileName1.find_last_of("\\.");
    int s2 = fileName2.find_last_of("\\.");
    string extension1 = fileName1.substr(s1 + 1, fileName1.length() - s1 - 1);
    string extension2 = fileName2.substr(s2 + 1, fileName2.length() - s2 - 1);

    if (isImageExtension(extension1) && isImageExtension(extension2)) {
        IplImage *image1 = cvLoadImage(fileName1.c_str(), CV_LOAD_IMAGE_COLOR);
        if(image1 == NULL) {
            cerr <<  "Cannot open file " << fileName1 << "!" << endl;
            return 1;
        }

        IplImage *image2 = cvLoadImage(fileName2.c_str(), CV_LOAD_IMAGE_COLOR);
        if(image2 == NULL) {
            cvReleaseImage(&image1);
            cerr <<  "Cannot open file " << fileName2 << "!" << endl;
            return 1;
        }

        if(image1->nChannels != image2->nChannels) {
            cvReleaseImage(&image1);
            cvReleaseImage(&image2);
            cerr << "Invalid channel count (" << fileName1 << "=" << image1->nChannels << ", " << fileName2 << "="
                    << image2->nChannels << ")!";
            return 1;
        }

        if(image1->width != image2->width || image1->height != image2->height) {
            cvReleaseImage(&image1);
            cvReleaseImage(&image2);
            cerr << "Images should have the same sizes!" << endl;
            return 1;
        }

        int width = image1->width, height = image1->height, depth = image1->depth;
        IplImage *helperImage = cvCreateImage(cvSize(2 * width, height), depth, image1->nChannels);
        IplImage *tmpImage1 = cvCreateImage(cvSize(width, height), depth, image1->nChannels);
        IplImage *tmpImage2 = cvCreateImage(cvSize(width, height), depth, image1->nChannels);


        double *mse = new double[image1->nChannels + 1];
        calculateMeanSquareError(image1, image2, mse, tmpImage1, tmpImage2);
        printPSNR(mse, image1->nChannels);

        if (!calc) {

            resizeImagesIfNecessary(&image1, &image2);

            if (diff) {
                extractDiff(image1, image2);
            }

            copyImages(image1, image2, helperImage);

            cvNamedWindow("Transform viewer", CV_WINDOW_AUTOSIZE);
            cvShowImage("Transform viewer", helperImage);

            cvWaitKey(0);

            cvDestroyWindow("Transform viewer");
        }

        cvReleaseImage(&image1);
        cvReleaseImage(&image2);
        cvReleaseImage(&helperImage);
        delete [] mse;
    }
    else if (isImageSequenceExtension(extension1) && isImageSequenceExtension(extension2)) {
        CvCapture *capture1;
        CvCapture *capture2;

        capture1 = cvCaptureFromAVI(fileName1.c_str());

        int frameH1    = (int) cvGetCaptureProperty(capture1, CV_CAP_PROP_FRAME_HEIGHT);
        int frameW1    = (int) cvGetCaptureProperty(capture1, CV_CAP_PROP_FRAME_WIDTH);
        int numFrames1 = (int) cvGetCaptureProperty(capture1, CV_CAP_PROP_FRAME_COUNT);
        int fps        = (int) cvGetCaptureProperty(capture1, CV_CAP_PROP_FPS);

        capture2 = cvCaptureFromAVI(fileName2.c_str());

        int frameH2    = (int) cvGetCaptureProperty(capture2, CV_CAP_PROP_FRAME_HEIGHT);
        int frameW2    = (int) cvGetCaptureProperty(capture2, CV_CAP_PROP_FRAME_WIDTH);
        int numFrames2 = (int) cvGetCaptureProperty(capture2, CV_CAP_PROP_FRAME_COUNT);

        if (frameH1 != frameH2 || frameW1 != frameW2 || numFrames1 != numFrames2) {
            cvReleaseCapture(&capture1);
            cvReleaseCapture(&capture2);
            cerr << "Image sequences should have the same sizes!" << endl;
            return 1;
        }

        int width = frameW1, height = frameH1;
        if(width > MAX_WIDTH) {
            float ratio = (float) width / MAX_WIDTH;
            width = MAX_WIDTH;
            height = height / ratio;
        }

        IplImage *helperImage = cvCreateImage(cvSize(2 * width, height), IPL_DEPTH_8U, 3);
        IplImage *tmpImage1 = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
        IplImage *tmpImage2 = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

        cvNamedWindow("Transform viewer", CV_WINDOW_AUTOSIZE);

        double *mse = new double[4], *totalMse = new double[4];
        for(int i = 0; i < 4; ++i) {
            totalMse[i] = 0.0;
        }
        int frame = 0, key = 0;
        while(++frame < numFrames1 && key != 'q') {
            IplImage *image1 = cvQueryFrame(capture1);
            IplImage *image2 = cvQueryFrame(capture2);

            calculateMeanSquareError(image1, image2, mse, tmpImage1, tmpImage2);
            for(int i = 0; i < 4; ++i) {
                totalMse[i] += mse[i];
            }

            if (!calc) {
                resizeImagesIfNecessary(&image1, &image2);

                if (diff) {
                    extractDiff(image1, image2);
                }

                copyImages(image1, image2, helperImage);

                cvShowImage("Transform viewer", helperImage);

                key = cvWaitKey(1000 / fps);
            }
        }

        for(int i = 0; i < 4; ++i) {
            totalMse[i] /= numFrames1;
        }
        printPSNR(totalMse, 3);

        cvReleaseImage(&helperImage);
        cvReleaseCapture(&capture1);
        cvReleaseCapture(&capture2);
        cvDestroyWindow("Transform viewer");
        delete [] mse;
        delete [] totalMse;
    } else {
        printUsage(programName);
        return 1;
    }

    return 0;
}

void printUsage(string &programName) {
    cerr << "Usage: " << programName << " [-diff | -calc] <input file name 1> <input file name 2>" << endl << endl;
    cerr << "For images: " << programName
            << " [-diff] <ogrinal image>.[jpg|pgm|bmp|tiff] <reconstructed image>.[jpg|pgm|bmp|tiff]" << endl;
    cerr << "For image sequences: " << programName
            << " [-diff] <orginal image sequence>.avi <reconstructed image sequence>.avi" << endl;
}

bool isImageExtension(string &extension) {
    return extension == JPG_EXTENSION || extension == BMP_EXTENSION || extension == PGM_EXTENSION
            || extension == TIFF_EXTENSION;
}

bool isImageSequenceExtension(string &extension) {
    return extension == AVI_EXTENSION;
}

void resizeImagesIfNecessary(IplImage **image1, IplImage **image2) {
    if((*image1)->width > MAX_WIDTH) {
        float ratio = (float) (*image1)->width / MAX_WIDTH;
        int newHeight = (*image1)->height / ratio;
        IplImage *tmpImg1 = cvCreateImage(cvSize(MAX_WIDTH, newHeight), (*image1)->depth, (*image1)->nChannels);
        IplImage *tmpImg2 = cvCreateImage(cvSize(MAX_WIDTH, newHeight), (*image1)->depth, (*image1)->nChannels);

        cvResize(*image1, tmpImg1);
        cvResize(*image2, tmpImg2);

        cvReleaseImage(image1);
        cvReleaseImage(image2);

        *image1 = tmpImg1;
        *image2 = tmpImg2;
    }
}

void copyImages(IplImage *srcImage1, IplImage *srcImage2, IplImage *dstImage) {
    cvSetImageROI(dstImage, cvRect(0,0, srcImage1->width, srcImage1->height));
    cvResize(srcImage1, dstImage);
    cvResetImageROI(dstImage);

    cvSetImageROI(dstImage, cvRect(srcImage1->width,0, srcImage2->width, srcImage2->height));
    cvResize(srcImage2, dstImage);
    cvResetImageROI(dstImage);
}

void calculateMeanSquareError(IplImage *img1, IplImage *img2, double *result, IplImage *tmpImg1, IplImage *tmpImg2) {
    if (img1->nChannels == 3) {
        cvCvtColor(img1, tmpImg1, CV_RGB2YCrCb);
        cvCvtColor(img2, tmpImg2, CV_RGB2YCrCb);
    } else {
        tmpImg1 = img1;
        tmpImg2 = img2;
    }

    result[img1->nChannels] = 0.0f;

    for(int i = 0; i < img1->nChannels; ++i) {
        result[i] = 0.0;

        for(int x = 0; x < img1->width; ++x) {
            for(int y = 0; y < img1->height; ++y) {
                unsigned char value1 = tmpImg1->imageData[y*img1->widthStep + x*img1->nChannels + i];
                unsigned char value2 = tmpImg2->imageData[y*img2->widthStep + x*img2->nChannels + i];
                int diff = value2 - value1;
                result[i] += diff * diff;

                if (i == 0 && img1->nChannels == 3) {
                    unsigned char B1 = img1->imageData[y*img1->widthStep + x*img1->nChannels];
                    unsigned char B2 = img2->imageData[y*img2->widthStep + x*img2->nChannels];
                    unsigned char G1 = img1->imageData[y*img1->widthStep + x*img1->nChannels + 1];
                    unsigned char G2 = img2->imageData[y*img2->widthStep + x*img2->nChannels + 1];
                    unsigned char R1 = img1->imageData[y*img1->widthStep + x*img1->nChannels + 2];
                    unsigned char R2 = img2->imageData[y*img2->widthStep + x*img2->nChannels + 2];

                    int gray1 = (B1 + G1 + R1) / 3;
                    int gray2 = (B2 + G2 + R2) / 3;

                    int diff = gray2 - gray1;

                    result[img1->nChannels] += diff * diff;
                }
            }
        }

        result[i] /= (img1->width * img1->height);
    }

    result[img1->nChannels] /= (img1->width * img1->height);
}

void extractDiff(IplImage *srcImage1, IplImage *srcImage2) {
    for(int i = 0; i < srcImage1->nChannels; ++i) {
        for(int x = 0; x < srcImage1->width; ++x) {
            for(int y = 0; y < srcImage1->height; ++y) {
                unsigned char value1 = srcImage1->imageData[y*srcImage1->widthStep + x*srcImage1->nChannels + i];
                unsigned char value2 = srcImage2->imageData[y*srcImage2->widthStep + x*srcImage2->nChannels + i];
                int value = value2 - value1;
                value = value < 0 ? -value : value;
                value = value > 255 ? 255 : value;
                srcImage2->imageData[y*srcImage2->widthStep + x*srcImage2->nChannels + i] = value;
            }
        }
    }
}


void printPSNR(double *mse, int nChannels) {
    cout.setf(std::ios::fixed);
    for(int i = 0; i < nChannels; ++i) {
        double psnr = 20.0 * log(255.0/sqrt(mse[i]))/log(10.0);
       cout << std::setprecision(2) << psnr << " & ";
    }
    double totalPSNR = 20.0 * log(255.0/sqrt(mse[nChannels]))/log(10.0);
    cout << std::setprecision(2) << totalPSNR << " \\\\" << endl;
}

