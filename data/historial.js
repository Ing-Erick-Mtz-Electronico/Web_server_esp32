const descarga = document.getElementById('btn');
const tabla = document.querySelector('#tabla');

const ordenTabla = ['sensor','medicion','fecha','hora']



function download(filename, text) {
    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
    element.setAttribute('download', filename);

    element.style.display = 'none';
    document.body.appendChild(element);

    element.click();

    document.body.removeChild(element);
}

//evento de descarga
function btDesc(){
    var i=0, j = 1,k=0,doctxt='';
   
    const sensor = document.querySelector('.list-sensor').value;
    fetch('/'+sensor)
    .then(response => {
        if(response.ok){
            response.text().then(coleccion => {
                if(coleccion !== 'error DB'){
                    for (i in coleccion){
                        if(coleccion[i]=='}'){
                            k = parseInt(i)+2;
                            doctxt = doctxt + coleccion.substring(j,k)+'\n';
                            j = k;
                        } 
                    }
                    //download(sensor+'.txt',doctxt);
                }else{
                    alert('Ingrese correctamente la direccion IP de la base de datos');
                }
            });
        }}).catch(() =>{
                    alert('Sin respuesta')
                });
    
};

descarga.addEventListener('click',btDesc,false);

mapa.on('click',e =>{
    var ipServidor = Servidor.value;
    var ipDataBase = DataBase.value;
    if(ipServidor == '' || ipDataBase == ''){
        alert('Se requieren ambas direcciones IP');
        return;
    }else{
        mapa.removeLayer(marcador);
        let coord = mapa.mouseEventToLatLng(e.originalEvent);
        marcador = L.marker([coord.lat,coord.lng]);
        marcador.bindPopup('Se muestran datos recientes dentro de un radio de 200\nmts a este punto: '+coord.lat+','+coord.lng).addTo(mapa);
        var coorString = JSON.stringify(coord);
        fetch('http://'+ipServidor+':3000/coordenadas/'+coorString+'&'+ipDataBase)
        .then(response => {
            if(response.ok){
                response.json().then(coleccion => {
                    if(coleccion.estado !== 'error DB'){

                        if(coleccion.estado !== 'fallido'){

                            const cuerpoTabla = document.createElement('tbody');
                            var cnt = tabla.children.length
                        
                            for(i=1;i<cnt;i++ ){
                                var rm = tabla.children[i]
                                tabla.removeChild(rm)
                            }
                            coleccion.forEach(D =>{
                                const fila = document.createElement('tr');
                                for(i in ordenTabla){
                                    const td = document.createElement('td');
                                    if(ordenTabla[i] == 'sensor'){
                                        td.innerText = D[ordenTabla[i]]+D['und'];
                                        fila.appendChild(td);
                                    }
                                    else{
                                        td.innerText = D[ordenTabla[i]]
                                        fila.appendChild(td);
                                    }
                                    
                                }
                                cuerpoTabla.appendChild(fila);
                            });
                            tabla.appendChild(cuerpoTabla);
                        }else{
        
                            const cuerpoTabla = document.createElement('tbody');
                            var cnt = tabla.children.length
                            for(i=1;i<cnt;i++ ){
                                var rm = tabla.children[i]
                                tabla.removeChild(rm)
                            }
                            const fila = document.createElement('tr');
                            for(i in ordenTabla){
                                const td = document.createElement('td');
                                td.innerHTML = '<br>';
                                fila.appendChild(td);
                            }
                            cuerpoTabla.appendChild(fila);
                            tabla.appendChild(cuerpoTabla);
                            alert('NO HAY DATOS CERCA DE LA COORDENADA');
                        }
                        
                    }else{
                        alert('Ingrese correctamente la direccion de la base de datos');
                    }
                });
            }}).catch(() =>{
                        alert('Ingrese correctamente la direccion del servisor')
                    });
    }
    
});