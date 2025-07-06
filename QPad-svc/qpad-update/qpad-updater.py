import sys
import os
import requests
from PyQt6.QtWidgets import (
    QApplication, QWidget, QLabel, QVBoxLayout, QPushButton, QProgressBar, QDialog,
    QHBoxLayout, QTextEdit
)
from PyQt6.QtGui import QPixmap, QIcon, QFont
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QUrl
from PyQt6.QtGui import QDesktopServices


REPO = "Soyzian/qpad"
VERSION_FILE = "currentversion.txt"
EXECUTABLE = "QPad-main.exe"
RELEASES_URL = f"https://github.com/{REPO}/releases"
GITHUB_API_RELEASES = f"https://api.github.com/repos/{REPO}/releases"



class DownloaderThread(QThread):
    progress_changed = pyqtSignal(int)
    download_finished = pyqtSignal(bool, str)

    def __init__(self, url, dest):
        super().__init__()
        self.url = url
        self.dest = dest

    def run(self):
        try:
            with requests.get(self.url, stream=True) as r:
                r.raise_for_status()
                total_length = int(r.headers.get('content-length', 0))
                downloaded = 0
                with open(self.dest, "wb") as f:
                    for chunk in r.iter_content(chunk_size=8192):
                        if chunk:
                            f.write(chunk)
                            downloaded += len(chunk)
                            progress = int(downloaded * 100 / total_length) if total_length else 0
                            self.progress_changed.emit(progress)
            self.download_finished.emit(True, self.dest)
        except Exception as e:
            self.download_finished.emit(False, str(e))


