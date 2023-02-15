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

// TO DO: figure out effects of pre-factor
// Can we switch between NSC and SMC easily?


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

    parsing_inputs_from_file(num_rods, rod_length, rod_radius, rod_density, box_width, box_height, box_thickness,
                             factor, file_path, friction_coefficient, cohesion, visualize, simulation_time);
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

    // void load_rods_from_file(ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double
    // rod_density, double box_height, double box_width, double box_thickness) {

    ChVector<> camera_position(0, box_height / 2, -4 * box_height);

    // Create a ChronoENGINE physical system
    EntnaglementSystem sys;

    // Create all the rigid bodies.
    // TO DO: consider using a class to pass geometric, mechanical parameters
    // create_some_falling_items(sys);
    // auto cyl = chrono_types::make_shared<ChBody>(); // tricky...
    // cyl = test_with_single_cylinder(sys);
    // cyl = test_with_single_cylinder(sys);
    // load_rods_from_file(sys, file_path, rod_radius, rod_length, rod_density, box_height, box_width, box_thickness,
    //                     friction_coefficient, cohesion);

    sys.load_rods_from_file();
    sys.create_walls();
    
    // Create the Irrlicht visualization system
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
        EntanglementSystem* msystem;
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

    // Simulation loop
    std::ofstream out_file;
    out_file.open("output.txt");

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
            std::vector<double> y_pos;
            for (int i = 5; i < sys.Get_bodylist().size(); i++) {
                pos = sys.Get_bodylist()[i]->GetPos();
                y_pos.push_back(pos[1]);
            }

            GetLog() << "average y: " << avg2(y_pos) << "\n";

            sys.DoStepDynamics(0.01);
        }
    } else {  // no visualization
        while (sys.GetChTime() < simulation_time) {
            // TO DO: write down callback function to get the position of the cylinder
            // TO DO: convert the quaternion to vector

            // Output header lines
            GetLog() << "time: " << sys.GetChTime() << '\n';

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
            sys.DoStepDynamics(0.01);
        }
    }
    out_file.close();
    return 0;
}
