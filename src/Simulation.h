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

            void step();

            uint frame;

            Params* params;

            vec2* positions;

            int count;
            int repeat;
    };
    
    float GumowskiF(float x, float a) {
        return a*x+(2.0*(1.0-a)*x*x)/((1.0+x*x)*(1.0+x*x));
    }

    void Simulation::step() {
        float a = params->a;
        float b = params->b;

        Rand::Seed(1);

        for (uint j = 0; j < repeat; j++) {
            positions[j*count].x = Rand::Float()*Rand::Sign();
            positions[j*count].y = Rand::Float()*Rand::Sign();

            for (int k = 1; k < count; k++) {
                int i = k+j*count;

                float x1 = positions[i-1].x;
                float y1 = positions[i-1].y;

                float x, y;

                x = b*y1+GumowskiF(x1, a);
                y = GumowskiF(x+1.0, a)-x1;

                positions[i].x = x;
                positions[i].y = y;
            }
        }

        frame++;
    }
}