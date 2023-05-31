#pragma once
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

cv::Mat plRemapMono8(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y);
cv::Mat plRemapMonoR16(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y);
cv::Mat plRemapMonoL16(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y);

// example map generation, flips the image horizontally
void CreateRemapDefinitionFlipHorz(cv::Mat& src, cv::Mat& map_x,
		cv::Mat& map_y);

void CreateRemapDefinitionFlipHorz(int rows, int cols, cv::Mat& map_x,
		cv::Mat& map_y) ;

// TODO : properties file for location mapX and mapY files
std::tuple<cv::Mat, cv::Mat> GetRemapXYMapsFromFiles(int rows, int cols,
		char* mapx = "data/mapx.out", char* mapy = "data/mapy.out") ;
