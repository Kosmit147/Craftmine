/**
 * @file application.hpp
 * @brief Declares the Application class, the main entry point for the Craftmine application.
 */

#pragma once

/**
 * @class Application
 * @brief The main application class for Craftmine.
 *
 * Inherits from zth::Application and is responsible for initializing and running the application.
 */
class Application : public zth::Application
{
public:
    /**
     * @brief Constructs the Application object.
     *
     * Initializes the application with the required specification.
     */
    explicit Application();
};
