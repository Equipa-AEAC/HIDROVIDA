#ifndef LCD_SCREENS_H
#define LCD_SCREENS_H

#include <LcdMenu.h>
#include <ItemValue.h>
#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include <ItemRange.h>
#include <ItemBool.h>
#include <ItemList.h>
#include "garden.h"
#include "hydro_lcd_data.h"

#include <LittleFS.h>
#include <FS.h>

extern MenuScreen* editTimeDateScreen;
extern MenuScreen* editDateScreen;
extern MenuScreen* editTimeScreen;
extern MenuScreen* schedulerScreen;
extern MenuScreen* addEventScreen;
extern MenuScreen* confirmDeleteEventsScreen;

extern MenuScreen* developerScreen;
extern MenuScreen* hydroMenuScreen;
extern MenuScreen* hydroQualidadeScreen;
extern MenuScreen* hydroNiveisScreen;
extern MenuScreen* hydroAmbienteScreen;

extern MenuScreen* editEventScreen;
extern MenuScreen* editScheduleScreen;
extern MenuScreen* dayOfWeekScreen;

extern EventId _selEventId;
extern uint8_t _selEventType;
extern uint8_t _selModule;
extern uint8_t _selAction;
extern uint8_t _selState;
extern uint8_t _selTimeKeyType;
extern uint8_t _selDaysOfWeek;

extern uint16_t _selYear;
extern uint8_t _selMonth;
extern uint8_t _selDay;
extern uint8_t _selHour;
extern uint8_t _selMinute;
extern uint8_t _selSecond;

void setupScreens(void);

void onRemoveAllConfirmed(void);

void processDeferredScreenRebuilds(void);
bool isAutoRefreshScreen(MenuScreen* screen);

// Guard: skip MENU_SCREEN definitions when lcd_screens.cpp includes this header
// (to avoid multiple definition errors at link time)
#ifndef LCD_SCREENS_NO_SCREENS

static void onYearChanged(const Ref<uint16_t> value) { _selYear = value; }
static void onMonthChanged(const Ref<uint8_t> value) { _selMonth = value; }
static void onDayChanged(const Ref<uint8_t> value) { _selDay = value; }

static void onHourChanged(const Ref<uint8_t> value) { _selHour = value; }
static void onMinuteChanged(const Ref<uint8_t> value) { _selMinute = value; }
static void onSecondChanged(const Ref<uint8_t> value) { _selSecond = value; }

static void syncSelTime(void) {
	_selYear = Garden::clock->getYear();
	_selMonth = Garden::clock->getMonth();
	_selDay = Garden::clock->getDay();
	_selHour = Garden::clock->getHour();
	_selMinute = Garden::clock->getMinute();
	_selSecond = Garden::clock->getSecond();
}
static void updateTime(void) {
	Garden::clock->adjust(DateTime(_selYear, _selMonth, _selDay, _selHour, _selMinute, _selSecond));
}

MENU_SCREEN(mainScreen, mainItems,
	ITEM_VALUE("Date", Garden::_dateBuffer),
	ITEM_VALUE("Time", Garden::_timeBuffer),
	ITEM_SUBMENU("Hydro >", hydroMenuScreen),
	ITEM_SUBMENU("Edit Time/Date >", editTimeDateScreen),
	ITEM_SUBMENU("Scheduler >", schedulerScreen)
);

