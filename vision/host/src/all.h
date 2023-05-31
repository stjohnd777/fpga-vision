#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include "xcl2.hpp"

#include "PLFast.h"
#include "PLMyThresholding.h"
#include "PLThreshold.h"
#include "PLRemap.h"
#include "PLSobel.h"
#include "PlGaussianFilter.h"
#include "PLMedian.h"

#include "ImageServer.h"

#include "net/net.h"
#include "camera/Camera.h"


#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "PLBilateralFilter.h"
using namespace cv;


#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <tuple>
#include <algorithm>
#include <vector>
#include <map>
#include <memory>
using namespace std;
