#pragma once

#define MAX_BUFFER_RESIZE_RATE 80

#include "Definitions.h"
#include "Simulation.h"
#include "GL.h"

namespace Render {
    GLfloat vertices[] = {
        -1.0, -1.0,
        1.0, -1.0,
        -1.0, 1.0,
        1.0, -1.0,
        1.0, 1.0,
        -1.0, 1.0
    };

    class Render {
        public:
            Render(GL::Window* window, Simulation::Simulation* simulation, bool pp) {
                this->window = window;
                this->simulation = simulation;
                this->pp = pp;

                GL_DEBUG_MARKER("start");

                programParticle = GL::LoadProgram(
                    "shaders/particle-vertex.glsl",
                    "shaders/particle-fragment.glsl"
                , "particle");

                if (!programParticle->isOk()) goto unsuccess;

                GL_DEBUG_MARKER("created programParticle");

                if (pp) {
                    programRender = GL::LoadProgram(
                        "shaders/filltex-vertex.glsl",
                        "shaders/render-fragment.glsl"
                    , "render");

                    if (!programRender->isOk()) goto unsuccess;

                    GL_DEBUG_MARKER("created programRender");

                    programPost1 = GL::LoadProgram(
                        "shaders/filltex-vertex.glsl",
                        "shaders/post1-fragment.glsl"
                    , "render");

                    if (!programPost1->isOk()) goto unsuccess;

                    GL_DEBUG_MARKER("created programPost1");

                    programPost2 = GL::LoadProgram(
                        "shaders/filltex-vertex.glsl",
                        "shaders/post2-fragment.glsl"
                    , "render");

                    if (!programPost2->isOk()) goto unsuccess;

                    GL_DEBUG_MARKER("created programPost2");

                    renderTexture1 = new GL::Texture2D();
                    renderTexture2 = new GL::Texture2D();
                }

                quadVAO = new GL::VertexArray();
                quadVertexBuffer = new GL::AttribBuffer();

                quadVAO->bind();

                quadVertexBuffer->bind();
                quadVertexBuffer->setLocation(0);
                quadVertexBuffer->enable(2, GL_FLOAT, sizeof(GLfloat));
                quadVertexBuffer->data(vertices, sizeof(vertices), GL_STATIC_DRAW);
                quadVertexBuffer->unbind();

                quadVAO->unbind();

                particleVAO = new GL::VertexArray();
                vertexBuffer = new GL::AttribBuffer();
                positionBuffer = new GL::AttribBuffer();

                particleVAO->bind();

                vertexBuffer->bind();
                vertexBuffer->setLocation(0);
                vertexBuffer->enable(2, GL_FLOAT, sizeof(GLfloat));
                vertexBuffer->data(vertices, sizeof(vertices), GL_STATIC_DRAW);
                vertexBuffer->unbind();

                positionBuffer->bind();
                positionBuffer->setLocation(1);
                positionBuffer->enable(2, GL_FLOAT, sizeof(float), 0, 0, 1);
                positionBuffer->unbind();
                
                particleVAO->unbind();

                frame = 0;

                ok = true;
                return;

                unsuccess:
                    ok = false;
            }

            ~Render() {
                delete programParticle;
                delete quadVAO;
                delete particleVAO;
                delete quadVertexBuffer;
                delete vertexBuffer;
                delete positionBuffer;

                if (pp) {
                    ppCleanup();

                    delete programPost1;
                    delete programPost2;
                    delete programRender;
                    delete renderTexture1;
                    delete renderTexture2;
                }
            }

            void render(
                vec2 camera, float zoom,
                float particlesize, int theme,
                vec3 clrmul, bool strongblur
            ) {
                if (!ok) return;

                uint ww = window->getWidth();
                uint wh = window->getHeight();

                float scrw, scrh;

                if (pp) {
                    ppCreate(); // Check for framebuffer resizing

                    framebuffer1->bind();

                    scrw = ppWidth;
                    scrh = ppHeight;
                } else {
                    scrw = ww;
                    scrh = wh;
                }

                float scrRatio = scrw/scrh;

                glViewport(0, 0, scrw, scrh);
                
                GL::Clear(0.1, 0.1, 0.1);
                GL::EnableBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                programParticle->use();
                glUniform2f(programParticle->uniform("uCamera"), camera.x, camera.y);
                glUniform1f(programParticle->uniform("uZoom"), zoom);
                glUniform1f(programParticle->uniform("uRatio"), scrRatio);
                glUniform1f(programParticle->uniform("uSize"), particlesize*0.01);
                glUniform1i(programParticle->uniform("uCount"), simulation->count*simulation->repeat);
                glUniform1i(programParticle->uniform("uTheme"), theme);

                particleVAO->bind();

                positionBuffer->bind();
                positionBuffer->data(simulation->positions, sizeof(float)*2*simulation->count*simulation->repeat, GL_DYNAMIC_DRAW);
                positionBuffer->unbind();

                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, simulation->count*simulation->repeat);

                particleVAO->unbind();

