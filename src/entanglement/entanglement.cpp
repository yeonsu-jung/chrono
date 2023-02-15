// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Alessandro Tasora
// Modified by: Yeonsu Jung
// =============================================================================
//
// Demo code about advanced contact feature: cohesion (using complementarity
// contact method)
//
// =============================================================================

// TO DO: effects of compliance? - why it makes the rods sink

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
#include <regex>
#include <iomanip> 

// progress bar
#include <chrono>

void progress_bar(int progress, int total, const int barWidth) {    
    int progressPercent = static_cast<int>(100.0 * progress / total);
    int barProgress = static_cast<int>(barWidth * progress / total);
    
    std::cout << "\r" << "[" << std::string(barProgress, '=') << std::string(barWidth - barProgress, ' ') << "] " << progressPercent << "%";
    std::cout.flush();
}

// Static values valid through the entire program (bad
// programming practice, but enough for quick tests)

// Some global variables used in this example
// Consider using a class instead (see the other examples) <----- what does this mean?
// TO DO: figure out effects of pre-factor
//

// Define a MyEventReceiver class which will be used to manage input
// from the GUI graphical user interface

// class MyEventReceiver : public IEventReceiver {
//   public:
//     MyEventReceiver(ChVisualSystemIrrlicht* vis) {
//         // store pointer application
//         m_vis = vis;

//         // ..add a GUI slider to control friction
//         scrollbar_friction = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 85, 650, 100), 0, 101);
//         scrollbar_friction->setMax(100);
//         scrollbar_friction->setPos(30);
//         text_friction =
//             m_vis->GetGUIEnvironment()->addStaticText(L"Friction coefficient:", rect<s32>(650, 85, 750, 100), false);

//         // ..add GUI slider to control the speed
//         scrollbar_cohesion = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 125, 650, 140), 0, 102);
//         scrollbar_cohesion->setMax(100);
//         scrollbar_cohesion->setPos(0);
//         text_cohesion =
//             m_vis->GetGUIEnvironment()->addStaticText(L"Cohesion [N]:", rect<s32>(650, 125, 750, 140), false);

//         // ..add GUI slider to control the compliance
//         scrollbar_compliance = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 165, 650, 180), 0, 103);
//         scrollbar_compliance->setMax(100);
//         scrollbar_compliance->setPos(0);
//         text_compliance =
//             m_vis->GetGUIEnvironment()->addStaticText(L"Compliance [mm/N]:", rect<s32>(650, 165, 750, 180), false);
//     }

//     bool OnEvent(const SEvent& event) {
//         // check if user moved the sliders with mouse..
//         if (event.EventType == EET_GUI_EVENT) {
//             s32 id = event.GUIEvent.Caller->getID();

//             switch (event.GUIEvent.EventType) {
//                 case EGET_SCROLL_BAR_CHANGED:
//                     if (id == 101)  // id of 'flow' slider..
//                     {
//                         s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
//                         GLOBAL_friction = (float)pos / 100;
//                     }
//                     if (id == 102)  // id of 'speed' slider..
//                     {
//                         s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
//                         GLOBAL_cohesion = (((float)pos) / 100) * 200000.0f;
//                     }
//                     if (id == 103)  // id of 'compliance' slider..
//                     {
//                         s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
//                         GLOBAL_compliance = (((float)pos) / 100) / 1000000.0f;
//                     }
//                     break;
//                 default:
//                     break;
//             }
//         }

//         return false;
//     }

//   private:
//     ChVisualSystemIrrlicht* m_vis;

//     IGUIScrollBar* scrollbar_friction;
//     IGUIStaticText* text_friction;
//     IGUIScrollBar* scrollbar_cohesion;
//     IGUIStaticText* text_cohesion;
//     IGUIScrollBar* scrollbar_compliance;
//     IGUIStaticText* text_compliance;
// };

