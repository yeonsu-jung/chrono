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
// FEA contacts
//
// =============================================================================

#include "chrono/physics/ChSystemSMC.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLoadContainer.h"
#include "chrono/geometry/ChTriangleMeshConnected.h"
#include "chrono/solver/ChIterativeSolverLS.h"

#include "chrono/fea/ChElementTetraCorot_4.h"
#include "chrono/fea/ChMesh.h"
#include "chrono/fea/ChMeshFileLoader.h"
#include "chrono/fea/ChContactSurfaceMesh.h"
#include "chrono/fea/ChContactSurfaceNodeCloud.h"
#include "chrono/assets/ChVisualShapeFEA.h"
#include "chrono/fea/ChElementCableANCF.h"
#include "chrono/fea/ChBuilderBeam.h"

#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::fea;
using namespace chrono::irrlicht;

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    // Create a Chrono::Engine physical system
    ChSystemSMC sys;

    sys.SetNumThreads(ChOMP::GetNumProcs(), 0, 1);

    //
    // CREATE THE PHYSICAL SYSTEM
    //

    // Set default effective radius of curvature for all SCM contacts.
    collision::ChCollisionInfo::SetDefaultEffectiveCurvatureRadius(1);

    // collision::ChCollisionModel::SetDefaultSuggestedEnvelope(0.0); // not needed, already 0 when using ChSystemSMC
    collision::ChCollisionModel::SetDefaultSuggestedMargin(
        0.006);  // max inside penetration - if not enough stiffness in material: troubles

    // Use this value for an outward additional layer around meshes, that can improve
    // robustness of mesh-mesh collision detection (at the cost of having unnatural inflate effect)
    double sphere_swept_thickness = 0.002;

    // Create the surface material, containing information
    // about friction etc.
    // It is a SMC (penalty) material that we will assign to
    // all surfaces that might generate contacts.

    auto mysurfmaterial = chrono_types::make_shared<ChMaterialSurfaceSMC>();
    mysurfmaterial->SetYoungModulus(6e4);
    mysurfmaterial->SetFriction(0.3f);
    mysurfmaterial->SetRestitution(0.2f);
    mysurfmaterial->SetAdhesion(0);

    // Create a floor:

    bool do_mesh_collision_floor = false;

    if (do_mesh_collision_floor) {
        auto mmeshbox = ChTriangleMeshConnected::CreateFromWavefrontFile(GetChronoDataFile("models/cube.obj"), true, true);
        // floor as a triangle mesh surface:
        auto mfloor = chrono_types::make_shared<ChBody>();
        mfloor->SetPos(ChVector<>(0, -1, 0));
        mfloor->SetBodyFixed(true);
        sys.Add(mfloor);

        mfloor->GetCollisionModel()->ClearModel();
        mfloor->GetCollisionModel()->AddTriangleMesh(mysurfmaterial, mmeshbox, false, false, VNULL, ChMatrix33<>(1),
                                                     sphere_swept_thickness);
        mfloor->GetCollisionModel()->BuildModel();
        mfloor->SetCollide(true);

        auto masset_meshbox = chrono_types::make_shared<ChTriangleMeshShape>();
        masset_meshbox->SetMesh(mmeshbox);
        masset_meshbox->SetTexture(GetChronoDataFile("textures/concrete.jpg"));
        mfloor->AddVisualShape(masset_meshbox);
    } else {
        // floor as a simple collision primitive:
        auto mfloor = chrono_types::make_shared<ChBodyEasyBox>(2, 0.1, 2, 2700, true, true, mysurfmaterial);
        mfloor->SetBodyFixed(true);
        mfloor->GetVisualShape(0)->SetTexture(GetChronoDataFile("textures/concrete.jpg"));
        sys.Add(mfloor);
    }

    // 2) an ANCF cable:
    int N = 4;
    auto mcontactcloud = chrono_types::make_shared<ChContactSurfaceNodeCloud>(mysurfmaterial);
    for (int i = 0; i < N; i++) {

        auto my_mesh_beams = chrono_types::make_shared<ChMesh>();
        auto msection_cable2 = chrono_types::make_shared<ChBeamSectionCable>();
        msection_cable2->SetDiameter(0.05);
        msection_cable2->SetYoungModulus(200e9);
        msection_cable2->SetBeamRaleyghDamping(0.05);

        // Create random points in a sphere
        ChVector<> p1 = ChVector<>(ChRandom(), ChRandom()+0.2, ChRandom());
        ChVector<> p2 = ChVector<>(ChRandom(), ChRandom()+0.2, ChRandom());

        ChBuilderCableANCF builder;

        builder.BuildBeam(my_mesh_beams,    // the mesh where to put the created nodes and elements
                          msection_cable2,  // the ChBeamSectionCable to use for the ChElementCableANCF elements
                          10,               // the number of ChElementCableANCF to create
                          p1,               // the 'A' point in space (beginning of beam)
                          p2);              // the 'B' point in space (end of beam)
                          
        
        my_mesh_beams->AddContactSurface(mcontactcloud);
        mcontactcloud->AddAllNodes(0.025);
        // mcontactsurf->AddFacesFromBoundary(sphere_swept_thickness);  // do this after my_mesh->AddContactSurface

        // Remember to add the mesh to the system!
        sys.Add(my_mesh_beams);

        auto mvisualizemeshbeam = chrono_types::make_shared<ChVisualShapeFEA>(my_mesh_beams);
        mvisualizemeshbeam->SetFEMdataType(ChVisualShapeFEA::DataType::NODE_SPEED_NORM);
        mvisualizemeshbeam->SetColorscaleMinMax(0.0, 5.50);
        mvisualizemeshbeam->SetSmoothFaces(true);
        my_mesh_beams->AddVisualShapeFEA(mvisualizemeshbeam);

        auto mvisualizemeshbeamnodes = chrono_types::make_shared<ChVisualShapeFEA>(my_mesh_beams);
        mvisualizemeshbeamnodes->SetFEMglyphType(ChVisualShapeFEA::GlyphType::NODE_DOT_POS);
        mvisualizemeshbeamnodes->SetFEMdataType(ChVisualShapeFEA::DataType::NONE);
        mvisualizemeshbeamnodes->SetSymbolsThickness(0.008);
        my_mesh_beams->AddVisualShapeFEA(mvisualizemeshbeamnodes);

    }
    
    // Remember to add the mesh to the system!

    // Create the Irrlicht visualization system
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
    vis->AttachSystem(&sys);
    vis->SetWindowSize(800, 600);
    vis->SetWindowTitle("FEA contacts");
    vis->Initialize();
    vis->AddLogo();
    vis->AddSkyBox();
    vis->AddTypicalLights();
    vis->AddCamera(ChVector<>(0.0, 0.6, -1.0));
    vis->AddTypicalLights();
    // vis->AddLightWithShadow(ChVector<>(1.5, 5.5, -2.5), ChVector<>(0, 0, 0), 3, 2.2, 7.2, 40, 512, ChColor(1, 1, 1));
    // vis->EnableContactDrawing(ContactsDrawMode::CONTACT_DISTANCES);
    vis->EnableShadows();

    // SIMULATION LOOP

    auto solver = chrono_types::make_shared<ChSolverMINRES>();
    sys.SetSolver(solver);
    solver->SetMaxIterations(40);
    solver->SetTolerance(1e-12);
    solver->EnableDiagonalPreconditioner(true);
    solver->EnableWarmStart(true);  // Enable for better convergence when using Euler implicit linearized

    sys.SetSolverForceTolerance(1e-10);

    while (vis->Run()) {
        vis->BeginScene();
        vis->Render();
        vis->EndScene();
        sys.DoStepDynamics(0.001);
        GetLog() << "time: " << sys.GetChTime() << "\n";
    }

    return 0;
}
