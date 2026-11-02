#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace fs = std::filesystem;

bool leerArchivo(const std::string &ruta, std::string &contenido) {
    std::ifstream archivo(ruta, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    archivo.seekg(0, std::ios::end);
    std::streamoff tamano = archivo.tellg();
    if (tamano < 0) {
        return false;
    }

    archivo.seekg(0, std::ios::beg);
    if (!archivo) {
        return false;
    }

    contenido.resize(static_cast<size_t>(tamano));
    if (tamano > 0) {
        archivo.read(contenido.data(), static_cast<std::streamsize>(tamano));
        if (!archivo) {
            return false;
        }
    }

    return true;
}

std::string obtenerExtension(const std::string &ruta) {
    size_t punto = ruta.find_last_of('.');
    size_t barra = ruta.find_last_of('/');
    if (punto == std::string::npos || (barra != std::string::npos && punto < barra)) {
        return "";
    }

    std::string extension = ruta.substr(punto + 1);
    for (char &caracter : extension) {
        caracter = static_cast<char>(std::tolower(static_cast<unsigned char>(caracter)));
    }
    return extension;
}

std::string obtenerTipoMIME(const std::string &extension) {
    if (extension == "html" || extension == "htm") return "text/html; charset=UTF-8";
    if (extension == "css") return "text/css; charset=UTF-8";
    if (extension == "js" || extension == "mjs") return "application/javascript; charset=UTF-8";
    if (extension == "json") return "application/json; charset=UTF-8";
    if (extension == "txt") return "text/plain; charset=UTF-8";
    if (extension == "xml") return "application/xml; charset=UTF-8";
    if (extension == "svg") return "image/svg+xml";
    if (extension == "png") return "image/png";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "gif") return "image/gif";
    if (extension == "webp") return "image/webp";
    if (extension == "avif") return "image/avif";
    if (extension == "ico") return "image/x-icon";
    if (extension == "woff") return "font/woff";
    if (extension == "woff2") return "font/woff2";
    if (extension == "ttf") return "font/ttf";
    if (extension == "otf") return "font/otf";
    if (extension == "pdf") return "application/pdf";
    if (extension == "mp4") return "video/mp4";
    if (extension == "webm") return "video/webm";
    if (extension == "wasm") return "application/wasm";
    return "application/octet-stream";
}

int valorHexadecimal(char caracter) {
    if (caracter >= '0' && caracter <= '9') return caracter - '0';
    if (caracter >= 'a' && caracter <= 'f') return caracter - 'a' + 10;
    if (caracter >= 'A' && caracter <= 'F') return caracter - 'A' + 10;
    return -1;
}

bool decodificarURL(const std::string &entrada, std::string &salida) {
    salida.clear();

    for (size_t i = 0; i < entrada.size(); ++i) {
        unsigned char caracter = static_cast<unsigned char>(entrada[i]);

        if (caracter == '%') {
            if (i + 2 >= entrada.size()) {
                return false;
            }

            int alto = valorHexadecimal(entrada[i + 1]);
            int bajo = valorHexadecimal(entrada[i + 2]);
            if (alto < 0 || bajo < 0) {
                return false;
            }

            unsigned char decodificado = static_cast<unsigned char>((alto << 4) | bajo);
            if (decodificado == 0 || decodificado < 0x20 || decodificado == 0x7f) {
                return false;
            }

            salida.push_back(static_cast<char>(decodificado));
            i += 2;
        } else {
            if (caracter == 0 || caracter < 0x20 || caracter == 0x7f) {
                return false;
            }
            salida.push_back(static_cast<char>(caracter));
        }
    }

    return true;
}

bool resolverRutaPublica(const fs::path &raizPublica, const std::string &rutaRelativa, fs::path &rutaResuelta) {
    std::error_code error;
    fs::path candidata = raizPublica / fs::path(rutaRelativa);
    fs::path canonica = fs::canonical(candidata, error);
    if (error) {
        return false;
    }

    auto componenteRaiz = raizPublica.begin();
    auto componenteCanonica = canonica.begin();
    for (; componenteRaiz != raizPublica.end(); ++componenteRaiz, ++componenteCanonica) {
        if (componenteCanonica == canonica.end() || *componenteRaiz != *componenteCanonica) {
            return false;
        }
    }

    if (!fs::is_regular_file(canonica, error) || error) {
        return false;
    }

    rutaResuelta = canonica;
    return true;
}

void enviarTodo(int cliente, const char *datos, size_t longitud) {
    size_t enviado = 0;

    while (enviado < longitud) {
        ssize_t bytes = send(cliente, datos + enviado, longitud - enviado, MSG_NOSIGNAL);
        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (bytes == 0) {
            break;
        }
        enviado += static_cast<size_t>(bytes);
    }
}

void enviarRespuesta(int cliente, const std::string &estado, const std::string &tipoMIME, const std::string &contenido) {
    std::string cabeceras = "HTTP/1.1 " + estado + "\r\n";
    cabeceras += "Content-Type: " + tipoMIME + "\r\n";
    cabeceras += "Content-Length: " + std::to_string(contenido.size()) + "\r\n";
    cabeceras += "Connection: close\r\n";
    cabeceras += "\r\n";

    enviarTodo(cliente, cabeceras.data(), cabeceras.size());
    if (!contenido.empty()) {
        enviarTodo(cliente, contenido.data(), contenido.size());
    }
}

void enviarError(int cliente, const std::string &estado, const std::string &titulo, const std::string &mensaje) {
    std::string contenido = "<!DOCTYPE html><html lang=\"es\"><head><meta charset=\"UTF-8\">";
    contenido += "<title>" + titulo + "</title></head><body>";
    contenido += "<h1>" + titulo + "</h1><p>" + mensaje + "</p>";
    contenido += "<p><a href=\"/\">Volver al inicio</a></p></body></html>";

    enviarRespuesta(cliente, estado, "text/html; charset=UTF-8", contenido);
    std::cout << "[RESPUESTA] " << estado << std::endl;
}

