#!/bin/sh

# Get prerequisite packages:
PREREQUISITE_PACKAGES="build-essential g++ libusb-1.0-0-dev zlib1g-dev libpng-dev libjpeg-dev libtiff-dev libasound2-dev libspeex-dev libopenal-dev libv4l-dev libdc1394-22-dev libtheora-dev libbluetooth-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev"
echo "Please enter your password to install Vrui's prerequisite packages"
sudo apt-get install $PREREQUISITE_PACKAGES
INSTALL_RESULT=$?

if [ $INSTALL_RESULT -ne 0 ]; then
	echo "Problem while downloading prerequisite packages; please fix the issue and try again"
	exit $INSTALL_RESULT
fi

# Create src directory:
echo "Creating source code directory $HOME/src"
cd $HOME
mkdir src
cd src
CD_RESULT=$?

if [ $CD_RESULT -ne 0 ]; then
	echo "Could not create source code directory $HOME/src. Please fix the issue and try again"
	exit $CD_RESULT
fi

# Determine current Vrui version:
#VRUI_CURRENT_RELEASE=$(wget -q -O - http://idav.ucdavis.edu/~okreylos/ResDev/Vrui/CurrentVruiRelease.txt)
#GETVERSION_RESULT=$?
#if [ $GETVERSION_RESULT -ne 0 ]; then
#	echo "Could not determine current Vrui release number; please check your network connection and try again"
#	exit $GETVERSION_RESULT
#fi
#read VRUI_VERSION VRUI_RELEASE <<< "$VRUI_CURRENT_RELEASE"
VRUI_VERSION="3.0"
VRUI_RELEASE="002"

# Download and unpack Vrui tarball:
echo "Downloading Vrui-$VRUI_VERSION-$VRUI_RELEASE into $HOME/src"
wget -O - http://idav.ucdavis.edu/~okreylos/ResDev/Vrui/Vrui-$VRUI_VERSION-$VRUI_RELEASE.tar.gz | tar xfz -
cd Vrui-$VRUI_VERSION-$VRUI_RELEASE
DOWNLOAD_RESULT=$?

if [ $DOWNLOAD_RESULT -ne 0 ]; then
	echo "Problem while downloading or unpacking Vrui; please check your network connection and try again"
	exit $DOWNLOAD_RESULT
fi

# Determine the number of CPUs on the host computer:
NUM_CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

# Build Vrui:
echo "Building Vrui on $NUM_CPUS CPUs"
make -j$NUM_CPUS
BUILD_RESULT=$?

if [ $BUILD_RESULT -ne 0 ]; then
	echo "Build unsuccessful; please fix any reported errors and try again"
	exit $BUILD_RESULT
fi

# Install Vrui
echo "Build successful; installing Vrui in $HOME/Vrui-$VRUI_VERSION"
make install
INSTALL_RESULT=$?

if [ $INSTALL_RESULT -ne 0 ]; then
	echo "Could not install Vrui in $HOME/Vrui-$VRUI_VERSION. Please fix the issue and try again"
	exit $INSTALL_RESULT
fi

# Build Vrui example applications
cd ExamplePrograms
echo "Building Vrui example programs on $NUM_CPUS CPUs"
make INSTALLDIR=$HOME/Vrui-$VRUI_VERSION -j$NUM_CPUS
BUILD_RESULT=$?

if [ $BUILD_RESULT -ne 0 ]; then
	echo "Build unsuccessful; please fix any reported errors and try again"
	exit $BUILD_RESULT
fi

# Install Vrui example applications
echo "Build successful; installing Vrui example programs in $HOME/Vrui-$VRUI_VERSION"
make INSTALLDIR=$HOME/Vrui-$VRUI_VERSION install
INSTALL_RESULT=$?

if [ $INSTALL_RESULT -ne 0 ]; then
	echo "Could not install Vrui example programs in $HOME/Vrui-$VRUI_VERSION. Please fix the issue and try again"
	exit $INSTALL_RESULT
fi

# Run ShowEarthModel
echo "To test installtion, try running: ~/Vrui-$VRUI_VERSION/bin/ShowEarthModel"
