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


    Text{
        text: glVisualization.fr
        y: parent.height - height
    }
}
