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
// Demo code about using paths for defining trajectories
//
// =============================================================================

#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkMotorRotationSpeed.h"
#include "chrono/physics/ChLinkTrajectory.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono/physics/ChMaterialSurface.h"
#include "chrono/particlefactory/ChParticleEmitter.h"
#include "chrono/particlefactory/ChParticleRemover.h"

#include "chrono_postprocess/ChPovRay.h"
#include "chrono_thirdparty/filesystem/path.h"

// #include "chrono/physics/ChSystemSMC.h"
#include "chrono/utils/ChUtilsCreators.h"
#include "chrono/core/ChRealtimeStep.h"

#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

#include <fstream>
#include <iostream>
#include <string>
// #include <vector>
#include <sstream>

// using namespace std; % why not allowed

// Use the namespace of Chrono
using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::particlefactory;
using namespace chrono::irrlicht;
// using namespace chrono::postprocess;

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    // Create a Chrono::Engine physical system
    ChSystemNSC sys;
    // ChSystemSMC sys;
    // GetLog() << sys.GetContactMethod() << "\n\n";

    const ChVector<> camera_position(15, 0, 0);
    const double container_width = 10;
    const double container_thickness = 0.1;
    const double container_height = 10;

    const double rod_alpha = 38;
    const double rod_length = 10;
    const double rod_diameter = rod_length / rod_alpha;

    float Y_c = 2.0e6f;
    float mu_c = 0.3f;
    float cr_c = 0.1f;

    // Parameters for the containing bin
    int binId = -200;
    double hDimX = 4e-1;         // length in x direction
    double hDimY = 4e-1;         // depth in y direction
    double hDimZ = 7.5e-1;       // height in z direction
    double hThickness = 0.5e-1;  // wall thickness

    // Create the Irrlicht visualization system
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
    vis->SetWindowSize(800, 600);
    vis->SetWindowTitle("Entanglement");
    vis->Initialize();
    vis->AddLogo();
    vis->AddSkyBox();
    vis->AddCamera(camera_position);
    // vis->AddTypicalLights();
    vis->AddLight(ChVector<>(60, 160, +60), 300, ChColor(0.7f, 0.7f, 0.7f));

    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);
    mat->SetRollingFriction(1e-3);

    // Create a wall
    auto mat_c = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat_c->SetFriction(0.4f);

    ChVector<> hdim(6, 6, 5);
    auto wall = chrono_types::make_shared<ChBody>();
    wall = utils::CreateCylindricalContainerFromBoxes(
        &sys, binId, mat_c, hdim, container_thickness, 25, ChVector<>(0, 0, 2 * container_thickness),
        Q_from_AngAxis(-CH_C_PI / 2, VECT_X), true, false, false, false, true);
    wall->SetPos(ChVector<> (0,-1,0));
    wall->GetVisualShape(0)->SetColor(ChColor(0.1f, 0.1f, 0.1f));

    // Create a body (for a test)
    // auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(0.1, 2, 8000, true, true, mat);
    // rod->SetPos(ChVector<>(0, 5, 0));
    // rod->SetRot(Q_from_AngAxis(0.1, VECT_Y));
    // rod->SetEvalContactCn(true);
    // rod->SetEvalContactCt(true);
    // rod->SetEvalContactKf(true);
    // rod->SetEvalContactSf(true);
    // sys.Add(rod);

    std::ifstream myFile;
    myFile.open(
        "/Users/yeonsu/Documents/github/rod-pile-elastica/0/"
        "SmallRandomRods_[R1.2_H1.5_L1_A0.01_N1289_Date2023-02-08_12-56-38].csv");
    int N = 0;
    while (myFile.good()) {
        std::string line;
        getline(myFile, line, '\n');
        N = N + 1;
    }
    myFile.close();

    myFile.open(
        "/Users/yeonsu/Documents/github/rod-pile-elastica/0/"
        "SmallRandomRods_[R1.2_H1.5_L1_A0.01_N1289_Date2023-02-08_12-56-38].csv");
    double v[6];
    
    int iter = 0;
    while (myFile.good()) {
        std::string line;
        getline(myFile, line, '\n');
        std::stringstream ss(line);

        // if (line.empty() || iter > 10) {
        if (iter > 200) {
            break;
        }
        iter += 1;

        for (int i = 0; i < 6; i++) {
            // while (ss.good()) {
            std::string substr;
            getline(ss, substr, ',');
            v[i] = std::stod(substr);
        }

        auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(0.005, 1, 8000, true, true, mat);
        rod->SetPos(ChVector<>((v[0] + v[3]) / 2, (v[1] + v[4]) / 2, (v[2] + v[5]) / 2));

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
    myFile.close();

    // Simulation loop
    double timestep = 0.02;
    double time_settle = 50;
    double chrono_time = 0;
    ChRealtimeStepTimer realtime_timer;

    vis->AttachSystem(&sys);
    // This means that contactforces will be shown in Irrlicht application
    vis->SetSymbolScale(0.2);
    vis->EnableContactDrawing(ContactsDrawMode::CONTACT_NORMALS);

    while (vis->Run() && chrono_time < time_settle) {
        vis->BeginScene();
        vis->Render();
        vis->EndScene();

        chrono_time = chrono_time + timestep;
        // GetLog() << "Acc force:" << rod->Get_accumulated_force() << "\n";
        // GetLog() << "Acc force:" << rod->GetAppliedForce() << "\n";

        sys.DoStepDynamics(timestep);
        realtime_timer.Spin(timestep);

        GetLog() << "time= " << sys.GetChTime() << "\n";
        // GetLog() << "position= " << rod->GetPos() << "\n";
        // 2) Create the incremental nnnn.dat and nnnn.pov files that will be load
        //    by the pov .ini script in POV-Ray (do this at each simulation timestep)
    }

    return 0;
}

