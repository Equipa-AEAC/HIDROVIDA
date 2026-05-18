#include "garden.h"
#include "ui/lcd_screens.h"

#include "modules/built_in_led.h"
#include "modules/relay_module.h"
#include "controls/interrupt_button.h"

TaskHandle_t SchedulerTask;
static QueueHandle_t SchedulerQueue = nullptr;

void printLcd(const char* line1, const char* line2 = nullptr) {
	Garden::controlPanel->menu->hide();
	Garden::controlPanel->menu->getRenderer()->display->clear();
	Garden::controlPanel->menu->getRenderer()->display->setCursor(0, 0);
	Garden::controlPanel->menu->getRenderer()->display->draw(line1);
	if (line2 != nullptr)
	{
		Garden::controlPanel->menu->getRenderer()->display->setCursor(0, 1);
		Garden::controlPanel->menu->getRenderer()->display->draw(line2);
	}
}

void Garden::initialize(void)
{
	Garden::i2cCheck();

	Serial.println("[Garden::initialize] Initializing LCD interface (I2C)...");
	lcd = new LiquidCrystal_I2C(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
	lcd->init();
	lcd->backlight();
	lcd->clear();

	Serial.println("[Garden::initialize] Creating LCD adapter and renderer...");
	lcdAdapter = new LiquidCrystal_I2CAdapter(lcd);
	renderer = new CharacterDisplayRenderer(lcdAdapter, LCD_COLS, LCD_ROWS);

	Serial.println("[Garden::initialize] Creating control panel and setting main screen...");

	Garden::controlPanel = new ControlPanel(renderer,
	                                        new InterruptButton(HardwarePinos::PinoBotaoEsquerda),
	                                        new InterruptButton(HardwarePinos::PinoBotaoDireita),
	                                        new InterruptButton(HardwarePinos::PinoBotaoAceitar));
	Garden::controlPanel->setScreen(mainScreen);

	Serial.println("[Garden::initialize] Mounting LittleFS...");

	if(!LittleFS.begin(true)){
		Serial.println("[Garden::initialize] LittleFS Mount Failed");
		printLcd("LittleFS Mount","Failed");
		while (true) {
			delay(1000);
		}
	}

	Serial.println("[Garden::initialize] Creating modules (hardcoded):");

	Serial.println(" > ClockModule");
	auto clock = new ClockModule();
	clock->initialize();
	
	clock->getTimestamp(Garden::_timestampBuffer);
	clock->getDate(Garden::_dateBuffer);
	clock->getTime(Garden::_timeBuffer);

	Garden::clock = clock;

	Serial.println("> BuiltInLedModule");
	auto ledModule = new BuiltInLedModule();
	ledModule->initialize();

	Serial.println("> RelayModule (Bomba de circulacao)");
	auto circulationPumpRelay = new RelayModule("Bomba de circulacao",
	                                           "CIRC",
	                                           HardwarePinos::PinoReleBombaCirculacao,
	                                           HardwareNiveis::NivelReleAtivo,
	                                           HardwareNiveis::NivelReleInativo);
	circulationPumpRelay->initialize();

	Serial.println("> RelayModule (Sistema de luz)");
	auto lightRelay = new RelayModule("Sistema de luz",
	                                  "LUZ",
	                                  HardwarePinos::PinoReleSistemaLuz,
	                                  HardwareNiveis::NivelReleAtivo,
	                                  HardwareNiveis::NivelReleInativo);
	lightRelay->initialize();

	Serial.println("[Garden::initialize] Starting scheduler task...");

	Garden::scheduler = new Scheduler();
	Garden::scheduler->Initialize();

	SchedulerQueue = xQueueCreate(16, sizeof(SchedulerMsg));
	if (SchedulerQueue == nullptr) {
		Serial.println("[Garden::initialize] Failed to create SchedulerQueue");
		printLcd("Failed to create", "SchedulerQueue");
		while (true) {
			delay(1000);
		}
	}

	xTaskCreatePinnedToCore(
		Garden::runScheduler,
		"SchedulerTask",
		10000,
		NULL,
		1,
		&SchedulerTask,
		0
	);

	delay(500); // Give the scheduler task some time to start

	Serial.println("[Garden::initialize] Registering modules with scheduler...");

	Garden::scheduler->registerModule(clock);
	Garden::scheduler->registerModule(ledModule);
	Garden::scheduler->registerModule(circulationPumpRelay);
	Garden::scheduler->registerModule(lightRelay);

	Serial.println("[Garden::initialize] Setting up screens...");

	setupScreens();
	
	Serial.println("[Garden::initialize] Initialization complete.");
}

void Garden::update(void)
{
	Garden::clock->update();
	Garden::clock->getTimestamp(Garden::_timestampBuffer);
	Garden::clock->getDate(Garden::_dateBuffer);
	Garden::clock->getTime(Garden::_timeBuffer);

	Garden::scheduler->syncModules();
	Garden::controlPanel->update();
}

void Garden::runScheduler(void *pvParameters)
{
	Serial.print("[Garden::runScheduler] SchedulerTask running on core ");
	Serial.println(xPortGetCoreID());

	SchedulerMsg msg;

	while (true) {
		if (xQueueReceive(SchedulerQueue, &msg, pdMS_TO_TICKS(200)) == pdPASS) {
			switch (msg.type) {
				case SchedulerMsgType::Reload:
					Serial.println("[Garden::runScheduler] RELOAD!");
					break;
				case SchedulerMsgType::TriggerEvent:
					Serial.println("[Garden::runScheduler] TRIGGER EVENT!");
					break;
			}
		}

		auto n = Garden::scheduler->dispatchDueEvents();
		if (n > 0)
			Serial.printf("[Garden::runScheduler] Just polled %d events\n", n);
			
		delay(500);
	}
}

void Garden::sendToScheduler(SchedulerMsg msg)
{
	xQueueSend(SchedulerQueue, &msg, pdMS_TO_TICKS(50));
}

void Garden::i2cCheck(void) {
	Serial.println("\n[Garden::i2cCheck] Verifying specific devices...");

	Wire.beginTransmission(LCD_ADDRESS);
	if (Wire.endTransmission() == 0) Serial.println("[Garden::i2cCheck] LCD found at 0x27");
	else Serial.println("[Garden::i2cCheck] LCD NOT FOUND at 0x27");
	
	Wire.beginTransmission(0x68);
	if (Wire.endTransmission() == 0) Serial.println("[Garden::i2cCheck] DS3231 RTC found at 0x68");
	else Serial.println("[Garden::i2cCheck] DS3231 RTC NOT FOUND at 0x68");
	
	Serial.println("[Garden::i2cCheck] Done\n");
}

ControlPanel* Garden::controlPanel = nullptr;
LiquidCrystal_I2C* Garden::lcd = nullptr;
LiquidCrystal_I2CAdapter* Garden::lcdAdapter = nullptr;
CharacterDisplayRenderer* Garden::renderer = nullptr;
ClockModule* Garden::clock = nullptr;
Scheduler* Garden::scheduler = nullptr;

char Garden::_timestampBuffer[20] = {0}; // "YYYY-MM-DD HH:MM:SS" + '\0'
char Garden::_dateBuffer[11] = {0}; // "YYYY-MM-DD" + '\0'
char Garden::_timeBuffer[9] = {0};  // "HH:MM:SS" + '\0'
