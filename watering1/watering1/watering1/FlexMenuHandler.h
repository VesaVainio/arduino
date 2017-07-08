#pragma once

#include <Arduino.h>

class FlexMenuHandler : public DisplayHandler {
protected:
  int itemIndex = 0;
  int itemCount;
  int menuItemCount;
  int flexItemMenuIndex;
  int flexItemCount;

  FlexMenuHandler(int menuItemCount_, int flexItemMenuIndex_, int flexItemCount_) {
    menuItemCount = menuItemCount_;
    flexItemMenuIndex = flexItemMenuIndex_;
    flexItemCount = flexItemCount_;
    itemCount = menuItemCount + flexItemCount - 1;
  }

  virtual void printMenuOnLcd() {};

  int getMenuIndex(int itemIndex) {
    if (itemIndex <= flexItemMenuIndex) {
      return itemIndex;
    }
    if (itemIndex <= flexItemMenuIndex + flexItemCount - 1) {
      return flexItemMenuIndex;
    }
    return itemIndex - (flexItemCount - 1);
  }

  int getFlexItemIndex(int itemIndex) {
    if (itemIndex <= flexItemMenuIndex || itemIndex > flexItemMenuIndex + flexItemCount - 1) {
      return 0;
    }

    return itemIndex - flexItemMenuIndex;
  }

  bool isFlexItem(int itemIndex) {
    return getMenuIndex(itemIndex) == flexItemMenuIndex;
  }

public:
  virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % itemCount;
		printMenuOnLcd();
		return this;
	}

};
