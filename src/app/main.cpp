/***************************************************************************
                            main.cpp  -  main for qgis mobileapp
                            based on src/browser/main.cpp
                              -------------------
              begin                : Wed Apr 04 10:48:28 CET 2012
              copyright            : (C) 2012 by Marco Bernasocchi
              email                : marco@bernawebdesign.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appinterface.h"
#include "fileutils.h"
#include "qfield.h"
#include "qgismobileapp.h"
#if WITH_SENTRY
#include "sentry_wrapper.h"
#endif

#include <qgsapplication.h>
#include <qgslogger.h>

#ifdef WITH_SPIX
#include <Spix/AnyRpcServer.h>
#include <Spix/QtQmlBot.h>
#endif

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QLocale>
#include <QMainWindow>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>
#include <QtWebView/QtWebView>

#include <proj.h>

#if HAVE_STATIC_QCA_PLUGINS
#include <QtPlugin>
Q_IMPORT_PLUGIN( opensslPlugin )
#endif

void initGraphics()
{
#ifdef WITH_SPIX
  // Set antialiasing method to vertex to get same antialiasing across environments
  qputenv( "QSG_ANTIALIASING_METHOD", "vertex" );
#endif

  // Enables antialiasing in QML scenes
  QSurfaceFormat format;
  format.setSamples( 4 );
  QSurfaceFormat::setDefaultFormat( format );
  QGuiApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
}

int main( int argc, char **argv )
{
  initGraphics();

  // Read settings, use a dummy app to get access to QSettings
  QCoreApplication *dummyApp = new QCoreApplication( argc, argv );
  QCoreApplication::setOrganizationName( "OPENGIS.ch" );
  QCoreApplication::setOrganizationDomain( "opengis.ch" );
  QCoreApplication::setApplicationName( qfield::appName );
  const QSettings settings;
  const QString customLanguage = settings.value( "/customLanguage", QString() ).toString();
#if WITH_SENTRY
  const bool enableSentry = settings.value( "/enableInfoCollection", true ).toBool();
  if ( enableSentry )
    sentry_wrapper::init();
  // Make sure everything flushes when exiting the app
  auto sentryClose = qScopeGuard( [] { sentry_wrapper::close(); } );
#endif

  Q_INIT_RESOURCE( qml );
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
  Q_INIT_RESOURCE( qmlqt6 );
#else
  Q_INIT_RESOURCE( qmlqt5 );
#endif

  QtWebView::initialize();

  if ( !customLanguage.isEmpty() )
    QgsApplication::setTranslation( customLanguage );

  PlatformUtilities *platformUtils = PlatformUtilities::instance();
  platformUtils->initSystem();

  // Let's make sure we have a writable path for the qgis_profile on every platform
  const QString profilePath = platformUtils->systemLocalDataLocation( QStringLiteral( "/qgis_profile" ) );
  QDir().mkdir( profilePath );
  delete dummyApp;

  QgsApplication app( argc, argv, true, profilePath, QStringLiteral( "mobile" ) );

  const QString qfieldFont( qgetenv( "QFIELD_FONT_TTF" ) );
  if ( !qfieldFont.isEmpty() )
  {
    const QString qfieldFontName( qgetenv( "QFIELD_FONT_NAME" ) );
    const int qfieldFontSize = QString( qgetenv( "QFIELD_FONT_SIZE" ) ).toInt();
    QFontDatabase::addApplicationFont( qfieldFont );
    app.setFont( QFont( qfieldFontName, qfieldFontSize ) );
  }

#ifdef RELATIVE_PREFIX_PATH
  qputenv( "GDAL_DATA", QDir::toNativeSeparators( PlatformUtilities::instance()->systemSharedDataLocation() + "/gdal" ).toLocal8Bit() );
  const QString projPath( QDir::toNativeSeparators( PlatformUtilities::instance()->systemSharedDataLocation() + "/proj" ) );
#else
  app.setPrefixPath( QGIS_PREFIX_PATH, true );
  const QString projPath;
#endif

  // cppcheck-suppress knownConditionTrueFalse
  // cppcheck-suppress reademptycontainer
  if ( !projPath.isNull() )
  {
    qInfo() << "Proj path: " << projPath;
    const char *projPaths[] { projPath.toUtf8().constData() };
    proj_context_set_search_paths( nullptr, 1, projPaths );
  }
  else
  {
    qInfo() << "Proj path: {System}";
  }

#if WITH_SENTRY
  sentry_wrapper::install_message_handler();
#endif
  app.initQgis();
  app.setThemeName( settings.value( "/Themes", "default" ).toString() );
#ifdef RELATIVE_PREFIX_PATH
  app.setPkgDataPath( PlatformUtilities::instance()->systemSharedDataLocation() + QStringLiteral( "/qgis" ) );
#endif
  app.createDatabase();

  QSettings::setDefaultFormat( QSettings::NativeFormat );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "OPENGIS.ch" );
  QCoreApplication::setOrganizationDomain( "opengis.ch" );
  QCoreApplication::setApplicationName( qfield::appName );

  QTranslator qfieldTranslator;
  QTranslator qtTranslator;
  if ( !customLanguage.isEmpty() )
  {
    qfieldTranslator.load( QStringLiteral( "qfield_%1" ).arg( customLanguage ), QStringLiteral( ":/i18n/" ), "_" );
    qtTranslator.load( QStringLiteral( "qt_%1" ).arg( customLanguage ), QStringLiteral( ":/i18n/" ), "_" );
  }
  if ( qfieldTranslator.isEmpty() )
    qfieldTranslator.load( QLocale(), "qfield", "_", ":/i18n/" );
  if ( qtTranslator.isEmpty() )
    qtTranslator.load( QLocale(), "qt", "_", ":/i18n/" );

  app.installTranslator( &qtTranslator );
  app.installTranslator( &qfieldTranslator );

  QgisMobileapp mApp( &app );

#ifdef WITH_SPIX
  spix::AnyRpcServer server;
  auto bot = new spix::QtQmlBot();
  bot->runTestServer( server );
#endif

  return app.exec();
}
