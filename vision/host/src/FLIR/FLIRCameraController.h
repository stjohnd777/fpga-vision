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

    class LoggingEventHandlerImpl : public LoggingEventHandler {
        void OnLogEvent(LoggingEventDataPtr loggingEventDataPtr);
    };

    /*
        DeviceDisplayName	Point Grey Research Blackfly BFLY-PGE-50A2M
        DeviceModelName	    Blackfly BFLY-PGE-50A2M
        DeviceType	        GigEVision
        DeviceVendorName	Point Grey Research
     */
    class FLIRCameraController {
    public :

        enum State { OPEN, CLOSED, HUNG, UNKNOWN } ;

        static State state ;

        static FLIRCameraController *GetInstance();

        static cv::Mat ConvertToCVMat(ImagePtr pImage);

        static unsigned int BUFFER_COUNT;

        chrono::milliseconds MIM_EXPOSURE_MS  = 3ms /100 ;
        chrono::seconds      MAX_EXPOSURE_SEC = 32s ;

        /**
         * If the camera valid for use.
         * @return
         */
        bool IsValid();

        /**
         *  Gets the next image, will block for the specified timeout until an image
         *  arrives
         *
         *  Most cameras support one stream so the default streamIndex is 0, but if a
         *  camera supports multiple streams the user can input the streamIndex
         *
         * @param grabTimeout  timeout period
         * @param streamIndex  select from which stream to grab (default 0)
         * @return
         */
        ImagePtr GetImage(uint64_t  grabTimeout  = 5000 ,uint64_t  streamIndex = 0 );

    public:
        static PixelFormatEnums defaultPixelFormat;
        static bool LOGGING ;


    public:
        string GetDeviceInfo();

    public:
        void   SetExposureTime(float exposure);
//        void   SetExposureTime(chrono::milliseconds exposure);
//        void   SetExposureTime(chrono::seconds exposure);
        void   SetAutoExposure();
        double GetExposureTime();

        double GetBlackLevel();
        void   SetBlacklevel(double blackLevel);

        float  GetGain();
        void   SetGain(float gain);

        double GetGamma();
        void   SetGamma(double gamma);

        void SetPixelMono8();
        void SetPixelMono12();
        void SetPixelMono16();
        void SetPixelFormat(PixelFormatEnums f);

        void SetBufferCount(unsigned int numBuffers){
            // Retrieve Stream Parameters device nodemap
            Spinnaker::GenApi::INodeMap& sNodeMap = m_CameraPtr->GetTLStreamNodeMap();

            // Retrieve Buffer Handling Mode Information
            CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
            if (!IsReadable(ptrHandlingMode) ||
                !IsWritable(ptrHandlingMode))
            {
                cout << "Unable to set Buffer Handling mode (node retrieval). Aborting..." << endl << endl;
                return ;
            }

            CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
            if (!IsReadable(ptrHandlingModeEntry))
            {
                cout << "Unable to get Buffer Handling mode (Entry retrieval). Aborting..." << endl << endl;
                return ;
            }

            // Set stream buffer Count Mode to manual
            CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
            if (!IsReadable(ptrStreamBufferCountMode) ||
                !IsWritable(ptrStreamBufferCountMode))
            {
                cout << "Unable to get or set Buffer Count Mode (node retrieval). Aborting..." << endl << endl;
                return ;
            }

            CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
            if (!IsReadable(ptrStreamBufferCountModeManual))
            {
                cout << "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..." << endl << endl;
                return ;
            }

            ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

            cout << "Stream Buffer Count Mode set to manual..." << endl;

            // Retrieve and modify Stream Buffer Count
            CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
            if (!IsReadable(ptrBufferCount) ||
                !IsWritable(ptrBufferCount))
            {
                cout << "Unable to get or set Buffer Count (Integer node retrieval). Aborting..." << endl << endl;
                return ;
            }

            // Display Buffer Info
            cout << endl << "Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName() << endl;
            cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
            cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;


            ptrBufferCount->SetValue(numBuffers);
        }

        unsigned int GetBufferCount(){
            // Retrieve Stream Parameters device nodemap
            Spinnaker::GenApi::INodeMap& sNodeMap = m_CameraPtr->GetTLStreamNodeMap();

            // Retrieve Buffer Handling Mode Information
            CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
            if (!IsReadable(ptrHandlingMode) ||  !IsWritable(ptrHandlingMode))
            {
                cout << "Unable to set Buffer Handling mode (node retrieval). Aborting..." << endl << endl;
                return -1;
            }

            CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
            if (!IsReadable(ptrHandlingModeEntry))
            {
                cout << "Unable to get Buffer Handling mode (Entry retrieval). Aborting..." << endl << endl;
                return -1;
            }

            // Set stream buffer Count Mode to manual
            CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
            if (!IsReadable(ptrStreamBufferCountMode) ||  !IsWritable(ptrStreamBufferCountMode))
            {
                cout << "Unable to get or set Buffer Count Mode (node retrieval). Aborting..." << endl << endl;
                return -1 ;
            }

            CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
            if (!IsReadable(ptrStreamBufferCountModeManual))
            {
                cout << "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..." << endl << endl;
                return -1;
            }

            ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

            cout << "Stream Buffer Count Mode set to manual..." << endl;

            // Retrieve and modify Stream Buffer Count
            CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
            if (!IsReadable(ptrBufferCount) ||
                !IsWritable(ptrBufferCount))
            {
                cout << "Unable to get or set Buffer Count (Integer node retrieval). Aborting..." << endl << endl;
                return -1 ;
            }

            // Display Buffer Info
            cout << endl << "Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName() << endl;
            cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
            cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;


            return ptrBufferCount->GetValue();
        }

    public:
        void clear();


    private:

        FLIRCameraController(unsigned int i);
        ~FLIRCameraController();

    private:

        static FLIRCameraController *instance;
        LoggingEventHandlerImpl m_LoggingEventHandler;

        std::mutex m_mutex;
        SystemPtr m_systemPtr;
        CameraList m_CamList;
        CameraPtr m_CameraPtr;
        ImageProcessor m_ImageProcessor;
    };

} // lmc

