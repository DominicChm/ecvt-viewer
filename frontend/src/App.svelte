<script lang="ts">
    import {onMount} from 'svelte'
    import TitledContainer from "./TitledContainer.svelte";
    import ctypes, {cStruct} from "c-type-util"

    let ws_connected = false;

    Uint8Array.from([1, 2, 3]);

    let ws;
    let lastData = new Uint8Array(0);
    let data = {engine: {}, primary: {}, secondary: {}}

    const ct = ctypes.cStruct({
        startBytes: ctypes.uint16,
        time: ctypes.uint32,
        // Engine
        engine: cStruct({
            engaged: ctypes.boolean,
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

    function handleMessage(event) {
        console.log('Message:', event.data);
        const evd = event.data;
        if (evd instanceof Blob) {
            evd.arrayBuffer().then(resolved => {
                const u8 = new Uint8Array(resolved);
                data = ct.readLE(resolved);
                lastData = u8;
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

</script>

<style>
    :global(body) {
        margin: 0;
        font-family: Arial, Helvetica, sans-serif;
    }

    table, th, td {
        border: 1px solid;
        border-collapse: collapse;
    }

    table {
        border: 1px solid #1C6EA4;
        background-color: #EEEEEE;
        width: 100%;
        text-align: left;
        border-collapse: collapse;
    }

    table td, table th {
        border: 1px solid #AAAAAA;
        padding: 3px 2px;
        width: 50%;
    }

    table tbody td {
        font-size: 13px;
    }

    table tr:nth-child(even) {
        background: #D0E4F5;
    }

    table thead {
        background: #1C6EA4;
        background: -moz-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
        background: -webkit-linear-gradient(top, #5592bb 0%, #327cad 66%, #1C6EA4 100%);
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
</style>

<div class="App">
    <p>Websocket: {ws_connected ? "Connected!" : "Disconnected."}</p>
    <p>Data: {lastData}</p>

    <TitledContainer title="General">
        <table>
            <thead>
            <td>Name</td>
            <td>Value</td>
            </thead>
            <tr>
                <td>Time</td>
                <td>{data.time}</td>
            </tr>

        </table>
    </TitledContainer>

    <TitledContainer title="Engine">
        <table>
            <thead>
            <td>Name</td>
            <td>Value</td>
            </thead>
            {#each Object.entries(data.engine) as [key, value]}
                <tr>
                    <td>{key}</td>
                    <td>{value}</td>
                </tr>
            {/each}
        </table>
    </TitledContainer>

    <TitledContainer title="Primary">
        <table>
            <thead>
            <td>Name</td>
            <td>Value</td>
            </thead>
            {#each Object.entries(data.primary) as [key, value]}
                <tr>
                    <td>{key}</td>
                    <td>{value}</td>
                </tr>
            {/each}
        </table>
    </TitledContainer>

    <TitledContainer title="Secondary">
        <table>
            <thead>
            <td>Name</td>
            <td>Value</td>
            </thead>
            {#each Object.entries(data.secondary) as [key, value]}
                <tr>
                    <td>{key}</td>
                    <td>{value}</td>
                </tr>
            {/each}
        </table>
    </TitledContainer>

</div>
