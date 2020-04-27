#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Komplexe Zahlen zur Wiederverwendung auf Heap ablegen
static struct Complex Z;
static struct Complex c0;

// Komplexe Zahl
struct Complex {
    double re;
    double im;
};

// Betrag einer Komplexen Zahl
double complexAbsolute(struct Complex *complex) {
    double reSq = complex->re * complex->re;
    double imSq = complex->im * complex->im;
    return sqrt(reSq + imSq);
}

// Eine Art constructor
struct Complex createComplex(double re, double im) {
    struct Complex complex;
    complex.re = re;
    complex.im = im;
    return complex;
}

// Komplexe Zahl quadrieren
struct Complex *squareComplex(struct Complex *complex) {
    double re = complex->re;
    double im = complex->im;
    complex->re = (re * re) - (im * im);
    complex->im = 2 * re * im;
    return complex;
}

// Eine Komplexe Zahl auf eine andere addieren
struct Complex *addComplex(struct Complex *from, struct Complex *to) {
    to->re += from->re;
    to->im += from->im;
    return to;
}

// Testen der divergenz eines Punktes in der Komplexen Ebene
// https://en.wikipedia.org/wiki/Mandelbrot_set
int mandelbrot(double re, double im, int iterations) {
    c0.re = re;
    c0.im = im;
    Z.re = 0;
    Z.im = 0;
    for (int i = 0; i < iterations; i++) {
        if (complexAbsolute(&Z) > 2.0) {
//            printf("Re: %f", Z.re);
//            printf("Im: %f", Z.im);
//            printf("\n\n");
            return i;
        }
        squareComplex(&Z);
        addComplex(&c0, &Z);
    }
    return 0;
}

// Rot/Grün und Gelb auf den gleichen Wert festlegen, um grayscale zu erzeugen
// Wert liegt zwischen 0 und 255
void applyShade(int value, int maxValue, unsigned char *rgb) {
    unsigned char rgbVal = (unsigned char) ((255.0 / (double) value) * (double) maxValue);
    for (int i = 0; i < sizeof(rgb); i++) {
        rgb[i] = rgbVal;
    }
}

// Schreibt header und pixel matrix im ppm Format
// wb -> write binary mode
// http://rosettacode.org/wiki/Bitmap/Write_a_PPM_file#C
void ppmImageFromArray(int wid, int hei, int dep, unsigned char matrix[wid][hei][dep], const char *imageName) {
    int nameLen = sizeof(&imageName);
    char fileName[nameLen + 4];
    for (int i = 0; i < nameLen; i++) {
        fileName[i] = imageName[i];
    }
    fileName[nameLen + 0] = '.';
    fileName[nameLen + 1] = 'p';
    fileName[nameLen + 2] = 'p';
    fileName[nameLen + 3] = 'm';
    FILE *file = fopen(fileName, "wb");
    // Header schreiben
    fprintf(file, "P6\n%d %d\n255\n", wid, hei);
    // Matrix schreiben
    for (int y = 0; y < wid; y++) {
        for (int x = 0; x < hei; x++) {
            fwrite(matrix[x][y], 1, 3, file);
        }
    }
    fclose(file);
}

// Eine komplexe Matrix mit linespace füllen (z.b. von [-2 > 1.8] und [-1.3 > 1.7])
void fillComplexMatrix(double fromX, double toX, double fromY, double toY, int width, int height, double matrix[width][height][2]) {
    double dx = fabs(fromX - toX) / (double) (width - 1);
    double dy = fabs(fromY - toY) / (double) (height - 1);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            matrix[x][y][0] = x * dx + fromX;
            matrix[x][y][1] = y * dy + fromY;
        }
    }
}

// Komplexe Matrix über Mandelbrot formular auf pixel array abbilden
// Für Divergenzgradient wird ein Grayscale benutzt (siehe applyShade() function)
void mapComplexMandelbrot(int wid, int hei, int dep, int iters, unsigned char imagePlain[wid][hei][dep], double complexMatrix[wid][hei][2]) {
    for (int x = 0; x < wid; x++) {
        for (int y = 0; y < hei; y++) {
            int value = mandelbrot(complexMatrix[x][y][0], complexMatrix[x][y][1], iters);
            applyShade(value, iters, imagePlain[x][y]);
        }
    }
}


int main() {
    // Realer abgebildeter Scope [min, max]
    double realScope[] = {-2.25, 0.75};
    // Imaginär abgebildeter Scope [min, max]
    double imaginaryScope[] = {-1.6, 1.6};
    // Iterationen per Punkt
    int iterations = 500;
    // Initialisiere complexe Zahl Z
    Z = createComplex(0, 0);
    // Initialisiere complexe Zahl c0
    c0 = createComplex(0, 0);
    // Name des Image files
    char filename[] = "testFile";
    // Dimensionen des Image files
    int wid = 1280, hei = 720, dep = 3;

    // Pixel matrix
    unsigned char *pixelMatrix = calloc(wid * hei * dep, sizeof(unsigned char));
    // Complexe Matrix
    double *complexMatrix = calloc(wid * hei * dep, sizeof(double));

    // Komplexe Matrix mit Linespace füllen
    fillComplexMatrix(realScope[0], realScope[1], imaginaryScope[0], imaginaryScope[1], wid, hei, complexMatrix);
    // Komplexe Matrix über Mandelbrot Formel auf Pixel matrix abbilden
    mapComplexMandelbrot(wid, hei, dep, iterations, pixelMatrix, complexMatrix);
    // Pixel Matrix im .ppm Format speichern
    ppmImageFromArray(wid, hei, dep, pixelMatrix, filename);
    return 0;
}