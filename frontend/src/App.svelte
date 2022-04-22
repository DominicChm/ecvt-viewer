<script lang="ts">
    import {onDestroy} from "svelte";
    import ctypes from "c-type-util"
    import Papa from "papaparse"
    import RTGraph from "./RTGraph.svelte";

    let savedData = [];
    let savedDataLen = 0;
    let saving = false;
    let zoomlevel = 500000;
    // @ts-ignore
    //=========== MODIFY THIS STRUCT TO MATCH INCOMING DATA! ==============//
    const ct = ctypes.cStruct({
        time: ctypes.uint32,
        rwSpeed: ctypes.int16,

        eState: ctypes.int8,
        eSpeed: ctypes.int16,
        ePID: ctypes.int16,
        eP: ctypes.int16,
        eI: ctypes.int16,
        eD: ctypes.int16,

        pState: ctypes.int8,
        pEncoder: ctypes.int32,
        pLoadCellForce: ctypes.int16,
        pMotorCurrent: ctypes.int16,
        pControllerOutput: ctypes.int16,

        sState: ctypes.int8,
        sEncoder: ctypes.int32,
        sLoadCellForce: ctypes.int16,
        sMotorCurrent: ctypes.int16,
        sControllerOutput: ctypes.int16,
        sEncoderPID: ctypes.int16,
        sLoadCellPID: ctypes.int16,
        sLoadCellP: ctypes.int16,
        sLoadCellI: ctypes.int16,
        sLoadCellD: ctypes.int16,
    });


    let ws_connected = false;

    let ws;
    let lastData = "NO DATA";
    let data = {}

    let isGraphing = true;
    let graphState = {}
    let graphData = [];
    let currentTime = 0;

    function handleData(newData) {
        //console.log(saveState.writer);
        const cleanedData = {...newData};
        delete cleanedData.startBytes;

        data = cleanedData;
        if (saving) savedData.push(data);
        savedDataLen = savedData.length;

        //Update graph data.
        if (isGraphing) graphData.push(newData);
    }

    function handlePostMessage() {
        if (graphData.length < 2) return; //Only run this if we have enough data to be valid.

        currentTime = graphData[graphData.length - 1].time;
        //Delete useless data off the end of graphdata, and trigger a render.
        let i = 0;
        for (; graphData[i + 1].time < currentTime - zoomlevel; i++) ; //Find index of first valid time.

        graphData = graphData.slice(i);
    }

    function handleMessage(event) {
        //console.log('Message:', event.data);
        const evd = event.data;
        if (evd instanceof Blob)
            evd.arrayBuffer().then(parseData);
    }

    let rawBuf = new Uint8Array(10000);
    let rawLen = 0;

    function parseData(data: ArrayBuffer) {
        //Concat new data onto old.
        rawBuf.set(new Uint8Array(data), rawLen);
        rawLen += data.byteLength;

        while (rawLen > ct.size + 2) {
            if (!(rawBuf[0] === 0xAA && rawBuf[1] === 0xAA)) {
                rawBuf.set(rawBuf.subarray(1)); //Chop off first byte.
                rawLen--;
                continue;
            }

            const parsed = ct.readLE(rawBuf.buffer, rawBuf.byteOffset);
            rawLen -= ct.size;
            rawBuf.set(rawBuf.subarray(ct.size))

            lastData = Array.from(rawBuf.slice(0, ct.size)).map(n => n.toString(16).padStart(2, "0")).join(" ");
            handleData(parsed);
        }

        //console.log(rawBuf, rawLen);
        handlePostMessage();
    }

    function connect() {
        ws = new WebSocket(`ws://${window.location.hostname}:80/ws`);
        //ws = new WebSocket(`ws://192.168.1.2:80/ws`);

        ws.onopen = function () {
            ws_connected = true;
        };

        ws.onmessage = handleMessage;

        ws.onclose = function (e) {
            console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
            ws_connected = false;

            setTimeout(function () {
                connect();
            }, 1000);
        };

        ws.onerror = function (err) {
            console.error('Socket encountered error: ', err, 'Closing socket');
            ws.close();
        };
    }

    connect();

    function startSave() {
        savedData = [];
        saving = true;
        console.log("Started saving!");
    }

    window.addEventListener("beforeunload", function (e) {
        stopSave();
    }, false);

    function stopSave() {
        if (!saving) return;

        console.log("Stopping saving!");
        const data = Papa.unparse(savedData);
        download("ecvtdat.csv", data);
        saving = false;
    }

    function download(filename, text) {
        let element = document.createElement('a');
        element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
        element.setAttribute('download', filename);
        element.style.display = 'none';
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    }

    onDestroy(() => {
        if (ws)
            ws.close();
    })

    function pauseGraphs() {
        isGraphing = false;
    }

    function startGraphs() {
        graphData = [];
        isGraphing = true;
    }

