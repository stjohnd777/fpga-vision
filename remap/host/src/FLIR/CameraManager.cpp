#include "CameraManager.h"

//
// Created by e438262 on 5/3/2023.
//

#include <opencv2/highgui/highgui.hpp>
#include <algorithm>
#include <sstream>
#include <thread>
#include <exception>

using namespace cv;
using namespace std;

namespace lmc {

void LoggingEventHandlerImpl::OnLogEvent(
		LoggingEventDataPtr loggingEventDataPtr) {
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

std::map<int, CameraManager *> instances;
SystemPtr CameraManager::s_SystemPtr;
CameraList CameraManager::s_CameraList;
bool CameraManager::LOGGING = false;
LoggingEventHandlerImpl CameraManager::s_LoggingEventHandler;

void CameraManager::Init(PixelFormatEnums f, unsigned int numBuffers) {
	s_SystemPtr = System::GetInstance();
	s_CameraList = s_SystemPtr->GetCameras();
	for (unsigned int i = 0; i < s_CameraList.GetSize(); i++) {

		CameraPtr aCameraPtr = s_CameraList.GetByIndex(i);
		aCameraPtr->Init();
		aCameraPtr->AcquisitionMode.SetValue(
				AcquisitionModeEnums::AcquisitionMode_Continuous);
		aCameraPtr->PixelFormat.SetValue(f);

//		Spinnaker::GenApi::INodeMap &sNodeMap = s_CameraList.GetByIndex(i)->GetTLStreamNodeMap();
//		CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
//		if (IsReadable(ptrHandlingMode) && IsWritable(ptrHandlingMode)) {
//			CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
//			if (IsReadable(ptrHandlingModeEntry)) {
//				CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
//				if (IsReadable(ptrStreamBufferCountMode) && IsWritable(ptrStreamBufferCountMode)) {
//	    			CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
//	    			if (IsReadable(ptrStreamBufferCountModeManual)) {
//	    				ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());
//	        			if (IsReadable(ptrBufferCount) && IsWritable(ptrBufferCount)) {
//	        				ptrBufferCount->SetValue(numBuffers);
//	        			}
//	    			}
//				}
//			}
//
//		}

		aCameraPtr->BeginAcquisition();
	}
}

GetImageResponse CameraManager::GetImageFromCamera(int index, PixelFormatEnums f, float exposure) {
	GetImageResponse ret;
	ImageProcessor imageProcessor;
	if (exposure != 0) {
		SetExposure(index, exposure);
	}
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	if (aCameraPtr->IsValid()) {
		ImagePtr pImage = aCameraPtr->GetNextImage();
		if (!pImage->IsIncomplete()) {
			ret.payload = imageProcessor.Convert(pImage, f);
			pImage->Release();
		    ret.t = std::time(0);
			ret.isSuccess = true;
		} else {
			ret.isSuccess = false;
			stringstream ss;
			ss << "Image incomplete: " << Image::GetImageStatusDescription(pImage->GetImageStatus()) << "..." << endl;
			ret.errorMessage = ss.str();

		}
	} else {
		ret.isSuccess = false;
		stringstream ss;
		ret.errorMessage = ss.str();
	}
	return ret;
}

unsigned int CameraManager::GetCameraCount() {
	s_SystemPtr = System::GetInstance();
	s_CameraList = s_SystemPtr->GetCameras();
	return s_CameraList.GetSize();
}

cv::Mat CameraManager::ConvertToCVMat(ImagePtr pImage) {
	auto paddingWidth = pImage->GetXPadding();
	auto paddingHeight = pImage->GetYPadding();
	auto width = pImage->GetWidth();
	auto height = pImage->GetHeight();

	auto bbp = pImage->GetBitsPerPixel();
	if (bbp == 8) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1,
				pImage->GetData(), pImage->GetStride());
	} else if (bbp == 12) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1,
				pImage->GetData(), pImage->GetStride());
	} else if (bbp == 16) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1,
				pImage->GetData(), pImage->GetStride());
	}
	return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1,
			pImage->GetData(), pImage->GetStride());
}

bool CameraManager::IsInValidState(int index) {
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	return aCameraPtr != nullptr ? aCameraPtr->IsValid() : false;
}

string CameraManager::GetDeviceInfo(int index) {
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
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
	return ss.str();
}

double CameraManager::GetExposure(int index) {
	CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
	return aCameraPtr->ExposureTime.GetValue();
}

