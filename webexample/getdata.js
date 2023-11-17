function getData(filename) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var data = JSON.parse(xhttp.responseText);
            for (kk in data) {
                document.getElementById(kk).value = data[kk];
            }
        }
    };
    xhttp.open("GET", filename, true);
    xhttp.send();
}