#pragma once

#include "Definitions.h"
#include "Random.h"

namespace Simulation {
    struct Params {
        int count = 1000;
        int repeat = 100;

        float a = 0.266;
        float b = 1.0;
        float y = 1.0;
        float d = 1.0;

        void clamp() {
 
        }
    };

    struct vec2 {
        float x, y;
    };

    class Simulation {
        public:
            Simulation(Params* params) {
                this->params = params;

                count = params->count;
                repeat = params->repeat;

                positions = MALLOC(vec2, count*repeat);

                frame = 0;
            }

            ~Simulation() {
                free(positions);
            }

            void step(int);

            uint frame;

            Params* params;

            vec2* positions;

            int count;
            int repeat;
    };
    
    float GumowskiF(float x, float a) {
        return a*x+(2.0*(1.0-a)*x*x)/((1.0+x*x)*(1.0+x*x));
    }

    void Simulation::step(int attractor) {
        float A = params->a;
        float B = params->b;
        float Y = params->y;
        float D = params->d;

        Rand::Seed(1);

        for (uint j = 0; j < repeat; j++) {
            positions[j*count].x = Rand::Float()*Rand::Sign();
            positions[j*count].y = Rand::Float()*Rand::Sign();

            for (int k = 1; k < count; k++) {
                int i = k+j*count;

                float x = positions[i-1].x;
                float y = positions[i-1].y;

                float x1, y1;

                if (attractor == 0) {
                    x1 = B*y+GumowskiF(x, A);
                    y1 = GumowskiF(x1+1.0, A)-x;
                }

                if (attractor == 1) {
                    x1 = y*std::sin(A*y)+y*std::cos(A*x);
                    y1 = std::sin(y)/B;
                }

                if (attractor == 2) {
                    x1 = std::sin(A*y)+Y*std::cos(A*x);
                    y1 = std::sin(B*x)+D*std::cos(B*y);
                }

                positions[i].x = x1;
                positions[i].y = y1;
            }
        }

        frame++;
    }
}