#include "programa.h"

//--------------------------------------
//-------------> INT MAIN <-------------
//--------------------------------------

int main(int argc, char *argv[])
{
  auto opciones_opt = parse_args(argc, argv);
  if (!opciones_opt)
  {
    std::cerr << "Error: Argumentos inválidos." << std::endl;
    help();
    return 1;
  }

  Opcion opciones = *opciones_opt;

  if (opciones.get_help())
  {
    help();
    return 0;
  }

  if (opciones.get_archivo().empty())
  {
    std::cerr << "Error: No se especificó un archivo." << std::endl;
    return 1;
  }

  auto resultado = read_all(opciones.get_archivo(), opciones);
  if (!resultado)
  {
    std::cerr << "Error al leer el archivo: " << std::strerror(resultado.error()) << std::endl;
    return 1;
  }

  SafeMap map = resultado.value();
  std::string contenido(map.get()); // Copiamos el contenido.
  
  send_response("Content-Length: " + std::to_string(contenido.size()), contenido);
}