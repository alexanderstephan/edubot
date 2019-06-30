$.ajaxSetup({timeout:1000});

function speedChanged(speed) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if(this.readyState == 4 && this.status == 200) {
            document.getElementById('myRange').value = this.responseText;
        }
    };
    xhttp.open("GET", "setSpeed?speed="+speed, true);
    xhttp.send();
    // Legacy approach
    //$.get("?speed=" + speed );
}
 // Needs to be refactored by using a css class
function autoChanged(id) {
    var xhttp = new XMLHttpRequest();
    if(document.getElementById(id).value == 'OFF') {
        document.getElementById(id).style.background = '#FFC400';
        document.getElementById(id).value = 'ON';
        
        document.getElementById('follow').style.background = '#263238';
        document.getElementById('follow').value = 'LOCKED';

        xhttp.open("GET","setAuto?auto=1",true);
        xhttp.send();
    }
    else if(document.getElementById(id).value == 'ON') {
        document.getElementById(id).style.background = '#283593';
        document.getElementById(id).value = 'OFF';

        document.getElementById('follow').style.background = '#283593';
        document.getElementById('follow').value = 'OFF';
        xhttp.open("GET","setAuto?auto=0",true);
        xhttp.send();
    }
}

function followChanged(id) {
    var xhttp = new XMLHttpRequest();
    if(document.getElementById(id).value == 'OFF') {
        document.getElementById(id).style.background = '#FFC400';
        document.getElementById(id).value = 'ON';

        document.getElementById('auto').style.background = '#263238';
        document.getElementById('auto').value = 'LOCKED'
        xhttp.open("GET", "setFollow?follow=1", true);
        xhttp.send();
    }
    else if(document.getElementById(id).value == 'ON') {
        document.getElementById(id).style.background = '#283593';
        document.getElementById(id).value = 'OFF';

        document.getElementById('auto').style.background = '#283593';
        document.getElementById('auto').value = 'OFF';
        xhttp.open("GET", "setFollow?follow=0", true);
        xhttp.send();
    }
}

function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            if(this.responseText == '0') {
                document.getElementById('auto').value = 'ON';
                document.getElementById('auto').style.background = '#FFC400';

                document.getElementById('follow').style.background = '#263238';
                document.getElementById('follow').value = 'LOCKED';
            }
            else if(this.responseText == '1') {
                document.getElementById('follow').style.background = '#FFC400';
                document.getElementById('follow').value = 'ON'

                document.getElementById('auto').style.background = '#263238';
                document.getElementById('auto').value = 'LOCKED'
            }
            else{
                document.getElementById('auto').style.background = '#283593';
                document.getElementById('auto').value = 'OFF';

                document.getElementById('follow').style.background = '#283593';
                document.getElementById('follow').value = 'OFF';
            }
        }
    };
    xhttp.open("GET", "readMode" , true);
    xhttp.send();
}

function getDistance(){
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("distance").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readDistance", true);
    xhttp.send();
}

function getHumid(){
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("humidity").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readHumid", true);
    xhttp.send();
}

function getTemp(){
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("temperature").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readTemp", true);
    xhttp.send();
}

function init() {
    getData();
}

setInterval(function() {
    getDistance();
    getData();
    getTemp();
    getHumid();
 },2000);