import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

import dtu.mek.petsc 1.0
import QtGraphicalEffects 1.0

Item {
    property string description
    property int xpos
    property int ypos
    Text {
        id: triangleIdx
        color: "blue"
        font.pointSize: 14
        x: xpos //glVisualization.x_t1
        y: ypos //glVisualization.y_t1
        text: description//glVisualization.t1
    }
    Glow {
        radius: 8
        samples: 16
        color: "lightblue"
        source: triangleIdx
        anchors.fill: triangleIdx
    }
}
