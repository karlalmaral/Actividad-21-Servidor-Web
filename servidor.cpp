#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Función para leer un archivo completo y devolver su contenido como string
std::string leerArchivo(const std::string &ruta) {
    std::ifstream archivo(ruta, std::ios::binary);
    if (!archivo.is_open()) {
        return "";
    }
    
    std::ostringstream flujo;
    flujo << archivo.rdbuf();
    return flujo.str();
}

// Función para obtener la extensión de un archivo
std::string obtenerExtension(const std::string &ruta) {
    size_t punto = ruta.find_last_of('.');
    if (punto == std::string::npos) {
        return "";
    }
    return ruta.substr(punto + 1);
}

// Función para obtener el tipo MIME según la extensión
std::string obtenerTipoMIME(const std::string &extension) {
    if (extension == "html" || extension == "htm") return "text/html; charset=UTF-8";
    if (extension == "css") return "text/css; charset=UTF-8";
    if (extension == "js") return "application/javascript; charset=UTF-8";
    if (extension == "png") return "image/png";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "gif") return "image/gif";
    if (extension == "ico") return "image/x-icon";
    if (extension == "txt") return "text/plain; charset=UTF-8";
    return "text/plain; charset=UTF-8";
}

// Función para sanitizar la ruta y evitar acceder fuera de public/
bool rutaSegura(const std::string &rutaSolicitada) {
    // Evitar rutas que intenten salir con .. 
    if (rutaSolicitada.find("..") != std::string::npos) {
        return false;
    }
    // Solo permitir rutas dentro del directorio public/
    if (rutaSolicitada.empty()) {
        return false;
    }
    return true;
}

