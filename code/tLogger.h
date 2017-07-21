#ifndef TLOGGER_H
#define TLOGGER_H

#include <fstream>
#include <string>

template<class T>
struct t_logger {
private:
    T& os;
    std::ofstream log;
public:
    t_logger() = delete;

    t_logger(std::string o_log_file_name, T& t) : log{ o_log_file_name }, os{t} {}

    t_logger(t_logger&& o_osl) : os{ o_osl.os }, log{ o_osl.log } {}

    ~t_logger() {
        log.close();
    }

    template <class F>
    t_logger& operator<<(F f) {
        os << f; log << f;
        return *this;
    }
};

template <class T>
t_logger<T> make_tLogger(T &obj, std::string o_log_file_name) {
    return t_logger<T>{o_log_file_name, obj};
}

#endif