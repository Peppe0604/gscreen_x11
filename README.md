# gscreen_x11

Un programma scritto interamente in C per catturare screenshot in ambienti basati su server grafico X11. 
Perfetto per chi utilizza Window Manager leggeri (come dwm, i3, bspwm) e cerca un tool minimale, veloce e senza dipendenze pesanti (come Qt o GTK). 

Questo progetto nasce come evoluzione naturale del mio tool `gscreen` (presente sul mio GitHub). Inizialmente avevo scritto `gscreen` per fare screenshot direttamente dal Framebuffer puro (`/dev/fb0`) del mio Raspberry PI5, ma mi sono reso conto che non era in grado di catturare le finestre e l'interfaccia grafica gestita dal server Xorg e da dwm. Di conseguenza, ho creato `gscreen_x11` per risolvere esattamente questo problema, interfacciandomi direttamente con il server grafico.
Credo ci siano un sacco di cose da migliorare, ma con il tempo magari si farà.

## Come funziona
Il programma si connette al server X11 tramite la libreria `libX11` per catturare la "Root Window" (l'intero desktop) tramite la funzione `XGetImage`. Successivamente, processa la matrice di pixel a 32-bit (estraendo i canali e convertendo il formato nativo BGRA di Xorg in RGB puro a 24-bit), e infine la impacchetta in un file immagine compresso utilizzando `libpng`.

## Dipendenze
Per compilare questo tool, hai bisogno dei file di sviluppo di `libpng` e `libX11`:
* Debian/Ubuntu/Raspberry PI OS: `sudo apt install libpng-dev libx11-dev`
* Arch Linux: `sudo pacman -S libpng libx11`

## Compilazione
Clona la repository e compila il sorgente linkando le librerie necessarie (X11 e PNG):
```bash
gcc gscreen_x11.c -o gscreen_x11 -lX11 -lpng
```
## Utilizzo
Avvia il programma dal terminale o, ancora meglio, associalo a una scorciatoia da tastiera nel tuo gestore di finestre (es. nel config.h di dwm).
Salva l'immagine con il nome di default (screen.png) nella directory corrente:
```bash
./gscreen_x11
```

Salva l'immagine con un nome o un percorso personalizzato:
```bash
./gscreen_x11 -n mioscreen.png
```
