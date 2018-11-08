
#include <qgslocatormodel.h>

#include "locatormodelsuperbridge.h"

LocatorModelSuperBridge::LocatorModelSuperBridge(QObject *parent )
  : QgsLocatorModelBridge(parent)
{
}

void LocatorModelSuperBridge::triggerResultAtRow(int row)
{
  const QModelIndex index = mLocatorModel->index(row,0);
  if (index.isValid())
    triggerResult(index);
}
