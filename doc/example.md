# Examples

## Deepstream test 1

You must have the following development packages installed

  - GStreamer-1.0
  - GStreamer-1.0 Base Plugins
  - GStreamer-1.0 gstrtspserver
  - X11 client-side library

To install these packages, execute the following command:

```
   sudo apt-get install libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev \
   libgstrtspserver-1.0-dev libx11-dev
```

### compile

```
cd /opt/nvidia/deepstream/deepstream/sources/apps/sample_apps/deepstream-test1
sudo make
```

### usage

  Two ways to run the application:

  1.Run with the h264 elementary stream. In this method, user needs to modify the source
    code of deepstream-test1-app to configure pipeline properties.

    $ ./deepstream-test1-app <h264_elementary_stream>

  2.Run with the yml file. In this method, user needs to update the yml file to configure
    pipeline properties.

    $ ./deepstream-test1-app <yml file>
    e.g. ./deepstream-test1-app dstest1_config.yml