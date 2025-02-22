import QtQuick
import QtQuick.Controls
import QtQml.Models
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import org.qfield
import org.qgis
import Theme
import "../.."
import ".."

RelationEditorBase {
  id: relationEditor

  relationEditorModel: OrderedRelationModel {
    //containing the current (parent) feature, the relation to the children
    //and the relation from the children to the other parent (if it's nm and cardinality is set)
    id: orderedRelationModel
    currentRelationId: relationId
    feature: currentFeature
    orderingField: relationEditorWidgetConfig['ordering_field']
    imagePath: relationEditorWidgetConfig['image_path']
    description: relationEditorWidgetConfig['description']

    property int featureFocus: -1

    onFailedReorder: {
      displayToast("Failed to reorder features.", "error");
    }
  }

  listView.model: DelegateModel {
    model: orderedRelationModel
    delegate: dragDelegate
  }

  Component {
    id: dragDelegate

    MouseArea {
      id: dragArea
      pressAndHoldInterval: 130

      property bool held: false

      property int indexFrom: -1
      property int indexTo: -1

      anchors.left: parent ? parent.left : undefined
      anchors.right: parent ? parent.right : undefined
      height: content.height

      drag.target: held ? content : undefined
      drag.axis: Drag.YAxis

      onPressAndHold: {
        if (isEnabled) {
          held = true;
        }
      }
      onReleased: {
        if (held === true) {
          held = false;
          orderedRelationModel.moveItems(indexFrom, indexTo);
        } else if (listView.currentIndex !== dragArea.DelegateModel.itemsIndex) {
          listView.currentIndex = dragArea.DelegateModel.itemsIndex;
          orderedRelationModel.triggerViewCurrentFeatureChange(listView.currentIndex);
        }
      }

      onClicked: {
        if (orderedRelationModel.relation.referencingLayer !== undefined) {
          geometryHighlighter.geometryWrapper.qgsGeometry = nmRelationId ? model.nmReferencingFeature.geometry : model.referencingFeature.geometry;
          geometryHighlighter.geometryWrapper.crs = orderedRelationModel.relation.referencingLayer.crs;
          mapCanvas.mapSettings.extent = FeatureUtils.extent(mapCanvas.mapSettings, orderedRelationModel.relation.referencingLayer, nmRelationId ? model.nmReferencingFeature : model.referencingFeature);
        }
      }

      Rectangle {
        id: content
        anchors {
          horizontalCenter: parent.horizontalCenter
          verticalCenter: parent.verticalCenter
        }
        width: dragArea.width
        height: row.implicitHeight + 4

        Ripple {
          clip: true
          width: parent.width
          height: parent.height
          pressed: dragArea.pressed
          anchor: content
          active: dragArea.pressed
          color: Material.rippleColor
        }

        color: "transparent"

        radius: 2
        Drag.active: dragArea.held
        Drag.source: dragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        states: State {
          when: dragArea.held

          AnchorChanges {
            target: content
            anchors.horizontalCenter: undefined
            anchors.verticalCenter: undefined
          }
        }

        Row {
          id: row
          anchors.fill: parent
          anchors.margins: 2

          height: Math.max(itemHeight, featureText.height)

          Image {
            id: featureImage
            source: ImagePath ? UrlUtils.fromString(ImagePath) : Theme.getThemeVectorIcon("ic_photo_notavailable_black_24dp")
            anchors.verticalCenter: parent.verticalCenter
            width: 48
            height: 48
            fillMode: Image.PreserveAspectFit
            visible: !!ImagePath
          }

          Text {
            id: featureText
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 8 - (featureImage.visible ? featureImage.width : 0) - viewButton.width - moveDownButton.width - moveUpButton.width - deleteButton.width
            topPadding: 5
            bottomPadding: 5
            font: Theme.defaultFont
            color: !isEnabled ? Theme.mainTextDisabledColor : Theme.mainTextColor
            text: Description || model.displayString
            elide: Text.ElideRight
            wrapMode: Text.WordWrap
          }

          QfToolButton {
            id: viewButton
            anchors.verticalCenter: parent.verticalCenter
            width: 48
            height: 48

            round: false
            iconSource: isEnabled ? Theme.getThemeVectorIcon('ic_edit_attributes_white_24dp') : Theme.getThemeVectorIcon('ic_baseline-list_white_24dp')
            iconColor: Theme.mainTextColor
            bgcolor: 'transparent'

            onClicked: {
              embeddedPopup.state = isEnabled ? 'Edit' : 'ReadOnly';
              embeddedPopup.currentLayer = orderedRelationModel.relation.referencingLayer;
              embeddedPopup.linkedRelation = orderedRelationModel.relation;
              embeddedPopup.linkedRelationOrderingField = orderedRelationModel.orderingField;
              embeddedPopup.linkedParentFeature = orderedRelationModel.feature;
              embeddedPopup.feature = model.referencingFeature;
              embeddedPopup.open();
            }
          }

          QfToolButton {
            id: moveDownButton
            anchors.verticalCenter: parent.verticalCenter
            visible: isEnabled
            width: visible ? 48 : 0
            height: 48
            opacity: (index === listView.count - 1) ? 0.3 : 1

            round: false
            iconSource: Theme.getThemeVectorIcon('ic_chevron_down')
            iconColor: Theme.mainTextColor
            bgcolor: 'transparent'

            onClicked: {
              if (index === listView.count - 1) {
                return;
              }
              orderedRelationModel.moveItems(index, index + 1);
            }
          }

          QfToolButton {
            id: moveUpButton
            anchors.verticalCenter: parent.verticalCenter
            visible: isEnabled
            width: visible ? 48 : 0
            height: 48
            opacity: (index === 0) ? 0.3 : 1

            round: false
            iconSource: Theme.getThemeVectorIcon('ic_chevron_up')
            iconColor: Theme.mainTextColor
            bgcolor: 'transparent'

            onClicked: {
              if (index === 0) {
                return;
              }
              orderedRelationModel.moveItems(index, index - 1);
            }
          }

          QfToolButton {
            id: deleteButton
            anchors.verticalCenter: parent.verticalCenter
            visible: isEnabled
            width: visible ? 48 : 0
            height: 48

            round: false
            iconSource: Theme.getThemeVectorIcon('ic_delete_forever_white_24dp')
            iconColor: Theme.mainTextColor
            bgcolor: 'transparent'

            onClicked: {
              deleteDialog.referencingFeatureId = model.referencingFeature.id;
              deleteDialog.referencingFeatureDisplayMessage = model.displayString;
              deleteDialog.nmReferencedFeatureId = nmRelationId ? model.model.nmReferencedFeature.id : 0;
              deleteDialog.nmReferencedFeatureDisplayMessage = nmRelationId ? model.nmDisplayString : '';
              deleteDialog.visible = true;
            }
          }
        }

        Rectangle {
          id: bottomLine
          anchors.bottom: parent.bottom
          height: 1
          color: Theme.controlBorderColor
          width: parent.width
        }
      }

      DropArea {
        anchors.fill: parent
        anchors.margins: 10

        onEntered: {
          if (dragArea.indexFrom === -1) {
            dragArea.indexFrom = drag.source.DelegateModel.itemsIndex;
          }
          dragArea.indexTo = dragArea.DelegateModel.itemsIndex;
          visualModel.items.move(drag.source.DelegateModel.itemsIndex, dragArea.DelegateModel.itemsIndex);
        }
      }
    }
  }
}
