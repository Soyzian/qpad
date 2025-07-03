import QtQuick
import QtQuick.Controls
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: qsTr("Estilo Oscuro con Color de Enfasis")

    // 🌑 Modo oscuro global
    Material.theme: Material.Dark
    Material.accent: SystemPalette { colorGroup: SystemPalette.Active }.highlight

    // 🪟 Fondo principal
    Rectangle {
        anchors.fill: parent
        color: "#121212" // oscuro

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20

            Text {
                text: "Hola, mundo 👋"
                color: "white"
                font.pixelSize: 24
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: "Botón de prueba"
                Material.accent: SystemPalette { colorGroup: SystemPalette.Active }.highlight
                Layout.alignment: Qt.AlignHCenter
            }

            Slider {
                from: 0
                to: 100
                value: 50
                Layout.preferredWidth: 200
                Material.accent: SystemPalette { colorGroup: SystemPalette.Active }.highlight
            }
        }
    }
}
