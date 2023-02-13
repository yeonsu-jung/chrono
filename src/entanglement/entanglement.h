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

// Use the main namespaces of Irrlicht
using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using namespace irr::io;
using namespace irr::gui;


std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys);
void create_walls(ChSystemNSC& sys, double box_width, double box_height, double box_thickness);
void create_some_falling_items(ChSystemNSC& sys);
void load_rods_from_file(ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double rod_density);
std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys);



#endif