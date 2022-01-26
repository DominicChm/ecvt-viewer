# ECVT Realtime Viewer
## How to use
1. `git clone https://github.com/DominicChm/ecvt-viewer.git` somewhere convenient
2. CD into the folder if you haven't already
### Building the frontend
1. `cd frontend` - CD into frontend from the project root
2. `npm install` - install dependencies
3. `npm run build` - builds necessary frontend files and readies them for upload
### Uploading the code
1. Make sure the ESP32 is plugged in via USB
2. `pio run -t uploadfs` - this uploads frontend (filesystem) data
3. `pio run -t upload` - this uploads the actual code
### Using the project
1. Make sure the ESP is plugged into a serial data source on pin `GPIO 16` and power
2. Connect to its WiFi network once it's started (default ssid `ecvt_data`, password `123456789`)
3. Go to http://ecvt.local/ (or http://192.168.1.1/) to open the interface.
   1. __NOTE: Make sure you use Chrome or the new Edge for file downloads to work!!! Untested in Safari but confirmed not working with Brave!__
4. View real-time data or download as needed!
### Modifying the data format
If the data struct changes, you need to modify it in two places.

___NOTE: MAKE SURE THE COMMUNICATIONS STRUCT IS PACKED BY ADDING `__attribute__((packed));` TO THE END OF IT!!!!!!!___ This makes sure there are no empty alignment bytes present that will break parsing in confusing ways. 
```c
struct Data
{
    //...
} __attribute__((packed));
```
1. Replace `Communication.h` in `./include` with the new (PACKED) version.
2. Modify the struct in `./frontend/src/App.svelte` to match the new format.
   1. Keep the object flat (no sub-structs or objects) or CSV serialization will break.
   2. The order and types need to be identical to the C struct, or parsing will break.
   3. IE. if an int16 representing sheave pressure was added, add `primarySheavePressure: ctypes.int16` in the same place as the C struct.
3. [Re-build the frontend](#building-the-frontend)
4. [Re-upload the code](#uploading-the-code)

The code you need to modify in `App.svelte`:
```javascript
const ct = ctypes.cStruct({
   startBytes: ctypes.uint16,
   time: ctypes.uint32,

    // Engine
    engineEngaged: ctypes.boolean,
    engineState: ctypes.int8,
    engineSpeed: ctypes.int16,
    enginePID: ctypes.int16,
    engineP: ctypes.int16,
    engineI: ctypes.int16,
    engineD: ctypes.int16,

    // Primary
    primaryState: ctypes.int8,
    primaryEnc: ctypes.int32,
    primaryLC: ctypes.int16,
    primaryPID: ctypes.int16,

    // Secondary
    secondaryState: ctypes.int8,
    secondaryEnc: ctypes.int32,
    secondaryLC: ctypes.int16,
    secondaryPID: ctypes.int16,
   });
```

## ESP Pinout Diagram
![ESP Pinout](https://i0.wp.com/microdigisoft.com/wp-content/uploads/2021/08/image-13.png?resize=1024%2C541&ssl=1)