#ifndef REDUCE_H
#define REDUCE_H

#include <sys/stat.h>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

struct files_data
{
    unsigned size;
    std::string path;
    std::vector<std::string> files;

    files_data() : size{ 0 } {}
    files_data(files_data& other) : size{ other.size }, path{ other.path }, files{ other.files } { }
    files_data(files_data&& m_other) noexcept : size{ m_other.size }, path{ m_other.path }, files{ m_other.files } { }

    void addPath(std::string file_path) { path = file_path; }

    files_data& addFiles(std::vector<std::string> v_files) {
        std::for_each(v_files.begin(), v_files.end(), [&](std::string& f_name) {
            files.push_back(f_name);
        });
        size += v_files.size();
        return *this;
    }

    files_data splitLoad(int i, int j)
    {
        files_data f_d; f_d.path = path; 
        f_d.files = std::vector<std::string>(files.begin() + i, files.begin() + j + 1); 
        f_d.size = f_d.files.size(); 
        return std::forward<files_data>(f_d);
    }

    std::vector<std::string>::iterator begin() { return files.begin(); }
    std::vector<std::string>::iterator end() { return files.end(); }

    files_data& operator=(files_data& f_d) { size = f_d.size; path = f_d.path; files = f_d.files; return *this; }
};

template <class T>
struct adder
{
    T t;
    adder() : t{} {}
    adder(T& t) : t{ t } {}
    adder(T&& t) : t{ t } {}
    void plus(T& o_t) { t = t + o_t; }
};

template <class T>
struct subber
{
    T t;
    subber() : t{} {}
    subber(T& t) : t{ t } {}
    subber(T&& t) : t{ t } {}
    void minus(T& o_t) { t = t - o_t; }
};

template<>
struct subber<unsigned>
{
    unsigned t;
    subber() : t{} {}
    subber(unsigned& t) : t{ t } {}
    subber(unsigned&& t) : t{ t } {}
    void minus(unsigned& o_t)
    {
        if (t != 0) t -= o_t;
    }
};

template <class T, class F>
void reduce(std::vector<T>& v_map_t, F& f_v_map_t)
{
    f_v_map_t(v_map_t);
}

inline void reduceMapVector(std::vector<std::map<std::string, unsigned>>& v_map)
{
    auto first_map = v_map.begin();

    for (auto next_map = std::next(first_map); next_map != v_map.end(); ++next_map)
    {
        std::for_each(next_map->begin(), next_map->end(), [&](auto n_pair)
        {
            auto it = first_map->find(n_pair.first);

            if (!(it == first_map->end())) it->second += n_pair.second;
            else first_map->emplace(n_pair);
        });

        next_map->clear();
    }

    v_map.resize(1);
}

template <class T>
void addVector(std::vector<T>& v_t)
{
    auto first = v_t.begin();

    for_each(next(first), v_t.end(), [&](T& i) { *first += i; });

    v_t.resize(1);
}

template <class T>
void coalesceVector(std::vector<std::vector<T>> &v_t) {
    auto first_v = v_t.begin();

    for (auto next_v = std::next(first_v); next_v != v_t.end(); ++next_v)
    {
        std::for_each(next_v->begin(), next_v->end(), [&](auto item)
        {
            first_v->push_back(item);
        });

        next_v->clear();
    }

    v_t.resize(1);
}

inline bool file_exist(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

#endif