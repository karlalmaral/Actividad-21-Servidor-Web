function mostrar(id, contenido) {
    const elemento = document.getElementById(id);

    if (elemento) {
        elemento.textContent = contenido;
    }
}

function iniciar() {
    const alumno = {
        nombre: "Ana",
        edad: 20,
        matricula: "00123"
    };

    mostrar(
        "demo1",
        [
            'nombre: "' + alumno.nombre + '" (' + typeof alumno.nombre + ")",
            "edad: " + alumno.edad + " (" + typeof alumno.edad + ")",
            'matricula: "' + alumno.matricula + '" (' + typeof alumno.matricula + ")"
        ].join("\n")
    );

    const textoJson = JSON.stringify(alumno, null, 2);
    mostrar("demo2", textoJson);

    mostrar(
        "demo3",
        [
            "typeof alumno: " + typeof alumno,
            "typeof textoJson: " + typeof textoJson
        ].join("\n")
    );

    const textoOtroAlumno = '{"nombre":"Luis","edad":22,"matricula":"00456"}';
    const otroAlumno = JSON.parse(textoOtroAlumno);
    mostrar(
        "demo4",
        [
            "nombre: " + otroAlumno.nombre + " (" + typeof otroAlumno.nombre + ")",
            "edad: " + otroAlumno.edad + " (" + typeof otroAlumno.edad + ")",
            "matricula: " + otroAlumno.matricula + " (" + typeof otroAlumno.matricula + ")",
            "tipo del objeto: " + typeof otroAlumno
        ].join("\n")
    );

    const textoInvalido = '{"nombre": "Carlos", edad: 21}';
    try {
        JSON.parse(textoInvalido);
        mostrar("demo5", "El texto inválido se convirtió sin errores.");
    } catch (error) {
        mostrar(
            "demo5",
            "JSON inválido capturado correctamente.\nError: " + error.message
        );
    }

    const claveAlumno = "actividad25_alumno";
    const lineasLocalStorage = [];

    try {
        const textoExistente = localStorage.getItem(claveAlumno);

        if (textoExistente === null) {
            const textoNuevo = JSON.stringify(alumno);
            localStorage.setItem(claveAlumno, textoNuevo);
            lineasLocalStorage.push("La clave no existía: se guardó el alumno en esta carga.");
            lineasLocalStorage.push("En las próximas cargas solo se leerán los datos guardados.");
        } else {
            lineasLocalStorage.push("La clave ya existía: no se vuelve a guardar el alumno.");
            lineasLocalStorage.push("En esta carga no se guardaron datos de nuevo: solo se leyeron.");
        }

        const textoRecuperado = localStorage.getItem(claveAlumno);
        const alumnoRecuperado = JSON.parse(textoRecuperado);
        lineasLocalStorage.push(
            "Alumno recuperado: " +
                alumnoRecuperado.nombre +
                ", " +
                alumnoRecuperado.edad +
                " años, matrícula " +
                alumnoRecuperado.matricula
        );
        lineasLocalStorage.push("Tipo del alumno recuperado: " + typeof alumnoRecuperado);
    } catch (error) {
        lineasLocalStorage.push(
            "No se pudo usar localStorage: " + error.message
        );
        lineasLocalStorage.push(
            "Las demás demostraciones continúan funcionando."
        );
    }

    mostrar("demo6", lineasLocalStorage.join("\n"));

    const objetoConFuncion = {
        nombre: "Elena",
        saludar: function () {
            return "Hola, soy " + this.nombre;
        }
    };
    const textoConFuncion = JSON.stringify(objetoConFuncion, null, 2);
    const objetoReconstruido = JSON.parse(textoConFuncion);

    mostrar(
        "demo7",
        [
            "typeof objetoConFuncion.saludar: " + typeof objetoConFuncion.saludar,
            "typeof objetoReconstruido.saludar: " + typeof objetoReconstruido.saludar,
            "",
            "JSON resultante (la función se omite):",
            textoConFuncion
        ].join("\n")
    );

    const copia = JSON.parse(JSON.stringify(alumno));
    const mismaReferencia = alumno === copia;
    const mismosCampos =
        alumno.nombre === copia.nombre &&
        alumno.edad === copia.edad &&
        alumno.matricula === copia.matricula;

    mostrar(
        "demo8",
        [
            "alumno === copia: " + mismaReferencia,
            "Campos con los mismos datos: " + mismosCampos,
            "Datos de la copia: " +
                copia.nombre +
                ", " +
                copia.edad +
                " años, matrícula " +
                copia.matricula
        ].join("\n")
    );
}

document.addEventListener("DOMContentLoaded", iniciar);
