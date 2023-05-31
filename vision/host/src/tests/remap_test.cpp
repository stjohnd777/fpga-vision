#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "../PLRemap.h"

// example map generation, flips the image horizontally
void CreateRemapDefinitionFlipHorz(cv::Mat& src, cv::Mat& map_x, cv::Mat& map_y){
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            float valx = (float)(src.cols - j - 1);
            float valy = (float)i;
            map_x.at<float>(i, j) = valx;
            map_y.at<float>(i, j) = valy;
        }
    }
}

int remap_test(std::string imagePath, int N){


    cv::Mat in = cv::imread(imagePath, 0);
    cv::Mat map_x(in.rows, in.cols, CV_32FC1);
    cv::Mat map_y(in.rows, in.cols, CV_32FC1);
    CreateRemapDefinitionFlipHorz(in,map_x, map_y);

    std::cout << "-------------------------------------------------------------" << std::endl;
    std::cout << "INFO: Run HLS remap()" << std::endl;
    auto start2 = std::chrono::steady_clock::now();
    for ( int i =0 ; i < N; i++){
    	cv::Mat out = plRemap(in, map_x, map_y);
    }
    auto end2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds2 = end2-start2;
    std::cout << "elapsed time: " << elapsed_seconds2.count() << "s\n";
    std::cout << "-------------------------------------------------------------" << std::endl;



    // OpenCV Reference: START TIMMER OPENCV
    cv::Mat ocv_remapped(in.rows, in.cols, in.type());
    std::cout << "-------------------------------------------------------------" << std::endl;
    std::cout << "INFO: Run OpenCV remap()" << std::endl;
    auto start = std::chrono::steady_clock::now();
    for ( int i =0 ; i < N; i++){
    	cv::remap(in, ocv_remapped, map_x, map_y, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
    std::cout << "-------------------------------------------------------------" << std::endl;
    // END TIMMER OPENCV
}
