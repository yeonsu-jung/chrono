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

// #include "chrono/physics/ChSystemSMC.h"
#include "chrono/utils/ChUtilsCreators.h"

#include "chrono/core/ChRealtimeStep.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

// Use the namespace of Chrono
using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::irrlicht;

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    // Create a Chrono::Engine physical system
    ChSystemNSC sys;
    // ChSystemSMC sys;
    // GetLog() << sys.GetContactMethod() << "\n\n";
    
    const ChVector<> camera_position(0, 0, -15);
    const double container_width = 10;
    const double container_thickness = 0.1;
    const double container_height = 10;

    const double rod_alpha = 38;
    const double rod_length = 10;
    const double rod_diameter = rod_length/rod_alpha;

    float Y_c = 2.0e6f;
    float mu_c = 0.3f;
    float cr_c = 0.1f;


    // Parameters for the containing bin
    int binId = -200;
    double hDimX = 4e-1;         // length in x direction
    double hDimY = 4e-1;         // depth in y direction
    double hDimZ = 7.5e-1;       // height in z direction
    double hThickness = 0.5e-1;  // wall thickness

    //
    // EXAMPLE 1:
    //    
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);
    // mat->SetCompliance(3.2e8f);
    // mat->SetComplianceT(3.2e8f);
    // mat->SetDampingF(1e4f);
    
    
    // Create a ChBody that contains the trajectory (a floor, fixed body)
    // auto floor = chrono_types::make_shared<ChBodyEasyBox>(container_width, 1, container_width, 1000, false, true, mat);
    // floor->SetBodyFixed(true);

    // // floor->GetVisualShape(0)->SetColor(ChColor(0.0f, 0.0f, 0.0f));
    // // floor->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/concrete.jpg"));

    // floor->SetEvalContactCn(true);
    // floor->SetEvalContactCt(true);
    // floor->SetEvalContactKf(true);
    // floor->SetEvalContactSf(true);
    // // wall->SetRestitution(0);
    // // floor->SetRot(Q_from_AngAxis(0.1,VECT_Z));
    // sys.Add(floor);

    // Create a wall
    auto mat_c = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat_c->SetFriction(mu_c);

    // utils::CreateBoxContainer(sys, 0, mat, hdim, 0.2, Vector(0, 0, -hdim.z), Q_from_AngAxis(0, VECT_Y), true, false,
    //                           true, false);
    ChVector<> hdim(5, 5, 5);
    utils::CreateCylindricalContainerFromBoxes(&sys,binId,mat,hdim,container_thickness,25,ChVector<>(0,0,0),Q_from_AngAxis(0,VECT_Y),true);
    // utils::CreateBoxContainer(sys, 0, mat, hdim, 0.2, Vector(0, 0, -hdim.z), Q_from_AngAxis(0, VECT_Y), true, false,true, false);

    // auto wall = chrono_types::make_shared<CreateCylindricalContainerFromBoxes>(sys,binId,mat,container_width,container_thickness,1,ChVector<>(0,0,0),Q_from_AngAxis(0,VECT_Z),true);


    // Create a body
    auto pendulum = chrono_types::make_shared<ChBodyEasyCylinder>(0.1, 2, 1000, true, true, mat);
    pendulum->SetPos(ChVector<>(0, 5, 0));
    pendulum->SetRot(Q_from_AngAxis(0.1,VECT_Z));

    pendulum->SetEvalContactCn(true);
    pendulum->SetEvalContactCt(true);
    pendulum->SetEvalContactKf(true);
    pendulum->SetEvalContactSf(true);

    sys.Add(pendulum);    

    // Create the Irrlicht visualization system
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
    vis->AttachSystem(&sys);
    vis->SetWindowSize(800, 600);
    vis->SetWindowTitle("Paths");
    vis->Initialize();
    vis->AddLogo();
    vis->AddSkyBox();
    vis->AddCamera(camera_position);
    vis->AddTypicalLights();

    // This means that contactforces will be shown in Irrlicht application
    vis->SetSymbolScale(0.2);
    vis->EnableContactDrawing(ContactsDrawMode::CONTACT_NORMALS);

    // Simulation loop
    double timestep = 0.001;
    ChRealtimeStepTimer realtime_timer;

    while (vis->Run()) {
        vis->BeginScene();
        vis->Render();
        vis->EndScene();

        // GetLog() << "Acc force:" << pendulum->Get_accumulated_force() << "\n";
        // GetLog() << "Acc force:" << pendulum->GetAppliedForce() << "\n";
        

        sys.DoStepDynamics(timestep);
        realtime_timer.Spin(timestep);
    }

    return 0;
}
