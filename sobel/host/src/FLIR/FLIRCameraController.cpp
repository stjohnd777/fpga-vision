//
// Created by e438262 on 5/3/2023.
//

#include "FLIRCameraController.h"

#include <opencv2/highgui/highgui.hpp>
#include <exception>

using namespace cv;

//#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
//#include <boost/log/sinks/text_file_backend.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/sources/severity_logger.hpp>
//#include <boost/log/sources/record_ostream.hpp>

//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace sinks = boost::log::sinks;
//namespace keywords = boost::log::keywords;


namespace lmc {

//    void init_logging()
//    {
//        logging::add_file_log("sample.log");
//
//        logging::core::get()->set_filter
//                (
//                        logging::trivial::severity >= logging::trivial::info
//                );
//    }

//    void init_logging()
//    {
//        logging::add_file_log
//                (
//                        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
//                        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
//                        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
//                        keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
//                );
//
//        logging::core::get()->set_filter
//                (
//                        logging::trivial::severity >= logging::trivial::info
//                );
//    }

    void LoggingEventHandlerImpl::OnLogEvent(LoggingEventDataPtr loggingEventDataPtr) {
        stringstream out;

        out << "--------Log Event Received----------" << endl;
        out << "Category: " << loggingEventDataPtr->GetCategoryName() << endl;
        out << "Priority Value: " << loggingEventDataPtr->GetPriority() << endl;
        out << "Priority Name: " << loggingEventDataPtr->GetPriorityName() << endl;
        out << "Timestmap: " << loggingEventDataPtr->GetTimestamp() << endl;
        out << "NDC: " << loggingEventDataPtr->GetNDC() << endl;
        out << "Thread: " << loggingEventDataPtr->GetThreadName() << endl;
        out << "Message: " << loggingEventDataPtr->GetLogMessage() << endl;
        out << "------------------------------------" << endl << endl;
        cout << out.str() << endl;
    }

    FLIRCameraController *FLIRCameraController::instance = nullptr;
    FLIRCameraController::State FLIRCameraController::state = UNKNOWN;
    PixelFormatEnums FLIRCameraController::defaultPixelFormat = PixelFormat_Mono16;
    bool FLIRCameraController::LOGGING = false;
    unsigned int FLIRCameraController::BUFFER_COUNT = 2;

