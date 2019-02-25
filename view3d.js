const rangePhi = document.getElementById('range_phi');
const labelPhi = document.getElementById('label_phi');
const rangeTheta = document.getElementById('range_theta');
const labelTheta = document.getElementById('label_theta');
const rangeZoom = document.getElementById('range_zoom');
const labelZoom = document.getElementById('label_zoom');

const wheelSensitivity = 0.5;
const dragSensitivity = 1.0;
const minZoom = 1.0;
const maxZoom = 20.0;

const defPhi = 0.0;
const defTheta = 0.0;
const defZoom = 8.0;

let api;
let mouseDown = false;
let lastX, lastY; // For tracking where user mouse is when clicked
let camera = {
    zoom: defZoom,
    phi: defPhi,
    theta: defTheta,
}

Module.onRuntimeInitialized = async _ => {
    api = {
        init: Module.cwrap('init', 'number', ['number', 'number']),
        draw: Module.cwrap('drawFrame', 'number', ['number', 'number', 'number']),
        getError: Module.cwrap('getError', 'number', []),
        isError: Module.cwrap('isError', 'number', []),
    };

    api.init(canvas.width, canvas.height);
    printError();
    drawFrame();
    
    canvas.onmousedown = e => {
        mouseDown = true;
        lastX = e.clientX;
        lastY = e.clientY;
    }
    canvas.onmouseup = _ => mouseDown = false;
    canvas.onmousemove = e => {
        if(!mouseDown) return;
        
        let dPhi = (e.clientY - lastY) * dragSensitivity;
        if(dPhi != 0) setPhi(camera.phi + dPhi);

        let dTheta = (e.clientX - lastX) * dragSensitivity;
        if(dTheta != 0) setTheta(camera.theta + dTheta);
        
        lastX = e.clientX;
        lastY = e.clientY;
    };
    canvas.addEventListener("wheel", e => setZoom(camera.zoom += Math.sign(e.deltaY) * wheelSensitivity));

    rangePhi.onchange = _ => setPhi(Number.parseFloat(rangePhi.value));
    rangeTheta.onchange = _ => setTheta(Number.parseFloat(rangeTheta.value));

    rangeZoom.min = minZoom;
    rangeZoom.max = maxZoom;
    rangeZoom.onchange = _ => setZoom(Number.parseFloat(rangeZoom.value));

    setPhi(defPhi);
    setTheta(defTheta);
    setZoom(defZoom);
};

function clamp(num, min, max) {
    return num <= min ? min : num >= max ? max : num;
}

function clock(num, max) {
    if(num > max) {
        num -= num * Math.floor(num / max);
    } else if(num < 0) {
        num += max;
    }

    return num;
}

function setPhi(newPhi) {
    newPhi = clock(newPhi, 360.0);
    labelPhi.textContent = newPhi;
    rangePhi.value = newPhi;
    camera.phi = newPhi;
    window.requestAnimationFrame(drawFrame);
}

function setTheta(newTheta) {
    newTheta = clock(newTheta, 360.0);
    labelTheta.textContent = newTheta;
    rangeTheta.value = newTheta;
    camera.theta = newTheta;
    window.requestAnimationFrame(drawFrame);
}

function setZoom(newZoom) {
    newZoom = clamp(newZoom, minZoom, maxZoom);
    labelZoom.textContent = newZoom;
    rangeZoom.value = newZoom;
    camera.zoom = newZoom;
    window.requestAnimationFrame(drawFrame);
}

function drawFrame() {
    api.draw(camera.zoom, camera.phi, camera.theta);
    printError();
}

function printError() {
    if(api.isError() == 1) {
        console.log(Pointer_stringify(api.getError()));
    }
}
