#include <opencv2/highgui/highgui.hpp>
#include "BlkFlyCameraManager.h"
#include <algorithm>
#include <sstream>
#include <thread>
#include <exception>

using namespace cv;
using namespace std;

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

std::map<int, BlkFlyCameraManager *> instances;
SystemPtr BlkFlyCameraManager::s_SystemPtr;
CameraList BlkFlyCameraManager::s_CameraList;
bool BlkFlyCameraManager::LOGGING = false;
LoggingEventHandlerImpl BlkFlyCameraManager::s_LoggingEventHandler;

void BlkFlyCameraManager::Init(PixelFormatEnums f, unsigned int numBuffers) {
	s_SystemPtr = System::GetInstance();
	s_CameraList = s_SystemPtr->GetCameras();
	for (unsigned int i = 0; i < s_CameraList.GetSize(); i++) {

		CameraPtr aCameraPtr = s_CameraList.GetByIndex(i);
		aCameraPtr->Init();
		aCameraPtr->AcquisitionMode.SetValue(
				AcquisitionModeEnums::AcquisitionMode_Continuous);
		aCameraPtr->PixelFormat.SetValue(f);
		aCameraPtr->BeginAcquisition();
	}
}

GetImageResponse BlkFlyCameraManager::GetImageFromCamera(int index,
		float exposure) {
	GetImageResponse ret;
	ImageProcessor imageProcessor;
	if (exposure != 0) {
		SetExposure(index, exposure);
	}
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	if (aCameraPtr->IsValid()) {
		ImagePtr pImage = aCameraPtr->GetNextImage();
		if (!pImage->IsIncomplete()) {
			ret.payload = imageProcessor.Convert(pImage, Configs::pixelFmt);
			pImage->Release();
			ret.t = std::time(0);
			ret.isSuccess = true;
		} else {
			ret.isSuccess = false;
			stringstream ss;
			ss << "Image incomplete: "
					<< Image::GetImageStatusDescription(
							pImage->GetImageStatus()) << "..." << endl;
			ret.errorMessage = ss.str();

		}
	} else {
		ret.isSuccess = false;
		stringstream ss;
		ret.errorMessage = ss.str();
	}
	return ret;
}

unsigned int BlkFlyCameraManager::GetCameraCount() {
	s_SystemPtr = System::GetInstance();
	s_CameraList = s_SystemPtr->GetCameras();
	return s_CameraList.GetSize();
}

cv::Mat BlkFlyCameraManager::ConvertToCVMat(ImagePtr pImage) {
	auto paddingWidth = pImage->GetXPadding();
	auto paddingHeight = pImage->GetYPadding();
	auto width = pImage->GetWidth();
	auto height = pImage->GetHeight();
	auto bbp = pImage->GetBitsPerPixel();

	if (bbp == 8) {
		return cv::Mat(
				height + paddingHeight,
				width + paddingWidth,
				CV_8UC1,
				pImage->GetData(),
				pImage->GetStride());
	} else if (bbp == 12) {
		return cv::Mat(
				height + paddingHeight,
				width + paddingWidth,
				CV_16UC1,
				pImage->GetData(),
				pImage->GetStride());
	} else if (bbp == 16) {
		return cv::Mat(
				height + paddingHeight,
				width + paddingWidth,
				CV_16UC1,
				pImage->GetData(),
				pImage->GetStride());
	}
	return cv::Mat(
			height + paddingHeight,
			width + paddingWidth,
			CV_8UC1,
			pImage->GetData(),
			pImage->GetStride());
}

bool BlkFlyCameraManager::IsInValidState(int index) {
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	return aCameraPtr != nullptr ? aCameraPtr->IsValid() : false;
}

string BlkFlyCameraManager::GetDeviceInfo(int index) {
	stringstream ss;
	try {
		CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
		INodeMap &nodeMap = aCameraPtr->GetTLDeviceNodeMap();
		FeatureList_t features;
		CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
		if (IsReadable(category)) {
			category->GetFeatures(features);
			FeatureList_t::const_iterator it;
			for (it = features.begin(); it != features.end(); ++it) {
				CNodePtr pfeatureNode = *it;
				ss << pfeatureNode->GetName() << " : ";
				CValuePtr pValue = (CValuePtr) pfeatureNode;
				ss
						<< (IsReadable(pValue) ?
								pValue->ToString() : "Node not readable");
				ss << endl;
			}
		} else {
			ss << "Device control information not readable." << endl;
		}
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError()
				<< " raised in function " << spinEx.GetFunctionName()
				<< " at line " << spinEx.GetLineNumber() << "." << endl;
	}
	return ss.str();
}

double BlkFlyCameraManager::GetExposure(int index) {
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	return aCameraPtr->ExposureTime.GetValue();
}

