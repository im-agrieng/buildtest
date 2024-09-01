#include "coordinatereferencesystemutils.h"
#include "geometryutils.h"
#include "positioningmodel.h"

#include <QVariant>
#include <qgsunittypes.h>

PositioningModel::PositioningModel( QObject *parent )
  : QStandardItemModel( parent )
{
  connect( this, &QStandardItemModel::dataChanged, this, &PositioningModel::onDataChanged );
}

void PositioningModel::setupConnections()
{
  connect( mPositioningSource, &Positioning::positionInformationChanged, this, &PositioningModel::refreshData );
}

void PositioningModel::refreshData()
{
  if ( !mPositioningSource )
  {
    return;
  }

  //! Update all info dynamically :)
  //QList<QPair<QString, QVariant>> details = mPositioningSource->device()->details();
  // for ( int i = 0; i < details.size(); ++i )
  // {
  //   const QString key = details[i].first;
  //   const QVariant value = details[i].second;

  //   updateInfo( key, value );
  // }

  const bool coordinatesIsXY = CoordinateReferenceSystemUtils::defaultCoordinateOrderForCrsIsXY( coordinateDisplayCrs() );
  const bool coordinatesIsGeographic = coordinateDisplayCrs().isGeographic();
  const QgsPoint coordinates = GeometryUtils::reprojectPoint( positioningSource()->sourcePosition(), CoordinateReferenceSystemUtils::wgs84Crs(), coordinateDisplayCrs() );
  const double distanceUnitFactor = QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Meters, distanceUnits() );
  const QString distanceUnitAbbreviation = QgsUnitTypes::toAbbreviatedString( distanceUnits() );

  QString coord1Label;
  QString coord2Label;

  if ( coordinatesIsGeographic )
  {
    coord1Label = coordinatesIsXY ? tr( "Lon" ) : tr( "Lat" );
    coord2Label = coordinatesIsXY ? tr( "Lat" ) : tr( "Lon" );
  }
  else
  {
    coord1Label = coordinatesIsXY ? tr( "X" ) : tr( "Y" );
    coord2Label = coordinatesIsXY ? tr( "Y" ) : tr( "X" );
  }

  QString coord1Value = "";
  QString coord2Value = "";

  if ( coordinatesIsXY )
  {
    if ( positioningSource()->positionInformation().longitudeValid() )
    {
      coord1Value = QLocale::system().toString( coordinates.x(), 'f', coordinatesIsGeographic ? 7 : 3 );
      coord2Value = QLocale::system().toString( coordinates.y(), 'f', coordinatesIsGeographic ? 7 : 3 );
    }
    else
    {
      coord1Value = tr( "N/A" );
      coord2Value = tr( "N/A" );
    }
  }
  else
  {
    if ( positioningSource()->positionInformation().latitudeValid() )
    {
      coord1Value = QLocale::system().toString( coordinates.y(), 'f', coordinatesIsGeographic ? 7 : 3 );
      coord2Value = QLocale::system().toString( coordinates.x(), 'f', coordinatesIsGeographic ? 7 : 3 );
    }
    else
    {
      coord1Value = tr( "N/A" );
      coord2Value = tr( "N/A" );
    }
  }

  QString altitude = "";
  if ( positioningSource()->positionInformation().elevationValid() )
  {
    altitude += QLocale::system().toString( positioningSource()->projectedPosition().z() * distanceUnitFactor, 'f', 3 ) + ' ' + distanceUnitAbbreviation + ' ';
    QStringList details = {};

    if ( positioningSource()->elevationCorrectionMode() == Positioning::ElevationCorrectionMode::OrthometricFromGeoidFile )
    {
      details.push_back( "grid" );
    }
    else if ( positioningSource()->elevationCorrectionMode() == Positioning::ElevationCorrectionMode::OrthometricFromDevice )
    {
      details.push_back( "ortho." );
    }
    if ( antennaHeight() != -1 )
    {
      details.push_back( "ant." );
    }
    if ( details.length() > 0 )
    {
      altitude += QString( " (%1)" ).arg( details.join( ", " ) );
    }
  }
  else
  {
    altitude = tr( "N/A" );
  }

  QString speed = "";
  if ( positioningSource()->positionInformation().speedValid() )
    speed = QLocale::system().toString( positioningSource()->positionInformation().speed(), 'f', 3 ) + " m/s";
  else
    speed = tr( "N/A" );


  QString hAccuracy = "";
  if ( positioningSource()->positionInformation().haccValid() )
    hAccuracy = QLocale::system().toString( positioningSource()->positionInformation().hacc() * distanceUnitFactor, 'f', 3 ) + ' ' + distanceUnitAbbreviation;
  else
    hAccuracy = tr( "N/A" );

  QString vAccuracy = "";
  if ( positioningSource()->positionInformation().vaccValid() )
    vAccuracy = QLocale::system().toString( positioningSource()->positionInformation().vacc() * distanceUnitFactor, 'f', 3 ) + ' ' + distanceUnitAbbreviation;
  else
    vAccuracy = tr( "N/A" );

  updateInfo( coord1Label, coord1Value );
  updateInfo( coord2Label, coord2Value );
  updateInfo( "Altitude", altitude );
  updateInfo( "Speed", speed );
  updateInfo( "H. Accuracy", hAccuracy );
  updateInfo( "V. Accuracy", vAccuracy );
}