void CameraManager::SetExposure(int index, float exposure = 0) {
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

void CameraManager::SetAutoExposure(int index) {
	try {
		CameraPtr aCameraPtr = s_CameraList.GetByIndex(index);
		aCameraPtr->ExposureMode.SetValue(
				ExposureModeEnums::ExposureMode_TriggerWidth);
		aCameraPtr->ExposureAuto.SetValue(
				ExposureAutoEnums::ExposureAuto_Continuous);
		aCameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Continuous);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

unsigned int CameraManager::GetBufferCount(int index) {
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
				<< "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..."
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

void CameraManager::Clear() {

	if (LOGGING) {
		s_SystemPtr->UnregisterLoggingEventHandler(s_LoggingEventHandler);
	}
	for (int i = 0; i < s_CameraList.GetSize(); i++) {
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
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1,
				pImage->GetData(), pImage->GetStride());
	} else if (bbp == 12) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1,
				pImage->GetData(), pImage->GetStride());
	} else if (bbp == 16) {
		return cv::Mat(height + paddingHeight, width + paddingWidth, CV_16UC1,
				pImage->GetData(), pImage->GetStride());
	}
	return cv::Mat(height + paddingHeight, width + paddingWidth, CV_8UC1,
			pImage->GetData(), pImage->GetStride());
}


GetImageResponse* SteroPair(int indexLeft, int indexRight){

	 GetImageResponse* res = new GetImageResponse[2] ;

	 auto getImage = [&](int index){
			int max_capture_attempts = 3;
			int capture_attempts = 0;
			while( !res[index].isSuccess) {
				if ( capture_attempts) {
					cout <<  res[index].errorMessage << endl;
				}
			   res[index] =  CameraManager::GetImageFromCamera(index,PixelFormatEnums::PixelFormat_Mono8);
			   capture_attempts ++;
			   if ( capture_attempts > max_capture_attempts){
				   cout << "Capture:" << index << "Giving Up in Camera:" << index << endl;
				   break;
			   }
			}
	 };
	 thread left(getImage,indexLeft);
	 thread right(getImage,indexRight);

	 left.join();
	 right.join();

	 return res;
}




////////////////////////////////////////////////////////////////////////////////////////


GigECamera::GigECamera(int index, PixelFormatEnums f) :
		m_index(index), m_f(f) {
	m_CameraPtr = System::GetInstance()->GetCameras().GetByIndex(index);
	m_CameraPtr->Init();
	m_CameraPtr->PixelFormat.SetValue(f);
	m_CameraPtr->AcquisitionMode.SetValue(
			AcquisitionModeEnums::AcquisitionMode_Continuous);
}

GetImageResponse GigECamera::GetImage(float exposure) {
	GetImageResponse ret;
	ImageProcessor imageProcessor;
	if (exposure != 0) {
		SetExposure(exposure);
	}
	if (m_CameraPtr->IsValid()) {
		ImagePtr pImage = m_CameraPtr->GetNextImage();
		if (!pImage->IsIncomplete()) {
			ret.payload = imageProcessor.Convert(pImage, m_f);
			pImage->Release();
			ret.isSuccess = true;
		} else {
			ret.isSuccess = false;
		}
	} else {
		ret.isSuccess = false;
	}
	return ret;
}

bool GigECamera::IsInValidState() {
	return m_CameraPtr != nullptr ? m_CameraPtr->IsValid() : false;
}

string GigECamera::GetDeviceInfo() {
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
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
	return ss.str();
}

double GigECamera::GetExposure() {
	return m_CameraPtr->ExposureTime.GetValue();
}

void GigECamera::SetExposure(float exposure) {
	try {

		m_CameraPtr->ExposureMode.SetValue(
				ExposureModeEnums::ExposureMode_Timed);
		m_CameraPtr->ExposureAuto.SetValue(ExposureAutoEnums::ExposureAuto_Off);
		m_CameraPtr->ExposureTime.SetValue(exposure);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}
void GigECamera::SetAutoExposure() {
	try {

		m_CameraPtr->ExposureMode.SetValue(
				ExposureModeEnums::ExposureMode_TriggerWidth);
		m_CameraPtr->ExposureAuto.SetValue(
				ExposureAutoEnums::ExposureAuto_Continuous);
		m_CameraPtr->GainAuto.SetValue(GainAutoEnums::GainAuto_Continuous);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

GigECamera::~GigECamera() {
	try {
		m_CameraPtr->EndAcquisition();
		m_CameraPtr->DeInit(); // DeInit Disconnects camera port, resets camera back to read access
		m_CameraPtr = nullptr;
	} catch (...) {
		m_CameraPtr = nullptr;
	}
}

} // lmc
