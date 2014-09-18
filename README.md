ThreadedPhysics
===============

Threaded physics library with cross-platform vector classes. 
Mostly an academic project to become more familiar with C++11 and different vector intrinsics - and Git.

Currently working:

- FPU and SSE Vector4 class, with Vector3 support methods (dot3, length3Squared, length3, cross)
- FPU and SSE Matrix4 class
- Double-buffered physics state for threaded velocity and position updates, with a locking getter to copy the current state for rendering or other uses
- Job-based collision detection with arbitrary number of worker threads (mostly lock-free)
- Job-based velocity integration with arbitrary number of worker threads (lock-free)
- Windows test app
- Sphere primitives
- Forward Euler integration
- Collision resolution for spheres
- 
To do:

Near term:
- Acceleration/impulse integration

Mid term:
- Simple renderer and visual test app
- Non-sphere primitives (cylinder, plane, point (?) )
- Rigid body physics
- Better integrators (Verlet, RK4)

Long term:
- Swept shape collision (will probably require some rethinking of order of operations)
- Arbitrary (skinned?) mesh collision
- Constraints
- Platform-agnostic renderer and test app
