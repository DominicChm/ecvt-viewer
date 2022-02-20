const {WebSocketServer} = require('ws');
const ctypes = require("c-type-util");
const {cStruct} = require("c-type-util");

const wss = new WebSocketServer({port: 80, path: "/ws"});

console.log("Started");
const raw_ct = ctypes.cStruct({
    startBytes: ctypes.uint16,
    time: ctypes.uint32,
    // Engine
    engaged: ctypes.boolean,
    eState: ctypes.int8,
    eSpeed: ctypes.int16,
    ePID: ctypes.int16,
    eP: ctypes.int16,
    eI: ctypes.int16,
    eD: ctypes.int16,
    // Primary
    pState: ctypes.int8,
    pEnc: ctypes.int32,
    pLC: ctypes.int16,
    pPID: ctypes.int16,
    // Secondary
    sState: ctypes.int8,
    sEnc: ctypes.int32,
    sLC: ctypes.int16,
    sPID: ctypes.int16,
});

const ct = ctypes.cStruct({
    startBytes: ctypes.uint16,
    time: ctypes.uint32,
    // Engine
    engaged: ctypes.boolean,
    engine: cStruct({
        State: ctypes.int8,
        Speed: ctypes.int16,
        PID: ctypes.int16,
        P: ctypes.int16,
        I: ctypes.int16,
        D: ctypes.int16,
    }),

    // Primary
    primary: cStruct({
        State: ctypes.int8,
        Enc: ctypes.int32,
        LC: ctypes.int16,
        PID: ctypes.int16,
    }),

    // Secondary
    secondary: cStruct({
        State: ctypes.int8,
        Enc: ctypes.int32,
        LC: ctypes.int16,
        PID: ctypes.int16,
    }),
});

wss.on('connection', function connection(ws) {
    console.log("Connection!");

    ws.on('message', function message(data) {
        console.log('received: %s', data);
    });

    ws.send('something');
    setInterval(send_garbage, 1000);

    function send_garbage() {
        let dat = raw_ct.allocLE({
            startBytes: 0xAAAA,
            time: 0x01,
            // Engine
            engaged: false,
            eState: 1,
            eSpeed: 2,
            ePID: 3,
            eP: 4,
            eI: 5,
            eD: 6,

            // Primary
            pState: 1,
            pEnc: 2,
            pLC: 3,
            pPID: 4,

            // Secondary
            sState: 1,
            sEnc: 2,
            sLC: 3,
            sPID: 4,
        });
        dat = new Uint8Array(dat);
        //dat = Uint8Array.from([0xAA, 0xAA]);

        dat[0] = 0xaa;
        ws.send(new Uint8Array(dat).slice(dat.byteLength / 2));
        ws.send(new Uint8Array(dat).slice(0, dat.byteLength / 2));

        console.log(dat);
        //console.log(new Uint8Array(dat));
    }
});