
datosjson = {
    "mediciones" : [{"temperatura":26.50,"humedad":33.69,"presion":1010.22,"altitud":25.23,"fecha":"2022-06-21T08:22:23"},
                    {"temperatura":24.50,"humedad":30.69,"presion":1011.22,"altitud":22.23,"fecha":"2022-06-22T05:22:23"},
                    {"temperatura":23.50,"humedad":35.69,"presion":1015.22,"altitud":27.23,"fecha":"2022-06-25T10:22:23"},
                    {"temperatura":29.50,"humedad":38.69,"presion":1020.22,"altitud":20.23,"fecha":"2022-06-26T05:22:23"},
                    {"temperatura":31.50,"humedad":33.69,"presion":1018.22,"altitud":21.23,"fecha":"2022-06-26T12:15:23"}]
}

dato = {
    "temperatura" : [{"medicion":23,"fecha":"22/07/2022","hora":"14:22:23"}]
}

function convertir(){
    var listSensores =["temperatura","presion","humedad","altitud"];
    var sensor ="temperatura";
    var obj = {}
    var obj2 = {}
    obj[sensor] = []
    var array = ""
    datosjson2.mediciones.forEach(e => {
        listSensores.forEach(i=>{
            obj2.medicion = e[i];
            obj2.fecha = e.fecha;
            obj2.hora = e.hora;
            obj[i].push(obj2);
            console.log(obj2);

        })
        obj2.medicion = e[sensor];
        obj2.fecha = e.fecha;
        obj2.hora = e.hora;
        obj[sensor].push(obj2);
        console.log(obj2);
    });

    array = JSON.stringify(obj);
    console.log(array)
}

//console.log(Math.round(new Date('1995-12-17T03:24:00').getTime()/1000.0))



function extData(sensor,data){
    list1 =[];
    list2 =[];

    data.mediciones.forEach(i => {
        list2.push(Math.round(new Date(i.fecha).getTime()));
        list2.push(i[sensor]);
        list1.push(list2);
        list2 = [];
    });

    return list1;
}

console.log(extData('humedad',datosjson));
