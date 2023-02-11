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

// Static values valid through the entire program (bad
// programming practice, but enough for quick tests)

float GLOBAL_friction = 0.4f;
float GLOBAL_cohesion = 0;
float GLOBAL_compliance = 0;
float GLOBAL_dampingf = 0.1f;

// Some global variables used in this example
// Consider using a class instead (see the other examples) <----- what does this mean?
// TO DO: figure out effects of pre-factor
//
int num_rods = 1000;
double factor = 10;
double rod_radius = 0.81 / 76 * factor*5;
double rod_length = 0.81 * factor*5;
double rod_density = 8000 * factor;

double box_height = 10 * factor;
double box_width = 20 * factor;
double box_thickness = 1 * factor;

// Define a MyEventReceiver class which will be used to manage input
// from the GUI graphical user interface

class MyEventReceiver : public IEventReceiver {
  public:
    MyEventReceiver(ChVisualSystemIrrlicht* vis) {
        // store pointer application
        m_vis = vis;

        // ..add a GUI slider to control friction
        scrollbar_friction = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 85, 650, 100), 0, 101);
        scrollbar_friction->setMax(100);
        scrollbar_friction->setPos(30);
        text_friction =
            m_vis->GetGUIEnvironment()->addStaticText(L"Friction coefficient:", rect<s32>(650, 85, 750, 100), false);

        // ..add GUI slider to control the speed
        scrollbar_cohesion = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 125, 650, 140), 0, 102);
        scrollbar_cohesion->setMax(100);
        scrollbar_cohesion->setPos(0);
        text_cohesion =
            m_vis->GetGUIEnvironment()->addStaticText(L"Cohesion [N]:", rect<s32>(650, 125, 750, 140), false);

        // ..add GUI slider to control the compliance
        scrollbar_compliance = m_vis->GetGUIEnvironment()->addScrollBar(true, rect<s32>(510, 165, 650, 180), 0, 103);
        scrollbar_compliance->setMax(100);
        scrollbar_compliance->setPos(0);
        text_compliance =
            m_vis->GetGUIEnvironment()->addStaticText(L"Compliance [mm/N]:", rect<s32>(650, 165, 750, 180), false);
    }

    bool OnEvent(const SEvent& event) {
        // check if user moved the sliders with mouse..
        if (event.EventType == EET_GUI_EVENT) {
            s32 id = event.GUIEvent.Caller->getID();

            switch (event.GUIEvent.EventType) {
                case EGET_SCROLL_BAR_CHANGED:
                    if (id == 101)  // id of 'flow' slider..
                    {
                        s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        GLOBAL_friction = (float)pos / 100;
                    }
                    if (id == 102)  // id of 'speed' slider..
                    {
                        s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        GLOBAL_cohesion = (((float)pos) / 100) * 200000.0f;
                    }
                    if (id == 103)  // id of 'compliance' slider..
                    {
                        s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        GLOBAL_compliance = (((float)pos) / 100) / 1000000.0f;
                    }
                    break;
                default:
                    break;
            }
        }

        return false;
    }

  private:
    ChVisualSystemIrrlicht* m_vis;

    IGUIScrollBar* scrollbar_friction;
    IGUIStaticText* text_friction;
    IGUIScrollBar* scrollbar_cohesion;
    IGUIStaticText* text_cohesion;
    IGUIScrollBar* scrollbar_compliance;
    IGUIStaticText* text_compliance;
};

void test_with_single_cylinder(ChSystemNSC& sys) {
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);

    auto cyl = chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius, rod_length, rod_density, true, true, mat);
    cyl->SetPos(ChVector<>(0, 0, 0));
    cyl->SetRot(Q_from_AngAxis(CH_C_PI / 2, VECT_X));
    sys.AddBody(cyl);
}

void load_rods_from_file(ChSystemNSC& sys) {
    std::string file_path = "C:/Users/yjung/Dropbox (Harvard University)/Entangled/EnsembleAnalysis/Small_0/SmallRandomRods_[R1.2_H1.5_L1_A0.01_N1289_Date2023-02-08_12-56-38].csv";
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);

    std::ifstream myFile;
    myFile.open(file_path);
    int N = 0;
    while (myFile.good()) {
        std::string line;
        std::getline(myFile, line, '\n');
        N = N + 1;
    }
    GetLog() << "N: " << N << "\n";
    myFile.close();

    GetLog() << "Creating rods..." << "\n";
    myFile.open(file_path);
    double v[6]; // should it be something like a pointer?
    int iter = 0;
    while (myFile.good()) {
        std::string line;
        std::getline(myFile, line, '\n');
        std::stringstream ss(line);

        // if (line.empty() || iter > 10) {
        if (iter > 50) {
            break;
        }
        GetLog() << "iter: " << iter << "\n";
        iter += 1;

        for (int i = 0; i < 6; i++) {
            // while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            v[i] = std::stod(substr);
        }

        auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius, rod_length, rod_density, true, true, mat);
        rod->SetPos(ChVector<>(0, 0, 0));
        // rod->SetPos(ChVector<>((v[0] + v[3]) / 2 * factor*5, (v[1] + v[4]) / 2 * factor*5, (v[2] + v[5]) / 2 * factor*5));
        rod->SetPos(ChVector<>((v[0]*factor*5,v[1]*factor*5,v[2]*factor*5)));
        rod->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));

        const ChVector<> v1(v[0], v[1], v[2]);
        const ChVector<> v2(v[3], v[4], v[5]);

        rod->SetRot(Q_from_Vect_to_Vect(v1, v2));
        rod->SetEvalContactCn(true);
        rod->SetEvalContactCt(true);
        rod->SetEvalContactKf(true);
        rod->SetEvalContactSf(true);
        sys.Add(rod);

        // GetLog() << v1[0] << '\n';
    }
}

