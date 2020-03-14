import QtQuick 2.0

import Qt.labs.settings 1.0 as LabSettings

LabSettings.Settings {

  property bool positioningActivated: false

  property string antennaHeight: "0"
  property bool antennaHeightActivated: false
  property bool skipAltitudeCorrection: false
}
