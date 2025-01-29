const convertApi = "http://localhost:3272"

async function convert(text) {
    const response = await fetch(convertApi, {
        method: 'POST',
        headers: {
            "Content-Type": "text/plain"
        },
        body: text
    });
    const value = await response.text();
    return value;
}