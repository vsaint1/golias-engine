#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


const char* json_str = R"(
  {
  "player": {
    "name": "golias_bento",
    "level": 1,
    "health": 100,
    "gold": 3193.5
  }
}
)";



int main(int argc, char* argv[]) {

    const char* player_file_path = "user://data/player.json";

    FileAccess playerData;
    playerData.open(player_file_path, ModeFlags::WRITE_READ);
    playerData.store_string(json_str);
    playerData.seek(0);


    LOG_INFO("Player data %s", playerData.get_file_as_str().c_str());

    FileAccess projectData;
    projectData.open("project.xml", ModeFlags::READ);
    LOG_INFO("Project data: %s", projectData.get_file_as_str().c_str());

    playerData.close();


    return 0;
}
