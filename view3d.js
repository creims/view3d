let api;

Module.onRuntimeInitialized = async _ => {
    api = {
        init: Module.cwrap('init', 'number', ['number', 'number']),
        draw: Module.cwrap('drawTriangle', 'number', []),
        getError: Module.cwrap('getError', 'number', []),
        isError: Module.cwrap('isError', 'number', []),
    };
    api.init(canvas.width, canvas.height);
    printError();
    drawFrame();
};

function drawFrame() {
    window.requestAnimationFrame(api.draw);
    printError();
}

function printError() {
    if(api.isError() == 1) {
        console.log(Pointer_stringify(api.getError()));
    }
}
