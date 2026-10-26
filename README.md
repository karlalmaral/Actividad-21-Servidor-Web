# Actividad 21: Nuestro primer servidor web

## ¿De qué trata este proyecto?

Este proyecto consiste en crear nuestro **primer servidor web** desde cero usando **C++** y **sockets TCP** en **Linux**. 

En lugar de usar un servidor web como Apache o Nginx, nosotros mismos programamos uno que escuche peticiones, las entienda y devuelva páginas web. 

## ¿Cómo funciona una petición HTTP?

Cuando abrimos una página web, ocurre lo siguiente:

1. **El cliente pide algo** - Nuestro navegador (o `curl`) envía una petición HTTP al servidor. Normalmente es un `GET` para pedir un archivo.
2. **El servidor recibe la petición** - Nuestro programa `servidor.cpp` se queda escuchando en el puerto `8080` y cuando llega una petición, la muestra por la terminal.
3. **El servidor busca el archivo** - Si pedimos `/`, el servidor busca `public/index.html`. Si pedimos otro archivo, lo busca dentro de la carpeta `public/`.
4. **El servidor responde** - Si el archivo existe, responde con **cabeceras HTTP** y el contenido del archivo. Si no existe, responde con un **404 Not Found**. Si intentan salir de `public/` con `..`, también lo bloquea por seguridad.
5. **Se cierra la conexión** - Después de responder, el servidor cierra la conexión con ese cliente, pero sigue esperando más peticiones.

## Estructura de carpetas

```text
Actividad-21/
├── servidor.cpp      # Código del servidor web
├── servidor          # Ejecutable (se genera al compilar)
├── public/           # Aquí van los archivos que se pueden ver
│   └── index.html    # Página principal que se muestra al entrar
├── data/             # Carpeta para guardar datos si se necesita
├── .gitignore        # Archivos que no se suben a Git
└── README.md         # Este archivo
```

## ¿Cómo compilar el servidor?

Abrimos una terminal dentro de la carpeta `Actividad-21` y escribimos:

```bash
g++ -o servidor servidor.cpp
```

Con esto se crea el archivo ejecutable llamado `servidor`.

## ¿Cómo ejecutar el servidor?

Una vez compilado, lo ejecutamos con:

```bash
./servidor
```

Veremos un mensaje como este:

```text
=== SERVIDOR WEB INICIADO ===
Escuchando en http://localhost:8080
Sirviendo archivos desde: public/
Presiona Ctrl+C para detener
=============================
```

El servidor se quedará corriendo hasta que lo detengamos con `Ctrl+C`.

## ¿Cómo abrirlo?

Teniendo el servidor encendido, podemos abrirlo de dos formas:

- **Con el navegador**: Abrimos nuestro navegador web y vamos a [http://localhost:8080](http://localhost:8080)
- **Con curl (desde otra terminal)**: 

```bash
curl http://localhost:8080
```

Ahí veremos el contenido de `index.html`.

## Tecnologías usadas

- **C++** - Lenguaje de programación
- **Sockets TCP** - Para comunicar cliente y servidor por red
- **Linux** - Sistema operativo donde lo probamos
