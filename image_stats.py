#!/usr/bin/env python3
# image_stats.py

import sys
import cv2
import numpy as np
import matplotlib.pyplot as plt
from collections import Counter

# Para el popup y para incrustar el boxplot
import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

def calcular_estadisticas(imagen_gray):
    """
    Recibe una imagen en escala de grises (2D numpy array).
    Devuelve diccionario con media, mediana, moda, varianza y desviación estándar.
    """
    pixeles = imagen_gray.flatten()
    
    media = float(np.mean(pixeles))
    mediana = float(np.median(pixeles))
    varianza = float(np.var(pixeles))
    desviacion = float(np.std(pixeles))
    
    # Para la moda, usamos Counter de la stdlib
    conteos = Counter(pixeles.tolist())
    # Counter.most_common(1) devuelve [(valor_moda, cuenta)], si hay empate, devuelve alguno.
    modo_val, modo_cuenta = conteos.most_common(1)[0]
    moda = float(modo_val)
    
    return {
        'media': media,
        'mediana': mediana,
        'moda': moda,
        'varianza': varianza,
        'desviacion': desviacion
    }

def crear_popup_con_estadisticas(path_imagen):
    """
    Carga la imagen en escala de grises, calcula estadísticas y muestra un
    popup en Tkinter con los valores y un boxplot incrustado.
    """
    # 1) Intentar leer la imagen en modo gray
    imagen_gray = cv2.imread(path_imagen, cv2.IMREAD_GRAYSCALE)
    if imagen_gray is None:
        # Si no se pudo leer, abrimos un popup de error y salimos
        root = tk.Tk()
        root.withdraw()
        tk.messagebox.showerror("Error", f"No se pudo cargar la imagen:\n{path_imagen}")
        sys.exit(1)
    
    # 2) Calcular estadísticas
    stats = calcular_estadisticas(imagen_gray)
    
    # 3) Crear la ventana principal de Tkinter
    root = tk.Tk()
    root.title("Estadísticas de la imagen")
    
    # Hacemos la ventana un poco más grande para que quepa el gráfico
    root.geometry("600x700")
    
    # Crear un Frame para las etiquetas de texto (stats)
    frame_text = ttk.Frame(root, padding=(10,10))
    frame_text.pack(side=tk.TOP, fill=tk.X)
    
    # Etiquetas con cada estadística
    ttk.Label(frame_text, text=f"Ruta de la imagen: {path_imagen}", wraplength=580).pack(anchor=tk.W, pady=(0,5))
    ttk.Label(frame_text, text=f"Media: {stats['media']:.2f}").pack(anchor=tk.W)
    ttk.Label(frame_text, text=f"Mediana: {stats['mediana']:.2f}").pack(anchor=tk.W)
    ttk.Label(frame_text, text=f"Moda: {stats['moda']:.0f}").pack(anchor=tk.W)
    ttk.Label(frame_text, text=f"Varianza: {stats['varianza']:.2f}").pack(anchor=tk.W)
    ttk.Label(frame_text, text=f"Desviación estándar: {stats['desviacion']:.2f}").pack(anchor=tk.W)
    
    # Separador
    ttk.Separator(root, orient=tk.HORIZONTAL).pack(fill=tk.X, pady=10)
    
    # 4) Crear figura de matplotlib para el boxplot
    fig, ax = plt.subplots(figsize=(5.5, 4))  # tamaño en pulgadas
    
    # Dibujar boxplot
    ax.boxplot(imagen_gray.flatten(), vert=True)
    ax.set_title("Boxplot de valores de píxel")
    ax.set_ylabel("Intensidad (0-255)")
    
    # 5) Incrustar la figura en un Canvas de Tkinter
    canvas = FigureCanvasTkAgg(fig, master=root)
    canvas.draw()
    canvas_widget = canvas.get_tk_widget()
    canvas_widget.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
    
    # 6) Botón para cerrar
    btn_close = ttk.Button(root, text="Cerrar", command=root.destroy)
    btn_close.pack(pady=(10, 10))
    
    # Iniciar bucle de eventos
    root.mainloop()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python3 image_stats.py <ruta_imagen_gris.png>")
        sys.exit(1)
    
    ruta_imagen = sys.argv[1]
    crear_popup_con_estadisticas(ruta_imagen)
