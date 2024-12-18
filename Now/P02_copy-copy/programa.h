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

std::expected<std::string_view, int> read_all(const std::string& archivo);

//--------------------------------------
//----------> SEND RESPONSE <-----------
//--------------------------------------

void send_response(std::string_view header, std::string_view body = {}) {
    std::cout << header << "\n"; // Imprime el header
    if (!body.empty()) {
        std::cout << "\n" << body << std::endl; // Línea en blanco y el body
    }
}