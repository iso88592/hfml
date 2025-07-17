function processLinks(text) {
    text = text.replace(/href=(['"])(.*?)\1/g, function(match, quote, url) {
        return `href="#" onclick="clickHfml('${url.replace(/'/g, "\\'")}')"`;
    });
    
    return text;
}


async function load(path) {
    let start = new Date();
    const response = await fetch("/examples/" + path);
    let b1 = new Date();
    const text = await response.text();
    let b2 = new Date();
    const converted = processLinks(await convert(text));
    let end = new Date();
    document.getElementById("source").innerText = text;
    document.getElementById("display").innerHTML = converted;
    document.getElementById("dsource").innerText = converted;
    document.getElementById("specs").innerText = "Page generated in " + (end.getTime() - start.getTime()) + " ms" + 
    " Fetch time: " + (b1.getTime() - start.getTime()) + " ms" + 
    " Conversion time: " + (end.getTime() - b2.getTime()) + " ms";
}

async function displayText() {
    const text = document.getElementById("source").innerText;
    const converted = await convert(text);
    document.getElementById("display").innerHTML = converted;
    document.getElementById("dsource").innerText = converted;
}

function changeTheme() {
    document.body.classList.toggle("dark");
    document.body.classList.toggle("light");
}

function clickHfml(path) {
    load(path);
}

function hfml_show(obj) {
    obj.classList.remove("hidden");
}

function hfml_hide(obj) {
    obj.classList.add("hidden");
}

function hfml_start_drag_move(e, obj) {
    window.dragged_element = obj;
    obj.start_x = e.pageX;
    obj.start_y = e.pageY;
    obj.dragged = true;
    const computedStyle = window.getComputedStyle(obj);
    obj.drag_left = parseInt(computedStyle.left, 10) || 0;
    obj.drag_top  = parseInt(computedStyle.top, 10) || 0;    
}
function hfml_stop_drag_move(e) {
    if (window.dragged_element == null) return;
    window.dragged_element.dragged = false;
    window.dragged_element = null;
}
function hfml_move_drag_move(e) {
    const obj = window.dragged_element;
    if (!obj) return;

    const dx = e.pageX - obj.start_x;
    const dy = e.pageY - obj.start_y;

    obj.drag_left += dx;
    obj.drag_top  += dy;

    const parent = obj.offsetParent || document.body;
    const parentRect = parent.getBoundingClientRect();
    const style = window.getComputedStyle(obj);

    const marginLeft   = parseInt(style.marginLeft)   || 0;
    const marginTop    = parseInt(style.marginTop)    || 0;
    const marginRight  = parseInt(style.marginRight)  || 0;
    const marginBottom = parseInt(style.marginBottom) || 0;

    const totalWidth  = obj.offsetWidth  + marginLeft + marginRight;
    const totalHeight = obj.offsetHeight + marginTop  + marginBottom;

    const maxLeft = parent.clientWidth  - totalWidth;
    const maxTop  = parent.clientHeight - totalHeight;

    obj.drag_left = Math.max(0, Math.min(obj.drag_left, maxLeft));
    obj.drag_top  = Math.max(0, Math.min(obj.drag_top, maxTop));

    obj.style.left = obj.drag_left + "px";
    obj.style.top  = obj.drag_top  + "px";

    obj.start_x = e.pageX;
    obj.start_y = e.pageY;

    e.preventDefault();
}


window.dragged_element = null;

window.addEventListener("load", function() {
    this.document.body.addEventListener("mousemove", hfml_move_drag_move);
    this.document.body.addEventListener("mouseup", hfml_stop_drag_move);
});

function processEvent(str) {
    if (str.includes("[event:")) {
        
    }
    if (str.includes("[error]")) {
        alert(str.replace(/.*{([^}]*)}.*/,"$1"));
    }
}

async function request(event, rq) {
    if (event.target.classList.contains("disabled")) {
        event.preventDefault();
        event.stopPropagation();
        return;
    }
    event.target.classList.add("disabled");
    try {
        const ctx = event.target.closest("context");
        const bind =ctx.getAttribute("bind");
        const request = bind + rq;
        const response = await fetch(request);
        const value = await response.text();
        processEvent(value);
    } catch (e) {
        alert(e);
    }
    event.target.classList.remove("disabled");
}