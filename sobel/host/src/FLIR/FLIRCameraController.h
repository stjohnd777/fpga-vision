//
// Created by e438262 on 5/3/2023.
//

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

#include <opencv2/core.hpp>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
namespace lmc {

    template<typename T>
	struct Response
    {
        bool isSuccess = false;
        T    payload;
        int  errorCode = -1;
        char errorMessage[256] = { 0 };

        void setErrorMessage(string str)
        {
            //memcpy(errorMessage, str.c_str(), str.length());
        }
    };

    typedef  Response<ImagePtr> GetImageResponse;

    class LoggingEventHandlerImpl : public LoggingEventHandler {
        void OnLogEvent(LoggingEventDataPtr loggingEventDataPtr);

        int m_LoggingLevel = 300;
    };

    /*
        DeviceDisplayName	Point Grey Research Blackfly BFLY-PGE-50A2M
       DeviceType	        GigEVision
        DeviceVendorName	Point Grey Research
     */
    class FLIRCameraController {
    public :
        static FLIRCameraController *GetInstance();
        static cv::Mat ConvertToCVMat(ImagePtr pImage);

        enum State { OPEN_CONFIGURABLE, ACQUIRING, HUNG, UNKNOWN } ;
        static const int LOGGING_LEVEL;
        static State state ;
        static PixelFormatEnums defaultPixelFormat;
        static unsigned int BUFFER_COUNT;
        static bool LOGGING ;
        chrono::milliseconds MIM_EXPOSURE_MS  = 3ms /100 ;
        chrono::seconds      MAX_EXPOSURE_SEC = 32s ;

    public:
        // API Says Camera is OK
        bool IsInValidState();
        // Gets the next image, will block for the specified timeout (ms) until an image  arrives
        // Most cameras support one stream so the default streamIndex is 0, but if a
        // camera supports multiple streams the user can input the streamIndex
        // @param grabTimeout  timeout period
        // @param streamIndex  select from which stream to grab (default 0)
        // @return ImagePtr
        GetImageResponse GetImage(uint64_t  grabTimeout  = 5000, uint64_t  streamIndex = 0 );

    public:
        string GetDeviceInfo();
        double GetExposure();
        void SetExposure(float exposure);
        void SetAutoExposure();

        void SetPixelMono8();
        void SetPixelMono12();
        void SetPixelMono16();
        void SetPixelFormat(PixelFormatEnums f);
        void SetBufferCount(unsigned int numBuffers);
        unsigned int GetBufferCount();

        void Clear();
        FLIRCameraController(unsigned int i);
        ~FLIRCameraController();

        float  GetGain();
        double GetBlackLevel();
        double GetGamma();

        // untested but likely to work, use at your own risk
        // changing exposure and gain for example ...
        /*
        void   SetBlacklevel(double blackLevel);
        void   SetGain(float gain);
        void   SetGamma(double gamma);
        */

    int IsHeartBeating(  bool enable);

    private:
        static FLIRCameraController *instance;
        LoggingEventHandlerImpl m_LoggingEventHandler;
        std::mutex m_mutex;
        SystemPtr m_SystemPtr = nullptr;
        CameraPtr m_CameraPtr = nullptr;
        CameraList m_CameraList;
        ImageProcessor m_ImageProcessor;
    };

} // lmc

