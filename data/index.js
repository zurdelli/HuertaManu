const btnGuardar = document.getElementById("btn-save");
const btnRele = document.getElementById("btn-rele");
const btnPlus = document.getElementById("btn-plus");
const btnCarga = document.getElementById("btn-datos");

let output = document.getElementById("outputHorario");

let contHorarios = 1;
let stateRele = false;
let listaHorarios = [];

btnRele.classList.add("releOn");


btnGuardar.addEventListener("click", sendDataHuerta, false);
btnPlus.addEventListener("click", creaHorarios);

btnCarga.addEventListener("click", getDataHuerta);

btnRele.addEventListener("click", function() {
    const xhttp = new XMLHttpRequest();
    xhttp.open('POST', 'rele'); 
    xhttp.setRequestHeader('Content-Type', 'text/plain');
    xhttp.send();

    handleStateRele();
});

function sendDataHuerta(event){
    const xhttp = new XMLHttpRequest();

    let horariosEl = document.getElementById("horarios");
    let minutosEl = document.getElementById("minutos");
    let d = new Date();
    
    // Creamos objeto para enviar
    let huertaData = {
        // horas: horariosEl.value,
        // Se divide por 1000 ya que se debe pasar en segundos, y
        // getTime retorna milisegundos. Se suma 7200 ya que 7200 segundos
        // = 2 horas, y el reloj RTC no funciona con timezones
        horaActual: `${((d.getTime() / 1000)+7200).toFixed(0)}`,
        horas: getHorarios(),
        minutos: minutosEl.value,
        dias: creaDias()
    }

    let receivedMsg = `<h2>Horario actual</h2>
                       Horas: ${huertaData.horas}<br>
                       Minutos activo: ${huertaData.minutos}<br>
                       Días: ${translateWeekDays(huertaData.dias)}`

    verificador(huertaData.horas);
    //document.getElementById("lastChange").innerHTML = receivedMsg;

    // Lo pasamos a string
    let data = JSON.stringify(huertaData);

    xhttp.addEventListener('load', function(event) {
        console.log('OK', xhttp);
      });
    
      xhttp.addEventListener('error', function(event) {
        console.log('error', xhttp);
      });
    
      // creamos request con metodo POST al endpoint /save
      xhttp.open('POST', 'save'); 
      xhttp.setRequestHeader('Content-Type', 'application/json');

      // enviamos json
      xhttp.send(data);

      minutosEl.value = "";

}

function getDataHuerta(event){
    let xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == XMLHttpRequest.DONE) {
           if (xhttp.status == 200) {
			  console.log((xhttp.responseText));
			  let json = JSON.parse(xhttp.responseText);
            //   console.log();
			  
			  //let receivedMsg = 'Horas:' + json.id + ' ' + (json.status == 1 ? "ON" : "OFF");
			  let receivedMsg = `Horas: ${json.horas}, min: ${json.minutos}, dias: ${json.dias}`
              document.getElementById('lastChange').textContent = receivedMsg;
           }
           else {
              console.log('error', xhttp);
           }
        }
    };

    xhttp.open("POST", "info", true);
    xhttp.send();
}

function translateWeekDays(dayTextBox){

    const daysOfTheWeek = ["Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"];
    let diasSeparadosPorComas = "";
    for (let index = 0; index < dayTextBox.length; index++) {
        if(dayTextBox[index] != ','){
            if(index == dayTextBox.length - 1){
                diasSeparadosPorComas += daysOfTheWeek[dayTextBox[index]] 
            } else {
                diasSeparadosPorComas += daysOfTheWeek[dayTextBox[index]] + ", ";
            }
            
        }
    }
    return diasSeparadosPorComas;
}

function verificador(horariosEl){
    const regexp = /([0-2]\d[.][0-5]\d,?)\1*/;

    const ok = regexp.exec(horariosEl);

    // output.textContent = ok
    // ? `Is okay, ${ok[0]}`
    // : `${horariosEl} no es valido!`;


}

function creaDias(){
    const botLunes = document.getElementById("lunes-el");
    const botMartes = document.getElementById("martes-el");
    const botMiercoles = document.getElementById("miercoles-el");
    const botJueves = document.getElementById("jueves-el");
    const botViernes = document.getElementById("viernes-el");
    const botSabado = document.getElementById("sabado-el");
    const botDomingo = document.getElementById("domingo-el");
    
    let dias = "";

    if (botLunes.checked) dias+="1";
    if (botMartes.checked) dias+="2";
    if (botMiercoles.checked) dias+="3";
    if (botJueves.checked) dias+="4";
    if (botViernes.checked) dias+="5";
    if (botSabado.checked) dias+="6";
    if (botDomingo.checked) dias+="0";

    return dias;
}

function creaHorarios(){
    let horarioDiv = document.createElement("div");
    let inHorario = document.createElement("input");
    let btnRemove = document.createElement("button");

    horarioDiv.id="horario";

    inHorario.type = "time";
    inHorario.id = `time${contHorarios}`;

    btnRemove.type = "submit";
    btnRemove.id=`rmv${contHorarios}`;
    btnRemove.textContent="x";
    btnRemove.className="botonX";
    btnRemove.addEventListener("click", function(){
        horariosDiv.removeChild(horarioDiv);
        contHorarios--;
        if (contHorarios>1){
            let rmvAnterior = document.getElementById(`rmv${contHorarios-1}`);
            rmvAnterior.setAttribute("style", "visibility:visible;");
        }
    });



    if (contHorarios>1){
        let rmvAnterior = document.getElementById(`rmv${contHorarios-1}`);
        rmvAnterior.setAttribute("style", "visibility:hidden;");
    }
    
    
    let horariosDiv = document.getElementById("horario-div");
    
    horarioDiv.appendChild(inHorario);
    horarioDiv.appendChild(btnRemove);
    horariosDiv.appendChild(horarioDiv);
    contHorarios++;


}

function getHorarios(){
    
    for (let index = 0; index < contHorarios; index++) {
        let horarioInput = document.getElementById(`time${index}`);
        // console.log(horarioInput);
        listaHorarios[index] = horarioInput.value.replace(':','.');
    }

    return listaHorarios;
}

function handleStateRele(){
    
    if (stateRele) {
        btnRele.classList.replace("releOff","releOn");
        btnRele.textContent = "Encender bomba";
    } else {
        btnRele.classList.replace("releOn","releOff");
        btnRele.textContent = "Apagar bomba"
    }


    stateRele = !stateRele;
}