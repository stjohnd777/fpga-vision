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
    class CameraManager {
    public :
        static CameraManager *GetInstance(int index = 0);

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

        static void Init(PixelFormatEnums f ){
    		m_SystemPtr = System::GetInstance();
    		m_CameraList = m_SystemPtr->GetCameras();
    		for (unsigned int i=0; i < m_CameraList.GetSize(); i++){
    			CameraPtr aCameraPtr = m_CameraList.GetByIndex(i);
    			aCameraPtr->Init();
    			aCameraPtr->AcquisitionMode.SetValue(AcquisitionModeEnums::AcquisitionMode_Continuous);
    			aCameraPtr->PixelFormat.SetValue(f);
    			aCameraPtr->BeginAcquisition();
    		}
        }

        static GetImageResponse GetImageFromCamera(int index,PixelFormatEnums f  ){
        	GetImageResponse ret;
        	ImageProcessor imageProcessor;
    		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
    		if (aCameraPtr->IsValid()) {
    			ImagePtr pImage = aCameraPtr->GetNextImage(10*1000,0);
    			if (!pImage->IsIncomplete()) {
    				ret.payload = imageProcessor.Convert(pImage,f);
    				pImage->Release();
    				ret.isSuccess = true;
    			}else {
    				ret.isSuccess = false;
    			}
    		}else {
    			ret.isSuccess = false;
    		}
    		return ret;
        }


        static unsigned int GetCameraCount(){
    		m_SystemPtr = System::GetInstance();
    		m_CameraList = m_SystemPtr->GetCameras();
        	return m_CameraList.GetSize();
        }


        // API Says Camera is OK
        bool IsInValidState(int index);
        // Gets the next image, will block for the specified timeout (ms) until an image  arrives
        // Most cameras support one stream so the default streamIndex is 0, but if a
        // camera supports multiple streams the user can input the streamIndex
        // @param grabTimeout  timeout period
        // @param streamIndex  select from which stream to grab (default 0)
        // @return ImagePtr
        GetImageResponse GetImage(int index, uint64_t  grabTimeout  = 7000, uint64_t  streamIndex = 0 );

    public:
        string GetDeviceInfo(int index = 0);
        double GetExposure(int index = 0);
        void   SetExposure(int index , float exposure);
        void   SetAutoExposure(int index = 0);

        void SetPixelMono8(int index);
        void SetPixelMono12(int index);
        void SetPixelMono16(int index);
        void SetPixelFormat(int index,PixelFormatEnums f);
        void SetBufferCount(int index,unsigned int numBuffers);
        unsigned int GetBufferCount(int index);

        void Clear(int index);
        CameraManager( int i);
        ~CameraManager();

        float  GetGain(int index);
        double GetBlackLevel(int index);
        double GetGamma(int index);




    private:
        static std::map<int,CameraManager *> instances;
        static SystemPtr m_SystemPtr ;
        static CameraList m_CameraList;

        LoggingEventHandlerImpl m_LoggingEventHandler;
        std::mutex m_mutex;


        ImageProcessor m_ImageProcessor;
    };

} // lmc

