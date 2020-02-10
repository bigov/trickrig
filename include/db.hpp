/*
 * file: db.hpp
 *
 * Заголовочный файл класса управления базой данных
 *
 */

#ifndef DB_HPP
#define DB_HPP

#include "wsql.hpp"
#include "vox.hpp"
#include "framebuf.hpp"
#include "glsl.hpp"
#include "trgl.hpp"

namespace tr {

struct side_data {
    uchar id;                       // Наименование стороны
    uchar vbo_data[bytes_per_side]; // Данные вершин, записываемые в VBO
};

// Структура для работы с форматом данных воксов
struct vox_data {
    int y;                        // Y-координата вокса
    std::vector<side_data> Sides; // Массив сторон
};

struct data_pack {
    int x, z;                     // координаты ячейки
    std::vector<vox_data> Voxes;  // Массив воксов, хранящихся в БД
};

class db
{
  public:
    db(void) {}
   ~db(void) {}
    v_str open_app(const std::string &); // загрузка данных приложения
    v_str map_open(const std::string &); // загрузка данных карты
    void map_close(std::shared_ptr<glm::vec3> ViewFrom, float* look_dir);
    void map_name_save(const std::string &Dir, const std::string &MapName);
    v_ch map_name_read(const std::string & dbFile);
    void save_window_layout(const layout&);
    void vox_data_append(vox*);
    void vox_data_delete(int x, int y, int z);
    void init_map_config(const std::string &);
    data_pack load_data_pack(int x, int z);
    data_pack blob_data_unpack(const std::vector<uchar>& VoxData);
    std::vector<uchar> blob_data_repack(const data_pack& DataPack);

  private:
    std::string MapDir       {}; // директория текущей карты (со слэшем в конце)
    std::string MapPFName    {}; // имя файла карты
    std::string CfgMapPFName {}; // файл конфигурации карты/вида
    std::string CfgAppPFName {}; // файл глобальных настроек приложения
    wsql SqlDb {};

    void init_app_config(const std::string &);
    v_str load_config(size_t params_count, const std::string &file_name);
    std::vector<uchar> load_blob_data(int x, int z);
    void _data_erase(int y, std::vector<uchar>& VoxData);
};

} //tr
#endif