class UpdateDialog(QDialog):
    def __init__(self, release, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Actualización disponible")
        self.setWindowIcon(QIcon("Ico/icoqpad.ico"))
        self.setFixedSize(380, 220)
        self.setStyleSheet("""
            QDialog {
                background-color: #1e1e1e;
                color: #ddd;
                font-family: Verdana, sans-serif;
                font-size: 11pt;
            }
            QPushButton {
                background-color: #007ACC;
                border: none;
                padding: 8px 15px;
                border-radius: 5px;
                color: white;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #005F9E;
            }
            QPushButton#cancel {
                background-color: #555;
            }
            QPushButton#cancel:hover {
                background-color: #777;
            }
        """)

        layout = QVBoxLayout()
        info_label = QLabel(f"Hay una nueva versión disponible: <b>{release['tag_name']}</b>")
        info_label.setWordWrap(True)
        layout.addWidget(info_label)

        body_text = release.get('body', 'No hay detalles disponibles.')
        text_edit = QTextEdit()
        text_edit.setReadOnly(True)
        text_edit.setPlainText(body_text)
        text_edit.setStyleSheet("background-color: #252526; color: #ddd; border: none;")
        layout.addWidget(text_edit)

        btn_layout = QHBoxLayout()
        self.btn_yes = QPushButton("Descargar e instalar")
        self.btn_no = QPushButton("Cancelar")
        self.btn_no.setObjectName("cancel")
        btn_layout.addStretch()
        btn_layout.addWidget(self.btn_no)
        btn_layout.addWidget(self.btn_yes)
        layout.addLayout(btn_layout)

        self.setLayout(layout)


class UpdateChecker(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("QPad - Actualizador")
        self.setWindowIcon(QIcon("Ico/icoqpad.ico"))  # Cambiar path si hace falta
        self.setFixedSize(420, 350)

        # Modo oscuro básico
        self.setStyleSheet("""
            QWidget {
                background-color: #121212;
                color: #ddd;
                font-family: Verdana, sans-serif;
                font-size: 11pt;
            }
            QLabel {
                margin-bottom: 10px;
            }
            QPushButton {
                background-color: #1f1f1f;
                border: 1px solid #444;
                padding: 8px 14px;
                border-radius: 6px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #333;
            }
            QProgressBar {
                border: 1px solid #444;
                border-radius: 8px;
                background-color: #222;
                height: 25px;
                text-align: center;
                color: white;
                font-weight: bold;
            }
            QProgressBar::chunk {
                background-color: #00AAFF;
                border-radius: 8px;
            }
        """)

        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # Imagen splash
        self.splash_label = QLabel()
        pix = QPixmap("splash.bmp")
        pix = pix.scaled(320, 160, Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation)
        self.splash_label.setPixmap(pix)
        layout.addWidget(self.splash_label, alignment=Qt.AlignmentFlag.AlignCenter)

        self.local_version_label = QLabel("Versión actual: -")
        layout.addWidget(self.local_version_label, alignment=Qt.AlignmentFlag.AlignCenter)

        self.remote_version_label = QLabel("Buscando última versión...")
        layout.addWidget(self.remote_version_label, alignment=Qt.AlignmentFlag.AlignCenter)

        self.progress = QProgressBar()
        self.progress.setValue(0)
        self.progress.hide()
        layout.addWidget(self.progress)

        self.btn_releases = QPushButton("Ver actualizaciones en GitHub")
        self.btn_releases.clicked.connect(lambda: QDesktopServices.openUrl(QUrl(RELEASES_URL)))
        self.btn_releases.hide()
        layout.addWidget(self.btn_releases)

        self.setLayout(layout)

        self.local_version = self.read_local_version()
        self.local_version_label.setText(f"Versión actual: {self.local_version}")
        self.check_for_updates()

    def read_local_version(self):
        try:
            with open(VERSION_FILE, "r", encoding="utf-8") as f:
                return f.read().strip()
        except Exception:
            return "Desconocida"

    def check_for_updates(self):
        try:
            r = requests.get(GITHUB_API_RELEASES, timeout=10)
            r.raise_for_status()
            releases = r.json()

            valid_releases = [rel for rel in releases if not rel['draft'] and not rel['prerelease']]

            if not valid_releases:
                self.remote_version_label.setText("No hay versiones públicas disponibles.")
                self.launch_app()
                return

            latest = valid_releases[0]
            latest_tag = latest['tag_name']

            self.remote_version_label.setText(f"Última versión: {latest_tag}")

            if self.version_greater(latest_tag, self.local_version):
                self.show_update_dialog(latest)
            else:
                self.remote_version_label.setText("Tu versión está actualizada.")

        except Exception as e:
            self.remote_version_label.setText(f"Error al buscar actualizaciones: {e}")

    def version_greater(self, v1, v2):
        def parse_ver(v):
            v = v.lstrip('vV')
            return tuple(int(x) if x.isdigit() else 0 for x in v.split('.'))
        return parse_ver(v1) > parse_ver(v2)

    def show_update_dialog(self, release):
        dialog = UpdateDialog(release, self)
        dialog.btn_yes.clicked.connect(lambda: self.start_download(release, dialog))
        dialog.btn_no.clicked.connect(lambda: (dialog.close(), self.launch_app()))
        dialog.exec()

    def start_download(self, release, dialog):
        asset = None
        for a in release.get('assets', []):
            if a['name'].lower().endswith('.exe'):
                asset = a
                break
        if not asset:
            dialog.close()
            self.remote_version_label.setText("No se encontró instalador en la release.")
            return

        download_url = asset['browser_download_url']
        dest_file = os.path.join(os.getcwd(), asset['name'])

        self.progress.show()
        self.downloader = DownloaderThread(download_url, dest_file)
        self.downloader.progress_changed.connect(self.progress.setValue)
        self.downloader.download_finished.connect(lambda success, info: self.install_update(success, info, dialog))
        self.downloader.start()

        dialog.close()

    def install_update(self, success, info, dialog):
        if not success:
            self.remote_version_label.setText(f"No se pudo descargar la actualización:\n{info}")
            return

        try:
            os.startfile(info)
        except Exception as e:
            self.remote_version_label.setText(f"No se pudo iniciar el instalador:\n{e}")
        self.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setFont(QFont("Verdana", 9))
    app.setStyle("Windows")
    w = UpdateChecker()
    w.show()
    sys.exit(app.exec())
