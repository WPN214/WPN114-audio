#include "qml_plugin.hpp"

#include <source/audio/audio.hpp>
#include <source/audio/soundfile.hpp>

#include <QQmlEngine>
#include <qqml.h>

void qml_plugin::registerTypes(const char *uri)
{
    Q_UNUSED    ( uri );

    qmlRegisterUncreatableType<StreamNode, 1> ( "WPN114", 1, 0, "StreamNode","Uncreatable");
    qmlRegisterType<WorldStream, 1> ( "WPN114", 1, 0, "AudioStream" );
}