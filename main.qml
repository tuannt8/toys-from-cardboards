import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

import dtu.mek.petsc 1.0
import QtGraphicalEffects 1.0

import "qml"

ApplicationWindow {
    id: globalWindows
    visible: true

    title: "Cardboard"

    width: 400
    height: 600

    property bool showControl: true
    property int fontSize: 18
    property string colortext: "blue"

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

    Item{
        // Single triangle
        visible: glVisualization.appState == 1 && !glVisualization.isAssistView
                 && !glVisualization.isCutting2faces

        TextShadow{
            description: glVisualization.f
            xpos: glVisualization.x_f
            ypos: glVisualization.y_f
        }
        TextShadow{
            description: glVisualization.f0
            xpos: glVisualization.x_f0
            ypos: glVisualization.y_f0
        }
        TextShadow{
            description: glVisualization.f1
            xpos: glVisualization.x_f1
            ypos: glVisualization.y_f1
        }
        TextShadow{
            description: glVisualization.f2
            xpos: glVisualization.x_f2
            ypos: glVisualization.y_f2
        }
    }

    Item{ // double triangle
        visible: glVisualization.appState == 1 && !glVisualization.isAssistView
                    && glVisualization.isCutting2faces
        TextShadow{
            description: glVisualization.t1
            xpos: glVisualization.x_t1
            ypos: glVisualization.y_t1
        }

        TextShadow{
            description: glVisualization.t2
            xpos: glVisualization.x_t2
            ypos: glVisualization.y_t2
        }


        TextShadow {
            xpos: glVisualization.x_n11
            ypos: glVisualization.y_n11
            description: glVisualization.n11
        }
        TextShadow {
            xpos: glVisualization.x_n12
            ypos: glVisualization.y_n12
            description: glVisualization.n12
        }
        TextShadow {
            xpos: glVisualization.x_t2
            ypos: glVisualization.y_t2
            description: glVisualization.t2
        }
        TextShadow {
            xpos: glVisualization.x_n21
            ypos: glVisualization.y_n21
            description: glVisualization.n21
        }
        TextShadow {
            xpos: glVisualization.x_n22
            ypos: glVisualization.y_n22
            description: glVisualization.n22
        }
    }

    Rectangle{ // Draw triangle
        anchors.fill: parent
        color: "#00000000"
        visible: glVisualization.appState == 1




        TextShadow {
            description: glVisualization.cutline
            xpos: 0
            ypos: 0
        }

        ColumnLayout
        {
            y: parent.height - height - 5

            RowLayout{
                Button{
                    text: glVisualization.isAssistView? "Cut view" : "assist view"
                    onClicked: {
                        glVisualization.isAssistView = !glVisualization.isAssistView
                    }
                }

                Button{
                    text: glVisualization.isCutting2faces? "Cutting double" : "Cutting single"
                    onClicked: {
                        glVisualization.changeCutMode()
                    }
                }
            }

            RowLayout{

                Button{
                    text: "Pre"
                    onClicked:
                    {
                        glVisualization.pre()
                    }
                }
                Button{
                    id: nextButton
                    text: "Next"
                    onClicked: glVisualization.next()
                }



                Text {
                    text: glVisualization.progress
                    color: "white"
                }
            }
        }
    }

    Rectangle{ // Draw 3D model
        anchors.fill: parent
        color: "#00000000"
        visible: glVisualization.appState == 0
        Rectangle{
            radius: 5
            anchors.fill: controlLayout
            color: "#70AAAAAA"
            visible: showControl
        }

        ColumnLayout{
            id: controlLayout
            y: showHide.y - height -  5
            visible: showControl
            spacing: 10

            Text {
                text: glVisualization.meshInfo
            }

            RowLayout{
                Text{
                    id: text1
                    text: "Reduce mesh: "
                }

                Slider{
                    id: reduceSlider
                    value: 1
                    from: 0.01
                    to: 1
                    live: false

                    onValueChanged: {
                        console.log("reduce to " + value)
                        glVisualization.reduceMesh(value)
                    }
                }

                Text {
                    id: text2
                    text: reduceSlider.value.toFixed(2)
                }
            }

            RowLayout{
                Text {
                    text: "Model height: "
                }

                Slider{
                    id: heightSlider
                    value: glVisualization.modelHeight()
                    from: 10
                    to: 150

                    onValueChanged: {
                        glVisualization.setModelHeight(value)
                    }
                }

                TextEdit{
                    text: heightSlider.value.toFixed(0) + " cm"
                }
            }

            RowLayout{
                Text {
                    text: "Main axis"
                }
                ComboBox{
                    currentIndex: glVisualization.getMainAxis();
                    model: ["X (red)", "Y (green)", "Z (blue)"]
                    onCurrentIndexChanged:{
                        glVisualization.setMainAxis(currentIndex)
                    }
                }
            }
        }

        RowLayout{
            y: parent.height - height - 5
            id: showHide

            Button{
                text: showControl? "Hide control" : "Show control"
                onClicked: showControl = !showControl
            }

            Button{
                text: "Load model"
                onClicked: {

                }
            }

            Button{
                text: "Start"
                onClicked: {
                    glVisualization.start()
                }
            }

        }
    }

}
