Install OpenNI
==============

The OpenNI SDK is required to use the kinect video using the OpenNI-Controller. Since OpenNI is no longer supported, you must follow special steps to install it on your system.

First, download the appropriate package for your system:

* For 32 bits linux : [Download Here](https://simple-openni.googlecode.com/files/OpenNI_NITE_Installer-Linux32-0.27.zip)
* For 64 bits linux : [Download Here](https://simple-openni.googlecode.com/files/OpenNI_NITE_Installer-Linux64-0.27.zip)

Next, follow above instructions. Note that the instructions are made for an x64 system. If you are in a 32 bits OS, simply replace "64" by "86".

- Extract the archive and go into it:
```sh
$ unzip OpenNI_NITE_Installer-Linux64-0.27.zip && cd OpenNI_NITE_Installer-Linux64-0.27
```

- Install the OpenNI SDK:
```sh
$ cd OpenNI-Bin-Dev-Linux-x64-v1.5.4.0
$ sudo ./install.sh
```

- Install the Sensor compatibility. If you use a kinect, move to:
```sh
$ cd ../kinect/Sensor-Bin-Linux-x64-v5.1.2.1
```
Otherwise, if you use an another depth sensor, for example one released by PrimeSense, move to:
```sh
$ cd ../Sensor-Bin-Linux-x64-v5.1.2.1
```
To install the sensor, use:
```sh
$ sudo ./install.sh
```

- To finish, install the NiTE middleware (required for skeleton compatibility):
```sh
$ cd ../../NITE-Bin-Dev-Linux-x64-v1.5.2.21
$ sudo ./install.sh
```

The OpenNI SDK is now successfully installed on your system. You can now build your own VRController app !