int main() {
    int servidorSocket, clienteSocket;
    struct sockaddr_in direccionServidor, direccionCliente;
    socklen_t tamanoDireccionCliente = sizeof(direccionCliente);
    
    // Crear socket TCP
    servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidorSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }
    
    // Permitir reutilizar la dirección
    int opcion = 1;
    if (setsockopt(servidorSocket, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion)) < 0) {
        std::cerr << "Error al configurar SO_REUSEADDR" << std::endl;
        return 1;
    }
    
    // Configurar dirección del servidor
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las interfaces
    direccionServidor.sin_port = htons(8080); // Puerto 8080
    
    // Enlazar socket con la dirección
    if (bind(servidorSocket, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) < 0) {
        std::cerr << "Error al enlazar el socket al puerto 8080" << std::endl;
        return 1;
    }
    
    // Escuchar conexiones
    if (listen(servidorSocket, 10) < 0) {
        std::cerr << "Error al escuchar conexiones" << std::endl;
        return 1;
    }
    
    std::cout << "=== SERVIDOR WEB INICIADO ===" << std::endl;
    std::cout << "Escuchando en http://localhost:8080" << std::endl;
    std::cout << "Sirviendo archivos desde: public/" << std::endl;
    std::cout << "Presiona Ctrl+C para detener" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Bucle para atender varias conexiones consecutivas
    while (true) {
        // Aceptar nueva conexión
        clienteSocket = accept(servidorSocket, (struct sockaddr*)&direccionCliente, &tamanoDireccionCliente);
        if (clienteSocket < 0) {
            std::cerr << "Error al aceptar conexión" << std::endl;
            continue;
        }
        
        std::cout << "\n[CONEXIÓN] Cliente conectado desde: " << inet_ntoa(direccionCliente.sin_addr) << std::endl;
        
        // Buffer para recibir la petición HTTP
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytesRecibidos = recv(clienteSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRecibidos > 0) {
            std::string peticion(buffer);
            std::cout << "[PETICIÓN HTTP]:" << std::endl;
            // Mostrar solo la primera línea de la petición
            size_t finLinea = peticion.find("\r\n");
            if (finLinea != std::string::npos) {
                std::cout << peticion.substr(0, finLinea) << std::endl;
            } else {
                std::cout << peticion << std::endl;
            }
            
            // Parsear la petición HTTP GET
            if (peticion.substr(0, 3) == "GET") {
                // Extraer la ruta solicitada
                size_t inicioRuta = peticion.find(' ') + 1;
                size_t finRuta = peticion.find(' ', inicioRuta);
                std::string rutaSolicitada = peticion.substr(inicioRuta, finRuta - inicioRuta);
                
                // Si solicita la raíz, servir index.html
                if (rutaSolicitada == "/" || rutaSolicitada.empty()) {
                    rutaSolicitada = "/index.html";
                }
                
                // Quitar el '/' inicial
                if (!rutaSolicitada.empty() && rutaSolicitada[0] == '/') {
                    rutaSolicitada = rutaSolicitada.substr(1);
                }
                
                // Verificar que la ruta sea segura
                if (!rutaSegura(rutaSolicitada)) {
                    std::string respuesta404 = "HTTP/1.1 404 Not Found\r\n";
                    respuesta404 += "Content-Type: text/html; charset=UTF-8\r\n";
                    respuesta404 += "Connection: close\r\n";
                    respuesta404 += "\r\n";
                    respuesta404 += "<html><head><title>404 Not Found</title></head><body><h1>404 - Archivo no encontrado</h1><p>Ruta no permitida</p></body></html>";
                    send(clienteSocket, respuesta404.c_str(), respuesta404.length(), 0);
                    std::cout << "[RESPUESTA] 404 Not Found - Ruta no permitida" << std::endl;
                } else {
                    // Construir ruta completa dentro de public/
                    std::string rutaCompleta = "public/" + rutaSolicitada;
                    std::string contenido = leerArchivo(rutaCompleta);
                    
                    if (contenido.empty()) {
                        // Archivo no encontrado - Respuesta 404
                        std::string respuesta404 = "HTTP/1.1 404 Not Found\r\n";
                        respuesta404 += "Content-Type: text/html; charset=UTF-8\r\n";
                        respuesta404 += "Connection: close\r\n";
                        respuesta404 += "\r\n";
                        respuesta404 += "<html><head><title>404 Not Found</title></head><body><h1>404 - Archivo no encontrado</h1><p>El archivo " + rutaSolicitada + " no existe.</p></body></html>";
                        send(clienteSocket, respuesta404.c_str(), respuesta404.length(), 0);
                        std::cout << "[RESPUESTA] 404 Not Found - " << rutaCompleta << std::endl;
                    } else {
                        // Archivo encontrado - Respuesta 200 OK
                        std::string extension = obtenerExtension(rutaSolicitada);
                        std::string tipoMIME = obtenerTipoMIME(extension);
                        
                        std::string respuesta200 = "HTTP/1.1 200 OK\r\n";
                        respuesta200 += "Content-Type: " + tipoMIME + "\r\n";
                        respuesta200 += "Content-Length: " + std::to_string(contenido.length()) + "\r\n";
                        respuesta200 += "Connection: close\r\n";
                        respuesta200 += "\r\n";
                        respuesta200 += contenido;
                        
                        send(clienteSocket, respuesta200.c_str(), respuesta200.length(), 0);
                        std::cout << "[RESPUESTA] 200 OK - " << rutaCompleta << " (" << contenido.length() << " bytes)" << std::endl;
                    }
                }
            } else {
                // No es GET
                std::string respuesta = "HTTP/1.1 405 Method Not Allowed\r\n";
                respuesta += "Content-Type: text/html; charset=UTF-8\r\n";
                respuesta += "Connection: close\r\n";
                respuesta += "\r\n";
                respuesta += "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 - Método no permitido</h1></body></html>";
                send(clienteSocket, respuesta.c_str(), respuesta.length(), 0);
                std::cout << "[RESPUESTA] 405 Method Not Allowed" << std::endl;
            }
        }
        
        // Cerrar conexión con el cliente
        close(clienteSocket);
        std::cout << "[CONEXIÓN] Cerrada" << std::endl;
    }
    
    // Cerrar socket del servidor
    close(servidorSocket);
    return 0;
}
