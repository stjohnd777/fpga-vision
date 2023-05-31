//
// Created by e438262 on 5/3/2023.
//

#include "FLIRCameraController.h"

#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace lmc {


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

    cv::Mat FLIRCameraController::ConvertToCVMat(ImagePtr pImage)
    {
        auto paddingWidth = pImage->GetXPadding();
        auto paddingHeight = pImage->GetYPadding();
        auto width = pImage->GetWidth();
        auto height = pImage->GetHeight();


        auto bbp = pImage->GetBitsPerPixel();
        if ( bbp == 8){
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1, pImage->GetData(), pImage->GetStride());
        }else if ( bbp ==12) {
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1, pImage->GetData(), pImage->GetStride());
        }else if ( bbp == 16){
            return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1, pImage->GetData(), pImage->GetStride());
        }

        return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1, pImage->GetData(), pImage->GetStride());
    }

    FLIRCameraController::FLIRCameraController(unsigned int i) {
        cout << "FLIRCameraController(0)" << endl;
        m_systemPtr = System::GetInstance();

        if ( LOGGING) {
            m_systemPtr->RegisterLoggingEventHandler(m_LoggingEventHandler);
        }

        m_CamList = m_systemPtr->GetCameras();
        const unsigned int numCameras = m_CamList.GetSize();
        assert(i <= numCameras);
        m_CameraPtr = m_CamList.GetByIndex(i);

        INodeMap &nodeMapTLDevice = m_CameraPtr->GetTLDeviceNodeMap();
        // This function needs to be called before any camera related API
        // calls such as BeginAcquisition(), EndAcquisition(), GetNodeMap(), GetNextImage()
        try {
            m_CameraPtr->Init();
        } catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
            state = HUNG;
            throw spinEx;
        }
        INodeMap &nodeMap = m_CameraPtr->GetNodeMap();



        CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        if (!IsReadable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode)) {
            throw "Unable to set acquisition mode to continuous (enum retrieval). Aborting...";
        }
        // Retrieve entry node from enumeration node
        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsReadable(ptrAcquisitionModeContinuous)) {
            throw "Unable to get or set acquisition mode to continuous (entry retrieval). Aborting...";

        }
        // Retrieve integer value from entry node
        const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
        // Set integer value from entry node as new value of enumeration node
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

        // Starts the image acquisition engine. The camera must be initialized via a call to
        // Init() before starting an acquisition.

        SetPixelFormat(defaultPixelFormat);

//        cout << "Buffer Count " <<  GetBufferCount() << endl;
//        SetBufferCount(lmc::FLIRCameraController::BUFFER_COUNT);
//        cout << "Buffer Count " <<  GetBufferCount() << endl;

        m_CameraPtr->BeginAcquisition();
        m_ImageProcessor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
        state = CLOSED;

    }

    FLIRCameraController *FLIRCameraController::GetInstance() {
        //static FLIRCameraController *instance = new FLIRCameraController(0);
        if (instance == nullptr) {
            instance = new FLIRCameraController(0);
        }
        return instance;
    }

    bool FLIRCameraController::IsValid(){
        return m_CameraPtr != nullptr ? m_CameraPtr->IsValid() : false;
    }

    ImagePtr FLIRCameraController::GetImage(uint64_t  grabTimeout,uint64_t  streamIndex   ) {
        ImagePtr convertedImage;
        try {
            std::lock_guard<std::mutex> guard(m_mutex);

            ImagePtr pImage;
            if ( IsValid() ) {
                pImage = m_CameraPtr->GetNextImage(grabTimeout, streamIndex);
                cout << "Camera InValid Sate: Reset Camera " << endl;
            }

            if (pImage->IsIncomplete()) {
                cout << "Image incomplete: " << Image::GetImageStatusDescription(pImage->GetImageStatus())
                     << "..." << endl;
            } else {
                const size_t width = pImage->GetWidth();
                const size_t height = pImage->GetHeight();
                char* data = (char*) pImage->GetData();
                convertedImage = m_ImageProcessor.Convert(pImage, defaultPixelFormat);
                pImage->Release();
            }
        } catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
        return convertedImage;
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
        }  catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
            m_CameraPtr->DeInit();
            m_CameraPtr->Init();
        }
        return ss.str();
    }


    void FLIRCameraController::SetPixelFormat(PixelFormatEnums f) {
        try {
            m_CameraPtr->PixelFormat.SetValue(f);
        }  catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }
    void FLIRCameraController::SetPixelMono8() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono8);
        }  catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void  FLIRCameraController::SetPixelMono12() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono12);
        }  catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }

    }

    void FLIRCameraController::SetPixelMono16() {
        try {
            m_CameraPtr->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono16);
        }  catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }



    double  FLIRCameraController::GetBlackLevel(){
        return m_CameraPtr->BlackLevel.GetValue();
    }

    void  FLIRCameraController::SetBlacklevel(double blackLevel){
        // Brightness is called black level in GenICam
        m_CameraPtr->BlackLevelSelector.SetValue(Spinnaker::BlackLevelSelectorEnums::BlackLevelSelector_All);
        //Set the absolute value of brightness to 1.5%.
        m_CameraPtr->BlackLevel.SetValue(blackLevel);
    }


    float FLIRCameraController::GetGain(){
        return  m_CameraPtr->Gain.GetValue();
    }

    void  FLIRCameraController::SetGain(float gain){
        try {
            m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Off);
            m_CameraPtr->Gain.SetValue(gain);
        }catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    double FLIRCameraController::GetGamma(){
        return m_CameraPtr->Gamma.GetValue();
    }
    void FLIRCameraController::SetGamma(double gamma){
        m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Off);
        m_CameraPtr->Gamma.SetValue(gamma);
    }

    double FLIRCameraController::GetExposureTime(){
        return m_CameraPtr->ExposureTime.GetValue();
    }
    void   FLIRCameraController::SetExposureTime(float exposure){
        try {
            m_CameraPtr->ExposureMode.SetValue(ExposureModeEnums::ExposureMode_Timed);
            m_CameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Off);
            m_CameraPtr->ExposureTime.SetValue(exposure);
        }catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }

    void FLIRCameraController::SetAutoExposure(){
        try {
            m_CameraPtr->ExposureMode.SetValue(ExposureModeEnums::ExposureMode_TriggerWidth);
            m_CameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Continuous);
            m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Continuous);
        }catch (std::exception& ex) {
            Spinnaker::Exception& spinEx = dynamic_cast<Spinnaker::Exception&>(ex);
            cout << "Error: " << spinEx.GetErrorMessage() << endl;
            cout << "Error code " << spinEx.GetError() << " raised in function " << spinEx.GetFunctionName()  << " at line " << spinEx.GetLineNumber() << "." << endl;
        }
    }




    void FLIRCameraController::clear() {
        if (FLIRCameraController::instance!= nullptr ) {
            delete FLIRCameraController::instance;
            FLIRCameraController::instance = nullptr;
            state = OPEN;
        }
    }

    FLIRCameraController::~FLIRCameraController() {
        cout << "~FLIRCameraController()" << endl;
        if ( LOGGING) {
            m_systemPtr->UnregisterLoggingEventHandler(m_LoggingEventHandler);
        }
        m_CameraPtr->EndAcquisition();
        m_CameraPtr->DeInit();// DeInit Disconnects camera port, resets camera back to read access
    }

} // lmc
