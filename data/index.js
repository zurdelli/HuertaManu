// let parrHoras = document.getElementById("horasEl");
// let parrMinutos = document.getElementById("minutosEl");
// let parrDias = document.getElementById("diasEl");

let parrLastChange = document.getElementById("footer");

const inpHoras = document.getElementById("horas");
const inpTiempo = document.getElementById("tiempo");
const inpDias = document.getElementById("dias");

const btn = document.getElementById("btnSave");

btn.addEventListener("click", function(){
    let innerHTML = `
    <h2>Last config</h2>
    <p>Horas: ${inpHoras.value}</p>
    <p>Tiempo encendido: ${inpTiempo.value}</p>
    <p>Dias: ${inpDias.value}</p>
    `

    parrLastChange.innerHTML = innerHTML;
});