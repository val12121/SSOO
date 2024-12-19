#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <iostream>
#include <optional>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <expected>

//--------------------------------------
//-----------> CLASS OPCION <-----------
//--------------------------------------

class Opcion
{
public:
  Opcion(bool flag_help_, bool flag_verbose_, const std::string archivo = "")
  {
    flag_help = flag_help_;
    flag_verbose = flag_verbose_;
  }
  Opcion() {};

  // Setters
  void set_verbose(bool flag) { flag_verbose = flag; }
  void set_archivo(std::string archivo) { archivo_ = archivo; }

  // Getters
  bool get_verbose() { return flag_verbose; }
  bool get_help() { return flag_help; }
  std::string get_archivo() const { return archivo_; }

private:
  bool flag_verbose = false;
  bool flag_help = false;
  std::string archivo_ = "";
};

//--------------------------------------
//----------> CLASS SAFEMAP <-----------
//--------------------------------------

class SafeMap
{
public:
  SafeMap() : address_(nullptr), size_(0) {}

  // Constructor para mapear un archivo

  SafeMap(void *address, size_t size, std::string str)
      : address_(address), size_(size), str_(str), sv_(str_) {}

  // Destructor que libera automáticamente el mapeo
  ~SafeMap()
  {
    if (address_ != nullptr)
    {
      if (munmap(address_, size_) == -1)
      {
        std::cerr << "Error al desmapear la memoria: "
                  << std::strerror(errno) << std::endl;
      }
    }
  }

  // Método para obtener el std::string_view asociado
  std::string_view get() const
  {
    if (!is_valid())
    {
      // std::cout << "Hola lenteja" << std::endl;
      throw std::runtime_error("Intento de acceso a un mapeo no válido.");
    }

    //std::cout << sv_.size() << std::endl;
    return sv_;
  }

  // Método para verificar si el mapeo es válido
  bool is_valid() const
  {
    return address_ != nullptr && size_ > 0;
  }

private:
  void *address_;       // Dirección de la región mapeada
  size_t size_;         // Tamaño de la región mapeada
  std::string str_;
  std::string_view sv_; // Vista de la región mapeada
};

//--------------------------------------
//------------> PARSE ARGS <------------
//--------------------------------------

std::optional<Opcion> parse_args(int argc, char *argv[]);

//--------------------------------------
//--------------> HELP <----------------
//--------------------------------------

void help();

//--------------------------------------
//------------> READ ALL <--------------
//--------------------------------------

std::expected<SafeMap, int> read_all(const std::string &archivo, Opcion opcion_);

//--------------------------------------
//----------> SEND RESPONSE <-----------
//--------------------------------------

void send_response(std::string_view header, std::string_view body = {});

#endif