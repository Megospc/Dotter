#pragma once

#include "Definitions.h"
#include "Random.h"

namespace Simulation {
    struct Params {
        int count = 1000;
        int repeat = 100;

        float a = 0.266;
        float b = 1.0;

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
        float a = params->a;
        float b = params->b;

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
                    x1 = b*y1+GumowskiF(x, a);
                    y1 = GumowskiF(x1+1.0, a)-x;
                }

                if (attractor == 1) {
                    x1 = y*std::sin(x*y/b)+std::cos(a*x-y);
                    y1 = x+std::sin(y)/b;
                }

                positions[i].x = x1;
                positions[i].y = y1;
            }
        }

        frame++;
    }
}