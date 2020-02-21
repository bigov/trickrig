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

struct side_data
{
    uchar id;                       // Определение стороны (Xp|Xn|Yp|Yn|Zp|Zn)
    uchar vbo_data[bytes_per_side]; // Данные вершин, записываемые в VBO (uchar d[])
};

// Структура для работы с форматом данных воксов
struct vox_data
{
    int y;                        // Y-координата вокса
    std::vector<side_data> Sides; // Массив сторон

    vox_data(int p_y, std::vector<side_data> && p_Sides)
      : y(p_y), Sides(std::move(p_Sides)) {}

    vox_data(vox_data&& other): y(other.y), Sides(std::move(other.Sides)) {}

    vox_data& operator= (const vox_data& other) = default;
};

struct data_pack
{
    int x, z;                     // координаты ячейки
    std::vector<vox_data> Voxes;  // Массив воксов, хранящихся в БД

    data_pack(int p_x, int p_z, std::vector<vox_data> && p_voxes)
      : x(p_x), z(p_z), Voxes(std::move(p_voxes)) {}

    data_pack(data_pack&& other)
      : x(other.x), z(other.z), Voxes(std::move(other.Voxes)) {}

    data_pack& operator= (const data_pack& other) = default;
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
    void vox_insert(vox*);
    void vox_delete(int x, int y, int z);
    void init_map_config(const std::string &);
    data_pack load_data_pack(int x, int z);
    data_pack blob_unpack(const std::vector<uchar>& BlobData);
    std::vector<uchar> blob_make(const data_pack& DataPack);
    vox_data vox_data_make(vox* pVox);

  private:
    std::string MapDir       {}; // директория текущей карты (со слэшем в конце)
    std::string MapPFName    {}; // имя файла карты
    std::string CfgMapPFName {}; // файл конфигурации карты/вида
    std::string CfgAppPFName {}; // файл глобальных настроек приложения
    wsql SqlDb {};

    void init_app_config(const std::string &);
    v_str load_config(size_t params_count, const std::string &FilePath);
    std::vector<uchar> load_blob_data(int x, int z);
    bool data_pack_vox_remove(data_pack& DataPack, int y);
    void blob_add_vox_data(std::vector<uchar>& BlobData, const vox_data& VoxData);
    void update_row(const std::vector<uchar>& BlobData, int x, int z);
};

} //tr
#endif
