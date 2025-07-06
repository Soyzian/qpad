# QPad

**QPad** es un editor de texto moderno y liviano, actualmente en desarrollo, con el objetivo de convertirse en un editor nativo para el nuevo formato **QTF** (Qt Text Format). Inspirado en editores clásicos como WordPad, QPad pretende ofrecer una experiencia simple, rápida y multiplataforma para la edición de textos enriquecidos.

---

## 🧾 ¿Qué es QTF?

**QTF (Qt Text Format)** es una propuesta de formato de texto enriquecido similar a RTF, pero orientado al ecosistema Qt. Se basa en:

- **QHTML**, una versión simplificada y controlada de HTML renderizable en Qt.
- Uso intensivo de etiquetas `<span>` para estilos inline.
- Ideal para almacenamiento portable, sin dependencias externas, y con visualización consistente en interfaces Qt.

---

## ✨ Características principales (roadmap)

- ✅ Editor de texto enriquecido básico
- ✅ Soporte para archivos `.qtf`, `.txt` y `.html`
- ⏳ Interfaz limpia y adaptativa con Qt6
- ⏳ Soporte parcial para estilos (negrita, cursiva, subrayado, color)
- ⏳ Visualización directa del QTF
- 🛠 Exportación e importación entre QTF ↔ HTML ↔ TXT
- 🔜 Syntax Highlighting para QTF
- 🔜 Previsualización en vivo
- 🔜 Plugin system

---

## 🚀 Instalación

### Desde binario

1. Descarga el último instalador desde la [sección Releases](https://github.com/Soyzian/qpad/releases)
2. Ejecuta el archivo `.exe` y sigue los pasos

### Desde el código fuente

```bash
git clone https://github.com/Soyzian/qpad.git
cd qpad
qmake
make
