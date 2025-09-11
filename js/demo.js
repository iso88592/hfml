let parserInfo = {};
let tokenizerList = [];

function tokenize(str) {
    const tokens = [];

    while (str.length > 0) {
        const match = tokenizerList.find(([regex]) => regex.test(str));

        if (!match) {
            return [`Syntax error near '${str}'`, tokens];
        }

        const [regex, name] = match;
        const found = str.match(regex)[0];

        const tokenInfo = parserInfo.tokens.find(t => t.name === name);
        const id = tokenInfo ? Number(tokenInfo.id) : 0;

        tokens.push([name, found, id]);
        str = str.slice(found.length);
    }

    tokens.push(["YYEOF", "YYEOF", 0]);
    return tokens;
}
function parse(str) {
    console.debug("==================");
    console.debug("Parsing `" + str + "`");
    const tokens = tokenize(str);
    let stateStack = [0];
    let symbolStack = [];
    let valueStack = [];
    let pos = 0;

    while (true) {
        const state = stateStack[stateStack.length - 1];
        const token = tokens[pos];

        console.debug(`State ${state}, token ${token[0]}`);

        const yyn = parserInfo.yypact[state];
        let action = 0;

        if (yyn === parserInfo.yyninf) {
            action = -parserInfo.yydefact[state];
        } else {
            const index = yyn + token[2];
            if (index >= 0 && index < parserInfo.yytable.length && parserInfo.yycheck[index] === token[2]) {
                action = parserInfo.yytable[index];
            } else {
                action = - parserInfo.yydefact[state];
            }
        }

        if (action === 0) {
            throw new Error(`Parse error at token ${token[1]} in state ${state}`);
        }
        if (action > 0) {
            console.debug(`shift -> state ${action}`);
            stateStack.push(action);
            symbolStack.push(token);
            valueStack.push(token);
            pos++;
            
        } else {
            const rule = -action;
            const lhs = parserInfo.yyr1[rule];
            const rhsLen = parserInfo.yyr2[rule];

            console.debug(`reduce by rule ${rule}, pop ${rhsLen}`);
            let children = [];

            stateStack.splice(-rhsLen);
            symbolStack.splice(-rhsLen);
            children = valueStack.splice(-rhsLen, rhsLen);

            const top = stateStack[stateStack.length - 1];
            const gotoIndex = parserInfo.yypgoto[lhs - parserInfo.yyntokens] + top;
            const gotoState = (gotoIndex >= 0 && gotoIndex < parserInfo.yytable.length &&
                parserInfo.yycheck[gotoIndex] === top) ?
                parserInfo.yytable[gotoIndex] :
                parserInfo.yydefgoto[lhs - parserInfo.yyntokens];
            console.debug(`goto state ${gotoState} for lhs ${lhs}`);

            stateStack.push(gotoState);
            symbolStack.push({type:lhs});
            valueStack.push({type: lhs, children});
        }

        if (stateStack[stateStack.length - 1] === parserInfo.yyfinal && token[0] == "YYEOF") {
            console.debug("ACCEPTED");
            return valueStack[0];
        }

    }
}

async function createParser() {
    console.log("Creating parser...");
    tokenizerList.push([/^</, "OPEN_COMP"]);
    tokenizerList.push([/^>/, "CLOSE_COMP"]);
    tokenizerList.push([/^\[/, "OPEN_BR"]);
    tokenizerList.push([/^]/, "CLOSE_BR"]);
    tokenizerList.push([/^[(]/, "OPEN_PAREN"]);
    tokenizerList.push([/^[)]/, "CLOSE_PAREN"]);
    tokenizerList.push([/^:/, "COLON"]);
    tokenizerList.push([/^,/, "COMMA"]);
    tokenizerList.push([/^</, "OPEN_COMP"]);
    tokenizerList.push([/^>/, "CLOSE_COMP"]);
    tokenizerList.push([/^=/, "EQUALS"]);
    tokenizerList.push([/^#/, "HASH"]);
    tokenizerList.push([/^[a-zA-Z][a-zA-Z0-9_$]*/, "IDENTIFIER"]);
    tokenizerList.push([/^[0-9]+/, "NUMBER"]);
    tokenizerList.push([/^{[^{}]+}/, "LITERAL"]);
    tokenizerList.push([/^{/, "OPEN_STR"]);
    tokenizerList.push([/^}/, "CLOSE_STR"]);
    const response = await fetch("parser/hfml.json");
    parserInfo = await response.json();
    console.log("Parser info received.");

}

function pruneEpsilon(ast) {
    if (!ast || !ast.children) return ast;
    ast.children = ast.children
        .map(pruneEpsilon)
        .filter(c => c !== null);
    if (ast.children.length === 0) {
        return null;
    }
    return ast;
}

function collapseSingleChild(ast) {
    if (!ast || !ast.children) return ast;

    ast.children = ast.children.map(collapseSingleChild);

    while (ast.children.length === 1) {
        const onlyChild = ast.children[0];
        if (!onlyChild.children) break;
        ast = { ...onlyChild };
    }

    return ast;
}


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

function popupError(str) {
    const container = document.getElementById("toast-container");
    const toast = document.createElement("div");
    toast.className = "toast";
    toast.classList.add("error");
    toast.textContent = str;
    container.appendChild(toast);
    requestAnimationFrame(() => toast.classList.add("show"));
    setTimeout(()=>{
        toast.classList.remove("show");
        toast.addEventListener("transitionend", () => toast.remove());
    }, 3000);    
}

function popupInfo(str) {
    const container = document.getElementById("toast-container");
    const toast = document.createElement("div");
    toast.className = "toast";
    toast.classList.add("info");
    toast.textContent = str;
    container.appendChild(toast);
    requestAnimationFrame(() => toast.classList.add("show"));
    setTimeout(()=>{
        toast.classList.remove("show");
        toast.addEventListener("transitionend", () => toast.remove());
    }, 3000);    
}

function findTag(ast, tagName) {
    let result = [];
    if (ast.type === 29) {
        if (ast.children[1].children[0][1] === tagName) {
            result.push(ast);
        }
    }
    for (let child in ast.children) {
        let other = findTag(ast.children[child], tagName);
        result = result.concat(other);
    }
    return result;
}

function extractText(ast) {
    let result = "";
    if (ast.type === 22) {
        let text = ast.children[0][1];
        result += text.substring(1, text.length - 1);
    }
    for (let child in ast.children) {
        let other = extractText(ast.children[child]);
        result += other;
    }
    return result;
}


function processEvent(str) {
    const ast = collapseSingleChild(pruneEpsilon(parse(str)));
    console.log(ast);

    let events = findTag(ast, "event");
    if (events.length != 0) {
        popupInfo(extractText(ast));
        return;
    }
    let errors = findTag(ast, "error");
    if (errors.length != 0) {
        popupError(extractText(ast));
        return;
    }
    popupError("Unable to handle event!");
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
        popupError(e);
    }
    event.target.classList.remove("disabled");
}

function toggleFullscreen() {
    document.getElementById("display").classList.toggle("full");
}

createParser();