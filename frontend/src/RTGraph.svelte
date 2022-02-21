<script>
    import {onMount} from "svelte";

    export let ymin = 0;
    export let ymax = 100;
    export let zoom = 100; //same unit as time
    export let currentTime = 0;
    export let xkey = "time";
    export let key;
    export let data;

    if (!data || !key) throw new Error("NO DATA OR KEY PASSED!");

    let canvas;


    onMount(() => {
        const ctx = canvas.getContext('2d');
        let frame = requestAnimationFrame(loop);
        console.log("SDAVBFHASG")

        function scalePoint(x, y) {
            let xmin = currentTime - zoom;
            let xmax = currentTime;
            return [canvas.width * (x - xmin) / (xmax - xmin), canvas.height * (1 - (y - ymin) / (ymax - ymin))]
        }

        function loop(t) {
            frame = requestAnimationFrame(loop);

            let rootPoint = data[data.length - 1];
            if (!rootPoint || rootPoint[xkey] == null || rootPoint[key] == null) return;
            ctx.strokeStyle = "limegreen";
            ctx.lineWidth = 2;

            ctx.clearRect(0, 0, canvas.width, canvas.height);

            ctx.beginPath();
            ctx.moveTo(...scalePoint(rootPoint[xkey], rootPoint[key]));
            for (let i = data.length - 2; i >= 0 && data[i][xkey] > currentTime - zoom; i--) {
                ctx.lineTo(...scalePoint(data[i][xkey], data[i][key]));
            }
            ctx.stroke();
        }

        function handleResize() {
            canvas.width = canvas.clientWidth;
            canvas.height = canvas.clientHeight;
        }

        window.addEventListener("resize", handleResize);
        handleResize();
        return () => {
            cancelAnimationFrame(frame);
            window.removeEventListener("resize", handleResize);
        };

    })


</script>

<style>
    canvas {
        border: 1px solid white;
        background: #222;
        box-sizing: border-box;

    }
</style>

<canvas bind:this={canvas} style="width: 100%; height: 100%;">
</canvas>
