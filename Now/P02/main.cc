#include "programa.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

int main(int argc, char *argv[])
{
    Opcion opciones{};
    auto resultado = parse_args(argc, argv, opciones);

    if (!resultado.has_value())
    {
        std::cerr << "No se pudieron parsear los argumentos correctamente." << std::endl;
        return 1;
    }

    Opcion opcion_resultado = resultado.value();
    std::string archivo = opcion_resultado.get_archivo();

    if (archivo.empty())
    {
        std::cerr << "Error: No se proporcionó un archivo válido.\n";
        return 1;
    }

    // Verificar si el archivo existe
    if (access(archivo.c_str(), F_OK) == -1)
    {
        std::cerr << "El archivo no existe: " << archivo << "\n";
        return 1;
    }

    // Intentar leer y mapear el archivo
    auto contenido = read_all(archivo);

    if (contenido)
    {
        auto safe_map = contenido.value();

        // Validar que el archivo no esté vacío
        if (safe_map.get().empty())
        {
            std::cerr << "El archivo está vacío.\n";
            send_response("Content-Length: 0");
            return 1;
        }

        // Enviar la respuesta con el contenido
        send_response(
            "Content-Length: " + std::to_string(safe_map.get().size()),
            safe_map.get()
        );
    }
    else
    {
        std::cerr << "Error al leer el archivo: " << strerror(contenido.error()) << "\n";
        send_response("404 Not Found");
        return 1;
    }

    return 0;
}