#include "programa.h"

//--------------------------------------
//-------------> INT MAIN <-------------
//--------------------------------------

int main(int argc, char *argv[]) {
    Opcion opciones{};
    auto resultado = parse_args(argc, argv);

    if (!resultado.has_value()) {
        std::cerr << "No se pudieron parsear los argumentos correctamente." << std::endl;
        return 1;
    }

    Opcion opcion_resultado = resultado.value();
    std::cout << "Archivo especificado: " << opcion_resultado.get_archivo() << std::endl;

    // Leer contenido del archivo
    auto contenido = read_all(opcion_resultado.get_archivo());
    if (contenido) {
        send_response("Content-Length: " + std::to_string(contenido->size()), contenido.value());
    } else {
        send_response("404 Not Found");
        return 1; // Salida con error
    }

    return 0;
}