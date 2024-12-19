#include "programa.h"

//--------------------------------------
//--------------> HELP <----------------
//--------------------------------------

void help()
{
  std::cout << "==============================================\n"
            << "--------------> AYUDA --help -h <-------------\n"
            << "==============================================\n";

  std::cout << "-----> Uso: programa [opciones] archivo" << std::endl;
  std::cout << "-----> -v  --verbose    Muestra información adicional" << std::endl;
  std::cout << "-----> -h  --help       Muestra esta ayuda" << std::endl;
}

//--------------------------------------
//------------> PARSE ARGS <------------
//--------------------------------------

std::optional<Opcion> parse_args(int argc, char *argv[])
{
  Opcion opciones;
  for (int i = 1; i < argc; i++)
  {
    std::string argumento = argv[i];

    if (argumento == "-h")
    {
      help();
      return opciones;
    }
    else if (argumento == "-v")
    {
      opciones.set_verbose(true);
    }
    else if (!argumento.empty() && argumento.at(0) != '-')
    {
      opciones.set_archivo(argumento);
    }
    else
    {
      std::cerr << "Argumento desconocido: " << argumento << std::endl;
      return std::nullopt;
    }
  }
  return opciones;
}

//--------------------------------------
//------------> READ ALL <--------------
//--------------------------------------

std::expected<SafeMap, int> read_all(const std::string &archivo, Opcion opcion)
{
  if (opcion.get_verbose()) {
    std::cerr << "open: intentando abrir el archivo \"" << archivo << "\"" << std::endl;
  }
  
  int fd = open(archivo.c_str(), O_RDONLY);
  if (fd == -1)
  {
    return std::unexpected(errno);
  }

  // Determinar el tamaño del archivo
  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    close(fd);
    return std::unexpected(errno);
  }
  size_t file_size = st.st_size;

  // Mapear el archivo en memoria
  void *mapped_file = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
  close(fd); // Se puede cerrar el archivo después de mapear

  if (mapped_file == MAP_FAILED)
  {
    return std::unexpected(errno);
  }
  // auto var = static_cast<const char*>(mapped_file);
  // std::cout << mapped_file << std::endl;
  const char *var = static_cast<const char *>(mapped_file);
  // std::cout << "dir1: " << address << " dir2: " << &var << std::endl;
  int aux = 0;
  std::string aux_s = "";

  //std::cout << *var << std::endl;
  while (aux < file_size)
  {

    aux_s += *var;
    // std::cout << *var << std::endl;
    ++var;
    ++aux;
  }
  // std::string_view sv(static_cast<const char *>(mapped_file), file_size);
  // std::cout << aux_s << std::endl;
  // std::cout << sv << std::endl;

  SafeMap SS(mapped_file, file_size, aux_s);
  //std::cout << SS.get() << std::endl;
  // Devolver el SafeMap
  return SS;
}
//--------------------------------------
//----------> SEND RESPONSE <-----------
//--------------------------------------

void send_response(std::string_view header, std::string_view body)
{
  std::cout << header << "\n";
  if (!body.empty())
  {
    std::cout << body;
  }
}