#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char *argv[])
{
    HDC hdcScreen, hdcMemDC;
    HBITMAP hbmScreen;
    BITMAP bmpScreen;
    BITMAPINFOHEADER bi;
    DWORD dwBmpSize;
    HANDLE hDIB;
    char *lpbitmap;
    DWORD dwWritten = 0;
    DWORD dwSizeofDIB;
    int width, height;
    unsigned char *pixels;
    char *filename;
    int x, y;

    /* Check if filename is provided */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <output_filename.png>\n", argv[0]);
        return 1;
    }
    filename = argv[1];

    /* Create a DC for the entire screen */
    hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
    hdcMemDC = CreateCompatibleDC(hdcScreen);

    if (!hdcScreen || !hdcMemDC) {
        fprintf(stderr, "Failed to create compatible DC\n");
        return 1;
    }

    width = GetDeviceCaps(hdcScreen, HORZRES);
    height = GetDeviceCaps(hdcScreen, VERTRES);

    /* Create a compatible bitmap for hdcScreen */
    hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    if (!hbmScreen) {
        fprintf(stderr, "Failed to create compatible bitmap\n");
        DeleteDC(hdcMemDC);
        DeleteDC(hdcScreen);
        return 1;
    }

    /* Select the bitmap into the compatible DC */
    SelectObject(hdcMemDC, hbmScreen);

    /* Bit block transfer into our compatible DC */
    if (!BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY)) {
        fprintf(stderr, "BitBlt failed\n");
        DeleteObject(hbmScreen);
        DeleteDC(hdcMemDC);
        DeleteDC(hdcScreen);
        return 1;
    }

    /* Get the BITMAP from the HBITMAP */
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    /* Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
       call HeapAlloc using a handle to the process's default heap. So instead of using GlobalAlloc, 
       we'll use HeapAlloc. */
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);

    /* Gets the "bits" from the bitmap and copies them into a buffer 
       which is pointed to by lpbitmap. */
    GetDIBits(hdcScreen, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    /* Allocate memory for the pixel data */
    pixels = (unsigned char *)malloc(width * height * 3);
    if (pixels == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        DeleteObject(hbmScreen);
        DeleteDC(hdcMemDC);
        DeleteDC(hdcScreen);
        return 1;
    }

    /* Convert BGR to RGB */
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int index = (height - 1 - y) * width * 3 + x * 3;
            pixels[y * width * 3 + x * 3 + 0] = lpbitmap[index + 2];  /* Red */
            pixels[y * width * 3 + x * 3 + 1] = lpbitmap[index + 1];  /* Green */
            pixels[y * width * 3 + x * 3 + 2] = lpbitmap[index + 0];  /* Blue */
        }
    }

    /* Save the image as PNG */
    if (!stbi_write_png(filename, width, height, 3, pixels, width * 3)) {
        fprintf(stderr, "Failed to write PNG to %s\n", filename);
    } else {
        printf("Screenshot saved as %s\n", filename);
        printf("Image dimensions: %d x %d\n", width, height);
    }

    /* Clean up */
    free(pixels);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    DeleteDC(hdcScreen);

    return 0;
}
