#ifndef MY_HEADER_FILE
#define MY_HEADER_FILE

#include "chrono/assets/ChTexture.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkMotorRotationSpeed.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"


std::shared_ptr<ChBody> test_with_single_cylinder(ChSystemNSC& sys);

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


#endif