// std::shared_ptr<ChBody>
std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys,
                                                  double rod_radius,
                                                  double rod_length,
                                                  double rod_density) {
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);
    mat->SetRollingFriction(0.05f);  // what value?
    mat->SetCohesion(0.8f);

    auto cyl =
        chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius, rod_length, rod_density, true, true, mat,
                                                      chrono_types::make_shared<collision::ChCollisionModelBullet>());
    cyl->SetPos(ChVector<>(0, 20, 0));
    cyl->SetRot(Q_from_AngAxis(0, VECT_Y));
    sys.AddBody(cyl);

    return cyl;
}

void load_rods_from_file(ChSystemNSC& sys, std::string file_path, double& box_height, double rod_density, double friction_coefficient, double cohesion, double& alpha) {    
    double rod_radius;
    double rod_length;
    double container_radius;
    double container_height;
    int num_rods;
    std::string generated_time;

    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(friction_coefficient);
    mat->SetCohesion(cohesion);
    std::cout << "Loading rods from file: " << file_path << std::endl;

    std::ifstream myFile;
    myFile.open(file_path);
    int N_skip = 0;
    while (myFile.good()) {
        std::string line;
        std::getline(myFile, line, '\n');
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "alpha") {
            iss >> alpha;            
        } else if (key == "rod_radius") {
            iss >> rod_radius;
        } else if (key == "rod_length") {
            iss >> rod_length;
        } else if (key == "container_radius") {
            iss >> container_radius;
        } else if (key == "container_height") {
            iss >> container_height;
        } else if (key == "num_rods") {
            iss >> num_rods;
        } else if (key == "generated_time") {
            iss >> generated_time;
        } else if (std::isdigit(key[0]) || key[0] == '-') {
            break;
        }
        N_skip++;
    }
    myFile.close();

    box_height = rod_length*4;
    double box_thickness = 1.0;

    int N = 0;
    myFile.open(file_path);
    std::string line;
    while (myFile.good()) {
        std::getline(myFile, line, '\n');
        N = N + 1;
    }
    GetLog() << "N: " << N - N_skip - 1 << "\n";
    myFile.close();

    assert(N - N_skip - 1 == num_rods);
    GetLog() << "Creating rods..."
             << "\n";
    myFile.open(file_path);
    double v[6];  // should it be something like a pointer?
    int iter = 0;
    while (myFile.good()) {
        if (iter < N_skip) {
            std::getline(myFile, line, '\n');
            iter += 1;
            continue;
        }

        std::getline(myFile, line, '\n');
        std::stringstream ss(line);

        // if (line.empty() || iter > 10) {
        if (iter > N - 2) {
            break;
        }

        // GetLog() << "iter: " << iter << "\n";
        iter += 1;
        for (int i = 0; i < 6; i++) {
            // while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            v[i] = std::stod(substr);
        }

        auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius,
                                                                rod_length,
                                                                rod_density,
                                                                true,
                                                                true,
                                                                mat,
                                                                chrono_types::make_shared<collision::ChCollisionModelBullet>());
        double local_factor = 1;
        rod->SetPos(ChVector<>((v[0] + v[3]) / 2 * local_factor,
                               (v[2] + v[5]) / 2 * local_factor - box_height/2 + rod_radius*2,
                               (v[1] + v[4]) / 2 * local_factor));
        // rod->SetPos(ChVector<>((v[0]*factor*5,v[1]*factor*5,v[2]*factor*5)));
        rod->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));

        const ChVector<> v1(v[0], v[2], v[1]);
        const ChVector<> v2(v[3], v[5], v[4]);

        // GetLog() << "Rod length: " << (v1-v2).Length() << "\n";

        rod->SetRot(Q_from_Vect_to_Vect(VECT_Y, v2 - v1));
        rod->SetEvalContactCn(true);
        rod->SetEvalContactCt(true);
        rod->SetEvalContactKf(true);
        rod->SetEvalContactSf(true);
        sys.Add(rod);
    }
}

