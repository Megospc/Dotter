#pragma once

#define FONT_PATH "assets/Reef/Reef.ttf"

#define ZOOM_STEP 1.01
#define MAX_ZOOM 150.0
#define MIN_ZOOM -500.0
#define CAMERA_STEP 0.1

#include "GUI.h"
#include "GL.h"
#include "Input.h"
#include "Simulation.h"
#include "Render.h"
#include "Files.h"
#include "Images.h"

#define CollapsingHeader(name) ImGui::Separator();if (ImGui::TreeNodeEx(name))
#define CollapsingEnd ImGui::TreePop()

namespace Interface {
    GL::Window* window = nullptr;

    ImFont* fontMedium[4];

    bool escaping = false;
    bool pause = false;
    bool rendering = true;
    bool postproc = false;
    bool fullscreen = false;
    bool confignoupdate = false;
    bool starred = false;
    bool showmouse = false;

    bool windowperformance = false;
    bool windowappearance = false;
    bool windowcamera = false;
    bool windowsettings = false;
    bool windowbuildinfo = false;

    float basecolor[3] = { 0.26, 0.59, 0.98 };
    float rounding = 5.0;
    
    string creatorname = "";

    Simulation::Params params;
    
    Simulation::Simulation* simulation = nullptr;
    Render::Render* render = nullptr;

    uint fps = 60;
    uint lastsecond;
    uint framecount = 0;
    uint windowwidth = 1600;
    uint windowheight = 900;
    uint starframe = 0;
    time_t laststar = 0;

    vec2 camera;
    float zoomsteps;
    float wheelsensitivity = 20.0;
    int interfacescale = 1;
    int ppReducing = 3.0;

    float particlesize = 1.0;
    int theme = 0;
    int themecount = 2;
    int attractor = 0;
    int attractorcount = 3;

    cstr themenames[] = {
        "Black-White",
        "Red-White"
    };

    cstr attractornames[] = {
        "Gumowski-Mira",
        "Bedhead",
        "Clifford"
    };

    float GetScale(int scale = interfacescale) {
        if (scale == 0) return 1.0;
        if (scale == 1) return 1.5;
        if (scale == 2) return 2.0;
        if (scale == 3) return 4.0;
        return 1.0;
    }

    float Scale(float size, int scale = interfacescale) {
        return size*GetScale(scale);
    }

    ImVec2 Scale(ImVec2 size, int scale = interfacescale) {
        return ImVec2(Scale(size.x), Scale(size.y));
    }

    float zoom() {
        return POW(ZOOM_STEP, zoomsteps);
    }

    bool mousedown = false;
    vec2 mousepos;
    vec2 smouse;
    vec2 scamera;
    
    vec2 scrsize() {
        return vec2(window->getWidth(), window->getHeight());
    }

    float simratio() {
        return 1.0;
    }
    float scrratio() {
        return (float)window->getWidth()/window->getHeight();
    }

    vec2 getsimpos(vec2 scrpos) {
        return scrpos*vec2(scrratio()/simratio(), 1.0)/zoom();
    }

    vec2 mousesim() {
        return (getsimpos(mousepos)+1.0-camera)/2.0;
    }

    vec2 prevmousesim(0.0, 0.0);

    void updatemouse() {
        mousepos = (Input::GetMouse()/scrsize()*2.0-1.0)*vec2(1.0, -1.0);
    }

    void updatecamera() {
        camera = scamera+(getsimpos(mousepos)-smouse);
    }

    void mousemove(float x, float y) {
        updatemouse();

        if (mousedown) updatecamera();
    }

    void mousestart() {
        updatemouse();

        mousedown = true;
        scamera = camera;
        smouse = getsimpos(mousepos);
    }

    void mouseend() {
        updatemouse();

        updatecamera();

        mousedown = false;
    }

    void mousebtn(ImGuiMouseButton button, bool down) {
        if (button == ImGuiMouseButton_Left) {
            if (down) mousestart();
            else mouseend();
        }
    }

    bool zoomtocursor = true;

    void mousewheel(float x, float y) {
        vec2 pos1 = mousesim();

        zoomsteps += y*wheelsensitivity;

        SCLAMP(zoomsteps, MIN_ZOOM, MAX_ZOOM);

        if (zoomtocursor) {
            vec2 pos2 = mousesim();

            vec2 d = pos2-pos1;

            camera += d*2.0;
        }
    }

