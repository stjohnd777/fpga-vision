//=============================================================================
// Copyright (c) 2001-2021 FLIR Systems, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

#ifndef FLIR_SPINNAKER_DEVICE_REMOVAL_EVENT_HANDLER_H
#define FLIR_SPINNAKER_DEVICE_REMOVAL_EVENT_HANDLER_H

#include "Interface/IDeviceRemovalEventHandler.h"

namespace Spinnaker
{
    /**
     * @defgroup SpinnakerEventClasses Spinnaker EventHandler Classes
     */

    /*@{*/

    /**
     *  @defgroup DeviceRemovalEventHandler_h DeviceRemovalEventHandler Class
     */

    /*@{*/

    /**
     * @brief An event handler for capturing the device removal event.
     */

    class SPINNAKER_API DeviceRemovalEventHandler : public IDeviceRemovalEventHandler
    {
      public:
        /**
         * Default Constructor
         */
        DeviceRemovalEventHandler();

        /**
         * Virtual Destructor
         */
        virtual ~DeviceRemovalEventHandler();

        /**
         * Device removal event callback.
         *
         * @param pCamera Reference tracked CameraPtr object of the camera removed from the system
         */
        virtual void OnDeviceRemoval(CameraPtr pCamera) = 0;

      protected:
        /**
         * Assignment operator.
         */
        DeviceRemovalEventHandler& operator=(const DeviceRemovalEventHandler&);
    };

    /*@}*/

    /*@}*/
} // namespace Spinnaker

#endif // FLIR_SPINNAKER_DEVICE_REMOVAL_EVENT_HANDLER_H