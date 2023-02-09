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



// #include "chrono/physics/ChSystemSMC.h"
#include "chrono/utils/ChUtilsCreators.h"

#include "chrono/core/ChRealtimeStep.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

// Use the namespace of Chrono
using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::particlefactory;
using namespace chrono::irrlicht;

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

    // visualization setup
    

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
    // mat->SetCompliance(3.2e8f);
    // mat->SetComplianceT(3.2e8f);
    // mat->SetDampingF(1e4f);    

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
    // auto pendulum = chrono_types::make_shared<ChBodyEasyCylinder>(0.1, 2, 1000, true, true, mat);
    // pendulum->SetPos(ChVector<>(0, 5, 0));
    // pendulum->SetRot(Q_from_AngAxis(0.1,VECT_Y));

    // pendulum->SetEvalContactCn(true);
    // pendulum->SetEvalContactCt(true);
    // pendulum->SetEvalContactKf(true);
    // pendulum->SetEvalContactSf(true);
    // sys.Add(pendulum);

    // Create emitter
    // Create an emitter:
    ChParticleEmitter emitter;

    // Ok, that object will take care of generating particle flows for you.
    // It accepts a lot of settings, for creating many different types of particle
    // flows, like fountains, outlets of various shapes etc.
    // For instance, set the flow rate, etc:

    emitter.ParticlesPerSecond() = 100;
    emitter.SetUseParticleReservoir(true);
    emitter.ParticleReservoirAmount() = 5000;

    // Our ChParticleEmitter object, among the main settings, it requires
    // that you give him four 'randomizer' objects: one is in charge of
    // generating random shapes, one is in charge of generating
    // random positions, one for random alignments, and one for random velocities.
    // In the following we need to instance such objects. (There are many ready-to-use
    // randomizer objects already available in chrono, but note that you could also
    // inherit your own class from these randomizers if the choice is not enough).

    // ---Initialize the randomizer for positions
    auto emitter_positions = chrono_types::make_shared<ChRandomParticlePositionRectangleOutlet>();
    emitter_positions->Outlet() =
        ChCoordsys<>(ChVector<>(0, 10, 0), Q_from_AngAxis(CH_C_PI_2, VECT_X));  // center and alignment of the outlet
    emitter_positions->OutletWidth() = 3.0;
    emitter_positions->OutletHeight() = 4.5;
    emitter.SetParticlePositioner(emitter_positions);

    // ---Initialize the randomizer for alignments
    auto emitter_rotations = chrono_types::make_shared<ChRandomParticleAlignmentUniform>();
    emitter.SetParticleAligner(emitter_rotations);

    // ---Initialize the randomizer for velocities, with statistical distribution
    auto mvelo = chrono_types::make_shared<ChRandomParticleVelocityConstantDirection>();
    mvelo->SetDirection(-VECT_Y);
    mvelo->SetModulusDistribution(0.0);
    emitter.SetParticleVelocity(mvelo);

    // ---Initialize the randomizer for creations, with statistical distribution

    // Create a ChRandomShapeCreator object (ex. here for box particles)
    auto mcreator_plastic = chrono_types::make_shared<ChRandomShapeCreatorCylinders>();
    mcreator_plastic->SetDiameterDistribution(
        chrono_types::make_shared<ChConstantDistribution>(0.1));
    mcreator_plastic->SetLengthFactorDistribution(
        chrono_types::make_shared<ChConstantDistribution>(20));
    mcreator_plastic->SetDensityDistribution(
        chrono_types::make_shared<ChConstantDistribution>(8000));
    mcreator_plastic->SetMaterial(mat);

    // Optional: define a callback to be exectuted at each creation of a box particle:
    class MyCreator_plastic : public ChRandomShapeCreator::AddBodyCallback {
        // Here do custom stuff on the just-created particle:
      public:
        virtual void OnAddBody(std::shared_ptr<ChBody> mbody,
                               ChCoordsys<> mcoords,
                               ChRandomShapeCreator& mcreator) override {
            mbody->GetVisualShape(0)->SetColor(ChColor(0.0f, 1.0f, (float)ChRandom()));            
            // mbody->SetEvalContactKf(true);
            // mbody->SetEvalContactSf(true);
        }
    };
    auto callback_plastic = chrono_types::make_shared<MyCreator_plastic>();
    mcreator_plastic->RegisterAddBodyCallback(callback_plastic);

    // Finally, tell to the emitter that it must use the creator above:
    emitter.SetParticleCreator(mcreator_plastic);

    // --- Optional: what to do by default on ALL newly created particles?
    //     A callback executed at each particle creation can be attached to the emitter.
    //     For example, we need that new particles will be bound to Irrlicht visualization:

    // a- define a class that implement your custom OnAddBody method...
    class MyCreatorForAll : public ChRandomShapeCreator::AddBodyCallback {
      public:
        virtual void OnAddBody(std::shared_ptr<ChBody> mbody,
                               ChCoordsys<> mcoords,
                               ChRandomShapeCreator& mcreator) override {
            // Enable Irrlicht visualization for all particles
            vis->BindItem(mbody);

            // Disable gyroscopic forces for increased integrator stabilty
            mbody->SetNoGyroTorque(true);
            mbody->SetEvalContactCn(true);
            mbody->SetEvalContactCt(true);
            mbody->SetEvalContactKf(true);
            mbody->SetEvalContactSf(true);
        }
        ChVisualSystemIrrlicht* vis;
    };

    // b- create the callback object...
    auto mcreation_callback = chrono_types::make_shared<MyCreatorForAll>();
    // c- set callback own data that he might need...
    mcreation_callback->vis = vis.get();
    // d- attach the callback to the emitter!
    emitter.RegisterAddBodyCallback(mcreation_callback);
    
    vis->AttachSystem(&sys);
    // This means that contactforces will be shown in Irrlicht application
    vis->SetSymbolScale(0.2);
    vis->EnableContactDrawing(ContactsDrawMode::CONTACT_NORMALS);

    // Simulation loop
    double timestep = 0.02;
    double elapsed = 0;

    ChRealtimeStepTimer realtime_timer;    
    
    // while (vis->Run()) {
    while (emitter.GetTotCreatedParticles() < 1000) {
        elapsed += timestep;
        // vis->BeginScene();
        // vis->Render();
        // vis->EndScene();

        // GetLog() << "Acc force:" << pendulum->Get_accumulated_force() << "\n";
        // GetLog() << "Acc force:" << pendulum->GetAppliedForce() << "\n";
        
        emitter.EmitParticles(sys, timestep);
        sys.DoStepDynamics(timestep);
        // realtime_timer.Spin(timestep);
        
        GetLog() << "Number of rods:" << emitter.GetTotCreatedParticles() << "\n";
    }

    return 0;
}