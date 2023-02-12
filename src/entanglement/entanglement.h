#ifndef MY_HEADER_FILE
#define MY_HEADER_FILE

#include "chrono/assets/ChTexture.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkMotorRotationSpeed.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"


std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys);


#endif