void create_some_falling_items(ChSystemNSC& sys) {
    // From now on, all created collision models will have a large outward envelope (needed
    // to allow some compliance with the plastic deformation of cohesive bounds
    ChCollisionModel::SetDefaultSuggestedEnvelope(0.3);

    // Shared contact material for falling objects
    auto obj_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    obj_mat->SetFriction(0.4f);

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
        mrigidBody->SetPos(
            ChVector<>(-5 + ChRandom() * 10 * factor, 4 + bi * 0.05 * factor, -5 + ChRandom() * 10 * factor));
        mrigidBody->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/rock.jpg"));
        sys.Add(mrigidBody);
    }    
}

void create_walls(ChSystemNSC& sys) {
    // Contact and visualization materials for container
    auto ground_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    auto ground_mat_vis = chrono_types::make_shared<ChVisualMaterial>(*ChVisualMaterial::Default());
    ground_mat_vis->SetKdTexture(GetChronoDataFile("textures/concrete.jpg"));

    // Create the five walls of the rectangular container, using fixed rigid bodies of 'box' type
    auto floorBody =
        chrono_types::make_shared<ChBodyEasyBox>(box_width, box_thickness, box_width, 1000, true, true, ground_mat);
    floorBody->SetPos(ChVector<>(0, -box_height / 2, 0));
    floorBody->SetBodyFixed(true);
    floorBody->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(floorBody);

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
}

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    // Create a ChronoENGINE physical system
    ChSystemNSC sys;

    // Create all the rigid bodies.

    // create_some_falling_items(sys);
    create_walls(sys); // sys - as a reference
    // test_with_single_cylinder(sys);
    load_rods_from_file(sys);

    // Create the Irrlicht visualization system
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
    vis->AttachSystem(&sys);
    vis->SetWindowSize(800, 600);
    vis->SetWindowTitle("Contacts with cohesion");
    vis->Initialize();
    // vis->AddLogo();
    vis->AddSkyBox();
    vis->AddCamera(ChVector<>(0, 5 * factor, -20 * factor));
    vis->AddTypicalLights();

    // This is for GUI tweaking of system parameters..
    MyEventReceiver receiver(vis.get());
    // note how to add the custom event receiver to the default interface:
    vis->AddUserEventReceiver(&receiver);

    // Modify some setting of the physical system for the simulation, if you want

    sys.SetSolverType(ChSolver::Type::PSOR);
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
            mat->static_friction = GLOBAL_friction;

            // Set compliance (normal and tangential at once)
            mat->compliance = GLOBAL_compliance;
            mat->complianceT = GLOBAL_compliance;
            mat->dampingf = GLOBAL_dampingf;

            // Set cohesion according to user setting:
            // Note that we must scale the cohesion force value by time step, because
            // the material 'cohesion' value has the dimension of an impulse.
            float my_cohesion_force = GLOBAL_cohesion;
            mat->cohesion = (float)msystem->GetStep() * my_cohesion_force;  //<- all contacts will have this cohesion!

            if (contactinfo.distance > 0.12)
                mat->cohesion = 0;

            // Note that here you might decide to modify the cohesion
            // depending on object sizes, type, time, position, etc. etc.
            // For example, after some time disable cohesion at all, just
            // add here:
            //    if (msystem->GetChTime() > 10) mat->cohesion = 0;
        };
        ChSystemNSC* msystem;
    };

    auto mycontact_callback = chrono_types::make_shared<MyContactCallback>();  // create the callback object
    mycontact_callback->msystem = &sys;                                        // will be used by callback

    // Use the above callback to process each contact as it is created.
    sys.GetContactContainer()->RegisterAddContactCallback(mycontact_callback);

    // Simulation loop
    while (vis->Run()) {
        vis->BeginScene();
        vis->Render();
        vis->EndScene();

        sys.DoStepDynamics(0.01);
    }

    return 0;
}
