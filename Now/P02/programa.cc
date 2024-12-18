#include "programa.h"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <system_error>

// Implementación de SafeMap
SafeMap::SafeMap(void *address, size_t size)
    : sv_{static_cast<char *>(address), size}, address_{address}, size_{size} {}

SafeMap::~SafeMap()
{
    if (address_)
    {
        munmap(address_, size_);
    }
}

std::string_view SafeMap::get() const
{
    if (!address_) {
        std::cerr << "Error: Dirección de memoria no válida\n";
        return std::string_view();  // Retorna un string vacío si la dirección no es válida
    }
    return std::string_view(static_cast<char*>(address_), size_);
}

// Implementación de Opcion
Opcion::Opcion(bool flag_help_, bool flag_verbose_, const std::string archivo)
    : flag_help(flag_help_), flag_verbose(flag_verbose_), archivo_(archivo) {}

Opcion::Opcion() {}

void Opcion::set_verbose(bool flag) { flag_verbose = flag; }
void Opcion::set_archivo(std::string archivo) { archivo_ = archivo; }

bool Opcion::get_verbose() const { return flag_verbose; }
bool Opcion::get_help() const { return flag_help; }
std::string Opcion::get_archivo() const { return archivo_; }

// Funciones auxiliares
void help()
{
    std::cout << "==============================================\n"
              << "--------------> AYUDA --help -h <-------------\n"
              << "==============================================\n";
}

std::optional<Opcion> parse_args(int argc, char *argv[], Opcion opciones)
{
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
            std::cout << "---> Has seleccionado la opción -v\n";
            opciones.set_verbose(true);
        }
        else if (argumento.at(0) != '-')
        {
            std::cout << "---> Has insertado un archivo\n";
            opciones.set_archivo(argumento);

            if (argumento.empty() && !opciones.get_help())
            {
                std::cerr << "Error: No se indicó un archivo.\n";
                return std::nullopt;
            }
            return opciones;
        }
        else
        {
            std::cerr << "Error: Opción desconocida: " << argumento << "\n";
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::expected<SafeMap, int> read_all(const std::string &archivo)
{
    int fd = open(archivo.c_str(), O_RDONLY);
    if (fd == -1)
    {
        std::cerr << "Error al abrir el archivo: " << strerror(errno) << "\n";
        return std::unexpected(errno);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) 
    {
        close(fd);
        std::cerr << "Error al obtener el tamaño del archivo: " << strerror(errno) << "\n";
        return std::unexpected(errno);
    }

    if (sb.st_size <= 0) 
    {
        close(fd);
        std::cerr << "Error: El archivo está vacío o tiene un tamaño inválido.\n";
        return std::unexpected(EINVAL); // Tamaño inválido
    }

    void *mapped_file = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED)
    {
        close(fd);
        std::cerr << "Error de mapeo: " << strerror(errno) << "\n";
        return std::unexpected(errno);
    }

    close(fd);
    return SafeMap(mapped_file, sb.st_size);
}

void send_response(std::string_view header, std::string_view body)
{
    std::cout << header << "\n";
    if (!body.empty())
    {
        std::cout << "\n" << body << std::endl;
    }
}