// void load_rods_from_file(ChSystemNSC& sys,
//                          std::string file_path,
//                          double rod_radius,
//                          double rod_length,
//                          double rod_density,
//                          double box_height,
//                          double box_width,
//                          double box_thickness,
//                          double friction_coefficient,
//                          double cohesion) {
//     // std::string file_path =
//     // "C:/Users/yjung/Documents/GitHub/generate_random_rods/RandomRods_[Alpha38_R15.200000000000001_H15.200000000000001_L15.20000000000001_A0.20_N119_Date2023-02-11_18-14-41].csv";
//     // file_path = "/Users/yeonsu/Documents/github/generate_random_rods/test.csv";
//     // file_path = "C:/Users/yjung/Documents/GitHub/generate_random_rods/test.csv"
//     auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
//     mat->SetFriction(friction_coefficient);
//     mat->SetCohesion(cohesion);
//     std::cout << "Loading rods from file: " << file_path << std::endl;

//     std::ifstream myFile;
//     myFile.open(file_path);
//     int N = 0;
//     std::string line;
//     while (myFile.good()) {
//         std::getline(myFile, line, '\n');
//         N = N + 1;
//     }
//     GetLog() << "N: " << N << "\n";
//     myFile.close();
    
//     myFile.open(file_path);
//     double v[6];  // should it be something like a pointer?
//     int iter = 0;
//     while (myFile.good()) {
//         std::getline(myFile, line, '\n');
//         std::stringstream ss(line);

//         // if (line.empty() || iter > 10) {
//         if (iter > N - 2) {
//             break;
//         }

//         // GetLog() << "iter: " << iter << "\n";
//         iter += 1;
//         for (int i = 0; i < 6; i++) {
//             // while (ss.good()) {
//             std::string substr;
//             std::getline(ss, substr, ',');
//             v[i] = std::stod(substr);
//         }

//         auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius, rod_length, rod_density, true, true, mat);
//         double local_factor = 1;
//         rod->SetPos(ChVector<>((v[0] + v[3]) / 2 * local_factor,
//                                (v[2] + v[5]) / 2 * local_factor - box_height / 2 - box_thickness * 1,
//                                (v[1] + v[4]) / 2 * local_factor));
//         // rod->SetPos(ChVector<>((v[0]*factor*5,v[1]*factor*5,v[2]*factor*5)));
//         rod->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));

//         const ChVector<> v1(v[0], v[2], v[1]);
//         const ChVector<> v2(v[3], v[5], v[4]);

//         // GetLog() << "Rod length: " << (v1-v2).Length() << "\n";

//         rod->SetRot(Q_from_Vect_to_Vect(VECT_Y, v2 - v1));
//         rod->SetEvalContactCn(true);
//         rod->SetEvalContactCt(true);
//         rod->SetEvalContactKf(true);
//         rod->SetEvalContactSf(true);
//         sys.Add(rod);

//         // GetLog() << v1[0] << '\n';
//     }
// }

void create_some_falling_items(ChSystemNSC& sys,
                               int num_rods,
                               double rod_radius,
                               double rod_length,
                               double rod_density,
                               double box_height,
                               double box_width,
                               double box_thickness) {
    // From now on, all created collision models will have a large outward envelope (needed
    // to allow some compliance with the plastic deformation of cohesive bounds
    ChCollisionModel::SetDefaultSuggestedEnvelope(0.3);

    // Shared contact material for falling objects
    auto obj_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    obj_mat->SetFriction(0.4f);
    obj_mat->SetCohesion(0.8f);

    for (int bi = 0; bi < num_rods; bi++) {
        // Create a bunch of ChronoENGINE rigid bodies which will fall..
        // auto mrigidBody = chrono_types::make_shared<ChBodyEasySphere>(0.81,      // radius
        //                                                               1000,      // density
        //                                                               true,      // visualization?
        //                                                               true,      // collision?
        //                                                               obj_mat);  // contact material
        // mrigidBody->SetPos(ChVector<>(-5 + ChRandom() * 10, 4 + bi * 0.05, -5 + ChRandom() * 10));
        // mrigidBody->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));
        // sys.Add(mrigidBody);

        // my change
        auto mrigidBody = chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius,   // radius
                                                                        rod_length,   // height
                                                                        rod_density,  // density
                                                                        true,         // visualization?
                                                                        true,         // collision?
                                                                        obj_mat);     // contact material
        mrigidBody->SetPos(ChVector<>(-5 + ChRandom() * 10, 4 + bi * 0.05, -5 + ChRandom() * 10));
        mrigidBody->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));
        sys.Add(mrigidBody);
    }
}

