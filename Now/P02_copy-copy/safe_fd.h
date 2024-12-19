#include <unistd.h>
#include <iostream>

class SafeFD
{
public:
  // Constructor predeterminado (descriptor inválido)
  SafeFD() : fd_(-1) {}

  // Constructor con descriptor
  explicit SafeFD(int fd) : fd_(fd) {}

  // Destructor: Cierra el descriptor si es válido
  ~SafeFD()
  {
    if (fd_ >= 0)
    {
      std::cerr << "Cerrando el descriptor de archivo: " << fd_ << std::endl;
      close(fd_);
    }
  }

  // Prohibir copias para evitar problemas con múltiples cierres
  SafeFD(const SafeFD &) = delete;
  SafeFD &operator=(const SafeFD &) = delete;

  // Permitir movimientos (transfiere la propiedad del descriptor)
  SafeFD(SafeFD &&other) noexcept : fd_(other.fd_)
  {
    other.fd_ = -1; // Invalida el descriptor en el origen
  }

  SafeFD &operator=(SafeFD &&other) noexcept
  {
    if (this != &other)
    {
      // Cierra el descriptor actual si es válido
      if (fd_ != -1)
      {
        close(fd_);
      }
      fd_ = other.fd_; // Transfiere el descriptor
      other.fd_ = -1;  // Invalida el descriptor en el origen
    }
    return *this;
  }

  // Devuelve el descriptor de archivo
  int get_fd() const { return fd_; }

  // Devuelve true si el descriptor es válido
  bool is_valid() const { return fd_ != -1; }

private:
  int fd_; // Descriptor de archivo
};