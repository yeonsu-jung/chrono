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
// TO DO: some sort of progress bar?

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

#ifdef _WIN32
    #include <direct.h>  // windows
#endif
#ifdef __APPLE__
    #include <sys/stat.h>  //
#endif

#include <cstdio>
#include <cstdio>
#include <string>

bool copy_file(const std::string& source_path, const std::string& dest_path) {
    // Open source file
    FILE* source_file = fopen(source_path.c_str(), "rb");
    if (source_file == nullptr) {
        return false;
    }

    // Open destination file
    FILE* dest_file = fopen(dest_path.c_str(), "wb");
    if (dest_file == nullptr) {
        fclose(source_file);
        return false;
    }

    // Copy data from source file to destination file
    const size_t buffer_size = 1024;
    char buffer[buffer_size];
    size_t bytes_read;
    size_t bytes_written;
    while ((bytes_read = fread(buffer, 1, buffer_size, source_file)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, dest_file);
        if (bytes_written != bytes_read) {
            fclose(source_file);
            fclose(dest_file);
            return false;
        }
    }

    // Close files and return success
    fclose(source_file);
    fclose(dest_file);
    return true;
}

std::string createNumberedDirectory(const std::string& dirPath, const std::string& dirName) {
    // Initialize directory name and counter
    std::string newDirName = dirName;
    int counter = 0;

    // Check if the directory already exists
#ifdef _WIN32
    while (_mkdir((dirPath + newDirName).c_str()) == -1) {
        // If the directory already exists, increment the counter and add it to the directory name
        counter++;
        newDirName = dirName + " (" + std::to_string(counter) + ")";

        if (counter > 15) {
            // If the counter is too high, something is wrong
            throw std::runtime_error("Could not create numbered directory");
        }
    }
#endif

#ifdef __APPLE__
    int status = mkdir((dirPath + newDirName).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    while (status == -1) {
        // If the directory already exists, increment the counter and add it to the directory name
        counter++;
        newDirName = dirName + " (" + std::to_string(counter) + ")";
        status = mkdir((dirPath + newDirName).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if (counter > 15) {
            // If the counter is too high, something is wrong
            throw std::runtime_error("Could not create numbered directory");
        }
    }
#endif

    // Return the path to the new directory
    return dirPath + newDirName + "/";
}

// consider event reciever for real time parameter adjustment
std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys,
                                                  double rod_radius,
                                                  double rod_length,
                                                  double rod_density) {
    auto mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    mat->SetFriction(0.4f);
    mat->SetRollingFriction(0.05f);  // what value?
    // mat->SetCohesion(0.0f);

    auto cyl =
        chrono_types::make_shared<ChBodyEasyCylinder>(rod_radius,
                                                rod_length,
                                                rod_density,
                                                true,
                                                true,
                                                mat,
                                                chrono_types::make_shared<collision::ChCollisionModelBullet>());
    cyl->SetPos(ChVector<>(0, 20, 0));
    cyl->SetRot(Q_from_AngAxis(0, VECT_Y));
    sys.AddBody(cyl);

    return cyl;
}

void load_rods_from_file(ChSystemNSC& sys,
                        const std::string file_path,                        
                        const double& friction_coefficient,
                        const double& cohesion,

                        int& const num_rods,
                        double& const rod_radius,
                        double& const rod_length,
                        double& const rod_density,
                        double& const box_radius,
                        double& const box_height,
                        double& const container_radius,
                        double& const container_height,
                        double& const alpha) {        
    
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
    box_height = rod_length * 4;

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

        auto rod = chrono_types::make_shared<ChBodyEasyCylinder>(
            rod_radius, rod_length, rod_density, true, true, mat,
            chrono_types::make_shared<collision::ChCollisionModelBullet>());
        double local_factor = 1;
        rod->SetPos(ChVector<>((v[0] + v[3]) / 2 * local_factor,
                               (v[2] + v[5]) / 2 * local_factor - box_height / 2 + rod_radius * 2,
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
    // ground_mat->SetCohesion(0.0f);
    auto ground_mat_vis = chrono_types::make_shared<ChVisualMaterial>(*ChVisualMaterial::Default());
    // ground_mat_vis->SetKdTexture(GetChronoDataFile("textures/concrete.jpg"));
    ground_mat_vis->SetDiffuseColor(ChColor(1, 1, 1));
    ;
    ground_mat_vis->SetOpacity(0.05);

    // Create the five walls of the rectangular container, using fixed rigid bodies of 'box' type
    auto floorBody =
        chrono_types::make_shared<ChBodyEasyBox>(box_width, box_thickness, box_width, density, true, true, ground_mat);
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

    auto topBody =
        chrono_types::make_shared<ChBodyEasyBox>(box_width, box_thickness, box_width, density, true, true, ground_mat);
    topBody->SetPos(ChVector<>(0, box_height / 2, 0));
    topBody->SetBodyFixed(true);
    topBody->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    topBody->SetEvalContactCn(true);
    topBody->SetEvalContactCt(true);
    topBody->SetEvalContactKf(true);
    topBody->SetEvalContactSf(true);
    sys.Add(topBody);

    auto wallBody1 = chrono_types::make_shared<ChBodyEasyBox>(
        box_thickness, box_height, box_width + box_thickness - 0.01, 1000, true, true, ground_mat);
    wallBody1->SetPos(ChVector<>(-box_width / 2, 0, 0));
    wallBody1->SetBodyFixed(true);
    wallBody1->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody1);

    auto wallBody2 = chrono_types::make_shared<ChBodyEasyBox>(
        box_thickness, box_height, box_width + box_thickness - 0.01, 1000, false, true, ground_mat);
    wallBody2->SetPos(ChVector<>(box_width / 2, 0, 0));
    wallBody2->SetBodyFixed(true);
    // wallBody2->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody2);

    auto wallBody3 = chrono_types::make_shared<ChBodyEasyBox>(box_width + box_thickness - 0.01, box_height,
                                                              box_thickness, 1000, true, true, ground_mat);
    wallBody3->SetPos(ChVector<>(0, 0, -box_width / 2));
    wallBody3->SetBodyFixed(true);
    wallBody3->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody3);

    auto wallBody4 = chrono_types::make_shared<ChBodyEasyBox>(box_width + box_thickness - 0.01, box_height,
                                                              box_thickness, 1000, true, true, ground_mat);
    wallBody4->SetPos(ChVector<>(0, 0, box_width / 2));
    wallBody4->SetBodyFixed(true);
    wallBody4->GetVisualShape(0)->SetMaterial(0, ground_mat_vis);
    sys.Add(wallBody4);
    return floorBody;
}

void parsing_inputs_from_file(double& rod_radius,
                              double& rod_density,                              
                              std::string& file_path,
                              double& friction_coefficient,
                              double& cohesion,
                              bool& visualize,
                              double& simulation_time,
                              double& time_step,
                              double& excitation_frequency,
                              double& excitation_amplitude) {
#ifdef _WIN32
    std::ifstream file("C:/Users/yjung/Documents/GitHub/chrono/build/bin/Release/inputs.txt");
#endif
#ifdef __APPLE__
    std::ifstream file("/Users/yeonsu/Documents/github/chrono/build/bin/inputs.txt");
#endif
    // std::ifstream file("./inputs.txt");
    
    std::string str;
    while (std::getline(file, str)) {
        std::istringstream iss(str);
        std::string key;
        iss >> key;
        if (key == "rod_radius") {
            iss >> rod_radius;
        } else if (key == "rod_density") {
            iss >> rod_density;
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

    // parameters
    int num_rods;
    double rod_radius;
    double alpha;
    double rod_length;
    double rod_density;
    double box_height;
    double box_width;
    double box_thickness = 1;
    double container_radius;
    double container_height;
    
    double friction_coefficient = 0.4;
    double cohesion = 0;

    std::string file_path;
    bool visualize = false;
    double simulation_time = 1;
    double time_step = 0.01;
    double excitation_frequency = 0;
    double excitation_amplitude = 0;

    parsing_inputs_from_file(rod_radius,
                            rod_density,
                            file_path,
                            friction_coefficient,
                            cohesion,
                            visualize,
                            simulation_time,
                            time_step,
                            excitation_frequency,
                            excitation_amplitude);

    // void load_rods_from_file(ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double
    // rod_density, double box_height, double box_width, double box_thickness) {

    // Create a ChronoENGINE physical system
    ChSystemNSC sys;

    
    // TO DO: consider using a class to pass geometric, mechanical parameters - must I?    

    // parse input for rods
    load_rods_from_file(sys,
                        file_path,                        
                        friction_coefficient,
                        cohesion,
                        num_rods,
                        rod_radius,
                        rod_length,
                        rod_density,
                        box_width,
                        box_height,
                        container_radius,
                        container_height,                        
                        alpha);

    // box_height = box_width;
    // box_width = rod_length*3;
    box_width = box_height;    

    std::cout << "rod_radius: " << rod_radius << std::endl;
    std::cout << "rod_density: " << rod_density << std::endl;    
    std::cout << "file_path: " << file_path << std::endl;
    std::cout << "friction_coefficient: " << friction_coefficient << std::endl;
    std::cout << "cohesion: " << cohesion << std::endl;
    std::cout << "visualize: " << visualize << std::endl;
    std::cout << "simulation_time: " << simulation_time << std::endl;
    std::cout << "time_step: " << time_step << std::endl;
    std::cout << "excitation_frequency: " << excitation_frequency << std::endl;
    std::cout << "excitation_amplitude: " << excitation_amplitude << std::endl;

    // Create all the rigid bodies.    
    std::shared_ptr<chrono::ChBody> floorBody;
    floorBody = create_walls(sys,
                            box_width,
                            box_height,
                            box_thickness,
                            rod_density,
                            friction_coefficient,
                            cohesion);

    // (ChSystemNSC& sys, std::string file_path, double rod_radius, double rod_length, double rod_density, double
    // box_height, double box_width, double box_thickness) Create the Irrlicht visualization system
    
    auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
    if (visualize) {        
        ChVector<> camera_position(0, 0, 1.2 * box_height);

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

    class ContactReporter : public ChContactContainer::ReportContactCallback {
      public:
        // ContactReporter() {}        
        // ContactReporter(std::ofstream& contact_outfile) { _contact_outfile = contact_outfile; }
        ContactReporter(std::ofstream& contact_outfile) : _contact_outfile(contact_outfile) {}

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
            
            // write to file
            _contact_outfile << pA.x() << " " << pA.y() << " " << pA.z() << " " << pB.x() << " " << pB.y() << " " << pB.z()
                    << " " << plane_coord(0, 0) << " " << plane_coord(0, 1) << " " << plane_coord(0, 2) << " "
                    << plane_coord(1, 0) << " " << plane_coord(1, 1) << " " << plane_coord(1, 2) << " "
                    << plane_coord(2, 0) << " " << plane_coord(2, 1) << " " << plane_coord(2, 2) << " " << distance
                    << " " << eff_radius << " " << cforce.x() << " " << cforce.y() << " " << cforce.z() << " "
                    << ctorque.x() << " " << ctorque.y() << " " << ctorque.z() << " "
                    << modA->GetPhysicsItem()->GetIdentifier() << " " << modB->GetPhysicsItem()->GetIdentifier()
                    << '\n';
            return true;
        }        
        std::ofstream& _contact_outfile;
    };

#ifdef _WIN32
    size_t found = file_path.find_last_of("/\\");
#endif

#ifdef __APPLE__
    size_t found = file_path.find_last_of("/");
#endif

    std::string file_name = file_path.substr(found + 1);
    std::string file_name_no_ext = file_name.substr(0, file_name.find_last_of("."));

    std::string tStepString = std::to_string(time_step * 1000);
    size_t dotPos = tStepString.find('.');
    if (dotPos != std::string::npos) {
        tStepString = tStepString.substr(0, dotPos + 3);
    }

    std::string simTimeString = std::to_string(time_step * 1000);
    dotPos = simTimeString.find('.');
    if (dotPos != std::string::npos) {
        simTimeString = simTimeString.substr(0, dotPos + 3);
    }

    std::string alphaString = std::to_string(alpha);
    dotPos = alphaString.find('.');
    if (dotPos != std::string::npos) {
        alphaString = alphaString.substr(0, dotPos + 2);
    }

#ifdef _WIN32
    std::string out_dir = "C:/Users/yjung/Dropbox (Harvard University)/Entangled/Sims/";
#endif
#ifdef __APPLE__
    std::string out_dir = "/Users/yeonsu/Dropbox (Harvard University)/Entangled/Sims/";
#endif

    std::string dirName =
        "alpha" + alphaString + "_" + file_name_no_ext + "_" + "tstep_" + tStepString + "simtime_" + simTimeString;
    std::string newDirPath = createNumberedDirectory(out_dir, dirName);
    
    std::ofstream fout;
    std::string fileName = newDirPath + "/metadata.txt";
    fout.open(fileName);
    fout << "alpha " << alpha << '\n';
    fout << "rod_radius " << rod_radius << '\n';
    fout << "rod_length " << rod_length << '\n';
    fout << "rod_density " << rod_density << '\n';
    fout << "box_width " << box_width << '\n';
    fout << "box_height " << box_height << '\n';
    fout << "friction_coefficient " << friction_coefficient << '\n';
    fout << "cohesion " << cohesion << '\n';
    fout << "time_step " << time_step << '\n';
    fout << "simulation_time " << simulation_time << '\n';
    fout << "file_name " << file_name << '\n';
    fout.close();

#ifdef _WIN32
    copy_file("C:/Users/yjung/Documents/GitHub/chrono/build/bin/Release/inputs.txt", newDirPath + "/inputs.txt");
#endif
#ifdef __APPLE__
    copy_file("/Users/yeonsu/Documents/github/chrono/build/bin/inputs.txt", newDirPath + "/inputs.txt");
#endif
    
    std::ofstream out_file;
    out_file.open(newDirPath + "/sim_data.txt");

    std::ofstream contact_outfile;
    contact_outfile.open(newDirPath + "/contacts.txt");

    auto creporter = chrono_types::make_shared<ContactReporter>(contact_outfile);

    GetLog() << "Start simulation" << '\n';
    GetLog() << "Number of bodies: " << sys.GetNbodies() << '\n';
    GetLog() << "Number of bodies: " << sys.Get_bodylist().size() << '\n';
    
    // Simulation loop
    int frame = 0;
    const int FLUSH_INTERVAL = 1000;

    if (visualize) {
        while (vis->Run()) {
            vis->BeginScene();
            vis->Render();
            vis->EndScene();
            
            sys.Set_G_acc(
                ChVector<>(0, -9.8 + excitation_amplitude * cos(CH_C_2PI * excitation_frequency * sys.GetChTime()), 0));
            
            std::cout << "\r"
                      << "time: " << sys.GetChTime() << '\t' << "Number of contacts: " << sys.GetNcontacts();

            frame++;
            sys.DoStepDynamics(time_step);
        }
    } else {  // no visualization
        while (sys.GetChTime() < simulation_time) {
            // TO DO: write down callback function to get the position of the cylinder

            GetLog() << '\r' << "time: " << sys.GetChTime() << "\t"
                     << "Number of contacts: " << sys.GetNcontacts();

            sys.Set_G_acc(
                ChVector<>(0,
                            -9.8 + excitation_amplitude * cos(CH_C_2PI * excitation_frequency * sys.GetChTime()),
                            0));
            
                
            out_file << "ITEM: TIMESTEP\n" << sys.GetChTime() << "\n";
            out_file << "ITEM: NUMBER OF ATOMS\n" << sys.Get_bodylist().size() << "\n";  // is this necessary?            
            out_file << "ITEM: ATOMS id type x y z vx vy vz u1 u2 u3 u4 w1 w2 w3 fx fy fz tx ty tz\n";            
            for (int i = 0; i < sys.Get_bodylist().size(); i++) {
                out_file << sys.Get_bodylist()[i]->GetIdentifier() << " 1 "
                         << sys.Get_bodylist()[i]->GetPos().x() << " " << sys.Get_bodylist()[i]->GetPos().y() << " "
                         << sys.Get_bodylist()[i]->GetPos().z() << " " << sys.Get_bodylist()[i]->GetPos_dt().x() << " "
                         << sys.Get_bodylist()[i]->GetPos_dt().y() << " " << sys.Get_bodylist()[i]->GetPos_dt().z() << " "
                         << sys.Get_bodylist()[i]->GetRot().e0() << " " << sys.Get_bodylist()[i]->GetRot().e1() << " "
                         << sys.Get_bodylist()[i]->GetRot().e2() << " " << sys.Get_bodylist()[i]->GetRot().e3() << " "
                         << sys.Get_bodylist()[i]->GetWvel_loc().x() << " " << sys.Get_bodylist()[i]->GetWvel_loc().y()
                         << " " << sys.Get_bodylist()[i]->GetWvel_loc().z() << " "
                         << sys.Get_bodylist()[i]->GetContactForce().x() << " "
                         << sys.Get_bodylist()[i]->GetContactForce().y() << " "
                         << sys.Get_bodylist()[i]->GetContactForce().z() << " "
                         << sys.Get_bodylist()[i]->GetContactTorque().x() << " "
                         << sys.Get_bodylist()[i]->GetContactTorque().y() << " "
                         << sys.Get_bodylist()[i]->GetContactTorque().z() << "\n";
                // id = sys.Get_bodylist()[i]->GetIdentifier();
                // pos = sys.Get_bodylist()[i]->GetPos();
                // vpos = sys.Get_bodylist()[i]->GetPos_dt();
                // rot = sys.Get_bodylist()[i]->GetRot();
                // vrot = sys.Get_bodylist()[i]->GetWvel_loc();
                // contact_force = sys.Get_bodylist()[i]->GetContactForce();
                // contact_torque = sys.Get_bodylist()[i]->GetContactTorque();
                // out_file << id << " 1 "<< pos[0] << " " << pos[1] << " " << pos[2] << " " << vpos[0] << " " << vpos[1]
                //          << " " << vpos[2] << " " << rot.e0() << " " << rot.e1() << " " << rot.e2() << " " << rot.e3()
                //          << " " << vrot[0] << " " << vrot[1] << " " << vrot[2] << " " << contact_force[0] << " "
                //          << contact_force[1] << " " << contact_force[2] << " " << contact_torque[0] << " "
                //          << contact_torque[1] << " " << contact_torque[2] << "\n";
            }

            contact_outfile << "ITEM: TIMESTEP\n" << sys.GetChTime() << "\n";
            contact_outfile << "ITEM: NUMBER OF CONTACTS\n" << sys.GetNcontacts() << "\n";  // is this necessary?
            contact_outfile << "pA.x pA.y pA.z pB.x pB.y pB.z pc00 pc01 pc02 pc10 pc11 pc12 pc20 pc21 pc22 distance "
                            "eff_radius cfx cfy cfz ctau_x ctau_y ctau_z\n";
            sys.GetContactContainer()->ReportAllContacts(creporter);

            sys.DoStepDynamics(time_step);

            frame++;
            if (frame % FLUSH_INTERVAL == 0) {
                out_file.flush();
                contact_outfile.flush();
            }
        }
    }
    out_file.close();
    contact_outfile.close();

    return 0;
}
