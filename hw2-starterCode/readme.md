CS420_Roller Coaster Project: Space Coaster 

System: Ubuntu 16.04, GTX1080
Usage: ./hw1 ../track_compelete.txt
Texture: Sky texture: space; ground texture: the earth

Extras

*Render a T-shaped rail cross-section. 
-Y

*Render double rail (like in real railroad tracks).
-Y

*Make your track circular and close it with C1 continuity (small credit, i.e., 1 point). 
-N

*Add OpenGL lighting to make your coaster look more realistic. 
-N

*Draw additional scene elements: texture-mapped wooden crossbars, a support structure that looks realistic, decorations on your track, something interesting in the world nearby, etc. 
-N

*Create tracks that mimic real world roller coasters such as Magic Mountain (close to Los Angeles; check out the Green Lantern in the video), or Gardaland in Italy. 
-N

*Generate your track from several different sequences of splines (multiple track files), so you can shuffle these around and create a random track. 
-Y
-use command: $,./hw1 ../track_random.txt
-explaination: after finishing one spline, it will begin the next random spline in the same direction, but due to the different tangent, the shift still seems strange. (not sure if it meet the requirement)

*Draw splines using recursive subdivision (vary step size to draw short lines) instead of using brute force (vary u with fixed step size). 
-Y
-In line 124, let moveMode = SUBDIVIDE

*Render the environment in a better (prettier, more efficient, more interesting, etc?) manner than described here. 
-Y
--use GL_TEXTURE_CUBE_MAP instead of the GL_TEXTURE_2D to draw a skybox

*Determine coaster normals in a better manner than described here.
-N

*Modify the velocity with which your camera moves to make it physically realistic in terms of gravity. Please see the equation on how to update u for every time step.
*Derive the steps that lead to the physically realistic equation of updating the u (i.e. u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du)), see the doc linked above in the previous item).
-Y
-In line 124, let moveMode = REAL ( the default is real)