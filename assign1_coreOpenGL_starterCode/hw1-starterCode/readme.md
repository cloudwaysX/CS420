System Info:
Ubuntu 16.04
GTX 1080
Able to render 786 gray scale image

default usage:
>make
>./hw1 <heightmapfilename> #file must in gray scale

press space to change the render mode
to make animation, uncomment the code in IdleFunction

Extra Credit:
1\ Use element arrays and glDrawElements.

2\ Use GL_TRIANGLE_STRIP

3\Support color (ImageIO::getBytesPerPixel == 3) in input images.
Usage: ./hw1 <heightmapfilename> 
#if file is in RGB scale then will automatically render the heightmap with color

4\Color the vertices based on color values taken from another image of equal size. However, your code should still also support smooth gradients as per the core requirements.
Usage: ./hw1 <heightmapfilename> <colormapfilename> 
#regardless of the color of heightmap is RGB or gray scale, they will always use the color in color map
#will exit with error if heighmap and colormap are not in the same size