<h1>Tools and Middleware asssignment: Bridges</h1>

<h2>Bullet constraints</h2>
In order for the constraints to be processed, they first have to be added to the DynamicWorld object in the visual_scene.

<h2>Hinge constraints</h2>
The first type of bridge that appears in the game is made of planks connected with hinge constraints. These constraints allow movement around an axis, so the movement of the planks is rather limited.

Firstly, planks are layed out along the X axis. Secondly, the constraints are added in the edges of the planks with the Z axis as the rotation axis, between every pair of consecutive planks.

Curiously, when planks are connected this way, if the bridge is big enough, the weight of all the planks will make the bridge bend, even if the amount of planks is just enough to fill the length of the bridge. That is why the parameter *__curvature_factor__* is introduced, which simply makes the bridge be constructed with less (or more) amount of planks, which results and the bridge being more tense (or loose).

Finally, it has been observed that if the weight of the planks is big enough that the centre of the bridge starts sinking (or the *__curvature_factor__* is low enough), the tension in the constraints will be so big that planks would start shaking. This effect can be reduced by the parameter *__force_stabilize__*, which effectively locks the angular limits of the hinge constraints. This means that the objects affected by the constraints cannot rotate, thus the planks stop shaking.

<h2>Spring constraints</h2>
The second type of bridge is made of planks connected with spring constraints. These constraints allow movement along an axis, and when the spring is enabled in that axis, the objects affected are moved towards the equilibrium point of the spring. Furthermore, the same constraint can set springs in each axis as well as rotation axis.

Firstly, planks are again layed out along the X axis. Secondly, the constraints are added in the edges of the planks between every pair of consecutive planks. In order to have the planks be stable and not rotate uncontrollably, two spring constraints are created between the two planks, one at each side of the bridge, which resembles much more realistically the behaviour and physics of a real bridge.

Springs can be given a lower and an upper limit, which controls how near or far the objects can be relative to each other. In this case, the lower limit is zero, since the planks can be pulled until they are touching. The upper limit can be specified at a desired value, which will control the maximum distance the planks can be from each other. The equilibrium point in all cases is set to zero, which will effectively make the springs try to pull the planks together as much as possible.

Several parameters can be determined:
- *__curvature_factor__* (float). Similar to hinge constraints, this will control the amount of planks that are created, resulting in how tense or loose the bridge is in its equilibrium point.
- *__stiffness__* (float). This parameters controls how prone to be impacted by external forces the spring constraints are, making the bridge firmer or more elastic. The problem comes when the bridge is too big: in this case, if the stiffness is too low, the bridge will sink as much as the upper limit of the springs allow it, making the stiffness effectively useless.
- *__damping__* (float). This parameter controls the back and forth movement of the springs, which results on the bounciness of the bridge.
- *__force_stabilize__* (bool). Similar to hinge constraints, this parameter is introduced for edge cases like the bridge being to long or tense, which will make the planks shake. However, in contrast to hinges, this parameter sets all 6 axis of the spring active (all axis and rotation axis have springs enabled). This results in all axis of the spring pulling towards the equilibrium point, so planks tend to shake less. This is less effective than locking the rotation of the planks, but allows for a more realistic behaviour.

<h2>Collision callbacks</h2>
In order to demonstrate collision callbacks, the idea was to fire spheres that change color on impact towards the bridges, allowing also this way to better observe the behaviour of the physics of the constraints in the bridge. Hence, the first task has been to enable to use the keyboard and mouse to freely move the camera in 3D space.

When the player fires, a new sphere is spawned in the position of the camera, and a central force is applied to it with the direction the camera is facing.

Collision callbacks are basically functions than can be specified to be called whenever a rigidbody starts, stays or stops colliding with another body. In order to pass information outside of the Bullet Physics library to the callback function, user pointers can be specified. This way, the mesh of the sphere can be accessed from the callback function in order to change the color on impact.

However, there has been a problem with the use of user pointers in the assignment. Whenever a user pointer is set, fired balls stop updating its render (they would appear to stay in their initial position, but their position is actually being updated, as they would collide with the bridges and make them move). Therefore, in order to let the game work or not crash, these feature has been disabled (although the code still exists commented).