std::shared_ptr<chrono::ChBody> create_walls(ChSystemNSC& sys,
                  double box_width,
                  double box_height,
                  double box_thickness,
                  double density,
                  double friction_coefficient,
                  double cohesion) {
    // Contact and visualization materials for container
    auto ground_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    ground_mat->SetFriction(friction_coefficient);
    ground_mat->SetCohesion(cohesion);
    auto ground_mat_vis = chrono_types::make_shared<ChVisualMaterial>(*ChVisualMaterial::Default());
    ground_mat_vis->SetKdTexture(GetChronoDataFile("textures/concrete.jpg"));

    // Create the five walls of the rectangular container, using fixed rigid bodies of 'box' type
    auto floorBody = chrono_types::make_shared<ChBodyEasyBox>(box_width, box_thickness, box_width, density, true, true, ground_mat);
    floorBody->SetPos(ChVector<>(0, -box_height / 2, 0));
    floorBody->SetBodyFixed(true);
    floorBody->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    floorBody->SetEvalContactCn(true);
    floorBody->SetEvalContactCt(true);
    floorBody->SetEvalContactKf(true);
    floorBody->SetEvalContactSf(true);

    auto mk = chrono_types::make_shared<ChMarker>();
    mk->SetName("floor_body");
    floorBody->AddMarker(mk);
    sys.Add(floorBody);

    auto topBody = chrono_types::make_shared<ChBodyEasyBox>(box_width, box_thickness, box_width, density, true, true, ground_mat);
    topBody->SetPos(ChVector<>(0, box_height / 2, 0));
    topBody->SetBodyFixed(true);
    topBody->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    topBody->SetEvalContactCn(true);
    topBody->SetEvalContactCt(true);
    topBody->SetEvalContactKf(true);
    topBody->SetEvalContactSf(true);
    sys.Add(topBody);

    auto wallBody1 = chrono_types::make_shared<ChBodyEasyBox>(
        box_thickness, box_height, box_width + box_thickness - 0.01, 1000, false, true, ground_mat);
    wallBody1->SetPos(ChVector<>(-box_width / 2, 0, 0));
    wallBody1->SetBodyFixed(true);
    // wallBody1->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody1);

    auto wallBody2 = chrono_types::make_shared<ChBodyEasyBox>(
        box_thickness, box_height, box_width + box_thickness - 0.01, 1000, false, true, ground_mat);
    wallBody2->SetPos(ChVector<>(box_width / 2, 0, 0));
    wallBody2->SetBodyFixed(true);
    // wallBody2->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody2);

    auto wallBody3 = chrono_types::make_shared<ChBodyEasyBox>(box_width + box_thickness - 0.01, box_height,
                                                              box_thickness, 1000, false, true, ground_mat);
    wallBody3->SetPos(ChVector<>(0, 0, -box_width / 2));
    wallBody3->SetBodyFixed(true);
    // wallBody3->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody3);

    auto wallBody4 = chrono_types::make_shared<ChBodyEasyBox>(box_width + box_thickness - 0.01, box_height,
                                                              box_thickness, 1000, false, true, ground_mat);
    wallBody4->SetPos(ChVector<>(0, 0, box_width / 2));
    wallBody4->SetBodyFixed(true);
    // wallBody4->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody4);
    return floorBody;
}

