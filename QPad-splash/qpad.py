import os
import threading
import webbrowser
import subprocess
import requests
import tkinter as tk
from tkinter import ttk, messagebox
from PIL import Image, ImageTk

# — Configuración —
REPO_OWNER  = "Soyzian"
REPO_NAME   = "qpad"
CV_FILE     = "cv.txt"
MAIN_EXE    = "QPad-main.exe"
SPLASH_BMP  = "splash.bmp"

# — Util: parse semántico de versiones simples —
def parse_version(v):
    # Quita prefijos como 'v' y divide componentes numéricos
    parts = v.lstrip('v').split('.')
    return tuple(int(p) if p.isdigit() else 0 for p in parts)

def is_newer(local, remote):
    try:
        return parse_version(remote) > parse_version(local)
    except:
        return local != remote

# — Leer/localizar versiones —
def get_local_version():
    try:
        with open(CV_FILE, "r") as f:
            return f.read().strip()
    except:
        return "0.0.0"

def get_latest_version():
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/releases/latest"
    r = requests.get(url, timeout=5)
    r.raise_for_status()
    data = r.json()
    return data["tag_name"], data["html_url"]

# — Lanzar app y cerrar —
def launch_main(window):
    try:
        subprocess.Popen([os.path.join(os.getcwd(), MAIN_EXE)], shell=True)
    except:
        pass
    window.destroy()

# — Worker de comprobación de actualizaciones —
def check_updates(local_label, remote_label, progress, splash):
    local = get_local_version()
    local_label.config(text=f"Versión instalada: {local}")

    try:
        # Simular avance inicial
        for i in range(0, 60, 5):
            progress['value'] = i
            splash.update_idletasks()
            threading.Event().wait(0.05)

        latest, html_url = get_latest_version()

        # Completar barra
        for i in range(60, 101, 10):
            progress['value'] = i
            splash.update_idletasks()
            threading.Event().wait(0.02)
    except Exception as e:
        latest, html_url = None, None

    if latest:
        remote_label.config(text=f"Última versión: {latest}")
        if is_newer(local, latest):
            messagebox.showwarning(
                "Actualización disponible",
                f"Hay una nueva versión de QPad: {latest}"
            )
    else:
        remote_label.config(text="No se pudo consultar la versión remota")

    # Espera breve y lanza la app
    splash.after(800, lambda: launch_main(splash))

# — Construye y muestra el splash —
def create_splash():
    root = tk.Tk()
    root.overrideredirect(True)
    root.configure(background="#0B1F0A")

    # Tema ttk más limpio
    style = ttk.Style(root)
    style.theme_use('clam')
    # Imagen BMP
    try:
        img = Image.open(SPLASH_BMP)
        photo = ImageTk.PhotoImage(img)
        lbl = tk.Label(root, image=photo, borderwidth=0)
        lbl.image = photo
        lbl.pack(pady=(10,5))
    except:
        tk.Label(root, text="QPad", fg="white", bg="#0B1F0A",
                 font=("Verdana", 16)).pack(pady=20)

    # Labels y progressbar
    local_lbl  = ttk.Label(root, text="Versión instalada: …", background="#0B1F0A", foreground="white")
    remote_lbl = ttk.Label(root, text="Última versión: …",   background="#0B1F0A", foreground="white")
    local_lbl.pack(pady=(0,2))
    remote_lbl.pack(pady=(0,5))

    progress = ttk.Progressbar(root, mode="determinate", length=250)
    progress.pack(pady=(0,10))

    # Centrar ventana
    root.update_idletasks()
    w, h = root.winfo_width(), root.winfo_height()
    ws, hs = root.winfo_screenwidth(), root.winfo_screenheight()
    x, y = (ws-w)//2, (hs-h)//2
    root.geometry(f"{w}x{h}+{x}+{y}")

    # Lanzar hilo de check
    threading.Thread(
        target=check_updates,
        args=(local_lbl, remote_lbl, progress, root),
        daemon=True
    ).start()

    root.mainloop()

if __name__ == "__main__":
    create_splash()
