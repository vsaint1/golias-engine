#pragma once

#include "core/ember_core.h"


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
Font LoadFont(const std::string& file_path, int font_size);


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
    - Orthographic projection
    - Ordered rendering

    Usage:
    BeginDrawing();

    // Draw anything without needing a camera

    EndDrawing();

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
   @param transform The transform
   @param color Color in RGBA
   @param kerning <optional> Kerning (spacing between characters)
   @return void

*/
void DrawText(Font& font, const std::string& text, Transform transform, Color color, float kerning = 0.0f);

/*!

   @brief Draw Texture quad at given Rectangle source
   - Draw the texture

   @version 0.0.1
   @param texture The loaded Texture
   @param rect The source rectangle
   @param color Color in RGBA
   @return void

*/
void DrawTexture(Texture texture, ember::Rectangle rect, Color color = {255, 255, 255, 255});

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
void DrawTextureEx(Texture texture, ember::Rectangle source, ember::Rectangle dest, glm::vec2 origin, float rotation,
                   Color color = {255, 255, 255, 255});

/*!

   @brief Draw Lines between two points

   @version 0.0.1
   @param start vec2 start point
   @param end vec2 end point
   @param color Color in RGBA
   @param thickness float Line thickness
   @return void

*/
void DrawLine(glm::vec2 start, glm::vec2 end, Color color, float thickness = 1.0f);

/*!

   @brief Begin 2D mode
   - Set up the Camera2D component

   Usage:
   BeginMode2D(camera);

   // Drawing with the camera view_matrix

   EndMode2D();

   @version 0.0.2
   @param Camera2D the camera (view_matrix)
   @return void

*/
void BeginMode2D(Camera2D& camera);

/*!

   @brief End 2D mode
   - Restore the default view_matrix


   @version 0.0.2
   @return void

*/
void EndMode2D();
