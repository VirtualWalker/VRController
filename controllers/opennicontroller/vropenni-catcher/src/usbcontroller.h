/*
 * This file is part of VRController.
 * Copyright (c) 2015 Fabien Caylus <toutjuste13@gmail.com>
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

/**
 * All classes in this file are used to control a sensor with the usb port.
 * For example, you can control the motor or the LEDs.
 * For an explanation of the kinect's USB protocol, see
 * <http://openkinect.org/wiki/Protocol_Documentation>
 */

#include <ni/XnUSB.h>
#include <ni/XnCppWrapper.h>
#include <QObject>
#include <QDebug>
#include <QString>

#define KINECT_VENDOR_ID 0x045e
#define KINECT_MOTOR_PRODUCT_ID 0x02b0
#define KINECT_CAMERA_PRODUCT_ID 0x02ae

// Represent all infos needed to connect to a device
struct USBDevicePath
{
    int vendor;
    int product;
    int bus;
    int device;

    // Return a string representation in the format:
    // "idVendor/idProduct@BusID/DeviceID"
    QString toString(bool useHexa = true) const
    {
        const int base = useHexa ? 16 : 10;
        const int leadingZero = useHexa ? 4 : 0;
        return QString("%1/%2@%3/%4").arg(vendor, leadingZero, base, QChar('0'))
                .arg(product, leadingZero, base, QChar('0'))
                .arg(bus, 0, 10)
                .arg(device, 0, 10);
    }
};

// Generic class used to send data in the USB port.
class USBController: public QObject
{
        Q_OBJECT

    private:
        XN_USB_DEV_HANDLE _dev;
        bool _init = false;

        int _lastAngle = 0;

    public:
        // The controller do nothing, use init() to start the controller.
        USBController(QObject *parent = nullptr) : QObject(parent) {}
        // Close the device
        ~USBController()
        {
            xnUSBCloseDevice(_dev);
        }

        XnStatus init(const USBDevicePath devicePath)
        {
            if(_init)
            {
                qWarning() << qPrintable(tr("The USB controller is already initialized !"));
                return 11;
            }

            XnStatus errorCode = XN_STATUS_OK;

            const QString devicePathStr = devicePath.toString();

            errorCode = xnUSBOpenDeviceByPath(devicePathStr.toStdString().c_str(), &_dev);
            if(errorCode != XN_STATUS_OK)
            {
                qWarning() << qPrintable(tr("Cannot open the usb device (%1). Error code: %3").arg(devicePathStr).arg(xnGetStatusString(errorCode)));
                return errorCode;
            }

            qDebug() << qPrintable(tr("USB device initialized (%1) !").arg(devicePathStr));
            _init = true;
            return errorCode;
        }

        // request and value depends on what you want to send
        XnStatus send(const XnUInt8& request, const XnUInt16& value)
        {
            if(!_init)
            {
                qWarning() << qPrintable(tr("The USB controller is not initialized !"));
                return 10;
            }

            XnStatus errorCode = XN_STATUS_OK;
            XnUChar emptyBuf[0x1];
            errorCode = xnUSBSendControl(_dev, XN_USB_CONTROL_TYPE_VENDOR, request, value, 0x0, emptyBuf, 0x0, 0);
            if(errorCode != XN_STATUS_OK)
                qWarning() << qPrintable(tr("Error when sending data to the USB device."));
            return errorCode;
        }

        bool initialized() const
        {
            return _init;
        }

        //
        // LEDs part
        //

        enum class LightType
        {
            LED_OFF = 0,
            LED_GREEN = 1,
            LED_RED = 2,
            LED_YELLOW = 3,
            LED_BLINK_YELLOW = 4,
            LED_BLINK_GREEN = 5,
            LED_BLINK_RED_YELLOW = 6
        };

        // Set the light to the specified color
        XnStatus setLight(LightType type)
        {
            return send(0x06, static_cast<XnUInt16>(type));
        }

        //
        // Motor part
        //

        // Move the sensor to the specified angle
        XnStatus moveToAngle(const int angle)
        {
            if(angle < -30 || angle > 30)
            {
                qWarning() << qPrintable(tr("You must specify an angle between -30° and 30°."));
                return 1;
            }

            _lastAngle = angle;

            // 0x31 is the request for the motor
            // The angle must be multiply by 2 before sending
            return send(0x31, (XnUInt16)(angle*2));
        }

        XnStatus increaseAngle()
        {
            return moveToAngle(_lastAngle + 2);
        }

        XnStatus decreaseAngle()
        {
            return moveToAngle(_lastAngle - 2);
        }
};


#endif // USBCONTROLLER_H
