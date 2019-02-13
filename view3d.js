let api;

Module.onRuntimeInitialized = async _ => {
    api = {
        init: Module.cwrap('init', 'number', ['number', 'number']),
        draw: Module.cwrap('drawTriangle', 'number', ['number']),
        getError: Module.cwrap('getError', 'number', []),
        isError: Module.cwrap('isError', 'number', []),
    };
    api.init(canvas.width, canvas.height);
    printError();
    
    drawFrame();
    canvas.onclick = drawFrame;
};

function drawFrame() {
    window.requestAnimationFrame( _ => api.draw(Math.random()));
    printError();
}



function printError() {
    if(api.isError() == 1) {
        console.log(Pointer_stringify(api.getError()));
    }
}