std::string primeraLinea(const std::string &peticion) {
    size_t fin = peticion.find('\n');
    std::string linea = (fin == std::string::npos) ? peticion : peticion.substr(0, fin);

    if (!linea.empty() && linea.back() == '\r') {
        linea.pop_back();
    }
    return linea;
}

int main() {
    std::signal(SIGPIPE, SIG_IGN);

    std::error_code errorRaiz;
    fs::path raizPublica = fs::canonical("public", errorRaiz);
    if (errorRaiz) {
        std::cerr << "Error: no se pudo resolver el directorio public/: " << errorRaiz.message() << std::endl;
        return 1;
    }
    if (!fs::is_directory(raizPublica, errorRaiz) || errorRaiz) {
        std::cerr << "Error: public/ no es un directorio válido" << std::endl;
        return 1;
    }

    int servidorSocket, clienteSocket;
    struct sockaddr_in direccionServidor, direccionCliente;
    socklen_t tamanoDireccionCliente = sizeof(direccionCliente);

    servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidorSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    int opcion = 1;
    if (setsockopt(servidorSocket, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion)) < 0) {
        std::cerr << "Error al configurar SO_REUSEADDR" << std::endl;
        close(servidorSocket);
        return 1;
    }

    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = INADDR_ANY;
    direccionServidor.sin_port = htons(8080);

    if (bind(servidorSocket, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) < 0) {
        std::cerr << "Error al enlazar el socket al puerto 8080" << std::endl;
        close(servidorSocket);
        return 1;
    }

    if (listen(servidorSocket, 10) < 0) {
        std::cerr << "Error al escuchar conexiones" << std::endl;
        close(servidorSocket);
        return 1;
    }

    std::cout << "=== SERVIDOR WEB INICIADO ===" << std::endl;
    std::cout << "Escuchando en http://localhost:8080" << std::endl;
    std::cout << "Sirviendo archivos desde: " << raizPublica.string() << std::endl;
    std::cout << "Presiona Ctrl+C para detener" << std::endl;
    std::cout << "=============================" << std::endl;

    while (true) {
        tamanoDireccionCliente = sizeof(direccionCliente);
        clienteSocket = accept(servidorSocket, (struct sockaddr*)&direccionCliente, &tamanoDireccionCliente);
        if (clienteSocket < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "Error al aceptar conexión" << std::endl;
            continue;
        }

        std::cout << "\n[CONEXIÓN] Cliente conectado desde: " << inet_ntoa(direccionCliente.sin_addr) << std::endl;

        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRecibidos = recv(clienteSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRecibidos > 0) {
            std::string peticion(buffer, static_cast<size_t>(bytesRecibidos));
            std::string linea = primeraLinea(peticion);
            std::cout << "[PETICIÓN HTTP] " << linea << std::endl;

            std::istringstream flujo(linea);
            std::string metodo, objetivo, version;
            flujo >> metodo >> objetivo >> version;

            if (metodo.empty() || objetivo.empty()) {
                enviarError(clienteSocket, "400 Bad Request", "400 - Petición inválida", "No se pudo interpretar la petición HTTP.");
            } else if (metodo != "GET") {
                enviarError(clienteSocket, "405 Method Not Allowed", "405 - Método no permitido", "Solo se permite el método GET.");
            } else {
                size_t posicionQuery = objetivo.find('?');
                if (posicionQuery != std::string::npos) {
                    objetivo.erase(posicionQuery);
                }

                size_t posicionFragmento = objetivo.find('#');
                if (posicionFragmento != std::string::npos) {
                    objetivo.erase(posicionFragmento);
                }

                std::string rutaDecodificada;
                if (!decodificarURL(objetivo, rutaDecodificada)) {
                    enviarError(clienteSocket, "400 Bad Request", "400 - Ruta inválida", "La ruta solicitada no tiene una codificación válida.");
                } else {
                    if (rutaDecodificada.empty()) {
                        rutaDecodificada = "/";
                    }

                    if (rutaDecodificada.front() != '/') {
                        enviarError(clienteSocket, "400 Bad Request", "400 - Ruta inválida", "La ruta solicitada debe comenzar con /.");
                    } else {
                        if (rutaDecodificada == "/") {
                            rutaDecodificada = "/index.html";
                        }

                        std::string rutaRelativa = rutaDecodificada.substr(1);
                        fs::path archivoResuelto;

                        if (!resolverRutaPublica(raizPublica, rutaRelativa, archivoResuelto)) {
                            enviarError(clienteSocket, "404 Not Found", "404 - Archivo no encontrado", "El archivo solicitado no existe o la ruta no está permitida.");
                        } else {
                            std::string contenido;
                            if (!leerArchivo(archivoResuelto.string(), contenido)) {
                                enviarError(clienteSocket, "404 Not Found", "404 - Archivo no encontrado", "No se pudo leer el archivo solicitado.");
                            } else {
                                std::string extension = obtenerExtension(rutaDecodificada);
                                std::string tipoMIME = obtenerTipoMIME(extension);
                                enviarRespuesta(clienteSocket, "200 OK", tipoMIME, contenido);
                                std::cout << "[RESPUESTA] 200 OK - " << archivoResuelto.string() << " (" << contenido.size() << " bytes)" << std::endl;
                            }
                        }
                    }
                }
            }
        }

        close(clienteSocket);
        std::cout << "[CONEXIÓN] Cerrada" << std::endl;
    }

    close(servidorSocket);
    return 0;
}
