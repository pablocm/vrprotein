VrProtein
=========

VrProtein es un software para la visualización e interacción de moléculas
pensado para ambiente virtuales inmersivos.

# Requisitos
* Alguna distribución de Linux (Testeado en Ubuntu 13.10)
* Tarjeta de video y drivers OpenGL decentes para gráficos 3D
* Vrui 3.0-002 (incluido en la instrucciones de instalación)
* g++ 4.7 o superior (para compilar C++11)

# Instalación
Antes de instalar VrProtein es necesario instalar 
[Vrui 3.0-002](http://idav.ucdavis.edu/~okreylos/ResDev/Vrui). En el sitio 
oficial están las instrucciones detalladas para su instalación. La forma 
rápida de instalarlo es con el script de instalación (incluido):
```
# Compilar Vrui
$ bash Build-Ubuntu.sh
# Compilar Vrprotein
$ make
``` 

# Uso
Antes de abrir la aplicación es necesario bajar los archivos de moléculas 
PDB (no incluidos en el repositorio git) y copiarlos a la carpeta 
```./datasets```. Estos se pueden descargar aquí: 
https://www.dropbox.com/s/ijh3ob4fc49i05u/datasets.zip

La aplicación se ejecuta con ```bin/vrprotein```. Por default la aplicación 
usará solo mouse y teclado. Para usar dispositivos extras (Rift, Hydra, 
Wiimotes, etc.), es necesario ejecutar en otra terminal la aplicación 
```VRDeviceDaemon```. Más información [aquí](http://doc-ok.org/?p=639) 
y [aquí](http://idav.ucdavis.edu/~okreylos/ResDev/LowCostVR/index.html).
