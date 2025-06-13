/**
 * @file main_layer.hpp
 * @brief Declares the MainLayer class, which represents the main application layer.
 */

#pragma once

/**
 * @class MainLayer
 * @brief Represents the main application layer.
 *
 * This layer is responsible for handling application-level events.
 */
class MainLayer : public zth::Layer
{
public:
    /**
     * @brief Constructs a MainLayer object.
     */
    MainLayer() = default;

    /**
     * @brief Deleted copy constructor and move constructor.
     */
    ZTH_NO_COPY_NO_MOVE(MainLayer)

    /**
     * @brief Destructor for MainLayer.
     */
    ~MainLayer() override = default;

public:
    /**
     * @brief Handles events for the main layer.
     * @param event The event to handle.
     */
    auto on_event(const zth::Event& event) -> void override;
};
