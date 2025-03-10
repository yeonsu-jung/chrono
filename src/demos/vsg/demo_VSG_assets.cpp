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
// Authors: Rainer Gericke
// =============================================================================
//
// Demosntration of the Chrono::VSG run-time visualization system
//
// =============================================================================

#include "chrono/physics/ChSystemNSC.h"
#include "chrono/physics/ChParticleCloud.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/geometry/ChLineNurbs.h"
#include "chrono/geometry/ChSurfaceNurbs.h"
#include "chrono/assets/ChBoxShape.h"
#include "chrono/assets/ChPathShape.h"
#include "chrono/assets/ChSphereShape.h"
#include "chrono/assets/ChEllipsoidShape.h"
#include "chrono/assets/ChConeShape.h"
#include "chrono/assets/ChCapsuleShape.h"
#include "chrono/assets/ChCylinderShape.h"
#include "chrono/assets/ChSurfaceShape.h"
#include "chrono/assets/ChModelFileShape.h"

#include "chrono_vsg/ChVisualSystemVSG.h"

// Use the namespace of Chrono
using namespace chrono;
using namespace chrono::geometry;
using namespace chrono::vsg3d;

int main(int argc, char* argv[]) {
    // Create a Chrono system
    ChSystemNSC sys;

    //
    // EXAMPLE 1:
    //

    // Create a ChBody, and attach assets that define 3D shapes for visualization purposes.
    // Note: these assets are independent from collision shapes!

    // Create a rigid body and add it to the physical system:
    auto floor = chrono_types::make_shared<ChBody>();
    floor->SetBodyFixed(true);

    // Contact material
    auto floor_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();

    // Define a collision shape
    floor->GetCollisionModel()->ClearModel();
    floor->GetCollisionModel()->AddBox(floor_mat, 10, 0.5, 10, ChVector<>(0, -1, 0));
    floor->GetCollisionModel()->BuildModel();
    floor->SetCollide(true);

    // Add body to system
    sys.Add(floor);

    // ==Asset== attach a 'box' shape.
    // Note that assets are managed via shared pointer, so they can also be shared).
    auto boxfloor = chrono_types::make_shared<ChBoxShape>();
    boxfloor->GetBoxGeometry().Size = ChVector<>(10, 0.5, 10);
    boxfloor->SetColor(ChColor(0.2f, 0.3f, 1.0f));
    floor->AddVisualShape(boxfloor, ChFrame<>(ChVector<>(0, -1, 0), QUNIT));

    // ==Asset== attach a 'path' shape populated with segments and arcs.
    auto pathfloor = chrono_types::make_shared<ChPathShape>();
    ChLineSegment mseg1(ChVector<>(1, 2, 0), ChVector<>(1, 3, 0));
    pathfloor->GetPathGeometry()->AddSubLine(mseg1);
    ChLineSegment mseg2(ChVector<>(1, 3, 0), ChVector<>(2, 3, 0));
    pathfloor->GetPathGeometry()->AddSubLine(mseg2);
    ChLineArc marc1(ChCoordsys<>(ChVector<>(2, 3.5, 0)), 0.5, -CH_C_PI_2, CH_C_PI_2);
    pathfloor->GetPathGeometry()->AddSubLine(marc1);
    pathfloor->SetColor(ChColor(0.0f, 0.5f, 0.8f));
    floor->AddVisualShape(pathfloor);

    // ==Asset== attach a 'nurbs line' shape:
    // First create the ChLineNurbs geometry, then put it inside a ChLineShape
    auto nurbs = chrono_types::make_shared<ChLineNurbs>();
    std::vector<ChVector<>> controlpoints = {ChVector<>(1, 2, -1), ChVector<>(1, 3, -1), ChVector<>(1, 3, -2),
                                             ChVector<>(1, 4, -2)};
    nurbs->SetupData(3, controlpoints);

    auto nurbsasset = chrono_types::make_shared<ChLineShape>();
    nurbsasset->SetLineGeometry(nurbs);
    nurbsasset->SetColor(ChColor(0.0f, 0.3f, 1.0f));
    floor->AddVisualShape(nurbsasset);

    // ==Asset== attach a 'nurbs surface' shape:
    // First create the ChSurfaceNurbs geometry, then put it inside a ChSurfaceShape
    auto surf = chrono_types::make_shared<ChSurfaceNurbs>();
    ChMatrixDynamic<ChVector<>> surfpoints(4, 2);  // u points, v points
    surfpoints(0, 0) = ChVector<>(1, 2, 3);
    surfpoints(1, 0) = ChVector<>(1, 3, 3);
    surfpoints(2, 0) = ChVector<>(2, 3, 3);
    surfpoints(3, 0) = ChVector<>(2, 4, 3);
    surfpoints(0, 1) = ChVector<>(1, 2, 1);
    surfpoints(1, 1) = ChVector<>(1, 3, 1);
    surfpoints(2, 1) = ChVector<>(3, 3, 1);
    surfpoints(3, 1) = ChVector<>(2, 4, 1);
    surf->SetupData(3, 1, surfpoints);

    auto surfasset = chrono_types::make_shared<ChSurfaceShape>();
    surfasset->SetSurfaceGeometry(surf);
    surfasset->SetWireframe(true);
    surfasset->SetColor(ChColor(0.2f, 0.8f, 0.3f));
    floor->AddVisualShape(surfasset, ChFrame<>(ChVector<>(3, -1, 3), QUNIT));

    //
    // EXAMPLE 2:
    //

    // Create the rigid body (this won't move, it is only for visualization tests)
    auto body = chrono_types::make_shared<ChBody>();
    body->SetBodyFixed(true);
    sys.Add(body);

    // Create a shared visualization material
    auto orange_mat = chrono_types::make_shared<ChVisualMaterial>();
    orange_mat->SetDiffuseColor(ChColor(0.9f, 0.4f, 0.2f));

    // ==Asset== Attach a 'sphere' shape
    auto sphere = chrono_types::make_shared<ChSphereShape>();
    sphere->GetSphereGeometry().rad = 0.5;
    sphere->AddMaterial(orange_mat);
    body->AddVisualShape(sphere, ChFrame<>(ChVector<>(-1, 0, 0), QUNIT));

    // ==Asset== Attach also a 'box' shape
    auto box = chrono_types::make_shared<ChBoxShape>();
    box->GetBoxGeometry().Size = ChVector<>(0.3, 0.5, 0.1);
    box->AddMaterial(orange_mat);
    body->AddVisualShape(box, ChFrame<>(ChVector<>(1, 1, 0), QUNIT));

    // ==Asset== Attach also a 'cylinder' shape
    auto cyl = chrono_types::make_shared<ChCylinderShape>();
    cyl->GetCylinderGeometry().p1 = ChVector<>(2, -0.2, 0);
    cyl->GetCylinderGeometry().p2 = ChVector<>(2.2, 0.5, 0);
    cyl->GetCylinderGeometry().rad = 0.3;
    cyl->AddMaterial(orange_mat);
    body->AddVisualShape(cyl);

    // ==Asset== Attach three instances of the same 'triangle mesh' shape
    auto mesh = chrono_types::make_shared<ChTriangleMeshShape>();
    mesh->GetMesh()->getCoordsVertices().push_back(ChVector<>(0, 0, 0));
    mesh->GetMesh()->getCoordsVertices().push_back(ChVector<>(0, 1, 0));
    mesh->GetMesh()->getCoordsVertices().push_back(ChVector<>(1, 0, 0));
    mesh->GetMesh()->getIndicesVertexes().push_back(ChVector<int>(0, 1, 2));
    mesh->AddMaterial(orange_mat);

    body->AddVisualShape(mesh, ChFrame<>(ChVector<>(2, 0, 2), QUNIT));
    body->AddVisualShape(mesh, ChFrame<>(ChVector<>(3, 0, 2), QUNIT));
    body->AddVisualShape(mesh, ChFrame<>(ChVector<>(2, 1, 2), QUNIT));

    // ==Asset== Attach a 'Wavefront mesh' asset, referencing a .obj file and offset it.
    // Only the first call of a distinct filename loads from disc; subsequent uses of the same model file read from a cache.
    auto objmesh = chrono_types::make_shared<ChModelFileShape>();
    objmesh->SetFilename(GetChronoDataFile("models/forklift/body.obj"));

    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(0, 0.0, 2)));
    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(3, 0.0, 2.5)));
    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(5, 0.0, 3)));
    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(4, 0.0, -3), Q_from_AngY(0.5 * CH_C_PI)));
    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(0, 0.0, -5), Q_from_AngY(CH_C_PI)));
    body->AddVisualShape(objmesh, ChFrame<>(ChVector<>(-4, 0.0, -6), Q_from_AngY(-CH_C_PI_4)));

    // ==Asset== Attach an array of boxes, each rotated to make a spiral
    for (int j = 0; j < 20; j++) {
        auto smallbox = chrono_types::make_shared<ChBoxShape>();
        smallbox->GetBoxGeometry().Size = ChVector<>(0.1, 0.1, 0.01);
        smallbox->SetColor(ChColor(j * 0.05f, 1 - j * 0.05f, 0.0f));
        ChMatrix33<> rot(Q_from_AngY(j * 21 * CH_C_DEG_TO_RAD));
        ChVector<> pos = rot * ChVector<>(0.4, 0, 0) + ChVector<>(0, j * 0.02, 0);
        body->AddVisualShape(smallbox, ChFrame<>(pos, rot));
    }

    //
    // EXAMPLE 3:
    //

    // Create a ChParticleClones cluster, and attach 'assets' that define a single "sample" 3D shape.
    // This will be shown N times in Irrlicht.

    // Create the ChParticleClones, populate it with some random particles,
    // and add it to physical system:
    auto particles = chrono_types::make_shared<ChParticleCloud>();

    // Note: the collision shape, if needed, must be specified before creating particles.
    // This will be shared among all particles in the ChParticleCloud.
    auto particle_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();

    particles->GetCollisionModel()->ClearModel();
    particles->GetCollisionModel()->AddSphere(particle_mat, 0.05);
    particles->GetCollisionModel()->BuildModel();
    particles->SetCollide(true);

    // Create the random particles
    for (int np = 0; np < 100; ++np)
        particles->AddParticle(ChCoordsys<>(ChVector<>(ChRandom() - 2, 1.5, ChRandom() + 2)));

    // Mass and inertia properties.
    // This will be shared among all particles in the ChParticleCloud.
    particles->SetMass(0.1);
    particles->SetInertiaXX(ChVector<>(0.001, 0.001, 0.001));

    // Do not forget to add the particle cluster to the system:
    sys.Add(particles);

    //  ==Asset== Attach a 'sphere' shape asset.. it will be used as a sample
    // shape to display all particles when rendering in 3D!
    auto sphereparticle = chrono_types::make_shared<ChSphereShape>();
    sphereparticle->GetSphereGeometry().rad = 0.05;
    particles->AddVisualShape(sphereparticle);

    //
    // EXAMPLE 4:
    //

    // Create a convex hull shape

    ChVector<> displ(1, 0.0, 0);
    std::vector<ChVector<>> points;
    points.push_back(ChVector<>(0.8, 0.0, 0.0) + displ);
    points.push_back(ChVector<>(0.8, 0.3, 0.0) + displ);
    points.push_back(ChVector<>(0.8, 0.3, 0.3) + displ);
    points.push_back(ChVector<>(0.0, 0.3, 0.3) + displ);
    points.push_back(ChVector<>(0.0, 0.0, 0.3) + displ);
    points.push_back(ChVector<>(0.8, 0.0, 0.3) + displ);
    auto hull = chrono_types::make_shared<ChBodyEasyConvexHullAuxRef>(
        points, 1000, true, true, chrono_types::make_shared<ChMaterialSurfaceNSC>());
    ////hull->SetFrame_REF_to_abs(ChFrame<>(ChVector<>(2,0.3,0)));
    ////hull->SetPos(ChVector<>(2,0.3,0));
    hull->Move(ChVector<>(2, 0.3, 0));

    // Create a visualization material
    auto cadet_blue = chrono_types::make_shared<ChVisualMaterial>();
    cadet_blue->SetDiffuseColor(ChColor(0.37f, 0.62f, 0.62f));
    hull->GetVisualShape(0)->SetMaterial(0, cadet_blue);
    hull->GetVisualShape(0)->GetMaterial(0)->SetOpacity(0.5);  // DepthSorted???
    sys.Add(hull);

    auto vis = chrono_types::make_shared<ChVisualSystemVSG>();
    vis->AttachSystem(&sys);
    vis->SetCameraVertical(CameraVerticalDir::Y);
    vis->SetWindowSize(ChVector2<int>(800, 600));
    vis->SetWindowPosition(ChVector2<int>(100, 300));
    vis->SetWindowTitle("Chrono VSG Assets");
    vis->SetUseSkyBox(true);
    vis->AddCamera(ChVector<>(-8, 8, -16));
    vis->SetCameraAngleDeg(40);
    vis->SetLightIntensity(1.0f);
    vis->SetLightDirection(1.5 * CH_C_PI_2, CH_C_PI_4);
    vis->AddGrid(0.5, 0.5, 12, 12, ChCoordsys<>(ChVector<>(0, -0.49, 0), Q_from_AngX(CH_C_PI_2)),
                 ChColor(0.31f, 0.43f, 0.43f));

    // add scenery objects, not bound to bodies
    auto Zup = Q_from_AngX(-CH_C_PI_2);

    auto sceneMesh1 = chrono_types::make_shared<ChModelFileShape>();
    sceneMesh1->SetFilename(GetChronoDataFile("models/red_teapot.obj"));
    int teapotId1 = vis->AddVisualModel(sceneMesh1, ChFrame<>(ChVector<>(0, 3.5, 3), Zup));
    if (teapotId1 == -1)
        GetLog() << "Could not get teapot!\n";
    int teapotId2 = vis->AddVisualModel(sceneMesh1, ChFrame<>(ChVector<>(-5, 3.5, 3), Zup));
    if (teapotId2 == -1)
        GetLog() << "Could not get teapot!\n";

    auto sceneMesh2 = chrono_types::make_shared<ChModelFileShape>();
    sceneMesh2->SetFilename(GetChronoDataFile("models/bunny.glb"));
    int bunndyId = vis->AddVisualModel(sceneMesh2, ChFrame<>(ChVector<>(-5, 0, 5)));
    if (bunndyId == -1)
        GetLog() << "Could not get bunny!\n";

    auto boxShape = chrono_types::make_shared<ChBoxShape>();
    boxShape->GetBoxGeometry().Size = ChVector<>(0.3, 2.5, 0.1);
    boxShape->AddMaterial(orange_mat);
    int boxId = vis->AddVisualModel(boxShape, ChFrame(ChVector<>(0, 0, 0), QUNIT));

    // Create a shared visualization material
    auto sphere_mat = chrono_types::make_shared<ChVisualMaterial>();
    sphere_mat->SetDiffuseColor(ChColor(0.9f, 0.9f, 0.9f));
    sphere_mat->SetKdTexture(GetChronoDataFile("textures/spheretexture.png"));
    auto sphereShape = chrono_types::make_shared<ChSphereShape>();
    sphereShape->GetSphereGeometry().rad = 0.75;
    sphereShape->SetMaterial(0, sphere_mat);
    int sphereId = vis->AddVisualModel(sphereShape, ChFrame(ChVector<>(-6, 0, -6), QUNIT));

    auto ell_mat = chrono_types::make_shared<ChVisualMaterial>();
    ell_mat->SetKdTexture(GetChronoDataFile("textures/concrete.jpg"));
    auto ellShape = chrono_types::make_shared<ChEllipsoidShape>();
    ellShape->GetEllipsoidGeometry().rad = ChVector<>(0.1, 0.1, 0.3);
    ellShape->SetMaterial(0, ell_mat);
    ChVector<> ellPos(-1, 1, -1);
    int ellId = vis->AddVisualModel(ellShape, ChFrame(ellPos, QUNIT));

    auto caps_mat = chrono_types::make_shared<ChVisualMaterial>();
    caps_mat->SetDiffuseColor(ChColor(0.8f, 0.5f, 0.2f));
    auto capsShape = chrono_types::make_shared<ChCapsuleShape>();
    capsShape->GetCapsuleGeometry().rad = 0.4;
    capsShape->GetCapsuleGeometry().hlen = 0.4;
    capsShape->SetMaterial(0, caps_mat);
    ChVector<> capsPos(-3, 1.5, -1);
    int capsId = vis->AddVisualModel(capsShape, ChFrame(capsPos, QUNIT));

    auto cone_mat = chrono_types::make_shared<ChVisualMaterial>();
    cone_mat->SetKdTexture(GetChronoDataFile("textures/pinkwhite.png"));
    auto coneShape = chrono_types::make_shared<ChConeShape>();
    coneShape->GetConeGeometry().rad = ChVector<>(0.3, 1.0, 0.3);
    coneShape->SetMaterial(0, cone_mat);
    ChVector<> conePos(-4, 1.5, -1);
    int coneId = vis->AddVisualModel(coneShape, ChFrame(conePos, QUNIT));

    vis->Initialize();

    unsigned int frame_number = 0;
    while (vis->Run()) {
        double time = sys.GetChTime();
        if (frame_number == 42) {
            vis->WriteImageToFile("newshot.png");  // does not work with frame == 0!
        }

        vis->UpdateVisualModel(teapotId1, ChFrame(ChVector<>(0, 3.5 + 0.5 * sin(CH_C_PI * time / 10), 3), Zup));
        vis->UpdateVisualModel(teapotId2, ChFrame(ChVector<>(-5, 3.5, 3), Zup * Q_from_AngY(time / 20)));
        vis->UpdateVisualModel(boxId, ChFrame(ChVector<>(0, 0.01 * time, 0), QUNIT));
        vis->UpdateVisualModel(ellId, ChFrame(ellPos, Zup * Q_from_AngY(0.2 * time) * Q_from_AngZ(0.1 * time)));

        if (time < 10.0)
            vis->UpdateVisualModel(sphereId, ChFrame(ChVector<>(6, 0, 6), QUNIT));
        else if (time >= 10.0 && time < 20.0)
            vis->UpdateVisualModel(sphereId, ChFrame(ChVector<>(-6, 0, 6), QUNIT));
        else if (time >= 20.0 && time < 30.0)
            vis->UpdateVisualModel(sphereId, ChFrame(ChVector<>(-6, 0, -6), QUNIT));
        else if (time >= 30.0)
            vis->UpdateVisualModel(sphereId, ChFrame(ChVector<>(6, 0, -6), QUNIT));

        vis->Render();
        sys.DoStepDynamics(0.01);

        frame_number++;
    }

    return 0;
}
