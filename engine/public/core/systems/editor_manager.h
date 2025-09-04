#pragma once
#include "engine_sys.h"

class Node2D;

void draw_editor(Node2D* root);

void draw_main_menu();

void draw_hierarchy(Node2D* root, float height);
void draw_viewport(float width, float height);
void draw_inspector(Node2D* selected_node, float width, float height);
void draw_console(float width, float height);
void draw_content_browser(float width, float height);

