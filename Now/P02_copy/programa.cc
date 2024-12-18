#include <iostream>
#include <optional>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <expected>

class Opcion
{
  public:
    Opcion(bool flag_help_, bool flag_verbose_, const std::string archivo = "") {
      flag_help = flag_help_;
      flag_verbose = flag_verbose_;
    }
    Opcion() {};
    
    //Setters
    void set_verbose( bool flag ) { flag_verbose = flag; }
    void set_archivo( std::string archivo ) { archivo_= archivo; }
  
    //Getters
    bool get_verbose() { return flag_verbose; }
    bool get_help() { return flag_help; }
    std::string get_archivo() { return archivo_; }

  private:
    bool flag_verbose = false;
    bool flag_help = false;
    std::string archivo_ ="";
};

void help() {
  std::cout << "==============================================\n"
            << "--------------> AYUDA --help -h <-------------\n"
            << "==============================================\n";
}

std::optional<Opcion> parse_args(int argc, char *argv[]) {
    Opcion opciones;
    for (int i = 1; i < argc; i++) {
        std::string argumento = argv[i];

        if (argumento == "-h") {
            help();
            return opciones;
        } 
        else if (argumento == "-v") {
            opciones.set_verbose(true);
        } 
        else if (!argumento.empty() && argumento.at(0) != '-') {
            opciones.set_archivo(argumento);
        } 
        else {
            std::cerr << "Argumento desconocido: " << argumento << std::endl;
            return std::nullopt;
        }
    }
    return opciones;
}

//--------------------------------------
//------------> READ ALL <--------------
//--------------------------------------

std::expected<std::string_view, int> read_all(const std::string& archivo) { 
  
  int fd = open( archivo.c_str(), O_RDONLY );
  std::cout << "Abriendo el archivo - open ( " << archivo << " )\n";
  
  if (fd == -1)
  {
    throw std::system_error( errno, std::system_category(), "Fallo en open()" );
  }
  int file_size = lseek( fd, 0, SEEK_END ); //Miramos el tamaño del archivo

  void* mapped_file = mmap( nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0 ); 
  //Se mapea el archivo, indicando su tamaño, sus permisos, y la memoria compartida
  if (mapped_file == MAP_FAILED) {
    close(fd);
    return std::unexpected(errno);
  }

  close(fd);
  return std::string_view(static_cast<char*>(mapped_file), file_size);
}

void send_response(std::string_view header, std::string_view body = {}) {
    std::cout << header << "\n"; // Imprime el header
    if (!body.empty()) {
        std::cout << "\n" << body << std::endl; // Línea en blanco y el body
    }
}

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