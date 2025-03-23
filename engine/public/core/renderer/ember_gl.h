#pragma once

#include "ember_core.h"

/*!

    @brief GLSL shader header (API `OpenGL`) 

    @version 0.0.1
*/
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

/*!

   @brief API `OpenGL` shader compilation
   - Compile the shader `FRAGMENT` or `VERTEX`

   @version 0.0.1
   @param type The shader type, `GL_FRAGMENT_SHADER` or `GL_VERTEX_SHADER` 
   @param src The shader source
   @return unsigned int  The compiled shader ID
*/
unsigned int CompileShader(unsigned int type, const char* src);

/*!

   @brief API `OpenGL` default shader program
   - Creates the default shader program

   @version 0.0.1
   @return unsigned int  The shader program ID
*/
unsigned int CreateShaderProgram();

/*!

   @brief Load a texture on `GPU` given a file path
   - Create a texture on `GPU`
   @see Texture

   @version 0.0.1
   @param file_path the path to the file in `assets` folder
   @return Texture 
*/
Texture LoadTexture(const std::string& file_path);

/*!

   @brief Load a `TTF` font on `GPU` given a file path
   - Bake the font on `GPU`


   @version 0.0.1
   @param file_path the path to the file in `assets` folder
   @return Font

*/
Font LoadFont(const std::string& file_path,  int font_size);


/*!

   @brief Unload the Texture from GPU (cleanup)

   @version 0.0.1
   @param texture The loaded Texture
   @return void

*/
void UnloadTexture(Texture texture);

/*!

   @brief Window background color

   @version 0.0.1
   @param color Color in RGBA
   @return void

*/
void ClearBackground(Color color);

/*!

   @brief Starting of the drawing procedure
    - Clear the background and start drawing
    - Draw between BeginDrawing and EndDrawing

   @version 0.0.1
   @return void

*/
void BeginDrawing();

/*!

   @brief End of the drawing procedure (swap buffers)

   @version 0.0.1
   @return void

*/
void EndDrawing();

/*!

   @brief Draw glyphs given a Loaded Font and text
   - Draw the text

   @see Font
   @see Glyph

   @version 0.0.1
   @param font The loaded Font `TTF`
   @param text The text to draw, can be dynamic
   @param position Position in `pixels` coordinates
   @param color Color in RGBA
   @param scale <optional> Scale factor
   @param kerning <optional> Kerning (spacing between characters)
   @return void

*/
void DrawText(Font& font, const std::string& text, glm::vec2 position, Color color, float scale = 1.0f, float kerning = 0.0f);

/*!

   @brief Draw Texture quad at given Rectangle source
   - Draw the texture

   @version 0.0.1
   @param texture The loaded Texture
   @param rect The source rectangle
   @param color Color in RGBA
   @return void

*/
void DrawTexture(Texture texture, Rectangle rect, Color color= {255, 255, 255, 255}) ;

/*!

   @brief Draw Texture quad extended
   - Draw the texture with extended parameters, ex: spritesheet's

   @version 0.0.1
   @param texture The loaded Texture
   @param rect The source Rectangle
   @param dest The destination Rectangle
   @param origin The origin point (texture origin e.g center)
   @param rotation The rotation angle (radians)
   @param color Color in RGBA
   @return void

*/
void DrawTextureEx(Texture texture, Rectangle source, Rectangle dest, glm::vec2 origin, float rotation, Color color = {255, 255, 255, 255});

/*!

   @brief Draw Lines between two points

   @version 0.0.1
   @param start vec2 start point
   @param end vec2 end point
   @param color Color in RGBA
   @return void

*/
void DrawLine(glm::vec2 start,glm::vec2 end, Color color);