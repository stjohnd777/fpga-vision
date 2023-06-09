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

#ifndef FLIR_SPINNAKER_DEVICE_ARRIVAL_EVENT_HANDLER_H
#define FLIR_SPINNAKER_DEVICE_ARRIVAL_EVENT_HANDLER_H

#include "Interface/IDeviceArrivalEventHandler.h"

namespace Spinnaker
{
    /**
     *  @defgroup SpinnakerEventClasses Spinnaker EventHandler Classes
     */

    /*@{*/

    /**
     *  @defgroup DeviceArrivalEventHandler_h DeviceArrivalEventHandler Class
     */

    /*@{*/

    /**
     * @brief An event handler for capturing the device arrival event.
     */
    class SPINNAKER_API DeviceArrivalEventHandler : public IDeviceArrivalEventHandler
    {
      public:
        /**
         * Default constructor.
         */
        DeviceArrivalEventHandler();

        /**
         * Virtual destructor.
         */
        virtual ~DeviceArrivalEventHandler();

        /**
         * Callback to the device arrival event.
         * 
         * @param pCamera Reference tracked CameraPtr object of the camera attached to the system
         */
        virtual void OnDeviceArrival(CameraPtr pCamera) = 0;

      protected:
        /**
         * Assignment operator.
         */
        DeviceArrivalEventHandler& operator=(const DeviceArrivalEventHandler&);
    };

    /*@}*/

    /*@}*/
} // namespace Spinnaker

#endif // FLIR_SPINNAKER_DEVICE_ARRIVAL_EVENT_HANDLER_H