void BlkFlyCameraManager::SetExposure(int index, float exposure = 0) {
	try {
		CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
		aCameraPtr->ExposureMode.SetValue(
				ExposureModeEnums::ExposureMode_Timed);
		aCameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Off);
		aCameraPtr->ExposureTime.SetValue(exposure);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

void BlkFlyCameraManager::SetAutoExposure(int index) {
	try {
		CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
		aCameraPtr->ExposureMode.SetValue(
				ExposureModeEnums::ExposureMode_TriggerWidth);
		aCameraPtr->ExposureAuto.SetValue(
				ExposureAutoEnums::ExposureAuto_Continuous);
	    class GigECamera {
	    public :
	         GigECamera(int index, PixelFormatEnums f = PixelFormatEnums::PixelFormat_Mono8);
	         GetImageResponse GetImage( float exposure = 0 );
	         bool IsInValidState();
	        string GetDeviceInfo();
	        double GetExposure() ;
	        void SetExposure( float exposure);
	        void  SetAutoExposure() ;
	        ~GigECamera();
	    private:
	        PixelFormatEnums m_f;
	        int m_index;
	        CameraPtr m_CameraPtr;
	    };

		aCameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Continuous);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

unsigned int BlkFlyCameraManager::GetBufferCount(int index) {
	// Retrieve Stream Parameters device nodemap
	Spinnaker::GenApi::INodeMap &sNodeMap =
			s_CameraList.GetByIndex(index)->GetTLStreamNodeMap();

	// Retrieve Buffer Handling Mode Information
	CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode(
			"StreamBufferHandlingMode");
	if (!IsReadable(ptrHandlingMode) || !IsWritable(ptrHandlingMode)) {
		cout
				<< "Unable to set Buffer Handling mode (node retrieval). Aborting..."
				<< endl << endl;
		return -1;
	    class GigECamera {
	    public :
	         GigECamera(int index, PixelFormatEnums f = PixelFormatEnums::PixelFormat_Mono8);
	         GetImageResponse GetImage( float exposure = 0 );
	         bool IsInValidState();
	        string GetDeviceInfo();
	        double GetExposure() ;
	        void SetExposure( float exposure);
	        void  SetAutoExposure() ;
	        ~GigECamera();
	    private:
	        PixelFormatEnums m_f;
	        int m_index;
	        CameraPtr m_CameraPtr;
	    };

	}

	CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
	if (!IsReadable(ptrHandlingModeEntry)) {
		cout
				<< "Unable to get Buffer Handling mode (Entry retrieval). Aborting..."
				<< endl << endl;
		return -1;
	}

	// Set stream buffer Count Mode to manual
	CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode(
			"StreamBufferCountMode");
	if (!IsReadable(ptrStreamBufferCountMode)
			|| !IsWritable(ptrStreamBufferCountMode)) {
		cout
				<< "Unable to get or set Buffer Count Mode (node retrieval). Aborting..."
				<< endl << endl;
		return -1;
	}

	CEnumEntryPtr ptrStreamBufferCountModeManual =
			ptrStreamBufferCountMode->GetEntryByName("Manual");
	if (!IsReadable(ptrStreamBufferCountModeManual)) {
		cout
				<< "Unable to get BCameraManageruffer Count Mode entry (Entry retrieval). Aborting..."
				<< endl << endl;
		return -1;
	}

	ptrStreamBufferCountMode->SetIntValue(
			ptrStreamBufferCountModeManual->GetValue());

	cout << "Stream Buffer Count Mode set to manual..." << endl;

	// Retrieve and modify Stream Buffer Count
	CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
	if (!IsReadable(ptrBufferCount) || !IsWritable(ptrBufferCount)) {
		cout
				<< "Unable to get or set Buffer Count (Integer node retrieval). Aborting..."
				<< endl << endl;
		return -1;
	}

	// Display Buffer Info
	cout << endl << "Default Buffer Handling Mode: "
			<< ptrHandlingModeEntry->GetDisplayName() << endl;
	cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
	cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;

	return ptrBufferCount->GetValue();
}

void BlkFlyCameraManager::Clear() {

	if (LOGGING) {
		s_SystemPtr->UnregisterLoggingEventHandler(s_LoggingEventHandler);
	}
	for (unsigned int i = 0; i < s_CameraList.GetSize(); i++) {
		auto c = s_CameraList.GetByIndex(i);
		if (c) {
			try {
				c->EndAcquisition();
				c->DeInit(); // DeInit Disconnects camera port, resets camera back to read access
				c = nullptr;
			} catch (...) {
				c = nullptr;
			}
		}
	}
	s_CameraList.Clear();
	if (s_SystemPtr) {
		s_SystemPtr->ReleaseInstance();
		s_SystemPtr = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

unsigned int GetGigECameraCount() {
	return System::GetInstance()->GetCameras().GetSize();
}

cv::Mat ConvertToCVMat(ImagePtr pImage) {
	auto paddingWidth = pImage->GetXPadding();
	auto paddingHeight = pImage->GetYPadding();
	auto width = pImage->GetWidth();
	auto height = pImage->GetHeight();

	auto bbp = pImage->GetBitsPerPixel();
	if (bbp == 8) {
		return cv::Mat(height + paddingHeight, width + paddingWidth,
		CV_8UC1, pImage->GetData(), pImage->GetStride());
	} else if (bbp == 12) {
		return cv::Mat(height + paddingHeight, width + paddingWidth,
		CV_16UC1, pImage->GetData(), pImage->GetStride());
	} else if (bbp == 16) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1,
				pImage->GetData(), pImage->GetStride());
	}
	return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1,
			pImage->GetData(), pImage->GetStride());
}

GetImageResponse* SteroPair(int indexLeft, int indexRight) {
	GetImageResponse* res = new GetImageResponse[2];
	auto getImage =
			[&](int index) {
				int max_capture_attempts = 3;
				int capture_attempts = 0;
				while( !res[index].isSuccess) {
					if ( capture_attempts) {
						cout << res[index].errorMessage << endl;
					}
					res[index] = BlkFlyCameraManager::GetImageFromCamera(index);
					capture_attempts ++;
					if ( capture_attempts > max_capture_attempts) {
						cout << "Capture:" << index << "Giving Up in Camera:" << index << endl;
						break;
					}
				}
			};
	thread left(getImage, indexLeft);
	thread right(getImage, indexRight);

	left.join();
	right.join();
	return res;
}



