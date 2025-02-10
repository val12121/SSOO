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
    else if (argumento == "-p")
    {
      opciones.set_flag_port(true);
      if (i + 1 < argc)
      {
        std::string argumento_plus = argv[i + 1];
        if (!argumento_plus.starts_with("-"))
        {
          i++;
          try
          {
            opciones.set_port(std::stoi(argumento_plus)); // Intenta convertir a int
            // Si no lanza una excepción, es un número válido
          }
          catch (const std::invalid_argument &e)
          {
            opciones.set_error_detected(true); // La cadena no es un número entero válido
          }
        }
      }
    }
    else if (argumento == "-b")
    {
      opciones.set_flag_base(true);
      if (i + 1 < argc)
      {
        std::string argumento_plus = argv[i + 1];
        if (!argumento_plus.starts_with("-"))
        {
          i++;
          if (std::filesystem::is_directory(argumento_plus)) {
            opciones.set_ruta(argumento_plus);
          } 
          else {
            opciones.set_error_detected(true);
          }
        }
      }
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
  if (opcion.get_verbose())
  {
    std::cerr << "---> open: intentando abrir el archivo \"" << archivo << "\"" << std::endl;
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
  // Tamaño del archivo a partir del struct stact
  size_t file_size = st.st_size;

  if (opcion.get_verbose())
  {
    std::cerr << "---> read: leyendo " << st.st_size << " bytes del archivo \"" << archivo << "\"" << std::endl;
  }

  // Mapear el archivo en memoria
  void *mapped_file = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
  close(fd); // Se puede cerrar el archivo después de mapear

  if (mapped_file == MAP_FAILED)
  {
    return std::unexpected(errno);
  }

  if (opcion.get_verbose())
  {
    std::cerr << "---> mmap: archivo \"" << archivo << "\" mapeado en memoria" << std::endl;
  }

  const char *var = static_cast<const char *>(mapped_file);
  int aux = 0;
  std::string aux_s = "";

  while (aux < file_size)
  {
    aux_s += *var;
    ++var;
    ++aux;
  }

  //SafeMap SS(mapped_file, file_size, aux_s);
  // Devolver el SafeMap
  return SafeMap(mapped_file, file_size, aux_s);
}
/*
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
*/
//--------------------------------------
//-------------> GETENV <---------------
//--------------------------------------

std::string getenv_port(const std::string &name)
{
  std::cout << name << std::endl;
  const char *value = std::getenv(name.c_str());
  // Nos aseguramos de que la variable este definida
  if (value != nullptr)
  {
    return std::string(value);
  }
  else
  {
    return "";
  }
}

//--------------------------------------
//-----------> MAKE SOCKET <------------
//--------------------------------------

std::expected<SafeFD, int> make_socket(uint16_t port)
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
  {
    return std::unexpected(errno);
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  //Establece la conexion entre los puertos
  if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1)
  {
    close(fd);
    return std::unexpected(errno);
  }

  SafeFD descriptor(fd);
  return descriptor; // Devuelve el descriptor de socket contenido en SafeFD
}

//--------------------------------------
//--------> LISTEN CONNECTION <---------
//--------------------------------------

int listen_connection(const SafeFD &socket)
{
  if (listen(socket.get_fd(), SOMAXCONN) == -1)
  {
    return errno;
  }
  return 0;
}

//--------------------------------------
//--------> ACCEPT CONNECTION <---------
//--------------------------------------

std::expected<SafeFD, int> accept_connection(const SafeFD &socket, sockaddr_in &client_addr)
{
  socklen_t client_len = sizeof(client_addr);
  int client_fd = accept(socket.get_fd(), reinterpret_cast<sockaddr *>(&client_addr), &client_len);
  if (client_fd == -1)
  {
    return std::unexpected(errno);
  }
  return SafeFD(client_fd); // Devuelve el descriptor del cliente envuelto en SafeFD
}

int send_response(const SafeFD &socket, std::string_view header, std::string_view body)
{
  if (body.data() == nullptr)
  {
    body = ""; // Asegura que el cuerpo sea válido incluso si es nulo
  }
  std::string respuesta = std::string(header) + "\r\n" + std::string(body);
  ssize_t sent = send(socket.get_fd(), respuesta.data(), respuesta.size(), 0);
  return (sent == -1) ? errno : 0;
}

//--------------------------------------
//---------> RECEIVE REQUEST <----------
//--------------------------------------

std::expected<std::string, int> receive_request(const SafeFD &socket, size_t max_size)
{
  std::string buffer(max_size, '\0');
  ssize_t bytes_received = recv(socket.get_fd(), buffer.data(), max_size, 0);

  if (bytes_received < 0)
  {
    return std::unexpected(errno);
  }
  else if (bytes_received == 0)
  {
    return std::unexpected(ECONNRESET); //Conexión terminada
  }

  buffer.resize(bytes_received);
  return buffer;
}