void parsing_inputs_from_file(int& num_rods,
                              double& rod_length,
                              double& rod_radius,
                              double& rod_density,
                              double& box_width,
                              double& box_height,
                              double& box_thickness,
                              double& factor,
                              std::string& file_path,
                              double& friction_coefficient,
                              double& cohesion,
                              bool& visualize,
                              double& simulation_time,
                              double& time_step,
                              double& excitation_frequency,
                              double& excitation_amplitude) {
    std::ifstream file("C:/Users/yjung/Documents/GitHub/chrono/build/bin/Release/inputs.txt");
    // std::ifstream file("/Users/yeonsu/Documents/github/chrono/build/bin/inputs.txt");
    // std::ifstream file("./inputs.txt");
    std::string str;
    while (std::getline(file, str)) {
        std::istringstream iss(str);
        std::string key;
        iss >> key;
        if (key == "num_rods") {
            iss >> num_rods;
        } else if (key == "rod_length") {
            iss >> rod_length;
        } else if (key == "rod_radius") {
            iss >> rod_radius;
        } else if (key == "rod_density") {
            iss >> rod_density;
        } else if (key == "box_width") {
            iss >> box_width;
        } else if (key == "box_height") {
            iss >> box_height;
        } else if (key == "box_thickness") {
            iss >> box_thickness;
        } else if (key == "factor") {
            iss >> factor;
        } else if (key == "file_path") {
            iss >> file_path;
        } else if (key == "friction_coefficient") {
            iss >> friction_coefficient;
        } else if (key == "cohesion") {
            iss >> cohesion;
        } else if (key == "visualize") {
            iss >> visualize;
        } else if (key == "simulation_time") {
            iss >> simulation_time;
        } else if (key == "time_step") {
            iss >> time_step;
        } else if (key == "excitation_frequency") {
            iss >> excitation_frequency;
        } else if (key == "excitation_amplitude") {
            iss >> excitation_amplitude;
        }
    }
}
// you are amazing copilot!

