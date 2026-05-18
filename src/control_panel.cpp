#include "control_panel.h"
#include "garden.h"

extern void processDeferredScreenRebuilds();
extern bool isAutoRefreshScreen(MenuScreen* screen);

ControlPanel::ControlPanel(CharacterDisplayRenderer *renderer, BaseButton *upButton, BaseButton *downButton, BaseButton *selectButton)
{
	this->menu = new LcdMenu(*renderer);
	this->menuRenderer = menu->getRenderer();
	this->upButton = upButton;
	this->downButton = downButton;
	this->selectButton = selectButton;
}

void ControlPanel::setScreen(MenuScreen *screen)
{
	if (startingScreen == nullptr) {
		startingScreen = screen;
	}
	menu->setScreen(screen);
}

void ControlPanel::update(void)
{
	menu->poll();

	if (selectButton->pressed()) {
		menu->process(ENTER);
	}
	if (upButton->pressed()) {
		menu->process(UP);
	}
	if (downButton->pressed()) {
		menu->process(DOWN);
	}
	
	upButton->poll();
	downButton->poll();
	selectButton->poll();

	processDeferredScreenRebuilds();

	const unsigned long now = millis();
	if (isAutoRefreshScreen(menu->getScreen()) && (now - lastRefreshMs >= 1000)) {
		lastRefreshMs = now;
		menu->refresh();
	}
}


ControlPanel::~ControlPanel()
{
	delete menu;
	delete upButton;
	delete downButton;
	delete selectButton;
}
