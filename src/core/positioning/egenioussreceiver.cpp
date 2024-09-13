#include "egenioussreceiver.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

EgenioussReceiver::EgenioussReceiver( QObject *parent )
  : AbstractGnssReceiver( parent ), mTcpSocket( new QTcpSocket( this ) )
{
  connect( mTcpSocket, &QTcpSocket::readyRead, this, &EgenioussReceiver::onReadyRead );
  connect( mTcpSocket, &QTcpSocket::errorOccurred, this, &EgenioussReceiver::onErrorOccurred );
}

void EgenioussReceiver::handleConnectDevice()
{
  mTcpSocket->connectToHost( QHostAddress::LocalHost, 1235 );

  if ( !mTcpSocket->waitForConnected( 3000 ) )
  {
    setValid( false );
    mLastError = mTcpSocket->errorString();
    emit lastErrorChanged( mLastError );
  }
  else
  {
    setValid( true );
    mSocketState = QAbstractSocket::ConnectedState;
    emit socketStateChanged( mSocketState );
  }
}

void EgenioussReceiver::handleDisconnectDevice()
{
  if ( mTcpSocket->state() == QAbstractSocket::ConnectedState )
  {
    mTcpSocket->disconnectFromHost();
    mTcpSocket->waitForDisconnected();
  }
}

QList<QPair<QString, QVariant>> EgenioussReceiver::details()
{
  QList<QPair<QString, QVariant>> dataList;

  QJsonDocument jsonDoc = QJsonDocument::fromJson( payload );
  QJsonObject jsonObject = jsonDoc.object();

  dataList.append( qMakePair( "q", jsonObject.value( "q" ).toString() ) );

  return dataList;
}

void EgenioussReceiver::onReadyRead()
{
  if ( valid() )
  {
    mReceivedData = mTcpSocket->readAll();
    if ( mReceivedData.size() < 9 )
    {
      return; // Received data is too short to process.
    }

    if ( static_cast<uint8_t>( mReceivedData[0] ) != 0xFE )
    {
      return; // Invalid start byte
    }

    uint32_t payloadLength;
    QDataStream dataStream( mReceivedData.mid( 4, 4 ) );
    dataStream.setByteOrder( QDataStream::LittleEndian );
    dataStream >> payloadLength;

    if ( mReceivedData.size() < 8 + payloadLength )
    {
      return; // Received data is too short to contain the payload.
    }

    payload = mReceivedData.mid( 8, payloadLength );

    QJsonDocument jsonDoc = QJsonDocument::fromJson( payload );

    if ( jsonDoc.isNull() || !jsonDoc.isObject() )
    {
      return; // Failed to parse JSON
    }

    QJsonObject jsonObject = jsonDoc.object();

    mLastGnssPositionInformation = GnssPositionInformation(
      jsonObject.value( "lat" ).toDouble(),
      jsonObject.value( "lon" ).toDouble(),
      jsonObject.value( "alt" ).toDouble(),
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN(),
      QList<QgsSatelliteInfo>(),
      0,
      0,
      0,
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN(),
      QDateTime(),
      QChar(),
      0,
      1 );

    emit lastGnssPositionInformationChanged( mLastGnssPositionInformation );
  }
}

void EgenioussReceiver::onErrorOccurred( QAbstractSocket::SocketError socketError )
{
  mLastError = mTcpSocket->errorString();
  emit lastErrorChanged( mLastError );
}
