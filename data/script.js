function startBlinking() {
    fetch('/StartBlinking')
        .then(response => response.text())
        .then(data => console.log(data));
}

function stopBlinking() {
    fetch('/StopBlinking')
        .then(response => response.text())
        .then(data => console.log(data));
}