// double avg2(std::vector<double> const& v) {
//     int n = 0;
//     double mean = 0.0;
//     for (auto x : v) {
//         double delta = x - mean;
//         mean += delta/++n;
//     }
//     return mean;
// }

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    int num_rods = 1200;
    double factor = 1;
    double rod_radius = 0.2;
    double alpha = 50;
    double rod_length = rod_radius * 2 * alpha;
    double rod_density = 8000;
    double box_height = 2 * rod_length;
    double box_width = 4 * rod_length;
    double box_thickness = 1;
    double friction_coefficient = 0.4;
    double cohesion = 0.4;
    std::string file_path = "";
    bool visualize = false;
    double simulation_time = 1;
    double time_step = 0.01;
    double excitation_frequency = 0;
    double excitation_amplitude = 0;

    parsing_inputs_from_file(num_rods, rod_length, rod_radius, rod_density, box_width, box_height, box_thickness,
                             factor, file_path, friction_coefficient, cohesion, visualize, simulation_time,time_step,
                             excitation_frequency, excitation_amplitude);
    std::cout << "num_rods: " << num_rods << std::endl;
    std::cout << "rod_length: " << rod_length << std::endl;
    std::cout << "rod_radius: " << rod_radius << std::endl;
    std::cout << "rod_density: " << rod_density << std::endl;
    std::cout << "box_width: " << box_width << std::endl;
    std::cout << "box_height: " << box_height << std::endl;
    std::cout << "box_thickness: " << box_thickness << std::endl;
    std::cout << "file_path: " << file_path << std::endl;
    std::cout << "friction_coefficient: " << friction_coefficient << std::endl;
    std::cout << "cohesion: " << cohesion << std::endl;
    std::cout << "visualize: " << visualize << std::endl;
    std::cout << "simulation_time: " << simulation_time << std::endl;
    std::cout << "time_step: " << time_step << std::endl;
    std::cout << "excitation_frequency: " << excitation_frequency << std::endl;
    std::cout << "excitation_amplitude: " << excitation_amplitude << std::endl;

   

    // void load_rods_from_file(ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double
    // rod_density, double box_height, double box_width, double box_thickness) {

    // Create a ChronoENGINE physical system
    ChSystemNSC sys;

    // Create all the rigid bodies.

    // TO DO: consider using a class to pass geometric, mechanical parameters
    // create_some_falling_items(sys);    
    // auto cyl = chrono_types::make_shared<ChBody>(); // tricky...
    // cyl = test_with_single_cylinder(sys);
    // cyl = test_with_single_cylinder(sys);
    // load_rods_from_file(sys, file_path, rod_radius, rod_length, rod_density, box_height, box_width, box_thickness,
    //                     friction_coefficient, cohesion);

    load_rods_from_file(sys,
                        file_path,
                        box_height,
                        rod_density,
                        friction_coefficient,
                        cohesion,
                        alpha);

    // box_height = box_width;
    // box_width = rod_length*3;
    box_width = box_height;
    

    ChVector<> camera_position(0, box_height/8, -box_height);
    
    std::shared_ptr<chrono::ChBody> floorBody;
    floorBody = create_walls(sys, box_width, box_height, box_thickness, rod_density, friction_coefficient, cohesion);

    // (ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double rod_density, double
    // box_height, double box_width, double box_thickness) Create the Irrlicht visualization system
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();

    if (visualize) {
        vis->AttachSystem(&sys);
        vis->SetWindowSize(800, 600);
        vis->SetWindowTitle("Contacts with cohesion");
        vis->Initialize();
        // vis->AddLogo();
        vis->AddSkyBox();
        vis->AddCamera(camera_position);
        vis->AddTypicalLights();
    }

    // This is for GUI tweaking of system parameters..
    // MyEventReceiver receiver(vis.get());
    // note how to add the custom event receiver to the default interface:
    // vis->AddUserEventReceiver(&receiver);

    // Modify some setting of the physical system for the simulation, if you want

    sys.SetSolverType(ChSolver::Type::MINRES);
    sys.SetSolverMaxIterations(20);

    // Cohesion in a contact depends on the cohesion in the surface property of the
    // touching bodies, but the user can override this value when each contact is created,
    // by instancing a callback as in the following example:

    class MyContactCallback : public ChContactContainer::AddContactCallback {
      public:
        virtual void OnAddContact(const collision::ChCollisionInfo& contactinfo,
                                  ChMaterialComposite* const material) override {
            // Downcast to appropriate composite material type
            auto mat = static_cast<ChMaterialCompositeNSC* const>(material);

            // Set friction according to user setting:
            // mat->static_friction = GLOBAL_friction;

            // Set compliance (normal and tangential at once)
            // mat->compliance = GLOBAL_compliance;
            // mat->complianceT = GLOBAL_compliance;
            // mat->dampingf = GLOBAL_dampingf;

            // Set cohesion according to user setting:
            // Note that we must scale the cohesion force value by time step, because
            // the material 'cohesion' value has the dimension of an impulse.
            float my_cohesion_force = 0.4;
            mat->cohesion = (float)msystem->GetStep() * my_cohesion_force;  //<- all contacts will have this cohesion!

            if (contactinfo.distance > 0.12)
                mat->cohesion = 0;

            // Note that here you might decide to modify the cohesion
            // depending on object sizes, type, time, position, etc. etc.
            // For example, after some time disable cohesion at all, just
            // add here:
            //    if (msystem->GetChTime() > 10) mat->cohesion = 0;
        }
        // virtual void OnReportContact() override {
        //     GetLog() << "OnReportContact" << std::endl;
        // };
        ChSystemNSC* msystem;
    };

    auto mycontact_callback = chrono_types::make_shared<MyContactCallback>();  // create the callback object
    mycontact_callback->msystem = &sys;

    class ContactReporter : public ChContactContainer::ReportContactCallback {
      public:
        ContactReporter() {}

      private:
        virtual bool OnReportContact(const ChVector<>& pA,
                                     const ChVector<>& pB,
                                     const ChMatrix33<>& plane_coord,
                                     const double& distance,
                                     const double& eff_radius,
                                     const ChVector<>& cforce,
                                     const ChVector<>& ctorque,
                                     ChContactable* modA,
                                     ChContactable* modB) override {
            // Check if contact involves box1
            GetLog() << "OnReportContact" << '\n';
            GetLog() << "modA: " << modA->GetPhysicsItem()->GetIdentifier() << '\n';
            GetLog() << "modB: " << modB->GetPhysicsItem()->GetIdentifier() << '\n';
            GetLog() << "cforce: " << cforce << '\n';
            GetLog() << "ctorque: " << ctorque << '\n';

            // GetLog() << "obj1: " << modA->GetPhysicsItem() << std::endl;
            // GetLog() << "obj2: " << modB->GetPhysicsItem() << std::endl;

            return true;
        }
    };

    // Use the above callback to process each contact as it is created.
    sys.GetContactContainer()->RegisterAddContactCallback(mycontact_callback);
    auto creporter = chrono_types::make_shared<ContactReporter>();
    // auto creporter = chrono_types::make_shared<ChContactContainer::ReportContactCallback>();

    size_t found = file_path.find_last_of("/\\");
    std::string file_name = file_path.substr(found + 1);
    std::string file_name_no_ext = file_name.substr(0, file_name.find_last_of("."));
    
    std::string tStepString = std::to_string(time_step*1000);
    size_t dotPos = tStepString.find('.');
    if (dotPos != std::string::npos) {
        tStepString = tStepString.substr(0, dotPos + 3);
    }

    std::string simTimeString = std::to_string(time_step*1000);
    dotPos = simTimeString.find('.');
    if (dotPos != std::string::npos) {
        simTimeString = simTimeString.substr(0, dotPos + 3);
    }

    std::string alphaString = std::to_string(alpha);
    dotPos = alphaString.find('.');
    if (dotPos != std::string::npos) {
        alphaString = alphaString.substr(0, dotPos + 3);
    }

    std::ofstream out_file;
    out_file.open("output_alpha" + alphaString + "_"+ file_name_no_ext + "_" + "tstep_" + tStepString + "simtime_" + simTimeString + ".txt");

    // if (!out_file.is_open()) {
    //     GetLog() << "Unable to open file" << std::endl;
    // }

    int id;
    ChVector<> pos;
    ChVector<> vpos;
    ChQuaternion<> rot;
    ChVector<> vrot;
    ChVector<> contact_force;
    ChVector<> contact_torque;    
    GetLog() << "Start simulation" << '\n';
    GetLog() << "Number of bodies: " << sys.GetNbodies() << '\n';
    GetLog() << "Number of bodies: " << sys.Get_bodylist().size() << '\n';
    // Simulation loop    
    if (visualize) {
        while (vis->Run()) {
            vis->BeginScene();
            vis->Render();
            vis->EndScene();

            // sys.assembly.Get_bodylist();

            // pos = cyl->GetPos();
            // GetLog() << sys << "\n";
            // GetLog() << "pos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
            // TO DO: why burst happens?
            // id = sys.Get_bodylist()[0]->GetIdentifier();
            // GetLog() << "id: " << id << "\n";

            GetLog() << "time: " << sys.GetChTime() << '\n';
            GetLog() << "Number of contacts: " << sys.GetNcontacts() << '\n';
            sys.Set_G_acc(ChVector<>(0, -9.8 + excitation_amplitude*cos( CH_C_PI*excitation_frequency*sys.GetChTime()),0));
            GetLog() << "Excitation: " << sys.Get_G_acc() << '\n';

            // floorBody->SetPos(ChVector<>(0, -box_height/2 + excitation_amplitude*cos( CH_C_PI*excitation_frequency*sys.GetChTime()),0) );

            // std::vector<double> y_pos;
            // for (int i = 5; i < sys.Get_bodylist().size(); i++) {                
            //     pos = sys.Get_bodylist()[i]->GetPos();
            //     y_pos.push_back(pos[1]);
            // }

            // GetLog() << "average y: " << avg2(y_pos) << "\n";

            sys.DoStepDynamics(time_step);
        }
    } else {  // no visualization
        while (sys.GetChTime() < simulation_time) {
            // TO DO: write down callback function to get the position of the cylinder
            // TO DO: convert the quaternion to vector
            
            // GetLog() << '\r' << "time: " << sys.GetChTime() << '\n';
            // GetLog() << "Number of contacts: " << sys.GetNcontacts() << '\n';
            std::cout << "\r" << "time: " << sys.GetChTime() << '\t' << "Number of contacts: " << sys.GetNcontacts();
            

            out_file << "ITEM: TIMESTEP\n" << sys.GetChTime() << "\n";
            out_file << "ITEM: NUMBER OF ATOMS\n" << sys.Get_bodylist().size() << "\n";  // is this necessary?
            // out_file << "ITEM: BOX BOUNDS pp pp pp\n";
            // out_file << "0 10\n0 10\n0 10\n";
            out_file << "ITEM: ATOMS id type x y z vx vy vz u1 u2 u3 u4 w1 w2 w3 fx fy fz tx ty tz\n";
            // Output particle data
            // out_file << "ITEM: ATOMS id type x y z vx vy vz fx fy fz\n";
            for (int i = 0; i < sys.Get_bodylist().size(); i++) {
                id = sys.Get_bodylist()[i]->GetIdentifier();
                pos = sys.Get_bodylist()[i]->GetPos();
                vpos = sys.Get_bodylist()[i]->GetPos_dt();
                rot = sys.Get_bodylist()[i]->GetRot();
                vrot = sys.Get_bodylist()[i]->GetWvel_loc();
                contact_force = sys.Get_bodylist()[i]->GetContactForce();
                contact_torque = sys.Get_bodylist()[i]->GetContactTorque();

                out_file << id << " 1 " << pos[0] << " " << pos[1] << " " << pos[2] << " " << vpos[0] << " " << vpos[1]
                         << " " << vpos[2] << " " << rot.e0() << " " << rot.e1() << " " << rot.e2() << " " << rot.e3()
                         << " " << vrot[0] << " " << vrot[1] << " " << vrot[2] << " " << contact_force[0] << " "
                         << contact_force[1] << " " << contact_force[2] << " " << contact_torque[0] << " "
                         << contact_torque[1] << " " << contact_torque[2] << "\n";
            }

            // out_file << pos[0] << ", " << pos[1] << ", " << pos[2] << ", " << rot.e0() << ", " << rot.e1() << ", "
            //         << rot.e2() << ", " << rot.e3() << "\n";

            // sys.GetContactContainer()->ReportAllContacts(creporter);

            // GetLog() << "id: " << id << "\n";
            // GetLog() << "pos: " << pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
            // GetLog() << "rot: " << rot.e0() << ", " << rot.e1() << ", " << rot.e2() << ", " << rot.e3() << "\n";
            // GetLog() << "contact force: " << contact_force[0] << ", " << contact_force[1] << ", " << contact_force[2]
            //          << "\n";
            // GetLog() << "contact torque: " << contact_torque[0] << ", " << contact_torque[1] << ", "
            //          << contact_torque[2] << "\n";
            sys.DoStepDynamics(time_step);
        }
    }
    out_file.close();
    return 0;
}
