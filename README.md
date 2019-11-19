VRController
============

This program is used to send data by Bluetooth to a VR android application as [RandCity]. Transmitted data are usually the rotation and the walk speed of the user to allow the app to move the player in a 3D world.

This application supports multiple plugins to control the orientation and the walk speed in differents ways:
* a controller with two dials to set the params
* an OpenNI plugin to analyse the images from a depth sensor (usually a Kinect)

Above plugins can be found in the controllers/ folder.

Third-party
-----------

VRController uses some third-party libraries to work correctly

* [Qt] - used for all GUIs
* [Jenkins] - used as the continous server integration
* [BlueZ] - default linux library for Bluetooth functionnality
* [QProgressIndicator] - add a progress indicator widget (released with LGPL license)
* [OpenNI] - a framework for 3D Depth Sensors and some usefull functionnalities like skeleton recognition (with the NiTE middleware)

Build Instructions
------------------

VRController only works on linux systems due to the use of the [BlueZ] library.
To build this project using qmake, run:

```sh
$ qmake VRController.pro -r -spec linux-g++
$ make
```

For the Bluetooth functionality, you need the BlueZ librairies and headers. On Ubuntu, you can install the packages "bluez" and "libbluetooth-dev".
If you don't want to use the bluetooth (usefull for testing without to wait for a bluetooth connection), add this in the qmake command:

```sh
CONFIG+=NO_BLUETOOTH
```

To be able to build the OpenNI controller, you will need the OpenNI libraries installed on your system. See more informations in the Install-OpenNI.md file. You will also need the OpenCV libs (if you are on Ubuntu, you can install the package "libopencv-dev") and the libusb dev files (on Ubuntu, it's the package "libusb-1.0-0-dev").cd c
If you don't want to build the OpenNI controller, use:

```sh
CONFIG+=NO_OPENNICONTROLLER
```

License
-------

VRController is licensed under the [GNU GPL] license v3. You can get a copy of this license in the LICENSE file at the top of this project hierarchy.


[Qt]:http://qt-project.org/
[Jenkins]:http://jenkins-ci.org/
[BlueZ]:http://www.bluez.org/
[GNU GPL]:http://www.gnu.org/licenses/gpl-3.0.en.html
[RandCity]:https://github.com/Toutjuste/RandCity
[QProgressIndicator]:https://github.com/mojocorp/QProgressIndicator
[OpenNI]:https://github.com/OpenNI/OpenNI

