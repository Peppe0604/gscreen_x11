#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <png.h>

#define ERROR -1
#define NUMBER_OF_COLORS 3         // RGB (Red, Green, Blue)
#define BPC 8
#define DEFAULT_NAME "screen.png"
#define NAME_FLAG "-n"
#define OFFSET_RED 16
#define OFFSET_GREEN 8
#define OFFSET_BLUE 0

// Macro per estrarre e scalare un singolo colore
// - pixel: il blocco a 32 bit letto dalla memoria
// - offset: da dove inizia il colore (letto da env.vinfo)
#define EXTRACT(pixel, offset) (((pixel) >> (offset)) & 0xFF)

XImage *captureScreen(Display **display){
    *display = XOpenDisplay(NULL);
    if (*display == NULL) {
        perror("Errore: Impossibile aprire la connessione con X11");
        return NULL;
    }

    Window root = DefaultRootWindow(*display);

    XWindowAttributes attr;
    XGetWindowAttributes(*display, root, &attr);

    XImage *image = XGetImage(*display, root, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);

    if (image == NULL) {
        perror("Errore: Impossibile catturare l'immagine da X11");
        XCloseDisplay(*display);
        return NULL;
    }

    return image;
}


uint8_t *convertX11(XImage *image){
    size_t totalPixel = image->width * image->height;

    uint8_t *newImage = malloc(totalPixel * NUMBER_OF_COLORS);
    if (newImage == NULL){
        perror("Errore surante l'allocazione dell'immagine converita");
        return NULL;
    }

    uint32_t *oldImage = (uint32_t *)image->data;
    for (size_t i = 0; i < totalPixel; i++){
        uint32_t pixel = oldImage[i];

        newImage[i * NUMBER_OF_COLORS] = EXTRACT(pixel, OFFSET_RED);
        newImage[i * NUMBER_OF_COLORS + 1] = EXTRACT(pixel, OFFSET_GREEN);
        newImage[i * NUMBER_OF_COLORS + 2] = EXTRACT(pixel, OFFSET_BLUE);
    }

    return newImage;
}

int savePng(uint8_t *fb, int width, int height, const char *name){
    int status = ERROR;

    FILE *fp = fopen(name, "wb");
    if (fp == NULL){
        perror("Errore durante l'allocazione del puntatore al file");
        return ERROR;
    }
    png_bytep *rowPointers = NULL; 

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png){
        perror("Errore durante la creazione della png_struct");
        goto close;
    }

    png_infop info = png_create_info_struct(png);
    if (!info){
        perror("Errore durante la creazione della png_info");
        png_destroy_write_struct(&png, NULL);
        goto close;
    }

    if (setjmp(png_jmpbuf(png))){
        perror("Errore interno a libpng durante la scrittura");
        goto free;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height,
            BPC, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    rowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    if (!rowPointers) {
        perror("Errore di allocazione per i row pointers");
        goto free;
    }
    for (int y = 0; y < height; y++) {
        rowPointers[y] = (png_bytep)(fb + (y * width * NUMBER_OF_COLORS));
    }

    png_write_image(png, rowPointers);
    png_write_end(png, NULL);

    status = 0;

free:
    if (rowPointers != NULL) free(rowPointers);
destroy:
    png_destroy_write_struct(&png, &info);
close:
    fclose(fp);
    return status; 
}

int main(int argc, char **argv){
    char *name = DEFAULT_NAME;
    if (argc == 3 && strcmp(argv[1], NAME_FLAG) == 0){
        name = argv[2];
    } else if (argc > 1) {
        printf("Formattazione non valida:\n-> ./gscreen\n-> ./gscreen -n <NAME>.png\n");
        return ERROR;
    }

    Display *display = NULL;
    XImage *image = captureScreen(&display);

    if (image == NULL) {
        printf("Impossibile procedere: errore nella cattura dello schermo.\n");
        return ERROR;
    }

    uint8_t *newFb = convertX11(image);

    if (newFb != NULL) {
        (savePng(newFb, image->width, image->height, name) == ERROR) ? 
        printf("Errore durante il salvataggio dello screen.\n") : printf("Screenshot salvato con successo!\n");

        free(newFb);
    }

    XDestroyImage(image);
    XCloseDisplay(display);

    return 0;
}
