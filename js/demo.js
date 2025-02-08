
async function load(path) {
    let start = new Date();
    const response = await fetch("/examples/" + path);
    let b1 = new Date();
    const text = await response.text();
    let b2 = new Date();
    const converted = await convert(text);
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