void PositioningModel::updateInfo( const QString &name, const QVariant &value )
{
  // Check if the item already exists
  for ( int row = 0; row < rowCount(); ++row )
  {
    QStandardItem *rowItem = item( row );
    if ( rowItem->data( VariableNameRole ).toString() == name )
    {
      // Item exists, update its value
      rowItem->setData( value.toString(), VariableValueRole );
      return;
    }
  }

  QStandardItem *nameItem = new QStandardItem( name );
  nameItem->setData( name, VariableNameRole );
  nameItem->setData( value.toString(), VariableValueRole );
  insertRow( rowCount(), QList<QStandardItem *>() << nameItem );
}


bool PositioningModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  QStandardItem *rowItem = item( index.row() );
  if ( !rowItem )
  {
    return false;
  }

  switch ( role )
  {
    case VariableNameRole:
      if ( rowItem->data( VariableNameRole ) == value )
      {
        return false;
      }

      rowItem->setData( value, VariableNameRole );
      return true;

    case VariableValueRole:

      if ( rowItem->data( VariableValueRole ) == value )
      {
        return false;
      }

      rowItem->setData( value, VariableValueRole );
      return true;

    default:
      break;
  }

  return false;
}

QHash<int, QByteArray> PositioningModel::roleNames() const
{
  QHash<int, QByteArray> names = QStandardItemModel::roleNames();
  names[VariableNameRole] = "VariableName";
  names[VariableValueRole] = "VariableValue";
  return names;
}

void PositioningModel::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( bottomRight )
  Q_UNUSED( roles )
}

Positioning *PositioningModel::positioningSource() const
{
  return mPositioningSource;
}

void PositioningModel::setPositioningSource( Positioning *newPositioningSource )
{
  if ( mPositioningSource == newPositioningSource )
    return;
  mPositioningSource = newPositioningSource;
  emit positioningSourceChanged();
}

double PositioningModel::antennaHeight() const
{
  return mAntennaHeight;
}

void PositioningModel::setAntennaHeight( double newAntennaHeight )
{
  if ( qFuzzyCompare( mAntennaHeight, newAntennaHeight ) )
    return;
  mAntennaHeight = newAntennaHeight;
  emit antennaHeightChanged();
}

Qgis::DistanceUnit PositioningModel::distanceUnits() const
{
  return mDistanceUnits;
}

void PositioningModel::setDistanceUnits( Qgis::DistanceUnit newDistanceUnits )
{
  if ( mDistanceUnits == newDistanceUnits )
    return;
  mDistanceUnits = newDistanceUnits;
  emit distanceUnitsChanged();
}

QgsCoordinateReferenceSystem PositioningModel::coordinateDisplayCrs() const
{
  return mCoordinateDisplayCrs;
}

void PositioningModel::setCoordinateDisplayCrs( const QgsCoordinateReferenceSystem &newCoordinateDisplayCrs )
{
  if ( mCoordinateDisplayCrs == newCoordinateDisplayCrs )
    return;
  mCoordinateDisplayCrs = newCoordinateDisplayCrs;
  emit coordinateDisplayCrsChanged();
}
