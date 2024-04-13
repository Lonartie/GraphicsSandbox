# Graphics Sandbox

- [Graphics Sandbox](#graphics-sandbox)
  - [About](#about)
  - [Installation](#installation)
  - [Features](#features)
  - [TODOs](#todos)

## About

This is a sandbox project for me to learn more about
OpenGL and graphics programming in general. The project
is written in C++ and uses Qt for the GUI. The project
is built with CMake and uses vcpkg for package management.

## Installation

Using vcpkg you have to install `qtbase` and `assimp` and for windows
you need `qttools` as well. Once you have installed the
required packages you can build the project with cmake.

## Features

- [x] Basic OpenGL rendering
- [x] 3D scene with camera controls
- [x] Rendering objects with solid colors
- [x] Rendering albedo images
- [x] Normal maps
- [x] Custom model loading (using assimp)

## TODOs

- [ ] Lightning and shadows
- [ ] Adding specular maps
- [ ] Implement shininess and roughness
- [ ] Implement cube maps / skyboxes
- [ ] Adding gizmos for invisible objects
- [ ] Implement picking and transformation controls
- [ ] Implement ray tracing / path tracing
- [ ] Implement SSAO