</script>

<style>
    :global(body) {
        margin: 0;
        font-family: Consolas, monospace;
        background: #424242;
        color: white;
        /*overflow-x: hidden;*/
    }

    table, th, td {
        border: 1px solid;
        border-collapse: collapse;
    }

    table {
        border: 1px solid #1C6EA4;
        background-color: #282828;
        width: 100%;
        text-align: left;
        border-collapse: collapse;
    }

    table td, table th {
        border: 1px solid #000000;
        padding: 5px 7px;
        width: 50%;
    }

    table tbody td {
        font-size: 13px;
    }

    table tr:nth-child(even) {
        background: #1c1c1c;
    }

    table thead {
        background: #1C6EA4;
        background: linear-gradient(to bottom, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
        border-bottom: 2px solid #444444;
        color: white !important;
        font-weight: bold;
    }

    table thead th {
        font-size: 15px;
        font-weight: bold;
        color: #FFFFFF;
        border-left: 2px solid #D0E4F5;
    }

    table thead th:first-child {
        border-left: none;
    }

    .bigButton {
        width: 100%;
        height: 3rem;
        position: fixed;
        bottom: 0;
        background: #46ad42;
        border: outset 5px #46ad42;
        color: white;
        font-weight: bold;
        font-size: large;
        font-family: Consolas, monospace;
    }

    .activated {
        color: white;
        background: #a41c1c;
        border: inset 5px #a41c1c;

    }

    .wsOk {
        color: #0bde13;
    }

</style>

<div class="App">
    <p class:wsOk={ws_connected}>
        Websocket: {ws_connected ? "Connected!" : "Disconnected."}</p>
    <!--    <p>Data: {lastData}</p>-->

    <table>
        <thead>
        <td>Name</td>
        <td>Value</td>
        <td>Graph</td>
        </thead>
        {#each Object.entries(data) as [key, value]}
            <tr>
                <td>{key}</td>
                <td>{value}</td>
                <td><input type="checkbox" bind:checked={graphState[key]}></td>
            </tr>
        {/each}
    </table>
    <!--    <pre>{JSON.stringify(graphState)}</pre>-->
    <div>
        <span>Graph View: <input type="range" min="500000" max="30000000"
                                 bind:value={zoomlevel}> {zoomlevel / 1000}
            ms</span>
        {#if (isGraphing)}
            <button on:click={pauseGraphs}>Pause Trace</button>
        {:else}
            <button on:click={startGraphs}>Resume Trace</button>
        {/if}
    </div>

    {#each Object.entries(data) as [key, value]}
        {#if (graphState[key] === true)}
            <div style="width: 100%; height: 100%">
                <RTGraph data={graphData} key={key} currentTime={currentTime}
                         zoom={zoomlevel}/>
            </div>
        {/if}
    {/each}

    <div style="height: 10rem"></div>

    {#if (!saving)}
        <button disabled={!ws_connected} class="bigButton" on:click={startSave}>
            Start Saving
        </button>
    {:else}
        <button class="activated bigButton" on:click={stopSave}>Finalize
            (Saved {savedDataLen})
        </button>
    {/if}
</div>
