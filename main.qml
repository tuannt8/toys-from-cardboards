import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

import dtu.mek.petsc 1.0


ApplicationWindow {
    id: globalWindows
    visible: true

    title: "Cardboard"

    width: 400
    height: 600

    GLWindow{
        visible: false
        id: glVisualization

        SequentialAnimation on t {
            NumberAnimation { to: 1; duration: 2500; easing.type: Easing.InQuad }
            NumberAnimation { to: 0; duration: 2500; easing.type: Easing.OutQuad }
            loops: Animation.Infinite
            running: true
        }
    }

    Text {
        id: triangleIdx
        x: parent.width / 2
        y: parent.height / 2
        text: qsTr("text")
    }


    Slider{
        width: parent.width
        y: parent.height - height
        value: 1
        from: 0.01
        to: 1
        live: false

        onValueChanged: {
            console.log("reduce to " + value)
            glVisualization.reduceMesh(value)
        }
    }
}
