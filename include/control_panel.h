#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <LcdMenu.h>
#include <MenuScreen.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <input/KeyboardAdapter.h>
#include "controls/base_button.h"

class ControlPanel {
private:
	MenuRenderer* menuRenderer;
public:
	LcdMenu* menu = NULL;
	MenuScreen* startingScreen = nullptr;
	unsigned long lastRefreshMs = 0;

	BaseButton* upButton = nullptr;
	BaseButton* downButton = nullptr;
	BaseButton* selectButton = nullptr;

	ControlPanel(CharacterDisplayRenderer *renderer, BaseButton* upButton, BaseButton* downButton, BaseButton* selectButton);

	void setScreen(MenuScreen* screen);
	void update(void);

	~ControlPanel();
};

#endif // CONTROL_PANEL_H
