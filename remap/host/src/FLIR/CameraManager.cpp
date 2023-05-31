#include "CameraManager.h"

//
// Created by e438262 on 5/3/2023.
//

#include <opencv2/highgui/highgui.hpp>
#include <algorithm>
#include <exception>

using namespace cv;


namespace lmc {

void LoggingEventHandlerImpl::OnLogEvent( LoggingEventDataPtr loggingEventDataPtr) {
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

std::map<int,CameraManager*> CameraManager::instances;
 std::map<int,CameraManager *> instances;
 SystemPtr CameraManager::m_SystemPtr ;
 CameraList CameraManager::m_CameraList;


CameraManager::State CameraManager::state = UNKNOWN;
PixelFormatEnums CameraManager::defaultPixelFormat = PixelFormat_Mono16;
bool CameraManager::LOGGING = false;
unsigned int CameraManager::BUFFER_COUNT = 2;

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

CameraManager *CameraManager::GetInstance(int index) {

	CameraManager * ret = nullptr;
	if (  instances.find(index) != instances.end()) {
		ret=  instances[index];
	}else {
		instances[index] = new CameraManager(index);
		ret =  instances[index];
	}
	return ret;


}

CameraManager::CameraManager( int index) {
	try {

		cout << "FLIRCameraController(" << index<< ")" << endl;
		m_SystemPtr = System::GetInstance();
		cout << "FLIRCameraController::FLIRCameraController => Retrieves Singleton Reference to System object " << endl;

		if (LOGGING) {
			m_SystemPtr->RegisterLoggingEventHandler(m_LoggingEventHandler);
		}

		m_CameraList = m_SystemPtr->GetCameras();
		const unsigned int numberCamerasFound = m_CameraList.GetSize();
		cout << "m_CameraList = m_SystemPtr->GetCameras() ... Found " << numberCamerasFound << endl;
		if (numberCamerasFound == 0) {
			Clear(index);
			throw "No Cameras Found On System";
		}

		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
		cout << "We have a Camera Pointer for index " << index << endl;


		//  This function needs to be called before any camera related API calls such as BeginAcquisition(), EndAcquisition(), GetNodeMap(), GetNextImage()
		aCameraPtr->Init();
		cout << "FLIRCameraController::FLIRCameraController(" << index << ") => CameraPtr->Init() " << endl;
		INodeMap &nodeMap = aCameraPtr->GetNodeMap();
		cout<< "FLIRCameraController::FLIRCameraController => m_CameraPtr->GetNodeMap() "<< endl;
		aCameraPtr->AcquisitionMode.SetValue(AcquisitionModeEnums::AcquisitionMode_Continuous);
		cout<< "FLIRCameraController::FLIRCameraController =>m_CameraPtr->AcquisitionMode.SetValue "<< endl;
		SetPixelFormat(index,defaultPixelFormat);

		cout << "Buffer Count " << GetBufferCount(index) << endl;
		SetBufferCount(index,lmc::CameraManager::BUFFER_COUNT);
		cout << "Buffer Count " << GetBufferCount(index) << endl;

		aCameraPtr->BeginAcquisition();
		cout<< "FLIRCameraController::FLIRCameraController => m_CameraPtr->BeginAcquisition() "<< endl;


		m_ImageProcessor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
		state = ACQUIRING;

	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cerr << "Error: " << spinEx.GetErrorMessage() << endl;
		cerr << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
		Clear(index);
		state = HUNG;
		throw spinEx;
	}
}

bool CameraManager::IsInValidState(int index) {
	CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
	return aCameraPtr != nullptr ? aCameraPtr->IsValid() : false;
}

GetImageResponse CameraManager::GetImage(int index, uint64_t grabTimeout, uint64_t streamIndex) {

	GetImageResponse ret;
	bool isSuccess = false;
	stringstream ss;
	try {

		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
		std::lock_guard < std::mutex > guard(m_mutex);

		if (aCameraPtr->IsValid()) {

			cout << "Camera Is In Valid State:" << index <<  endl;

			cout << "GetNextImage(" << index << "," << grabTimeout << "," << streamIndex << ")"<< endl;
			ImagePtr pImage = aCameraPtr->GetNextImage(grabTimeout,streamIndex);

			if (!pImage->IsIncomplete()) {
				cout << "Get Image is Complete " << endl;
				cout << "Create Local Copy (Not on Camera in Buffer) Image with ImageProcessor  " << endl;
				ret.payload = m_ImageProcessor.Convert(pImage,defaultPixelFormat);
				// Release image Images retrieved directly from the camera (i.e. non-converted  images)  need to be released in order to keep from filling the buffer.
				cout<< "Release Image Tied to the Camera Buffer as to not fill the Camera Buffer"<< endl;
				pImage->Release();
				isSuccess = true;
				ret.isSuccess = isSuccess;

			} else {

				isSuccess = false;
				ret.isSuccess = isSuccess;
				ss << "Image incomplete: "<< Image::GetImageStatusDescription(pImage->GetImageStatus()) << "..." << endl;
				cout << ss.str();
				ret.setErrorMessage(ss.str());
			}
		} else {

		}

		return ret;

	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		ss << "Error: " << spinEx.GetErrorMessage() << endl;
		ss << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
		cout << ss.str();
		ret.setErrorMessage(ss.str());
		return ret;
	}

}

string CameraManager::GetDeviceInfo(int index) {
	stringstream ss;
	try {
		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
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
	CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
	return aCameraPtr->ExposureTime.GetValue();
}

void CameraManager::SetExposure(int index, float exposure) {
	try {
		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
		aCameraPtr->ExposureMode.SetValue( ExposureModeEnums::ExposureMode_Timed);
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
		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
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


void CameraManager::SetPixelFormat(int index, PixelFormatEnums f) {
	try {
		CameraPtr aCameraPtr = m_CameraList.GetByIndex(index);
		aCameraPtr->PixelFormat.SetValue(f);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

void CameraManager::SetPixelMono8(int index) {
	try {
		m_CameraList.GetByIndex(index)->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono8);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

void CameraManager::SetPixelMono12(int index) {
	try {
		m_CameraList.GetByIndex(index)->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono12);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}

}

void CameraManager::SetPixelMono16(int index) {
	try {
		m_CameraList.GetByIndex(index)->PixelFormat.SetValue(PixelFormatEnums::PixelFormat_Mono16);
	} catch (std::exception &ex) {
		Spinnaker::Exception &spinEx = dynamic_cast<Spinnaker::Exception &>(ex);
		cout << "Error: " << spinEx.GetErrorMessage() << endl;
		cout << "Error code " << spinEx.GetError() << " raised in function "
				<< spinEx.GetFunctionName() << " at line "
				<< spinEx.GetLineNumber() << "." << endl;
	}
}

void CameraManager::SetBufferCount(int index,unsigned int numBuffers) {
	// Retrieve Stream Parameters device nodemap
	Spinnaker::GenApi::INodeMap &sNodeMap =m_CameraList.GetByIndex(index)->GetTLStreamNodeMap();

	// Retrieve Buffer Handling Mode Information
	CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode(
			"StreamBufferHandlingMode");
	if (!IsReadable(ptrHandlingMode) || !IsWritable(ptrHandlingMode)) {
		cout
				<< "Unable to set Buffer Handling mode (node retrieval). Aborting..."
				<< endl << endl;
		return;
	}

	CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
	if (!IsReadable(ptrHandlingModeEntry)) {
		cout
				<< "Unable to get Buffer Handling mode (Entry retrieval). Aborting..."
				<< endl << endl;
		return;
	}

	// Set stream buffer Count Mode to manual
	CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode(
			"StreamBufferCountMode");
	if (!IsReadable(ptrStreamBufferCountMode)
			|| !IsWritable(ptrStreamBufferCountMode)) {
		cout
				<< "Unable to get or set Buffer Count Mode (node retrieval). Aborting..."
				<< endl << endl;
		return;
	}

	CEnumEntryPtr ptrStreamBufferCountModeManual =
			ptrStreamBufferCountMode->GetEntryByName("Manual");
	if (!IsReadable(ptrStreamBufferCountModeManual)) {
		cout
				<< "Unable to get Buffer Count Mode entry (Entry retrieval). Aborting..."
				<< endl << endl;
		return;
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
		return;
	}

	// Display Buffer Info
	cout << endl << "Default Buffer Handling Mode: "
			<< ptrHandlingModeEntry->GetDisplayName() << endl;
	cout << "Default Buffer Count: " << ptrBufferCount->GetValue() << endl;
	cout << "Maximum Buffer Count: " << ptrBufferCount->GetMax() << endl;

	ptrBufferCount->SetValue(numBuffers);
}


unsigned int CameraManager::GetBufferCount(int index) {
	// Retrieve Stream Parameters device nodemap
	Spinnaker::GenApi::INodeMap &sNodeMap = m_CameraList.GetByIndex(index)->GetTLStreamNodeMap();

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

///////////////////////////////////////////////////////////////////////

double CameraManager::GetBlackLevel(int index) {
	return m_CameraList.GetByIndex(index)->BlackLevel.GetValue();
}

float CameraManager::GetGain(int index) {
	return m_CameraList.GetByIndex(index)->Gain.GetValue();
}

double CameraManager::GetGamma(int index) {
	return m_CameraList.GetByIndex(index)->Gamma.GetValue();
}


void CameraManager::Clear(int index ) {
	if (CameraManager::instances[index] != nullptr) {
		delete CameraManager::instances[index];
		CameraManager::instances[index] = nullptr;
		state = OPEN_CONFIGURABLE;
	}
}

CameraManager::~CameraManager() {

	cout << "~FLIRCameraController()" << endl;
	if (LOGGING) {
		m_SystemPtr->UnregisterLoggingEventHandler(m_LoggingEventHandler);
	}

	for ( int i ; i <  m_CameraList.GetSize(); i++){
		auto c = m_CameraList.GetByIndex(i);
		if (c) {
					try {
						c->EndAcquisition();
						c->DeInit();// DeInit Disconnects camera port, resets camera back to read access
						c = nullptr;
					} catch (...) {
						c = nullptr;
					}
				}
	}


	m_CameraList.Clear();
	if (m_SystemPtr) {
		m_SystemPtr->ReleaseInstance();
		m_SystemPtr = nullptr;
	}
}



} // lmc
