#include "programa.h"

//--------------------------------------
//-------------> INT MAIN <-------------
//--------------------------------------

int main(int argc, char *argv[]) {
    auto opciones = parse_args(argc, argv);
    if (!opciones.has_value()) {
        std::cerr << "No se pudieron parsear los argumentos correctamente." << std::endl;
        return 1;
    }

    const auto& opcion_resultado = opciones.value();
    auto resultado = read_all(opcion_resultado.get_archivo());
    if (!resultado) {
        send_response("404 Not Found");
        return 1;
    }

    SafeMap mapa = std::move(resultado.value());
    send_response("Content-Length: " + std::to_string(mapa.get().size()), mapa.get());

    // Al finalizar, el destructor de SafeMap desmapeará la región automáticamente
    return 0;
}