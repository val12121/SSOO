#include "programa.h"

//--------------------------------------
//--------------> HELP <----------------
//--------------------------------------

void help() {
  std::cout << "==============================================\n"
            << "--------------> AYUDA --help -h <-------------\n"
            << "==============================================\n";
}

//--------------------------------------
//------------> PARSE ARGS <------------
//--------------------------------------


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

std::expected<SafeMap, int> read_all(const std::string& archivo) {
    int fd = open(archivo.c_str(), O_RDONLY);
    if (fd == -1) {
        return std::unexpected(errno);
    }

    // Determinar el tamaño del archivo
    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return std::unexpected(errno);
    }
    size_t file_size = st.st_size;

    // Mapear el archivo en memoria
    void* mapped_file = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);  // Se puede cerrar el archivo después de mapear

    if (mapped_file == MAP_FAILED) {
        return std::unexpected(errno);
    }

    // Devolver el SafeMap
    return SafeMap(mapped_file, file_size);
}
