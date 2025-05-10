#pragma once

#include "core/component/camera.h"
#include "core/engine_structs.h"

#pragma region OPENGL/ES

#include "core/renderer/mesh.h"
#include "core/renderer/opengl/shader_gl.h"

#pragma endregion

#pragma region METAL


#pragma endregion

#include "core/audio/ember_audio.h"
#include "core/ember_utils.h"
#include "core/system_info.h"




/*!
    @brief Renderer struct

    @version 0.0.1
*/
struct Renderer {
    virtual ~Renderer() = default;

    Renderer() = default;

    SDL_Window* window = nullptr;

    int viewport[2] = {480, 270};

    RendererType type = OPENGL;

    virtual Shader* GetDefaultShader() = 0;

    virtual Shader* GetTextShader() = 0;

    /*!

   @brief Load a texture on `GPU` given a file path
   - Create a texture on `GPU`
   @see Texture


     * @version


     * * *

 * * 0.0.1
   @param file_path the path to the file in `assets` folder
   @return
     * Texture
*/
   virtual Texture LoadTexture(const std::string& file_path) = 0;

    /*!

   @brief Load a `TTF` font on `GPU` given a file path
   - Bake the font on `GPU`


   @version 0.0.1

     *
     * @param


 * * *
 * file_path the path to the file in `assets` folder
   @return Font

*/
    virtual Font LoadFont(const std::string& file_path, int font_size) = 0;

    /*!

   @brief Unload the Font from CPU/GPU (cleanup)

   @version 0.0.9
   @param font The loaded Font
   @return


     * * *

 * *
 * void

*/
    virtual void UnloadFont(const Font& font) = 0;

    /*!

   @brief Unload the Texture from GPU (cleanup)

   @version 0.0.1
   @param texture The loaded Texture

     *
     * @return



 * * * * void

*/
    virtual void UnloadTexture(const Texture& texture) = 0;

    /*!

   @brief Window background color

   @version 0.0.1
   @param color Color in RGBA
   @return void

*/
    virtual void ClearBackground(const Color& color) = 0;

    /*!

   @brief Starting of the drawing procedure
    - Clear the background and start drawing
    - Draw between

     * *

     * *

 * * BeginDrawing and EndDrawing
    - Orthographic projection
    - Ordered rendering

    Usage:


     * * BeginDrawing();





 * * * * // Draw anything without needing a camera

    EndDrawing();

   @version 0.0.1


     * * @return void

*/
    virtual void BeginDrawing() = 0;

    /*!

   @brief End of the drawing procedure (swap buffers)

   @version 0.0.1
   @return void

*/
    virtual void EndDrawing() = 0;

    /*!

   @brief Draw glyphs given a Loaded Font and text
   - Draw the text

   @see Font
   @see Glyph

 @version


     * * *


 * * * 0.0.1
   @param font The loaded Font `TTF`
   @param text The text to draw, can be dynamic
   @param transform
 @param

     * * transform
 * The

 * *
 * transform
   @param color Color in RGBA
   @param font_size
   @param kerning
     *
     * <optional> Kerning (spacing between
 * characters)

 * @return

 * * void

*/
    virtual void DrawText(const Font& font, const std::string& text, Transform transform, Color color, float font_size = 0.0f,
                  float kerning = 0.0f) = 0;

    /*!

   @brief Draw Texture quad at given Rectangle source
   - Draw the texture

   @version 0.0.1
   @param
     *
     * texture


 * * *
 * The loaded Texture
   @param texture
   @param rect The source rectangle
   @param color
     * Color in RGBA
 @return
     * void

*/
    virtual void DrawTexture(const Texture& texture, ember::Rectangle rect, Color color = {255, 255, 255, 255}) = 0;

    /*!

   @brief Draw Texture quad extended
   - Draw the texture with extended parameters, ex: spritesheet's


 *

     * * @version


 * * * 0.0.1
   @param texture The loaded Texture
   @param rect The source Rectangle
   @param
     * dest
     * The
 * destination
 *

 * * Rectangle
   @param origin The origin point (texture origin e.g center)

     * @param rotation
   @param
     * rotation The
 * rotation angle
 *
 *
 * (radians)
   @param color Color in RGBA

     * @return void

*/
    virtual void DrawTextureEx(const Texture& texture, const ember::Rectangle& source, const ember::Rectangle& dest,
                               glm::vec2 origin, float rotation, const Color& color = {255, 255, 255, 255}) = 0;

    /*!

   @brief Draw Lines between two points

   @version 0.0.1
   @param start vec2 start point
   @param end
     * vec2

     * *
 * end

 * * point
   @param color Color in RGBA
   @param thickness float Line thickness
 @return
     * void

*/
    virtual void DrawLine(glm::vec2 start, glm::vec2 end, const Color& color, float thickness = 1.0f) = 0;


    /*!

   @brief Draw rectangle

   @param rect rectangle source
   @param color Color in RGBA
   @param thickness
 *

     * * float


 * * * Line thickness
   @return void

   @version 0.0.6

*/
    virtual void DrawRect(const ember::Rectangle& rect, const Color& color, float thickness = 1.0f) = 0;

    /*!

   @brief Draw rectangle filled


   @param rect rectangle source
   @param color Color in RGBA
   @return
 *

     * * void




 * * * @version 0.0.6


*/
    virtual void DrawRectFilled(const ember::Rectangle& rect, const Color& color) = 0;

    /*!

   @brief Begin 2D mode
   - Set up the Camera2D component

   Usage:
   BeginMode2D(camera);

   // Drawing

     * *
     * with


 * * * the camera view_matrix

   EndMode2D();

   @version 0.0.2
   @param camera the camera 2D

     * * (view_matrix)

 * @return


 * * * void

*/
    virtual void BeginMode2D(const Camera2D& camera) = 0;

    /*!

   @brief End 2D mode
   - Restore the default view_matrix


   @version 0.0.2

   @return void

*/
    virtual void EndMode2D() = 0;

    /*!

   @brief Begin Canvas Procedure
   - Used for Draw Static `UI`

   @version 0.0.7

   @return void

*/
    virtual void BeginCanvas() = 0;

    /*!

   @brief End Canvas Procedure
    - Restore GL state

   @see BeginCanvas

   @version 0.0.7

   @return

     * * void

*/
    virtual void EndCanvas() = 0;

    virtual void Resize(int view_width, int view_height) = 0;

    /*
        @brief Set the current context 
        - Opengl `SDL_GLContext`
        - Metal `MTLDevice`
        
 */
    virtual void SetContext(const void* ctx) = 0;

    /*
        @brief Get the current context
        - dynamic Cast to `SDL_GLContext` or `MTLDevice`
    */
    virtual void* GetContext() = 0;

    virtual void Destroy() = 0;
};






