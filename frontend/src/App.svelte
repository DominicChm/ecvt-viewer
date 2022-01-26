<script lang="ts">
    import streamSaver from 'streamsaver'
    import ctypes, {cStruct} from "c-type-util"
    import Papa from "papaparse"

    let ws_connected = false;

    Uint8Array.from([1, 2, 3]);

    let ws;
    let lastData = "NO DATA";
    let data = {engine: {}, primary: {}, secondary: {}}

    let saveState: {
        stream: WritableStream | null,
        numSaved: number,
        writer: WritableStreamDefaultWriter<any> | null
    } = {
        stream: null,
        writer: null,
        numSaved: 0,
    };

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

    function handleData(newData) {
        //console.log(saveState.writer);
        const cleanedData = {...newData};
        delete cleanedData.startBytes;

        data = cleanedData;

        const csv = Papa.unparse([cleanedData], {header: saveState.numSaved === 0});
        console.log(csv, newData);
        if (saveState.writer?.write(new TextEncoder().encode(csv + "\n")))
            saveState.numSaved += 1;

    }

    function handleMessage(event) {
        console.log('Message:', event.data);
        const evd = event.data;
        if (evd instanceof Blob) {
            evd.arrayBuffer().then(resolved => {
                const raw = new Uint8Array(resolved);
                handleData(ct.readLE(resolved));
                lastData = Array.from(raw).map(n => n.toString(16).padStart(2, "0")).join(" ");
            });
        }
    }

    function connect() {
        ws = new WebSocket(`ws://${window.location.hostname}:80/ws`);
        ws.onopen = function () {
            // subscribe to some channels
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
        saveState.stream = streamSaver.createWriteStream('ecvt_data.csv', {
            writableStrategy: undefined, // (optional)
            readableStrategy: undefined  // (optional)
        });

        saveState.writer = saveState.stream.getWriter();
        console.log("Started saving!");
    }

    window.addEventListener("beforeunload", function (e) {
        stopSave();
    }, false);

    function stopSave() {
        console.log("Stopping saving!");
        saveState.writer?.close();
        saveState = {
            numSaved: 0,
            stream: null,
            writer: null,
        }
    }

</script>

<style>
    :global(body) {
        margin: 0;
        font-family: Consolas, monospace;
        background: #424242;
        color: white;

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
    <strong>Make sure you're using Chrome or the new Edge!! File saving doesn't work on other browsers.</strong>
    <p class:wsOk={ws_connected}>Websocket: {ws_connected ? "Connected!" : "Disconnected."}</p>
    <p>Data: {lastData}</p>

    <table>
        <thead>
        <td>Name</td>
        <td>Value</td>
        </thead>
        {#each Object.entries(data) as [key, value]}
            <tr>
                <td>{key}</td>
                <td>{value}</td>
            </tr>
        {/each}
    </table>
    <div style="height: 10rem"></div>

    {#if (!saveState.stream)}
        <button disabled={!ws_connected} class="bigButton" on:click={startSave}>Start Saving</button>
    {:else}
        <button class="activated bigButton" on:click={stopSave}>Stop (Saved {saveState.numSaved})</button>
    {/if}
</div>
