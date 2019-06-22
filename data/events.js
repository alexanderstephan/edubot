$.ajaxSetup({timeout:1000});

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

function getData(id) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            if(this.responseText == 'ON') {
                    document.getElementById(id).value = this.response;
                    document.getElementById(id).style.background = '#03DAC6';
            }
            else {
                document.getElementById(id).value = 'OFF';
                document.getElementById(id).color = 'red';
            }
        }
    };
        xhttp.open("GET",readMode,true);
        xhttp.send();
}
setInterval(function() {
},5000);

function speedChanged(speed) {	
    $.get("?speed=" + speed );
}