    cv::Mat FLIRCameraController::ConvertToCVMat(ImagePtr pImage) {
        auto paddingWidth = pImage->GetXPadding();
        auto paddingHeight = pImage->GetYPadding();
        auto width = pImage->GetWidth();
        auto height = pImage->GetHeight();

        auto bbp = pImage->GetBitsPerPixel();
        if (bbp == 8) {
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1, pImage->GetData(),
                           pImage->GetStride());
        } else if (bbp == 12) {
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1, pImage->GetData(),
                           pImage->GetStride());
        } else if (bbp == 16) {
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1, pImage->GetData(),
                           pImage->GetStride());
        }
        return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1, pImage->GetData(), pImage->GetStride());
    }

    FLIRCameraController *FLIRCameraController::GetInstance() {
        if (instance == nullptr) {
            instance = new FLIRCameraController(0);
        }
        return instance;
    }

    FLIRCameraController::FLIRCameraController(unsigned int i) {
        try {
            cout << "FLIRCameraController(" << i << ")" << endl;
            m_SystemPtr = System::GetInstance();
            cout << "FLIRCameraController::FLIRCameraController => Retrieves Singleton Reference to System object "
                 << endl;

            if (LOGGING) { m_SystemPtr->RegisterLoggingEventHandler(m_LoggingEventHandler); }

            m_CameraList = m_SystemPtr->GetCameras();
            const unsigned int numberCamerasFound = m_CameraList.GetSize();
            if (numberCamerasFound == 0) {
                Clear();
                throw  "No Cameras Found On System" ;
            }
            m_CameraPtr = m_CameraList.GetByIndex(i);
            //  This function needs to be called before any camera related API calls such as BeginAcquisition(), EndAcquisition(), GetNodeMap(), GetNextImage()
            m_CameraPtr->Init();
            cout << "FLIRCameraController::FLIRCameraController => m_CameraPtr->Init() " << endl;
            INodeMap &nodeMap = m_CameraPtr->GetNodeMap();
            cout << "FLIRCameraController::FLIRCameraController => m_CameraPtr->GetNodeMap() " << endl;
            m_CameraPtr->AcquisitionMode.SetValue(AcquisitionModeEnums::AcquisitionMode_Continuous);
            cout << "FLIRCameraController::FLIRCameraController =>m_CameraPtr->AcquisitionMode.SetValue " << endl;
            SetPixelFormat(defaultPixelFormat);
            cout << "Buffer Count " << GetBufferCount() << endl;
            SetBufferCount(lmc::FLIRCameraController::BUFFER_COUNT);
            cout << "Buffer Count " << GetBufferCount() << endl;

      
			// Begin acquiring images
			//
			// *** NOTES ***
			// What happens when the camera begins acquiring images depends on the
			// acquisition mode.
			//      Single frame captures only a single image,
			//      multi frame captures a set number of images,
			//      and continuous captures a continuous stream of images.
			//
			// Because we are most always in retrieval continuous mode has been set.
			//
			// *** LATER ***
			// Image acquisition must be ended when no more images are needed.
			//
            m_CameraPtr->BeginAcquisition();

            cout << "FLIRCameraController::FLIRCameraController => m_CameraPtr->BeginAcquisition() " << endl;
            m_ImageProcessor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
            state = ACQUIRING;

        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cerr << "Error: " << spinEx.GetErrorMessage() << endl;
            cerr << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
            Clear();
            state = HUNG;
            throw spinEx;
        }
    }

    bool FLIRCameraController::IsInValidState() {
        return m_CameraPtr != nullptr ? m_CameraPtr->IsValid() : false;
    }



    GetImageResponse FLIRCameraController::GetImage(uint64_t grabTimeout, uint64_t streamIndex) {

		GetImageResponse ret;
        bool isSuccess = false;
        stringstream ss;
    	try  {
            
	        std::lock_guard<std::mutex> guard(m_mutex);

	        if (m_CameraPtr->IsValid())
	        {
	        	cout << "Camera Is In Valid State " << endl;
                //
				// Retrieve next received image
				//
				// *** NOTES ***
				// Capturing an image houses images on the camera buffer. Trying
				// to capture an image that does not exist will hang the camera.
				//
				// *** LATER ***
				// Once an image from the buffer is saved and/or no longer
				// needed, the image must be released in order to keep the
				// buffer from filling up.
				//
                
                cout << "GetNextImage(" << grabTimeout << "," << streamIndex << ")" << endl;
		        ImagePtr pImage = m_CameraPtr->GetNextImage(grabTimeout, streamIndex);

		        
                // Ensure image completion
				 //
				 // *** NOTES ***
				 // Images can easily be checked for completion. This should be
				 // done whenever a complete image is expected or required.
				 // Further, check image status for a little more insight into
				 // why an image is incomplete.
				 //
		        if (!pImage->IsIncomplete())  {

                    cout << "Get Image is Complete " << endl;
 
				    // *** NOTES ***
				    // Images can be converted between pixel formats by using
				    // the appropriate enumeration value.
				    //
				    // Unlike the original image, the converted one does not need to
				    // be released as  it does not affect the camera buffer.
				    //
				    // When converting images, color processing algorithm is an
				    // optional parameter.
				    //
                    cout << "Create Local Copy (Not on Camera in Buffer) Image with ImageProcessor  " << endl;
		        	ret.payload=  m_ImageProcessor.Convert(pImage, defaultPixelFormat);
                  
				    // Release image
				    // Images retrieved directly from the camera (i.e. non-converted  images)
				    // need to be released in order to keep from filling the buffer.
                    cout << "Release Image Tied to the Camera Buffer as to not fill the Camera Buffer" << endl;
                    pImage->Release();

                    isSuccess = true;

                    ret.isSuccess = isSuccess;

		        }
                // Further, check image status for a little more insight into
				// why an image is incomplete.
				//
                ss << "Image incomplete: " << Image::GetImageStatusDescription(pImage->GetImageStatus()) << "..." << endl;
                cout << ss.str();
                ret.setErrorMessage(ss.str());
                
	        }
            ss << "Return Local Image Pointer : Success " << isSuccess << endl;
            cout << ss.str();
            ret.setErrorMessage(ss.str());
            return ret;
        }
        catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            ss << "Error: " << spinEx.GetErrorMessage() << endl;
            ss << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
            cout << ss.str();
            ret.setErrorMessage(ss.str());
            return ret;
        }
        
    }

    string FLIRCameraController::GetDeviceInfo() {
        stringstream ss;
        try {
            INodeMap &nodeMap = m_CameraPtr->GetTLDeviceNodeMap();
            FeatureList_t features;
            CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
            if (IsReadable(category)) {
                category->GetFeatures(features);
                FeatureList_t::const_iterator it;
                for (it = features.begin(); it != features.end(); ++it) {
                    CNodePtr pfeatureNode = *it;
                    ss << pfeatureNode->GetName() << " : ";
                    CValuePtr pValue = (CValuePtr) pfeatureNode;
                    ss << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                    ss << endl;
                }
            } else {
                ss << "Device control information not readable." << endl;
            }
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
        return ss.str();
    }

    double FLIRCameraController::GetExposure() {
        return m_CameraPtr->ExposureTime.GetValue();
    }

    void FLIRCameraController::SetExposure(float exposure) {
        try {
            m_CameraPtr->ExposureMode.SetValue(ExposureModeEnums::ExposureMode_Timed);
            m_CameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Off);
            m_CameraPtr->ExposureTime.SetValue(exposure);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetAutoExposure() {
        try {
            m_CameraPtr->ExposureMode.SetValue(ExposureModeEnums::ExposureMode_TriggerWidth);
            m_CameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Continuous);
            m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Continuous);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetPixelFormat(PixelFormatEnums f) {
        try {
            m_CameraPtr->PixelFormat.SetValue(f);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetPixelMono8() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono8);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetPixelMono12() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono12);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }

    }

    void FLIRCameraController::SetPixelMono16() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono16);
        } catch (std::exception &ex) {
            Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()
                 << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetBufferCount(unsigned int numBuffers) {
        // Retrieve Stream Parameters device nodemap
        Spinnaker::GenApi::INodeMap &sNodeMap = m_CameraPtr->GetTLStreamNodeMap();

        // Retrieve Buffer Handling Mode Information
        CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
        if (!IsReadable(ptrHandlingMode) ||
            !IsWritable(ptrHandlingMode)) {
            cout << "Unable to set Buffer Handling mode (node retrieval). Aborting..." << endl << endl;
            return;
        }

        CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
        if (!IsReadable(ptrHandlingModeEntry)) {
            cout << "Unable to get Buffer Handling mode (Entry retrieval). Aborting..." << endl << endl;
            return;
        }

        // Set stream buffer Count Mode to manual
        CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
        if (!IsReadable(ptrStreamBufferCountMode) ||
            !IsWritable(ptrStreamBufferCountMode)) {
            cout << "Unable to get or set Buffer Count Mode (node retrieval). Aborting..." << endl << endl;
            return;
        }

        CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
        if (!IsReadable(ptrStreamBufferCountModeManual)) {
            cout << "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..." << endl << endl;
            return;
        }

        ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

        cout << "Stream Buffer Count Mode set to manual..." << endl;

        // Retrieve and modify Stream Buffer Count
        CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
        if (!IsReadable(ptrBufferCount) ||
            !IsWritable(ptrBufferCount)) {
            cout << "Unable to get or set Buffer Count (Integer node retrieval). Aborting..." << endl << endl;
            return;
        }

        // Display Buffer Info
        cout << endl << "Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName() << endl;
        cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
        cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;


        ptrBufferCount->SetValue(numBuffers);
    }

    unsigned int FLIRCameraController::GetBufferCount() {
        // Retrieve Stream Parameters device nodemap
        Spinnaker::GenApi::INodeMap &sNodeMap = m_CameraPtr->GetTLStreamNodeMap();

        // Retrieve Buffer Handling Mode Information
        CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
        if (!IsReadable(ptrHandlingMode) || !IsWritable(ptrHandlingMode)) {
            cout << "Unable to set Buffer Handling mode (node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
        if (!IsReadable(ptrHandlingModeEntry)) {
            cout << "Unable to get Buffer Handling mode (Entry retrieval). Aborting..." << endl << endl;
            return -1;
        }

        // Set stream buffer Count Mode to manual
        CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
        if (!IsReadable(ptrStreamBufferCountMode) || !IsWritable(ptrStreamBufferCountMode)) {
            cout << "Unable to get or set Buffer Count Mode (node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
        if (!IsReadable(ptrStreamBufferCountModeManual)) {
            cout << "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..." << endl << endl;
            return -1;
        }

        ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

        cout << "Stream Buffer Count Mode set to manual..." << endl;

        // Retrieve and modify Stream Buffer Count
        CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
        if (!IsReadable(ptrBufferCount) ||
            !IsWritable(ptrBufferCount)) {
            cout << "Unable to get or set Buffer Count (Integer node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        // Display Buffer Info
        cout << endl << "Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName() << endl;
        cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
        cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;


        return ptrBufferCount->GetValue();
    }

    ///////////////////////////////////////////////////////////////////////

    double FLIRCameraController::GetBlackLevel() {
        return m_CameraPtr->BlackLevel.GetValue();
    }

    float FLIRCameraController::GetGain() {
        return m_CameraPtr->Gain.GetValue();
    }

    double FLIRCameraController::GetGamma() {
        return m_CameraPtr->Gamma.GetValue();
    }

//    void  FLIRCameraController::SetBlacklevel(double blackLevel){
//        // Brightness is called black level in GenICam
//        m_CameraPtr->BlackLevelSelector.SetValue(Spinnaker::BlackLevelSelectorEnums::BlackLevelSelector_All);
//        //Set the absolute value of brightness to 1.5%.
//        m_CameraPtr->BlackLevel.SetValue(blackLevel);
//    }
//    void  FLIRCameraController::SetGain(float gain){
//        try {
//            m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Off);
//            m_CameraPtr->Gain.SetValue(gain);
//        }catch (std::exception& ex) {
//            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
//            cout << "Error: " << spinEx.GetErrorMessage() << endl;
//            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
//        }
//    }
//    void FLIRCameraController::SetGamma(double gamma){
//        m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Off);
//        m_CameraPtr->Gamma.SetValue(gamma);
//    }


    void FLIRCameraController::Clear() {
        if (FLIRCameraController::instance != nullptr) {
            delete FLIRCameraController::instance;
            FLIRCameraController::instance = nullptr;
            state = OPEN_CONFIGURABLE;
        }
    }

    FLIRCameraController::~FLIRCameraController() {
        cout << "~FLIRCameraController()" << endl;
        if (LOGGING) {
            m_SystemPtr->UnregisterLoggingEventHandler(m_LoggingEventHandler);
        }
        if (m_CameraPtr) {
            try {
                m_CameraPtr->EndAcquisition();
                m_CameraPtr->DeInit();// DeInit Disconnects camera port, resets camera back to read access
                m_CameraPtr = nullptr;
            } catch (...) {
                m_CameraPtr = nullptr;
            }
        }
        m_CameraList.Clear();
        if (m_SystemPtr) {
            m_SystemPtr->ReleaseInstance();
            m_SystemPtr = nullptr;
        }
    }


    // Disables or enables heartbeat on GEV cameras so debugging does not incur timeout errors
    // Write to boolean node controlling the camera's heartbeat
    // *** NOTES ***
    // This applies only to GEV cameras.
    // GEV cameras have a heartbeat built in, but when debugging applications the
    // camera may time out due to its heartbeat. Disabling the heartbeat prevents
    // this timeout from occurring, enabling us to continue with any necessary 
    // debugging.
    // *** LATER ***
    // Make sure that the heartbeat is reset upon completion of the debugging.  
    // If the application is terminated unexpectedly, the camera may not locked
    // to Spinnaker indefinitely due to the the timeout being disabled.  When that 
    // happens, a camera power cycle will reset the heartbeat to its default setting.
    int FLIRCameraController::IsHeartBeating(bool enable) {

        INodeMap &nodeMapTLDevice = m_CameraPtr->GetTLDeviceNodeMap();
        INodeMap &nodeMap = m_CameraPtr->GetNodeMap();

        CEnumerationPtr ptrDeviceType = nodeMapTLDevice.GetNode("DeviceType");
        if (!IsReadable(ptrDeviceType)) { return -1; }
        if (ptrDeviceType->GetIntValue() != DeviceType_GigEVision) { return 0; }

        if (enable) {
            cout << endl << "Resetting heartbeat..." << endl << endl;
        } else {}

        CBooleanPtr ptrDeviceHeartbeat = nodeMap.GetNode("GevGVCPHeartbeatDisable");
        if (!IsWritable(ptrDeviceHeartbeat)) {
            cout << "Unable to configure heartbeat. Continuing with execution as this may be non-fatal..." << endl;
        } else {
            ptrDeviceHeartbeat->SetValue(enable);
            if (!enable) {
                cout << "WARNING: Heartbeat has been disabled for the rest of this example run." << endl;
                cout << "         Heartbeat will be reset upon the completion of this run.  If the " << endl;
                cout << "         example is aborted unexpectedly before the heartbeat is reset, the" << endl;
                cout << "         camera may need to be power cycled to reset the heartbeat." << endl << endl;
            } else {
                cout << "Heartbeat has been reset." << endl;
            }
        }
        return 0;
    }

} // lmc