MENU_SCREEN(hydroMenuScreen, hydroMenuItems,
	ITEM_SUBMENU("Qualidade >", hydroQualidadeScreen),
	ITEM_SUBMENU("Niveis >", hydroNiveisScreen),
	ITEM_SUBMENU("Ambiente >", hydroAmbienteScreen),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(hydroQualidadeScreen, hydroQualidadeItems,
	ITEM_VALUE("Temp", HydroLcdData::valorTemperatura),
	ITEM_VALUE("pH", HydroLcdData::valorPh),
	ITEM_VALUE("TDS", HydroLcdData::valorTds),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(hydroNiveisScreen, hydroNiveisItems,
	ITEM_VALUE("Tanque 1", HydroLcdData::valorTanque1),
	ITEM_VALUE("Tanque 2", HydroLcdData::valorTanque2),
	ITEM_VALUE("Reposicao", HydroLcdData::valorReposicao),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(hydroAmbienteScreen, hydroAmbienteItems,
	ITEM_VALUE("Turbidez", HydroLcdData::valorTurbidez),
	ITEM_VALUE("Fase", HydroLcdData::valorFase),
	ITEM_VALUE("Dose", HydroLcdData::valorDosagem),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(editTimeDateScreen, editTimeDateItems,
	ITEM_COMMAND("Edit Date >", []() {
		syncSelTime();
		Garden::controlPanel->menu->setScreen(editDateScreen);
	}),
	ITEM_COMMAND("Edit Time >", []() {
		syncSelTime();
		Garden::controlPanel->menu->setScreen(editTimeScreen);
	}),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(editDateScreen, editDateItems,
	ITEM_RANGE_REF("Year", _selYear, (uint16_t)1, (uint16_t)2000, (uint16_t)2100, onYearChanged, "%04u"),
	ITEM_RANGE_REF("Month", _selMonth, (uint8_t)1, (uint8_t)1, (uint8_t)12, onMonthChanged, "%02u"),
	ITEM_RANGE_REF("Day", _selDay, (uint8_t)1, (uint8_t)1, (uint8_t)31, onDayChanged, "%02u"),
	ITEM_COMMAND("< Back", []() {
		updateTime();
		Garden::controlPanel->menu->setScreen(editTimeDateScreen);
	})
);

MENU_SCREEN(editTimeScreen, editTimeItems,
	ITEM_RANGE_REF("Hour", _selHour, (uint8_t)1, (uint8_t)0, (uint8_t)23, onHourChanged, "%02u"),
	ITEM_RANGE_REF("Minute", _selMinute, (uint8_t)1, (uint8_t)0, (uint8_t)59, onMinuteChanged, "%02u"),
	ITEM_RANGE_REF("Second", _selSecond, (uint8_t)1, (uint8_t)0, (uint8_t)59, onSecondChanged, "%02u"),
	ITEM_COMMAND("< Back", []() {
		updateTime();
		Garden::controlPanel->menu->setScreen(editTimeDateScreen);
	})
);

MENU_SCREEN(schedulerScreen, schedulerItems,
	//ITEM_SUBMENU("Add event +", addEventScreen),
	ITEM_BASIC("Add event +"),
	ITEM_SUBMENU("Remove all !", confirmDeleteEventsScreen),
	ITEM_COMMAND("Apply", []() {
		Garden::sendToScheduler(SchedulerMsg{SchedulerMsgType::Reload});
		Garden::controlPanel->menu->process(BACK);
	}),
	ITEM_COMMAND("Discard", []() {
		Garden::controlPanel->menu->process(BACK);
	}),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

MENU_SCREEN(confirmDeleteEventsScreen, confirmDeleteEventsItems,
	ITEM_BASIC("Are you sure?"),
	ITEM_COMMAND("Yes", []() {
		onRemoveAllConfirmed();
		Garden::controlPanel->menu->process(BACK);
	}),
	ITEM_COMMAND("No", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

static void lcdDraw2Lines(const String& line1, const String& line2 = "");
static void lcdShowStep(const String& step, const String& detail = "", uint16_t ms = 900);
static void lcdShowPagedLine2(const String& title, String text, uint16_t ms = 900);
static String fsReadTextForLcd(const char* path);
static String fsListDirForLcd(const char* dirname);
static void testFileIO(const char * path);

MENU_SCREEN(developerScreen, developerItems,
	ITEM_COMMAND("Test File I/O", []() {
		Garden::controlPanel->menu->hide();
		Garden::controlPanel->menu->getRenderer()->display->clear();
		Garden::controlPanel->menu->getRenderer()->display->setCursor(0, 0);
		Garden::controlPanel->menu->getRenderer()->display->draw("Testing File I/O...");
		testFileIO("/testfile.bin");
		Garden::controlPanel->menu->show();
		Garden::controlPanel->menu->refresh();
	}),
	ITEM_COMMAND("Test File System", []() {
		Garden::controlPanel->menu->hide();

		lcdShowStep("FS Test", "Starting...");

		lcdShowStep("1) createDir", "/mydir");
		if (!LittleFS.mkdir("/mydir"))
		{
			lcdShowStep("E: 1) createDir", "Failed!", 3000);
			return;
		}

		lcdShowStep("2) writeFile", "/mydir/hello1.txt");

		File file = LittleFS.open("/mydir/hello1.txt", FILE_WRITE);
		if (!file) {
			lcdShowStep("E: 2) writeFile", "Failed to open!", 3000);
			return;
		}
		if (!file.print("Hello1")) {
			lcdShowStep("E: 2) writeFile", "Write failed!", 3000);
			file.close();
			return;
		}
		file.close();

		lcdShowStep("3) deleteFile", "/mydir/hello1.txt");
		if (!LittleFS.remove("/mydir/hello1.txt")) {
			lcdShowStep("E: 3) deleteFile", "Failed!", 3000);
			return;
		}

		lcdShowStep("4) removeDir", "/mydir");
		if (!LittleFS.rmdir("/mydir")) {
			lcdShowStep("E: 4) removeDir", "Failed!", 3000);
			return;
		}

		lcdShowStep("5) writeFile", "/hello.txt");
		File file2 = LittleFS.open("/hello.txt", FILE_WRITE);
		if (!file2) {
			lcdShowStep("E: 5) writeFile", "Failed to open!", 3000);
			return;
		}
		if (!file2.print("Hello ")) {
			lcdShowStep("E: 5) writeFile", "Write failed!", 3000);
			file2.close();
			return;
		}
		file2.close();

		lcdShowStep("6) appendFile", "/hello.txt");
		File file3 = LittleFS.open("/hello.txt", FILE_APPEND);
		if (!file3) {
			lcdShowStep("E: 6) appendFile", "Failed to open!", 3000);
			return;
		}
		if (!file3.print("World!\r\n")) {
			lcdShowStep("E: 6) appendFile", "Write failed!", 3000);
			file3.close();
			return;
		}
		file3.close();

		lcdShowStep("7) readFile", "/hello.txt");
		if (!LittleFS.exists("/hello.txt")) {
			lcdShowStep("E: 7) readFile", "File does not exist!", 3000);
			return;
		}
		lcdShowPagedLine2("READ hello.txt", fsReadTextForLcd("/hello.txt"));

		lcdShowStep("8) renameFile", "hello->foo");
		if (!LittleFS.rename("/hello.txt", "/foo.txt")) {
			lcdShowStep("E: 8) renameFile", "Failed!", 3000);
			return;
		}

		lcdShowStep("9) readFile", "/foo.txt");
		lcdShowPagedLine2("READ foo.txt", fsReadTextForLcd("/foo.txt"));

		lcdShowStep("10) deleteFile", "/foo.txt");
		if (!LittleFS.remove("/foo.txt")) {
			lcdShowStep("E: 10) deleteFile", "Failed!", 3000);
			return;
		}

		lcdShowStep("11) testFileIO", "/test.txt");
		testFileIO("/test.txt");

		lcdShowStep("12) deleteFile", "/test.txt");
		if (!LittleFS.remove("/test.txt")) {
			lcdShowStep("E: 12) deleteFile", "Failed!", 3000);
			return;
		}

		lcdShowStep("FS Test", "Done", 1200);

		Garden::controlPanel->menu->show();
		Garden::controlPanel->menu->refresh();
	}),
	ITEM_COMMAND("Format FS", []() {
		if (LittleFS.format()) {
			Serial.println("Filesystem formatted successfully");
		} else {
			Serial.println("Failed to format filesystem");
		}
	}),
	ITEM_COMMAND("Reboot", []() {
		esp_restart();
	}),
	ITEM_COMMAND("< Back", []() {
		Garden::controlPanel->menu->process(BACK);
	})
);

static void lcdDraw2Lines(const String& line1, const String& line2) {
    auto* display = Garden::controlPanel->menu->getRenderer()->display;
    display->clear();
    display->setCursor(0, 0);
    display->draw(line1.substring(0, LCD_COLS).c_str());
    display->setCursor(0, 1);
    display->draw(line2.substring(0, LCD_COLS).c_str());
}

static void lcdShowStep(const String& step, const String& detail, uint16_t ms) {
    lcdDraw2Lines(step, detail);
    delay(ms);
}

static void lcdShowPagedLine2(const String& title, String text, uint16_t ms) {
    text.replace("\r", " ");
    text.replace("\n", " ");
    if (text.length() == 0) text = "<empty>";

    for (size_t i = 0; i < text.length(); i += LCD_COLS) {
        lcdDraw2Lines(title, text.substring(i, i + LCD_COLS));
        delay(ms);
    }
}

static String fsReadTextForLcd(const char* path) {
    File f = LittleFS.open(path, "r");
    if (!f) return "<open error>";
    String s = f.readString();
    f.close();
    return s;
}

static String fsListDirForLcd(const char* dirname) {
    File root = LittleFS.open(dirname);
    if (!root || !root.isDirectory()) return "<dir open error>";

    String out;
    File file = root.openNextFile();
    while (file) {
        if (out.length()) out += ", ";
        String name = file.name();
        if (name.startsWith("/")) name.remove(0, 1);
        out += name;
        file = root.openNextFile();
    }
    return out.length() ? out : "<empty>";
}

static void testFileIO(const char * path){
  Serial.printf("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = LittleFS.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }

  size_t i;
  Serial.print("- writing" );
  uint32_t start = millis();
  for(i=0; i<2048; i++){
    if ((i & 0x001F) == 0x001F){
      Serial.print(".");
     }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();

  file = LittleFS.open(path);
  start = millis();
  end = start;
  i = 0;
  if(file && !file.isDirectory()){
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- reading" );
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
       }
       file.read(buf, toRead);
       if ((i++ & 0x001F) == 0x001F){
         Serial.print(".");
       }
       len -= toRead;
      }
    Serial.println("");
    end = millis() - start;
    Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- failed to open file for reading");
  }
}

#endif // LCD_SCREENS_NO_SCREENS

#endif // LCD_SCREENS_H
