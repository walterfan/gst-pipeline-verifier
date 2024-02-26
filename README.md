# GStreamer Pipeline Verifier

Verifier of GStreamer Pipeline 

## Usage

```sh
./bin/gst-pipeline-verifier -p <pipeline_name> [-f <config_file>] [-v] [-l]
```
* -p <pipeline_name> : specify pipeline name that want to verify
                       the detailed pipeline configuration refers to config_file

* -f <config_file> : specify configuration file, it is ./etc/config.yaml by default

* -l : list the pipelines in configuration file

* -v : print current version



refer to [GStreamer cookbook](https://github.com/walterfan/gstreamer-cookbook)


## Environment

### Ubuntu

run the below command to intall gstreamer on ubuntu

```
sudo apt-get install -y build-essential \
cmake \
gstreamer1.0-tools \
libgstreamer1.0-dev \
libglib2.0-dev \
gstreamer1.0-nice \
gstreamer1.0-plugins-bad \
gstreamer1.0-plugins-ugly \
gstreamer1.0-plugins-good \
libgstreamer-plugins-bad1.0-dev \
gstreamer1.0-plugins-base-apps \
libfmt-dev \
libspdlog-dev \
libyaml-cpp-dev \
libjson-glib-dev \
libgtest-dev

```

for MACOS, run similar commands by brew, but suggest downloading package from gstream site

```sh
brew install gstreamer

brew install googletest
```


for ubuntu, the google test library may not be found

```sh
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo cmake --build . --target install

```

### SRS for stream publishing and subscrbing

* start SRS by docker

```sh
export CANDIDATE="192.168.0.108"
sudo docker run --rm --env CANDIDATE=$CANDIDATE \
  -p 1935:1935 -p 1975:8080 -p 1985:1985 -p 1995:8000/udp \
  registry.cn-hangzhou.aliyuncs.com/ossrs/srs:5 \
  objs/srs -c conf/rtmp2rtc.conf
```

* push local video stream from mp4 to rtmp

```
gst-launch-1.0 -vv filesrc location=material/talk.mp4 \
! decodebin \
! videoconvert ! identity drop-allocation=1 \
! x264enc tune=zerolatency ! flvmux streamable=true \
! rtmpsink location='rtmp://192.168.0.108:1935/live/waltertest'
```

* or run the C++ program

```
./build-with-vcpkg.sh
./bin/gst-pipeline-verify -f ./etc/config.yaml -p pipeline_test_rtmp
```


## build source code

```sh
sudo apt-get install -y \
gstreamer1.0-tools \
libgstreamer1.0-dev \
libglib2.0-dev \
gstreamer1.0-nice \
gstreamer1.0-plugins-bad \
gstreamer1.0-plugins-ugly \
gstreamer1.0-plugins-good \
libgstreamer-plugins-bad1.0-dev \
gstreamer1.0-plugins-base-apps \
libfmt-dev \
libspdlog-dev \
libyaml-cpp-dev \
libgtest-dev \
libsoup2.4-dev \
libjson-glib-dev

mkdir -p build
cd build
cmake ..
make
```

### use vcpkg

* if you have not install vcpkg, please install it first

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install spdlog
./vcpkg install fmt
./vcpkg install yaml-cpp
```

* build with dependencies by vcpkg

```
./build-with-vcpkg.sh
```

### use conan

* if you have not install conan, please install it first
```
python3 -m venv venv
source venv/bin/activate
pip install conan
conan profile detect --force
```

* create or update [conanfile.txt](conanfile.txt) for dependencies
  
* then run the following script to build
  
```
./build-with-conan.sh

```


