#include "programa.h"

std::string ruta_definitiva (std::string ruta_adsoluta, std::string next_part) {
  std::istringstream istring (next_part);
  std::string get;
  std::string ruta_relativa;

  istring >> get >> ruta_relativa; //Separa por espacios lo que mande el cliente
  if (get != "GET") {
    return "";
  }
  
  std::string ruta_adsoluta_add = ruta_adsoluta + ruta_relativa;
  if (!std::filesystem::is_regular_file(ruta_adsoluta_add)) {
    return "";
  }

  return ruta_adsoluta_add;
}

int main(int argc, char *argv[])
{
  // Procesar los argumentos de línea de comandos
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

   // if (opciones.get_archivo().empty())
   // {
   //   std::cerr << "Error: No se especificó un archivo." << std::endl;
   //   return 1;
   // }

  if (opciones.get_error_detected()) {
    std::cerr << "Error al inicializar: " << std::endl;
    return 1;
  }

  // Obtener el puerto desde los argumentos o la variable de entorno
  std::string port_env = getenv_port("DOCSERVER_PORT"); // Variable de entorno, DOCSERVER

  uint16_t port = port_env.empty() ? 8080 : static_cast<uint16_t>(std::stoi(port_env));
  if (opciones.get_port() > 0)
  {
    port = opciones.get_port();
    std::cout << port;
  }

  // Crear el socket
  auto socket_result = make_socket(port);
  if (!socket_result)
  {
    std::cerr << "Error al crear el socket: " << std::strerror(socket_result.error()) << std::endl;
    return 1;
  }

  SafeFD server_socket = std::move(socket_result.value());

  if (opciones.get_verbose())
  {
    std::cerr << "---> listen: Puesto en modo escucha a través del puerto: " << port << std::endl;
  }
  // Poner el socket a la escucha
  int listen_result = listen_connection(server_socket);
  if (listen_result != 0)
  {
    std::cerr << "Error en listen: " << std::strerror(listen_result) << std::endl;
    return 1;
  }

  // Bucle principal para aceptar conexiones
  while (true)
  {
    sockaddr_in client_addr;
    auto client_result = accept_connection(server_socket, client_addr);
    if (!client_result)
    {
      std::cerr << "Error al aceptar conexión: " << std::strerror(client_result.error()) << std::endl;
      continue; // Error leve, sigue esperando conexiones
    }

    if (opciones.get_verbose())
    {
      std::cerr << "---> accept: Conexión aceptada" << std::endl;
    }

    std::string ruta_adsoluta;
    if (opciones.get_flag_base()) {
      ruta_adsoluta = opciones.get_ruta();
    } else {
      std::string base = getenv_port("DOCSERVER_BASEDIR");
      if (base.empty()) {
        char* aux = getcwd (NULL, 0);
        ruta_adsoluta = aux;
        free(aux);
      } else {
        ruta_adsoluta = base;
      }
    } 

    auto recibo = receive_request(client_result.value(), 4096);

    std::string ruta_final = ruta_definitiva(ruta_adsoluta, recibo.value());
    if (ruta_final.empty()) {
      std::cerr << "404 BAD REQUEST" << std::endl;
      continue;
    }

    SafeFD client_socket = std::move(client_result.value());

    // Leer el archivo solicitado
    auto resultado = read_all(ruta_final, opciones);
    if (!resultado)
    {
      int error_code = resultado.error();
      if (error_code == EACCES)
      {
        send_response(client_socket, "403 Forbidden");
        std::cerr << "Error: Permiso denegado al leer archivo." << std::endl;
      }
      else if (error_code == ENOENT)
      {
        send_response(client_socket, "404 Not Found");
        std::cerr << "Error: Archivo no encontrado." << std::endl;
      }
      else
      {
        std::cerr << "Error fatal al leer archivo: " << std::strerror(error_code) << std::endl;
        return 1; // Error fatal
      }
      continue; // Sigue con la siguiente conexión
    }

    // Enviar respuesta con el contenido del archivo
    SafeMap map = resultado.value();
    int send_status = send_response(client_socket, "Content-Length: " + std::to_string(map.get().size()), map.get());
    if (send_status == ECONNRESET)
    {
      std::cerr << "Error: Conexión cerrada por el cliente antes de completar la respuesta." << std::endl;
    }
    else if (send_status != 0)
    {
      std::cerr << "Error fatal al enviar respuesta: " << std::strerror(send_status) << std::endl;
      return 1;
    }
  }

  return 0;
}
