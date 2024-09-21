// x11 version of pngshot.c
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char *argv[]) {
    Display *display;
    Window root;
    XWindowAttributes attributes;
    XImage *image;
    int width, height;
    unsigned char *pixels;
    int x, y;
    unsigned long pixel;
    XColor colors[256];
    Colormap colormap;
    int i;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <output_filename.png>\n", argv[0]);
        return 1;
    }
    filename = argv[1];

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    root = DefaultRootWindow(display);

    if (!XGetWindowAttributes(display, root, &attributes)) {
        fprintf(stderr, "Failed to get window attributes\n");
        XCloseDisplay(display);
        return 1;
    }

    width = attributes.width;
    height = attributes.height;

    image = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    if (image == NULL) {
        fprintf(stderr, "Failed to get image\n");
        XCloseDisplay(display);
        return 1;
    }

    printf("Detected color depth: %d\n", image->depth);

    if (image->depth != 8) {
        fprintf(stderr, "This program is optimized for 8-bit color depth. Current depth: %d\n", image->depth);
        XDestroyImage(image);
        XCloseDisplay(display);
        return 1;
    }

    colormap = DefaultColormap(display, DefaultScreen(display));
    for (i = 0; i < 256; i++) {
        colors[i].pixel = i;
        colors[i].flags = DoRed | DoGreen | DoBlue;
    }
    XQueryColors(display, colormap, colors, 256);

    pixels = (unsigned char *)malloc(width * height * 3);
    if (pixels == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        XDestroyImage(image);
        XCloseDisplay(display);
        return 1;
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pixel = XGetPixel(image, x, y);
            pixels[(y * width + x) * 3 + 0] = colors[pixel].red >> 8;
            pixels[(y * width + x) * 3 + 1] = colors[pixel].green >> 8;
            pixels[(y * width + x) * 3 + 2] = colors[pixel].blue >> 8;
        }
    }

    if (!stbi_write_png(filename, width, height, 3, pixels, width * 3)) {
        fprintf(stderr, "Failed to write PNG to %s\n", filename);
    } else {
        printf("Screenshot saved as %s\n", filename);
        printf("Image dimensions: %d x %d x %d\n", width, height, image->depth);
    }

    free(pixels);
    XDestroyImage(image);
    XCloseDisplay(display);

    return 0;
}

