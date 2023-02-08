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

// Use the namespace of Chrono
using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::particlefactory;
using namespace chrono::irrlicht;
using namespace chrono::postprocess;


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

        // Create an exporter to POVray
    ChPovRay pov_exporter = ChPovRay(&sys);

    // Important: set the path to the template:
    pov_exporter.SetTemplateFile(GetChronoDataFile("POVRay_chrono_template.pov"));

    // Set the path where it will save all .pov, .ini, .asset and .dat files, a directory will be created if not
    // existing
    pov_exporter.SetBasePath(GetChronoOutputPath() + "POVRAY_1");

    // Optional: change the default naming of the generated files:
    // pov_exporter.SetOutputScriptFile("rendering_frames.pov");
    // pov_exporter.SetOutputDataFilebase("my_state");
    // pov_exporter.SetPictureFilebase("picture");

    // --Optional: modify default light
    pov_exporter.SetLight(ChVector<>(-3, 4, 2), ChColor(0.15f, 0.15f, 0.12f), false);

    // --Optional: add further POV commands, for example in this case:
    //     create an area light for soft shadows
    //     create a Grid object; Grid() parameters: step, linewidth, linecolor, planecolor
    //   Remember to use \ at the end of each line for a multiple-line string.
    pov_exporter.SetCustomPOVcommandsScript(
        " \
	light_source {   \
      <2, 10, -3>  \
	  color rgb<1.2,1.2,1.2> \
      area_light <4, 0, 0>, <0, 0, 4>, 8, 8 \
      adaptive 1 \
      jitter\
    } \
	object{ Grid(1,0.02, rgb<0.7,0.8,0.8>, rgbt<1,1,1,1>) rotate <0, 0, 90>  } \
    ");

    
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);
    
    // Create a wall
    auto mat_c = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat_c->SetFriction(mu_c);
    
    // utils::CreateBoxContainer(sys, 0, mat, hdim, 0.2, Vector(0, 0, -hdim.z), Q_from_AngAxis(0, VECT_Y), true, false,
    //                           true, false);
    ChVector<> hdim(6, 6, 5);
    // std::shared_ptr<ChBody> body(system->NewBody());

    auto wall = chrono_types::make_shared<ChBody>();

    wall = utils::CreateCylindricalContainerFromBoxes(&sys,binId,mat_c,hdim,container_thickness,25,ChVector<>(0,0,-2*container_thickness),Q_from_AngAxis(-CH_C_PI/2,VECT_X),true,false,false,false,true);        
    wall->GetVisualShape(0)->SetColor(ChColor(0.1f,0.1f,0.1f));

    // Create a body (for a test)
    auto pendulum = chrono_types::make_shared<ChBodyEasyCylinder>(0.1, 2, 1000, true, true, mat);
    pendulum->SetPos(ChVector<>(0, 5, 0));
    pendulum->SetRot(Q_from_AngAxis(0.1,VECT_Y));

    pendulum->SetEvalContactCn(true);
    pendulum->SetEvalContactCt(true);
    pendulum->SetEvalContactKf(true);
    pendulum->SetEvalContactSf(true);
    sys.Add(pendulum);

    pov_exporter.AddAll();
    pov_exporter.ExportScript();

    // Simulation loop
    double timestep = 0.02;
    double time_settle = 1;
    double chrono_time = 0;
    ChRealtimeStepTimer realtime_timer;    
    
    while (chrono_time < time_settle) {
        chrono_time = chrono_time + timestep;
        // GetLog() << "Acc force:" << pendulum->Get_accumulated_force() << "\n";
        // GetLog() << "Acc force:" << pendulum->GetAppliedForce() << "\n";
                
        sys.DoStepDynamics(timestep);
        realtime_timer.Spin(timestep);
        
        GetLog() << "time= " << sys.GetChTime() << "\n";
        // 2) Create the incremental nnnn.dat and nnnn.pov files that will be load
        //    by the pov .ini script in POV-Ray (do this at each simulation timestep)
        pov_exporter.ExportData();
    }

    return 0;
}
