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

class db
{
  public:
    db(void) {}
   ~db(void) {}
    v_str open_app(const std::string &); // загрузка данных приложения
    v_str map_open(const std::string &); // загрузка данных карты
    void map_close(std::shared_ptr<glm::vec3> ViewFrom, float* look);
    void map_name_save(const std::string &Dir, const std::string &MapName);
    v_ch map_name_read(const std::string & dbFile);
    void save_window_layout(const layout&);
    //void save_vox(vox*);
    void vox_data_save(vox*);
    void vox_data_delete(int x, int y, int z);
    std::vector<uchar> load_vox_data(int x, int z);
    void erase_vox(vox*);
    void init_map_config(const std::string &);
    //std::unique_ptr<vox> get_vox(const i3d&);

  private:
    std::string MapDir       {}; // директория текущей карты (со слэшем в конце)
    std::string MapPFName    {}; // имя файла карты
    std::string CfgMapPFName {}; // файл конфигурации карты/вида
    std::string CfgAppPFName {}; // файл глобальных настроек приложения
    wsql SqlDb {};

    void init_app_config(const std::string &);
    v_str load_config(size_t params_count, const std::string &file_name);
    void _data_erase(int y, std::vector<uchar>& VoxData);
};

} //tr
#endif