                if (pp) {
                    GL::DisableBlending();

                    framebuffer1->unbind();

                    glViewport(0, 0, ppWidth, ppHeight);

                    float blur1 = 1.0/600.0;
                    float blur2 = strongblur ? 1.0/300.0 : 0.0;

                    // Post processing 1
                    renderTexture1->bind(GL_TEXTURE0);
                    framebuffer2->bind();
                    programPost1->use();
                    glUniform2f(programPost1->uniform("uBlur"), blur1/scrRatio, blur1);
                    drawquad();
                    framebuffer2->unbind();
                    renderTexture1->unbind();

                    // Post processing 2
                    renderTexture2->bind(GL_TEXTURE0);
                    framebuffer1->bind();
                    programPost2->use();
                    glUniform3f(programPost2->uniform("uColorMul"), clrmul.x, clrmul.y, clrmul.z);
                    glUniform2f(programPost2->uniform("uBlur"), blur2/scrRatio, blur2);
                    drawquad();
                    framebuffer1->unbind();
                    renderTexture2->unbind();

                    // Rendering from framebuffer

                    glViewport(0, 0, ww, wh);

                    renderTexture1->bind(GL_TEXTURE0);
                    programRender->use();
                    drawquad();
                    renderTexture1->unbind();
                }

                GL_DEBUG_MARKER("frame "+std::to_string(frame));

                frame++;
            }

            float ppReducing = 1.0;
        
        private:
            bool ok;

            Simulation::Simulation* simulation;

            GL::Window* window;

            GL::VertexArray* quadVAO;
            GL::VertexArray* particleVAO;

            GL::Program* programParticle;

            GL::AttribBuffer* vertexBuffer;
            GL::AttribBuffer* positionBuffer;
            GL::AttribBuffer* quadVertexBuffer;

            uint frame;

            void drawquad() {
                quadVAO->bind();

                glDrawArrays(GL_TRIANGLES, 0, 6);

                quadVAO->unbind();
            }

            GL::Program* programRender;
            GL::Program* programPost1;
            GL::Program* programPost2;

            GL::Framebuffer* framebuffer1 = nullptr;
            GL::Renderbuffer* renderbuffer1 = nullptr;
            GL::Texture2D* renderTexture1;
            GL::Framebuffer* framebuffer2 = nullptr;
            GL::Renderbuffer* renderbuffer2 = nullptr;
            GL::Texture2D* renderTexture2;

            // pp = PostProcessing

            uint ppLastUpdate = 0;
            uint ppWidth = 0;
            uint ppHeight = 0;

            bool pp;

            void ppCleanup() {
                if (renderbuffer1 != nullptr) delete renderbuffer1;
                if (framebuffer1 != nullptr) delete framebuffer1;
                if (renderbuffer2 != nullptr) delete renderbuffer2;
                if (framebuffer2 != nullptr) delete framebuffer2;
            }

            void initFramebuffer(GL::Framebuffer** fbuf, GL::Renderbuffer** rbuf, GL::Texture2D* rtex, uint width, uint height) {
                *rbuf = new GL::Renderbuffer();
                *fbuf = new GL::Framebuffer();

                rtex->bind(GL_TEXTURE0);
                rtex->wrap(GL_CLAMP_TO_EDGE);
                rtex->filter(GL_LINEAR);
                rtex->data(NULL, width, height, GL_RGBA, GL_RGBA, GL_FLOAT);

                GL_DEBUG_MARKER("rtex inited");

                (*rbuf)->bind();
                GL_DEBUG_MARKER("rbuf binded");

                (*rbuf)->storage(GL_DEPTH24_STENCIL8, width, height);

                GL_DEBUG_MARKER("rbuf inited");

                (*fbuf)->bind();
                (*fbuf)->texture(rtex);
                (*fbuf)->renderbuffer(*rbuf);
                (*fbuf)->unbind();

                GL_DEBUG_MARKER("fbuf inited");
            }

            void ppCreate() { //  Create (recreate) framebuffer
                uint width = (float)window->getWidth()/ppReducing;
                uint height = (float)window->getHeight()/ppReducing;

                if (framebuffer1 == nullptr || renderbuffer1 == nullptr || framebuffer2 == nullptr || renderbuffer2 == nullptr) goto required; // Skip tests if required

                if (ppWidth == width && ppHeight == height) return; // No need to resize
                if (frame-ppLastUpdate < MAX_BUFFER_RESIZE_RATE) return; // Resizing too frequently

                required:

                ppCleanup();

                GL_DEBUG_MARKER("start resize framebuffer");

                initFramebuffer(&framebuffer1, &renderbuffer1, renderTexture1, width, height);
                GL_DEBUG_MARKER("framebuffer1 inited");
                initFramebuffer(&framebuffer2, &renderbuffer2, renderTexture2, width, height);
                GL_DEBUG_MARKER("framebuffer2 inited");

                programPost1->use();
                glUniform1i(programPost1->uniform("uTexture"), 0);
                programPost2->use();
                glUniform1i(programPost2->uniform("uTexture"), 0);
                programRender->use();
                glUniform1i(programRender->uniform("uTexture"), 0);

                ppWidth = width;
                ppHeight = height;

                ppLastUpdate = frame;
            }
        
        private:

    };
}