    void cleanup() {
        if (simulation != nullptr) delete simulation;
        if (render != nullptr) delete render;
    }

    uint seed() { // Get random seed
        srand((uint)(Logger::Bench::Time()*1000.0));

        for (uint i = 0; i < 10; i++) rand();

        return rand();
    }

    void recreateRender() {
        if (simulation == nullptr) return;

        if (render != nullptr) delete render;
        
        render = new Render::Render(window, simulation, postproc);
    }

    void start() {
        cleanup();

        simulation = new Simulation::Simulation(&params);
        render = new Render::Render(window, simulation, postproc);
    }

    void updateFullscreen() {
        window->fullscreen(fullscreen, windowwidth, windowheight);
    }

    bool keyshift = false;

    void keyboard(ImGuiKey key, int action) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == ImGuiKey_Minus) zoomsteps -= 10.0;
            if (key == ImGuiKey_Equal) zoomsteps += 10.0;

            SCLAMP(zoomsteps, MIN_ZOOM, MAX_ZOOM);

            if (!mousedown) {
                float step = keyshift ? CAMERA_STEP/10.0:CAMERA_STEP/zoom();

                if (key == ImGuiKey_LeftArrow) camera.x += step;
                if (key == ImGuiKey_RightArrow) camera.x -= step;
                if (key == ImGuiKey_DownArrow) camera.y += step;
                if (key == ImGuiKey_UpArrow) camera.y -= step;
            }
        }

        if (action == GLFW_PRESS) {
            if (key == ImGuiKey_Escape) escaping = true;

            if (key == ImGuiKey_Space || key == ImGuiKey_P) pause = !pause;

            if (key == ImGuiKey_O || key == ImGuiKey_I) camera = vec2(0.0, 0.0);
            if (key == ImGuiKey_O || key == ImGuiKey_0) zoomsteps = 0.0;

            if (key == ImGuiKey_R) start();

            if (key == ImGuiKey_F11) {
                fullscreen = !fullscreen;

                updateFullscreen();
            }

            if (key == ImGuiKey_LeftShift) keyshift = true;
        }

        if (action == GLFW_RELEASE) {
            if (key == ImGuiKey_LeftShift) keyshift = false;
        }
    }

    string scalestr(int scale) {
        switch (scale) {
            case 0: return "x1";
            case 1: return "x1.5";
            case 2: return "x2";
            case 3: return "x4";
        }

        return "x1.5";
    }

    int strscale(string str) {
        if (str == "x1") return 0;
        if (str == "x1.5") return 1;
        if (str == "x2") return 2;
        if (str == "x4") return 3;

        return 1;
    }

    void SaveConfig() {
        string str = "";

        if (fullscreen) str += KeyVal("fullscreen", "");
        if (postproc) str += KeyVal("postprocessing", "");
        if (zoomtocursor) str += KeyVal("zoom-to-mouse", "");
        if (showmouse) str += KeyVal("show-mouse-orbit", "");
        if (starred) str += KeyVal("starred", "");

        str += KeyVal("last-star", std::to_string(laststar));
        
        if (windowperformance) str += KeyVal("window-performance", "");
        if (windowappearance) str += KeyVal("window-appearance", "");
        if (windowcamera) str += KeyVal("window-camera", "");
        if (windowsettings) str += KeyVal("window-settings", "");
        if (windowbuildinfo) str += KeyVal("window-buildinfo", "");

        if (creatorname.length() > 0) str += KeyVal("creator", creatorname);

        str += KeyVal("wheel", wheelsensitivity);
        str += KeyVal("width", windowwidth);
        str += KeyVal("height", windowheight);
        str += KeyVal("theme", theme);
        str += KeyVal("interface", scalestr(interfacescale));
        str += KeyVal("pp-reducing", ppReducing);

        str += KeyVal("color-r", basecolor[0], "%.2f");
        str += KeyVal("color-g", basecolor[1], "%.2f");
        str += KeyVal("color-b", basecolor[2], "%.2f");
        str += KeyVal("rounding", rounding, "%.0f");

        File::Data data;

        data.data = (char*)str.c_str();
        data.length = str.length();

        File::Write("config.txt", data);
    }

    void LoadConfig() {
        File::Data data = File::Read("config.txt", true);

        if (data.length == -1) return;

        fullscreen = false;
        confignoupdate = false;
        postproc = false;

        basecolor[0] = 0.26, basecolor[1] = 0.59, basecolor[2] = 0.98;

        windowperformance = false;
        windowappearance = false;
        windowcamera = false;
        windowsettings = false;
        windowbuildinfo = false;
        zoomtocursor = false;
        starred = false;

        laststar = 0;
        theme = 0;

        creatorname = "";

        basecolor[0] = 0.26, basecolor[1] = 0.59, basecolor[2] = 0.98;

        string str = data.data;

        string line = "";

        for (uint i = 0; i < str.length(); i++) {
            char c = str[i];

            if (c == '\n') {
                KeyVal data(line);
                
                if (data.key == "fullscreen") fullscreen = true;
                if (data.key == "postprocessing") postproc = true;
                if (data.key == "zoom-to-mouse") zoomtocursor = true;
                if (data.key == "show-mouse-orbit") showmouse = true;
                if (data.key == "starred") starred = true;

                if (data.key == "last-star") laststar = stol(data.val);

                if (data.key == "window-performance") windowperformance = true;
                if (data.key == "window-appearance") windowappearance = true;
                if (data.key == "window-camera") windowcamera = true;
                if (data.key == "window-settings") windowsettings = true;
                if (data.key == "window-buildinfo") windowbuildinfo = true;

                if (data.key == "noupdate") confignoupdate = true;

                if (data.key == "wheel") wheelsensitivity = stof(data.val);
                if (data.key == "pp-reducing") ppReducing = stoi(data.val);
                if (data.key == "width") windowwidth = stoi(data.val);
                if (data.key == "height") windowheight = stoi(data.val);
                if (data.key == "theme") theme = stoi(data.val);
                if (data.key == "interface") interfacescale = strscale(data.val);

                if (data.key == "creator") creatorname = data.val;

                if (data.key == "color-r") basecolor[0] = stof(data.val);
                if (data.key == "color-g") basecolor[1] = stof(data.val);
                if (data.key == "color-b") basecolor[2] = stof(data.val);
                if (data.key == "rounding") rounding = stof(data.val);

                line = "";
            } else {
                if (c != '\r') line += c;
            }
        }
    }

    void StyleRescale() {        
        GUI::style->WindowPadding = Scale(ImVec2(3.0, 2.5));
        GUI::style->WindowRounding = Scale(rounding);
        GUI::style->WindowBorderSize = Scale(1.0);
        GUI::style->WindowMinSize = Scale(ImVec2(32.0, 32.0));
        GUI::style->WindowTitleAlign = Scale(ImVec2(0.0, 0.5));
        GUI::style->ChildRounding = Scale(rounding);
        GUI::style->ChildBorderSize = Scale(1.0);
        GUI::style->PopupRounding = Scale(rounding);
        GUI::style->PopupBorderSize = Scale(1.0);
        GUI::style->FramePadding = Scale(ImVec2(2.0, 1.5));
        GUI::style->FrameRounding = Scale(rounding);
        GUI::style->FrameBorderSize = Scale(0.0);
        GUI::style->ItemSpacing = Scale(ImVec2(5.0, 3.0));
        GUI::style->ItemInnerSpacing = Scale(ImVec2(3.0, 3.0));
        GUI::style->CellPadding = Scale(ImVec2(3.0, 1.5));
        GUI::style->TouchExtraPadding = Scale(ImVec2(0.0, 0.0));
        GUI::style->IndentSpacing = Scale(14.0);
        GUI::style->ColumnsMinSpacing = Scale(4.0);
        GUI::style->ScrollbarSize = Scale(10.0);
        GUI::style->ScrollbarRounding = Scale(rounding);
        GUI::style->GrabMinSize = Scale(12.0);
        GUI::style->GrabRounding = Scale(rounding);
        GUI::style->LogSliderDeadzone = Scale(4.0);
        GUI::style->TabRounding = Scale(rounding);
        GUI::style->TabBorderSize = Scale(0.0);
        GUI::style->TabMinWidthForCloseButton = Scale(0.0);
        GUI::style->TabBarBorderSize = Scale(0.0);
        GUI::style->TableAngledHeadersAngle = 30.0;
        GUI::style->ButtonTextAlign = ImVec2(0.5, 0.3);
        GUI::style->SelectableTextAlign = ImVec2(0.0, 0.0);
        GUI::style->SeparatorTextBorderSize = Scale(2.0);
        GUI::style->SeparatorTextPadding = Scale(ImVec2(14.0, 2.0));
        GUI::style->DisplayWindowPadding = Scale(ImVec2(12.0, 12.0));
        GUI::style->DisplaySafeAreaPadding = Scale(ImVec2(2.0, 2.0));
        GUI::style->MouseCursorScale = 1.0;
        GUI::style->CurveTessellationTol = 1.25/GetScale();
        GUI::style->CircleTessellationMaxError = 0.3/GetScale();
    }

    void UpdateColors() {
        GUI::Style(vec3(basecolor[0], basecolor[1], basecolor[2]));
    }

    GL::Texture2D* emojiStar;
    GL::Texture2D* emojiHeart;

    bool Init() {
        LoadConfig();

        window = new GL::Window("Dotter", windowwidth, windowheight, true);

        updateFullscreen();

        if (!window->isOk()) return false;

        if (!GUI::Init(window)) return false;

        { // Load icon
            Img::Data raw = Img::Load("assets/icon.png");

            GLFWimage image = Img::ToGLFW(raw);

            window->icon(&image);

            raw.destroy();
        }

        for (uint i = 0; i < 4; i++) {
            fontMedium[i] = GUI::LoadTTF(FONT_PATH, Scale(16.0, i));
        }

        UpdateColors();

        Input::Window(window);
        Input::Keyboard(keyboard);
        Input::MouseMove(mousemove);
        Input::MouseButton(mousebtn);
        Input::MouseWheel(mousewheel);

        emojiStar = Img::Texture("assets/star.png");
        emojiHeart = Img::Texture("assets/heart.png");

        StyleRescale();

        start();

        zoomsteps = 0.0;
        camera = vec2(0.0, 0.0);

        lastsecond = time(NULL);

        return true;
    }

    void SmallOffset(string id, float size = 8.0) {
        ImGui::InvisibleButton(("smalloffset"+id).c_str(), ImVec2(10.0, Scale(size)));
    }

    void DragFloat(cstr label, float* v, float speed, float min, float max, cstr fmt = "%.2f") {
        ImGui::DragFloat(label, v, speed, min, max, fmt);
        SCLAMP(*v, min, max);
    }
    void DragInt(cstr label, int* v, float speed, int min, int max, cstr fmt = "%d") {
        ImGui::DragInt(label, v, speed, min, max, fmt);
        SCLAMP(*v, min, max);
    }

    void KeyHint(string key, float offset = 0.0) {
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x-Scale(15.0+offset));
        ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 0.3), key.c_str());
    }

    void WarnText(cstr text) {
        ImGui::TextColored(ImVec4(1.0, 1.0, 0.2, 1.0), text);
    }
    
    void frame() {
        ImVec2 buttonTiny = Scale(ImVec2(18.0, 18.0));
        ImVec2 buttonShort = Scale(ImVec2(40.0, 18.0));
        ImVec2 buttonMedium = Scale(ImVec2(80.0, 18.0));
        ImVec2 buttonDouble = Scale(ImVec2(165.0, 18.0));

        simulation->step(attractor, mousesim(), showmouse);

        static float colorTone[3] = { 1.0, 1.0, 1.0 };

        if (rendering) {
            render->ppReducing = ppReducing;

            render->render(
                camera, zoom(),
                particlesize, theme,
                showmouse,
                (escaping ? vec3(0.5):vec3(1.0))*vec3(colorTone[0], colorTone[1], colorTone[2]),
                escaping
            );
        } else GL::Clear(0.0, 0.0, 0.0);

        if (escaping) {
            ImGui::PushFont(fontMedium[interfacescale]);

            ImGui::Begin("Quit");
            ImGui::Text("Are you sure you really wish to quit the app?");

            if (ImGui::Button("Quit", buttonMedium)) window->close();
            ImGui::SameLine();
            if (ImGui::Button("Stay", buttonMedium)) escaping = false;

            ImGui::End();

            ImGui::PopFont();

            return;
        }

        ImGui::PushFont(fontMedium[interfacescale]);

        ImGui::Begin("Forcell");

        if (ImGui::Button(pause ? "Continue":"Pause", buttonMedium)) pause = !pause;
        ImGui::SameLine();
        if (ImGui::Button("Quit", buttonMedium)) escaping = true;
        KeyHint("[space]", 20.0);

        ImGui::Text("Time steps: %d", simulation->frame);

        SmallOffset("pre-headers");

        ImGui::Checkbox("Performance", &windowperformance);
        ImGui::Checkbox("Appearance", &windowappearance);
        ImGui::Checkbox("Camera", &windowcamera);
        ImGui::Checkbox("Settings", &windowsettings);
        ImGui::Checkbox("Build info", &windowbuildinfo);

        SmallOffset("pre-star");
        
        if (starred) {
            ImGui::Image((void*)(intptr_t)emojiHeart->getID(), Scale(ImVec2(18.0, 18.0)));

            ImGui::SameLine(Scale(25.0));

            ImGui::TextColored(ImVec4(1.0, 0.5, 0.7, 1.0), "Big thanks!");
        } else if (time(NULL)-laststar > 86400) {
            float size = sin((float)starframe/30.0)*0.05+0.2;

            ImGui::Image((void*)(intptr_t)emojiStar->getID(), Scale(ImVec2(18.0, 18.0)), ImVec2(size, size), ImVec2(1.0-size, 1.0-size));

            ImGui::SameLine(Scale(25.0));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0, 0.0, 0.0, 0.0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0, 0.0, 0.0, 0.0));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.8, 0.2, 1.0));
            if (ImGui::Button("Star me on GitHub")) {
                Browser("https://github.com/Megospc/Dotter/");
                starred = true;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.3));
            if (ImGui::Button("X")) laststar = time(NULL);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor(3);
            starframe++;
        }
        
        ImGui::End();

        if (windowperformance) {
            ImGui::Begin("Performance", &windowperformance);

            ImGui::Text("Frames per second: %d", fps);

            ImGui::End();
        }

        if (windowappearance) {
            ImGui::Begin("Appearance", &windowappearance);

            ImGui::Checkbox("Rendering", &rendering);

            if (rendering) {
                ImGui::Checkbox("Show mouse orbit", &showmouse);

                ImGui::SetNextItemWidth(Scale(100.0));
                ImGui::SliderFloat("Particle size", &particlesize, 0.1, 2.0, "%.1f");

                ImGui::SetNextItemWidth(Scale(100.0));
                ImGui::Combo("Theme", &theme, themenames, themecount, 8);

                if (ImGui::Checkbox("Post-processing", &postproc)) recreateRender();

                if (postproc) {
                    ImGui::SetNextItemWidth(Scale(120.0));
                    ImGui::SliderInt("##resolutionreducing", &ppReducing, 1.0, 5.0, "Resolution reducing x%d");

                    ImGui::SetNextItemWidth(Scale(120.0));
                    ImGui::ColorEdit3("Color tone", colorTone);
                }
            }

            ImGui::Text("Interface scale:");
            if (ImGui::RadioButton("x1", &interfacescale, 0)) StyleRescale();
            ImGui::SameLine();
            if (ImGui::RadioButton("x1.5", &interfacescale, 1)) StyleRescale();
            ImGui::SameLine();
            if (ImGui::RadioButton("x2", &interfacescale, 2)) StyleRescale();
            ImGui::SameLine();
            if (ImGui::RadioButton("x4", &interfacescale, 3)) StyleRescale();

            if (ImGui::Checkbox("Fullscreen mode", &fullscreen)) updateFullscreen();
            KeyHint("[F11]", 10.0);

            CollapsingHeader("Interface style") {
                ImGui::ColorEdit3("Main color", basecolor);

                ImGui::SetNextItemWidth(Scale(80.0));
                ImGui::SliderFloat("Rounding", &rounding, 0.0, 10.0, "%.0f");

                if (ImGui::Button("Reset", buttonDouble)) {
                    basecolor[0] = 0.26, basecolor[1] = 0.59, basecolor[2] = 0.98;
                    rounding = 5.0;
                }

                UpdateColors();
                StyleRescale();

                CollapsingEnd;
            }

            ImGui::End();
        }

        if (windowcamera) {
            ImGui::Begin("Camera", &windowcamera);

            ImGui::Text("Camera: %.3f:%.3f", -camera.x, -camera.y);
            ImGui::SameLine(ImGui::GetWindowWidth()-buttonMedium.x-GUI::style->ItemSpacing.x);
            if (ImGui::Button("Reset##resetcamera", buttonMedium)) camera = vec2(0.0, 0.0);

            ImGui::SetNextItemWidth(Scale(60.0));
            ImGui::SliderFloat("##zoom", &zoomsteps, MIN_ZOOM, MAX_ZOOM, "");
            ImGui::SameLine();
            ImGui::Text("Zoom: %.2f", zoom());
            ImGui::SameLine(ImGui::GetWindowWidth()-buttonMedium.x-GUI::style->ItemSpacing.x);
            if (ImGui::Button("Reset##resetzoom", buttonMedium)) zoomsteps = 0.0;

            ImGui::SetNextItemWidth(Scale(60.0));
            DragFloat("Mouse wheel sensitivity", &wheelsensitivity, 0.05, 0.0, 1000.0, "%.1f");

            vec2 mpos = mousesim();

            ImGui::Text("Mouse pointer: %.3f:%.3f", mpos.x, mpos.y);

            ImGui::Checkbox("Zoom to mouse pointer", &zoomtocursor);

            if (ImGui::Button("Go to center", buttonMedium)) camera = vec2(0.0, 0.0), zoomsteps = 0.0;
            KeyHint("[O]");

            ImGui::End();
        }

        if (windowsettings) {
            ImGui::Begin("Settings", &windowsettings);

            ImGui::SetNextItemWidth(Scale(80.0));
            DragInt("Particles count", &params.count, 10.0, 0, 100000);
            ImGui::SetNextItemWidth(Scale(80.0));
            DragInt("Particles loop", &params.repeat, 0.5, 0, 1000);

            SmallOffset("settings-pre-params");

            ImGui::SetNextItemWidth(Scale(100.0));
            ImGui::Combo("Attractor", &attractor, attractornames, attractorcount, 8);

            if (attractor == 0) {
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("a", &params.a, 0.001, -0.3, 0.3, "%.3f");
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("b", &params.b, 0.001, 0.8, 1.0, "%.3f");
            }
            if (attractor == 1) {
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("a", &params.a, 0.001, -1.0, 1.0, "%.3f");
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("b", &params.b, 0.001, -1.0, 1.0, "%.3f");
            }
            if (attractor == 2) {
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("a", &params.a, 0.003, 0.0, 3.0, "%.3f");
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("b", &params.b, 0.003, 0.0, 3.0, "%.3f");
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("y", &params.y, 0.002, 0.0, 1.9, "%.3f");
                ImGui::SetNextItemWidth(Scale(60.0));
                DragFloat("d", &params.d, 0.002, 0.0, 1.9, "%.3f");
            }

            if (ImGui::Button("Restart", buttonMedium)) start();
            KeyHint("[R]");

            ImGui::End();
        }

        params.clamp();

        if (windowbuildinfo) {
            ImGui::Begin("Build info", &windowbuildinfo);

            ImGui::BulletText(VERSION);

            #ifdef OPTIMIZATIONS_ON
            ImGui::BulletText("Optimizations enabled");
            #else
            ImGui::BulletText("Optimizations disabled (debug build)");
            #endif

            ImGui::Text("Copyright (c) 2024 Megospc");

            ImGui::End();
        }

        ImGui::PopFont();

        framecount++;

        uint cursecond = time(NULL);

        if (cursecond != lastsecond) {
            fps = framecount;

            framecount = 0;

            lastsecond = cursecond;
        }

        if (!fullscreen) {
            windowwidth = window->getWidth();
            windowheight = window->getHeight();
        }
    }

    void Destroy() {
        if (!confignoupdate) SaveConfig();

        cleanup();

        GUI::Destroy();

        delete emojiStar;

        delete window;
    }

    void Begin() {
        GUI::Begin(frame);
    }
}
