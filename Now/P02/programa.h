#ifndef PROGRAMA_H
#define PROGRAMA_H

#include <string_view>
#include <string>
#include <optional>
#include <expected>

class SafeMap {
public:
    SafeMap() = default;
    SafeMap(void *address, size_t size);
    ~SafeMap();

    std::string_view get() const;

private:
    std::string_view sv_;
    void *address_ = nullptr;
    size_t size_ = 0;
};

class Opcion {
public:
    Opcion(bool flag_help_, bool flag_verbose_, const std::string archivo = "");
    Opcion();

    // Setters
    void set_verbose(bool flag);
    void set_archivo(std::string archivo);

    // Getters
    bool get_verbose() const;
    bool get_help() const;
    std::string get_archivo() const;

private:
    bool flag_verbose = false;
    bool flag_help = false;
    std::string archivo_ = "";
};

void help();
std::optional<Opcion> parse_args(int argc, char *argv[], Opcion opciones);
std::expected<SafeMap, int> read_all(const std::string &archivo);
void send_response(std::string_view header, std::string_view body = {});

#endif