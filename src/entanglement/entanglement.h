#ifndef MY_HEADER_FILE
#define MY_HEADER_FILE

#include "chrono/assets/ChTexture.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkMotorRotationSpeed.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

// Use the namespaces of Chrono
using namespace chrono;
using namespace chrono::collision;
using namespace chrono::irrlicht;

#include "chrono/assets/ChTexture.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkMotorRotationSpeed.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <entanglement.h>
#include <vector>

// class EntanglementSystem : public ChSystem {
//   public:
//     EntanglementSystem() : ChSystemNSC() {}
//     virtual ~EntanglementSystem() {}

//     virtual void Update() override {
//         // Update all auxiliary data
//         ChSystemNSC::Update();

//         // Perform custom post-processing
//         // ...
//     }

//   protected:
//     double alpha;
//     double rod_radius;
//     double rod_length;
//     double container_radius;
//     double container_height;
//     int num_rods;
//     std::string generated_time;

//     double friction_coefficient;
//     double cohesion;
//     double box_height;
//     double rod_density;
//     std::string file_path;

//     int num_rods = 1200;
//     double factor = 1;
//     double rod_radius = 0.2;
//     double alpha = 50;1
//     double rod_length = rod_radius * 2 * alpha;
//     double rod_density = 8000;
//     double box_height = 2 * rod_length;
//     double box_width = 4 * rod_length;
//     double box_thickness = 1;
//     double friction_coefficient = 0.4;
//     double cohesion = 0.4;
//     std::string file_path = "";
//     bool visualize = false;
//     double simulation_time = 1;

//     std::shared_ptr<ChBody> test_with_single_cylinder()
//     void load_rods_from_file()
//     void create_some_falling_items()
//     void create_walls()
//     void parsing_inputs_from_file()
// };


double avg2(std::vector<double> const& v) {
    int n = 0;
    double mean = 0.0;
    for (auto x : v) {
        double delta = x - mean;
        mean += delta / ++n;
    }
    return mean;
}
#endif
