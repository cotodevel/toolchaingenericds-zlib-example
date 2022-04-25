// Includes
#include "WoopsiTemplate.h"
#include "woopsiheaders.h"
#include "bitmapwrapper.h"
#include "bitmap.h"
#include "graphics.h"
#include "rect.h"
#include "gadgetstyle.h"
#include "fonts/newtopaz.h"
#include "woopsistring.h"
#include "colourpicker.h"
#include "filerequester.h"
#include "soundTGDS.h"
#include "main.h"
#include "posixHandleTGDS.h"
#include "keypadTGDS.h"

#include "zlib.h"
#include "gui_console_connector.h"
#include "dswnifi_lib.h"
#include "TGDSLogoLZSSCompressed.h"
#include "biosTGDS.h"
#include "global_settings.h"
#include "TGDSMemoryAllocator.h"
#include "ipcfifoTGDSUser.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "xenofunzip.h"
#include "zipDecomp.h"

static const char* myVersion = ZLIB_VERSION;

__attribute__((section(".dtcm")))
WoopsiTemplate * WoopsiTemplateProc = NULL;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::startup(int argc, char **argv) {
	
	Rect rect;

	/** SuperBitmap preparation **/
	// Create bitmap for superbitmap
	Bitmap* superBitmapBitmap = new Bitmap(164, 191);

	// Get a graphics object from the bitmap so that we can modify it
	Graphics* gfx = superBitmapBitmap->newGraphics();

	// Clean up
	delete gfx;

	// Create screens
	AmigaScreen* newScreen = new AmigaScreen(TGDSPROJECTNAME, Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(newScreen);
	newScreen->setPermeable(true);

	// Add child windows
	AmigaWindow* controlWindow = new AmigaWindow(0, 13, 256, 33, "Controls", Gadget::GADGET_DRAGGABLE, AmigaWindow::AMIGA_WINDOW_SHOW_DEPTH);
	newScreen->addGadget(controlWindow);

	// Controls
	controlWindow->getClientRect(rect);

	_Index = new Button(rect.x, rect.y, 41, 16, "Index");	//_Index->disable();
	_Index->setRefcon(2);
	controlWindow->addGadget(_Index);
	_Index->addGadgetEventHandler(this);
	
	_lastFile = new Button(rect.x + 41, rect.y, 17, 16, "<");
	_lastFile->setRefcon(3);
	controlWindow->addGadget(_lastFile);
	_lastFile->addGadgetEventHandler(this);
	
	_nextFile = new Button(rect.x + 41 + 17, rect.y, 17, 16, ">");
	_nextFile->setRefcon(4);
	controlWindow->addGadget(_nextFile);
	_nextFile->addGadgetEventHandler(this);
	
	_play = new Button(rect.x + 41 + 17 + 17, rect.y, 40, 16, "Play");
	_play->setRefcon(5);
	controlWindow->addGadget(_play);
	_play->addGadgetEventHandler(this);
	
	_stop = new Button(rect.x + 41 + 17 + 17 + 40, rect.y, 40, 16, "Stop");
	_stop->setRefcon(6);
	controlWindow->addGadget(_stop);
	_stop->addGadgetEventHandler(this);
	
	
	// Add File listing screen
	_fileScreen = new AmigaScreen("File List", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(_fileScreen);
	_fileScreen->setPermeable(true);
	_fileScreen->flipToTopScreen();
	// Add screen background
	_fileScreen->insertGadget(new Gradient(0, SCREEN_TITLE_HEIGHT, 256, 192 - SCREEN_TITLE_HEIGHT, woopsiRGB(0, 31, 0), woopsiRGB(0, 0, 31)));
	
	// Create FileRequester
	_fileReq = new FileRequester(10, 10, 150, 150, "Files", "/", GADGET_DRAGGABLE | GADGET_DOUBLE_CLICKABLE);
	_fileReq->setRefcon(1);
	_fileScreen->addGadget(_fileReq);
	_fileReq->addGadgetEventHandler(this);
	currentFileRequesterIndex = 0;
	
	_MultiLineTextBoxLogger = NULL;	//destroyable TextBox
	
	char outputConsole[256+1];
	memset(outputConsole, 0, sizeof(outputConsole));
	

	//Check zlib version
	if (zlibVersion()[0] != myVersion[0]){
		sprintf(outputConsole, "%s \n", "incompatible zlib version. Turn off NDS. ");
		while(1==1){
			handleARM9SVC();	/* Do not remove, handles TGDS services */
			IRQVBlankWait();
		}
	} 
	else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0){
		sprintf(outputConsole, "%s \n", "warning: different zlib version. Turn off NDS. ");
		while(1==1){
			handleARM9SVC();	/* Do not remove, handles TGDS services */
			IRQVBlankWait();
		}
	}
	else{
		sprintf(outputConsole, "%s[%s] \n", "ZLIB Version:", ZLIB_VERSION);
	}
	
	// Add ZLIB Version
	_alert = new Alert(2, 2, 200, 80, "ZLIB Status", WoopsiString(outputConsole));
	newScreen->addGadget(_alert);
	
	enableDrawing();	// Ensure Woopsi can now draw itself
	redraw();			// Draw initial state
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::shutdown() {
	Woopsi::shutdown();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::waitForAOrTouchScreenButtonMessage(MultiLineTextBox* thisLineTextBox, const WoopsiString& thisText){
	thisLineTextBox->appendText(thisText);
	scanKeys();
	while((!(keysDown() & KEY_A)) && (!(keysDown() & KEY_TOUCH))){
		scanKeys();
	}
	scanKeys();
	while((keysDown() & KEY_A) && (keysDown() & KEY_TOUCH)){
		scanKeys();
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleValueChangeEvent(const GadgetEventArgs& e) {

	// Did a gadget fire this event?
	if (e.getSource() != NULL) {
	
		// Is the gadget the file requester?
		if ((e.getSource()->getRefcon() == 1) && (((FileRequester*)e.getSource())->getSelectedOption() != NULL)) {
			
			//Handle .zip decompression
			WoopsiString strObj = ((FileRequester*)e.getSource())->getSelectedOption()->getText();
			memset(currentFileChosen, 0, sizeof(currentFileChosen));
			strObj.copyToCharArray(currentFileChosen);
			
			//Destroyable Textbox implementation init
			Rect rect;
			_fileScreen->getClientRect(rect);
			_MultiLineTextBoxLogger = new MultiLineTextBox(rect.x, rect.y, 262, 170, "Loading\n...", Gadget::GADGET_DRAGGABLE, 5);
			_fileScreen->addGadget(_MultiLineTextBoxLogger);
			
			_MultiLineTextBoxLogger->removeText(0);
			_MultiLineTextBoxLogger->moveCursorToPosition(0);
			_MultiLineTextBoxLogger->appendText(WoopsiString("Unzipping -> dir: [/] ...\n"));
			
			if(handleDecompressor(currentFileChosen, (char*)&logBuf[0]) == 0){
				WoopsiTemplateProc->_MultiLineTextBoxLogger->appendText(WoopsiString("Unpack OK"));
				WoopsiTemplateProc->_MultiLineTextBoxLogger->appendText(WoopsiString((char*)&logBuf[0]));
			}
			else{
				WoopsiTemplateProc->_MultiLineTextBoxLogger->appendText(WoopsiString("Fail write \n Corrupted ZIP \n Not a .ZIP file"));
			}
			waitForAOrTouchScreenButtonMessage(_MultiLineTextBoxLogger, "Press (A) or tap touchscreen to continue. \n");
			
			_MultiLineTextBoxLogger->invalidateVisibleRectCache();
			_fileScreen->eraseGadget(_MultiLineTextBoxLogger);
			_MultiLineTextBoxLogger->destroy();	//same as delete _MultiLineTextBoxLogger;
			//Destroyable Textbox implementation end
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleLidClosed() {
	// Lid has just been closed
	_lidClosed = true;

	// Run lid closed on all gadgets
	s32 i = 0;
	while (i < _gadgets.size()) {
		_gadgets[i]->lidClose();
		i++;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleLidOpen() {
	// Lid has just been opened
	_lidClosed = false;

	// Run lid opened on all gadgets
	s32 i = 0;
	while (i < _gadgets.size()) {
		_gadgets[i]->lidOpen();
		i++;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleClickEvent(const GadgetEventArgs& e) {
	switch (e.getSource()->getRefcon()) {
		//_Index Event
		case 2:{
			//Get fileRequester size, if > 0, set the first element selected
			FileRequester * freqInst = _fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			if(freqListBox->getOptionCount() > 0){
				freqListBox->setSelectedIndex(0);
			}
			currentFileRequesterIndex = 0;
		}	
		break;
		
		//_lastFile Event
		case 3:{
			FileRequester * freqInst = _fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			if(currentFileRequesterIndex > 0){
				currentFileRequesterIndex--;
			}
			if(freqListBox->getOptionCount() > 0){
				freqListBox->setSelectedIndex(currentFileRequesterIndex);
			}
		}	
		break;
		
		//_nextFile Event
		case 4:{
			FileRequester * freqInst = _fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			if(currentFileRequesterIndex < (freqListBox->getOptionCount() - 1) ){
				currentFileRequesterIndex++;
				freqListBox->setSelectedIndex(currentFileRequesterIndex);
			}
		}	
		break;
		
		//_play Event
		case 5:{
			//Play WAV/ADPCM if selected from the FileRequester
			WoopsiString strObj = _fileReq->getSelectedOption()->getText();
			memset(currentFileChosen, 0, sizeof(currentFileChosen));
			strObj.copyToCharArray(currentFileChosen);
			pendPlay = 1;
		}	
		break;
		
		//_stop Event
		case 6:{
			pendPlay = 2;
		}	
		break;
	}
}

__attribute__((section(".dtcm")))
u32 pendPlay = 0;

char currentFileChosen[256+1];

//Called once Woopsi events are ended: TGDS Main Loop
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O2")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Woopsi::ApplicationMainLoop(){
	//Earlier.. main from Woopsi SDK.
	
	//Handle TGDS stuff...
	
	
	
	switch(pendPlay){
		case(1):{
			internalCodecType = playSoundStream(currentFileChosen, _FileHandleVideo, _FileHandleAudio);
			if(internalCodecType == SRC_NONE){
				//stop right now
				pendPlay = 2;
			}
			else{
				pendPlay = 0;
			}
		}
		break;
		case(2):{
			stopSoundStreamUser();
			pendPlay = 0;
		}
		break;
	}
}