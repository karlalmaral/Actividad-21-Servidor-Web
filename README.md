# Actividad 22: servidor de archivos estáticos

## ¿De qué trata este proyecto?

Este proyecto continúa el servidor web de la **Actividad 21**, programado desde cero en **C++** con **sockets TCP** sobre Linux. Ahora el servidor sirve archivos estáticos de forma genérica desde `public/`: páginas HTML, hojas de estilo, JavaScript, imágenes y un favicon.

No se usan Apache, Nginx ni bibliotecas externas. Solo se emplean C++17, la biblioteca estándar, `std::filesystem` y los sockets del sistema.

## Lo que se conserva de la Actividad 21

- El servidor escucha en el puerto `8080`.
- Atiende peticiones HTTP `GET`.
- La ruta `/` se convierte en `/index.html`.
- Los archivos se leen en modo binario.
- Se envían cabeceras `Content-Type`, `Content-Length` y `Connection: close`.
- Se responde `404` cuando el archivo no existe y `405` cuando el método no es `GET`.
- El servidor sigue esperando conexiones después de atender a cada cliente.
- La página principal de la Actividad 21 se conserva en `public/index.html`.

## ¿Cómo funciona una petición HTTP?

1. El navegador o `curl` envía una petición al servidor.
2. El servidor lee la primera línea de la petición, por ejemplo `GET /alumnos.html HTTP/1.1`.
3. Si hay una query string, como `/index.html?version=1`, se elimina antes de buscar el archivo.
4. La ruta se decodifica, se resuelve dentro de `public/` y se comprueba que el archivo esté realmente dentro de ese directorio.
5. Si el archivo existe, se lee en binario y se responde con `200 OK` y su `Content-Type`.
6. Si no existe o la ruta intenta salir de `public/`, se responde `404 Not Found`.
7. Después de responder, se cierra la conexión con ese cliente.

## Estructura de carpetas

```text
Actividad-21/
├── servidor.cpp          # Código del servidor web
├── servidor              # Ejecutable generado al compilar
├── public/               # Archivos que se pueden servir
│   ├── index.html        # Página principal
│   ├── login.html        # Página de ejemplo, sin autenticación
│   ├── alumnos.html      # Página de ejemplo, sin registro de alumnos
│   ├── materias.html     # Página de ejemplo
│   ├── favicon.svg       # Favicon en formato SVG
│   ├── favicon.ico       # Favicon compatible con navegadores antiguos
│   ├── css/
│   │   └── estilos.css
│   ├── js/
│   │   └── app.js
│   └── imagenes/
│       └── ejemplo.svg
├── data/                 # Carpeta reservada para datos
├── .gitignore            # Archivos que no se suben a Git
└── README.md             # Este archivo
```

## ¿Cómo compilar el servidor?

Desde la carpeta del proyecto:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -o servidor servidor.cpp
```

## ¿Cómo ejecutar el servidor?

```bash
./servidor
```

Veremos un mensaje similar a:

```text
=== SERVIDOR WEB INICIADO ===
Escuchando en http://localhost:8080
Sirviendo archivos desde: /ruta/al/proyecto/public
Presiona Ctrl+C para detener
=============================
```

El servidor se detiene con `Ctrl+C`.

## Páginas de ejemplo

Con el servidor encendido:

- [http://localhost:8080/](http://localhost:8080/)
- [http://localhost:8080/login.html](http://localhost:8080/login.html)
- [http://localhost:8080/alumnos.html](http://localhost:8080/alumnos.html)
- [http://localhost:8080/materias.html](http://localhost:8080/materias.html)

`login.html`, `alumnos.html` y `materias.html` son solo demostraciones de que el servidor entrega archivos. La autenticación y el registro de alumnos todavía no están implementados.

## Cómo se sirven los archivos

- Cualquier archivo dentro de `public/` se puede pedir con una ruta relativa. Por ejemplo, `/css/estilos.css` sirve `public/css/estilos.css`.
- La ruta `/` equivale a `/index.html`.
- La query string se ignora: `/index.html?version=1` sirve el mismo archivo que `/index.html`.
- Se reconocen varios tipos MIME: HTML, CSS, JavaScript, JSON, SVG, PNG, JPEG, GIF, WebP, ICO, fuentes, PDF, vídeo y otros. Los tipos desconocidos se envían como `application/octet-stream`.
- Los archivos se leen en modo binario, por lo que las imágenes y el favicon se entregan sin corromperse.
- Un archivo vacío se sirve correctamente con `200 OK` y `Content-Length: 0`.

## Seguridad de las rutas

La ruta solicitada se decodifica antes de comprobarla. Después se calcula su ruta canónica y se comparan sus componentes con los de la ruta canónica de `public/`. No basta con comparar un prefijo de texto.

Esto impide:

- Salir de `public/` mediante `..`.
- Usar variantes codificadas como `%2e%2e`.
- Acceder a enlaces simbólicos que apunten fuera de `public/`.
- Servir directorios en lugar de archivos.

Los errores de `std::filesystem` se manejan con `std::error_code`, sin lanzar excepciones que cierren el servidor.

## Agregar archivos sin recompilar ni reiniciar

Como el servidor lee el archivo en cada petición, basta con copiar un archivo nuevo dentro de `public/`:

```bash
echo "archivo nuevo" > public/nuevo.txt
curl http://localhost:8080/nuevo.txt
```

No hace falta recompilar ni reiniciar el servidor.

## Pruebas rápidas

Con el servidor corriendo:

```bash
curl -i http://localhost:8080/
curl -i 'http://localhost:8080/index.html?version=1'
curl -i http://localhost:8080/css/estilos.css
curl -i http://localhost:8080/js/app.js
curl -i http://localhost:8080/imagenes/ejemplo.svg
curl -i http://localhost:8080/favicon.ico
curl -i http://localhost:8080/no-existe.html
curl --path-as-is -i http://localhost:8080/../servidor.cpp
curl --path-as-is -i 'http://localhost:8080/%2e%2e/servidor.cpp'
curl -i -X POST http://localhost:8080/
```

También se puede comprobar que una imagen se descarga sin cambios:

```bash
curl -s http://localhost:8080/imagenes/ejemplo.svg -o /tmp/ejemplo.svg
cmp public/imagenes/ejemplo.svg /tmp/ejemplo.svg
```

Para probar un enlace simbólico fuera de `public/`, se puede usar un archivo temporal inofensivo:

```bash
echo "contenido temporal" > /tmp/archivo-fuera-de-public.txt
ln -s /tmp/archivo-fuera-de-public.txt public/enlace-temporal
curl -i http://localhost:8080/enlace-temporal
rm public/enlace-temporal /tmp/archivo-fuera-de-public.txt
```

La respuesta debe ser `404`.

## Tecnologías usadas

- **C++17**
- **Sockets TCP** de Linux
- **std::filesystem** para resolver y validar rutas
- **Linux**

## Pendientes

- Autenticación real en `login.html`.
- Registro y consulta real de alumnos.
- Gestión de materias.
- Fondo navideño.
