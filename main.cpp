#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QDebug>

#include <iostream>

#include "glWindow.h"


#include <assert.h>

using namespace std;

int appState = 0;


int main(int argc, char *argv[])
{  
    // Init app
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    // Link C++ class with QML
    qmlRegisterType<GLWindow>("dtu.mek.petsc", 1, 0, "GLWindow");

//    qmlRegisterType<TopOptParameterControl>("dtu.mek.petsc", 1, 0, "TopOptParam");


    QQmlApplicationEngine engine;



    engine.load("qrc:/main.qml");

    // Run app
    int re = app.exec();

    return re;
}
