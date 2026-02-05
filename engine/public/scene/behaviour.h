#pragma once

#include "game_object.h"

namespace golias {

    /**
    * @brief Base class for gameplay behaviours.
    */
    class Behaviour : public GameObject {
    public:
        virtual ~Behaviour() = default;

        /// Called when the object is created
        virtual void Awake();

        /// Called once before the first Update
        virtual void Start();

        /**
     * @brief Called every frame
     * @param deltaTime Time since last frame (seconds)
     */
        virtual void Update(float deltaTime);

        /// Called before the object is destroyed
        virtual void OnDestroy();

        /// Destroys this behaviour
        void Destroy();

    protected:
        Behaviour() = default;
    };


} // namespace golias
