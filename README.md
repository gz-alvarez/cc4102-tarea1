# Tarea 1 
## Integrantes
- [Gonzalo Álvarez]
- [Diego Espinoza E.]
- [Camila Ramírez B.]

## Instrucciones:
La experimentación se realizó en un sistema operativo Linux.

## Instrucciones de ejecución
El archivo main.cpp viene con un main comentado, donde se realizan los experimentos para Quicksort, Mergesort y el cálculo de la aridad. 
Para ver el cálculo de la aridad se tendría que descomentar unas líneas en el main.
Para compilarlo, se ejecutó el siguiente comando en la terminal:
```
$ g++ -std=c++23 -g -I./include -o main src/main.cpp src/fs_block.cpp src/quicksort_disk.cpp src/mergesort_disk.cpp
```

Para ejecutarlo, se usó el siguiente comando en la terminal:
```
$ ./main
```

## Estructura del proyecto
El proyecto se compone principalmente de estos 3 archivos:
- **main.cpp**: Contiene el código con los experimentos y el main.
- **quicksort_disk.cpp**: Contiene el algoritmo de Quicksort externo.
- **mergesort_disk.cpp**: Contiene el algoritmo de Mergesort externo.

Además de otros archivos con headears en la carpeta include.