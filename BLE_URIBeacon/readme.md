# UriBeacon

UriBeacons advertise a bit of information (usually a URL) to any nearby device. This example creates a fully-functional UriBeacon; you just need to tell it what to broadcast.

The UriBeacon's default broadcast is the URL ``"http://uribeacon.org"``. You can replace this URL with your own by editing ``main.cpp``:

```
uriBeaconConfig = new URIBeaconConfigService(ble, params, !fetchedFromPersistentStorage, "http://uribeacon.org", defaultAdvPowerLevels);
```

Please note that the UriBeacon isn’t limitless in size. It can only accept 18 characters, with “HTTP://www” counting as one, and the suffix “.org” (or “.com”) counting as another. If your URL is very long, you’ll have to use services like [bit.ly](https://bitly.com/) and [tinyurl.com](http://tinyurl.com/) to get a short version. 

**Tip:** You can [learn more about UriBeacons](https://docs.mbed.com/docs/ble-intros/en/latest/mbed_Classic/URIBeacon/) in the mbed Classic version of this example.

# Running the application

## Requirements

The sample application can be seen on any BLE scanner or Physical Web application on a smartphone. Please install one of the following:

- The Physical Web app for [iOS](https://itunes.apple.com/us/app/physical-web/id927653608?mt=8) and for [Android](https://play.google.com/store/apps/details?id=physical_web.org.physicalweb).

- [nRF Master Control Panel](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) for Android.

- [LightBlue](https://itunes.apple.com/gb/app/lightblue-bluetooth-low-energy/id557428110?mt=8) for iPhone.

If you want something slightly more advanced, you can follow the [links on Google's project page](https://github.com/google/uribeacon/tree/uribeacon-final#contents)  to get client apps such as this [Android app](https://github.com/google/uribeacon/releases/tag/v1.2) (use the `uribeacon-sample-release.apk` download at the bottom of the page).

Hardware requirements are in the [main readme](https://github.com/ARMmbed/ble-examples/blob/master/README.md).

## Building instructions

Building instructions for all mbed OS samples are in the [main readme](https://github.com/ARMmbed/ble-examples/blob/master/README.md).

## Checking for success

1. Build the application and install it on your board as explained in the building instructions.

1. Open the BLE scanner on your phone.

1. Find your device.

1. Check that the URL is correct.

**Note:** UriBeacons have two modes: *configuration* and *normal*. The beacon goes into configuration mode on startup, and remains there for 60 seconds. For those 60 seconds, it's visible as configurable only in generic BLE scanners; Physical Web apps will not show the configurable beacon at all. After 60 seconds, the beacon switches to normal mode, and is then visible on all apps but is no longer configurable.
